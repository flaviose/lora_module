import struct
from yaml import load
import intelhex as ih
import hashlib
import argparse
import os
import csv

try:
    from yaml import CLoader as Loader, CDumper as Dumper
except ImportError:
    from yaml import Loader, Dumper


def generateExtendedSettings(config, topic, values=None):
    f = open(config, "r")
    stream = f.read()

    data = load(stream, Loader=Loader)

    data = data[topic]

    data["items"].update({"hash": {'type': 'hash', 'size': 32, 'data': '', 'desc': f"{topic} hash"}})

    offset = 0

    if not values is None:
        for key in data["items"]:
            if key in values.keys():
                if data["items"][key]["type"] in ["uint", "int"]:
                    data["items"][key]["data"] = int(values[key])
                else:
                    data["items"][key]["data"] = values[key]

    for key in data["items"]:
        setting_item = data['items'][key]

        if not offset % 4 == 0:
            offset += 4-(offset % 4)

        if setting_item["type"] == "key":
            assert(isinstance(setting_item["data"], str))
            item_data = bytes.fromhex(str(setting_item["data"]))
            assert(len(item_data) == setting_item["size"])

        elif setting_item["type"] in ["uint", "int"]:
            assert(setting_item["size"] in [1, 2, 4])
            assert(isinstance(setting_item["data"], int))
            if setting_item["type"] == "uint":
                assert(setting_item["data"] >= 0)
                assert(setting_item["data"] < pow(2, 8*setting_item["size"]))
            else:
                assert (setting_item["data"] >= -pow(2, 8 * setting_item["size"]-1))
                assert (setting_item["data"] < pow(2, 8 * setting_item["size"]-1))

            format = "<" if setting_item["le"] else ">"
            if setting_item["size"] == 1:
                format += "B"
            elif setting_item["size"] == 2:
                format += "H"
            else:
                format += "I"
            if setting_item["type"] == "int":
                format = format.lower()
            item_data = struct.pack(format, setting_item["data"])

        elif setting_item["type"] == "string":
            assert(isinstance(setting_item["data"], str))
            assert(len(setting_item["data"]) <= setting_item["size"] - 1)
            item_data = bytearray(setting_item["data"], "utf-8")
            item_data.append(0)

        elif setting_item["type"] == "hash":
            item_data = bytearray()
            pass

        else:
            print("Unknown type: " + setting_item["type"])
            exit(1)

        setting_item["data_bytes"] = item_data
        setting_item["offset"] = offset
        offset += setting_item["size"]

    return data


def generateSettingsHex(config, topic, path, values=None, bsp="l0"):
    extendedSettings = generateExtendedSettings(config, topic, values)

    settings_hex = ih.IntelHex()

    offset = extendedSettings["config"]["offset_" + bsp]

    for key in extendedSettings['items']:
        setting_item = extendedSettings['items'][key]

        if key == "hash":
            hash_data = bytearray(settings_hex.tobinarray())
            m = hashlib.sha256()
            m.update(hash_data)
            setting_item["data_bytes"] = m.digest()

        settings_hex[offset + setting_item["offset"]:] = list(setting_item["data_bytes"])

        if not setting_item["size"] % 4 == 0:
            settings_hex[offset + setting_item["offset"]+setting_item["size"]:] = [0x00]*(4-(setting_item["size"] % 4))

    settings_hex.write_hex_file(path)


def generateSettingsPrototype(config, path):
    extendedSettings_keys = generateExtendedSettings(config, "keys")
    extendedSettings_settings = generateExtendedSettings(config, "settings")
    settings_complete = {"keys": extendedSettings_keys["items"], "settings": extendedSettings_settings["items"]}

    content = """#ifndef __APPLICATION_SETTINGS_APP_H__
#define __APPLICATION_SETTINGS_APP_H__

#include \"stdint.h\"
#include \"stdbool.h\"\n\n"""

    for section in settings_complete.keys():
        content += """typedef struct {\n"""

        offset = 0
        gap = 1

        for key in settings_complete[section]:
            setting_item = settings_complete[section][key]

            if not setting_item["offset"] == offset:
                content += f"    {'uint8_t':<12}{'gap' + str(gap) + '[' + str(setting_item['offset'] - offset) + '];':<25}// 0x{offset:02X} gap{gap}\n"
                offset += setting_item['offset'] - offset
                gap += 1

            if setting_item["type"] == "key":
                type = "uint8_t"
            elif setting_item["type"] == "string":
                type = "uint8_t"
            elif setting_item["type"] in ["uint", "int"]:
                type = setting_item["type"] + str(setting_item["size"]*8) + "_t"
            elif setting_item["type"] == "hash":
                type = "uint8_t"
            else:
                print("Unknown type: " + setting_item["type"])
                exit(1)

            array = f"[{setting_item['size']}]" if setting_item["type"] in ["key", "string", "hash"] else ""
            content += f"    {type:<12}{key+array+';':<25}// 0x{setting_item['offset']:02X} {setting_item['desc']}\n"

            offset += setting_item['size']

        content += f"}} {section}_t;\n\n"

    content += "#define SETTINGS_APP_PRINT() \\\n"
    content += "    SMTC_HAL_TRACE_INFO(\"Settings:\\n\"); \\\n"

    for section in settings_complete.keys():
      for key in settings_complete[section]:
          setting_item = settings_complete[section][key]

          if setting_item["type"] == "key":
              if key == "nwkey":
                  values = ", ".join([f"{section}.{key}[{i}]" for i in range(setting_item["size"]-2,setting_item["size"])])
                  content += f"    SMTC_HAL_TRACE_INFO(\" - {key+':':<25} {'??'*14}{'%02X'*2}\\n\", {values}); \\\n"
              else:
                  values = ", ".join([f"{section}.{key}[{i}]" for i in range(0,setting_item["size"])])
                  content += f"    SMTC_HAL_TRACE_INFO(\" - {key+':':<25} {'%02X'*setting_item['size']}\\n\", {values}); \\\n"
          elif setting_item["type"] == "string":
              content += f"    SMTC_HAL_TRACE_INFO(\" - {key+':':<25} \\\"%s\\\"\\n\", {section}.{key}); \\\n"
          elif setting_item["type"] == "uint":
              content += f"    SMTC_HAL_TRACE_INFO(\" - {key+':':<25} %u\\n\", {section}.{key}); \\\n"
          elif setting_item["type"] == "int":
              content += f"    SMTC_HAL_TRACE_INFO(\" - {key+':':<25} %d\\n\", {section}.{key}); \\\n"
          elif setting_item["type"] == "hash":
              #values = ", ".join([f"{section}.{key}[{i}]" for i in range(0,setting_item["size"])])
              #content += f"    SMTC_HAL_TRACE_INFO(\" - {key+':':<25} {'%X'*setting_item['size']}\\n\", {values}); \\\n"
              pass
          else:
              print("Unknown type: " + setting_item["type"])
              exit(1)

    content += """\n#endif // __APPLICATION_SETTINGS_APP_H__\n"""

    f = open(path, "w")
    f.write(content)
    f.close()


def readCsv(path, line):
    with open(path, newline='') as csvfile:
        spamreader = csv.reader(csvfile, delimiter=',', quotechar='"')
        header = None
        data = None
        for row in spamreader:
            if row[0] == "line":
                header = row
            elif int(row[0]) == int(line):
                data = row

        if not data is None:
            settings = {}
            for i, setting in enumerate(header[1:]):
                settings.update({setting: data[i+1]})
            return settings
        else:
            return {}

def commission(args):
    settings = readCsv(args.csv, args.line)
    generateSettingsHex(args.settings, args.type, path="extensions/utils/settings.hex", values=settings, bsp=args.bsp)
    os.system(f"extensions/utils/jflash -d {args.jlinkdev} -f extensions/utils/settings.hex")
    os.remove("extensions/utils/settings.hex")

def prototype(args):
    generateSettingsPrototype(args.settings, path=args.output)

def generate_hex(args):
    if not args.csv is None and not args.line is None:
        settings = readCsv(args.csv, args.line) 
        generateSettingsHex(args.settings, args.type, path=args.output, values=settings, bsp=args.bsp)
    else:
        generateSettingsHex(args.settings, args.type, path=args.output, bsp=args.bsp)

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Process some integers.')

    subparsers = parser.add_subparsers(help='command')
    parser.add_argument("--settings", default='application/settings.yml')

    commission_parser = subparsers.add_parser("commission")
    prototype_parser = subparsers.add_parser("prototype")
    generate_hex_parser = subparsers.add_parser("generate_hex")

    commission_parser.set_defaults(func=commission)
    commission_parser.add_argument("csv", type=str, help='path of the csv sheet with the settings')
    commission_parser.add_argument("line", type=int, help='line number of the settings in the csv sheet')
    commission_parser.add_argument("type", type=str, help='type of the settings (keys or settings)')
    commission_parser.add_argument("--bsp", type=str, help='specify the bsp (l0 or l4)', default="l0")
    commission_parser.add_argument("--jlinkdev", type=str, help='jlink device type', default="STM32L071RZ")

    prototype_parser.set_defaults(func=prototype)
    prototype_parser.add_argument("--output", help='path of the output .h file', default="application/settings_app.h")

    generate_hex_parser.set_defaults(func=generate_hex)
    generate_hex_parser.add_argument("type", type=str, help='type of the settings (keys or settings)')
    generate_hex_parser.add_argument("--bsp", type=str, help='specify the bsp (l0 or l4)', default="l0")
    generate_hex_parser.add_argument("--csv", type=str, help='path of the csv sheet with the settings')
    generate_hex_parser.add_argument("--line", type=int, help='line number of the settings in the csv sheet')
    generate_hex_parser.add_argument("--output", help='path of the output hex file', default="extensions/utils/settings.hex")

    args = parser.parse_args()
    args.func(args)
