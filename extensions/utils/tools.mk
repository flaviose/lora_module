JFLASH ?= extensions/utils/jflash

.PHONY: jerase jload settings jloadee jloadeek jloadees format format_bsp format_extensions

jerase:
	$(JFLASH) -d $(JLINKDEV) -e

jload: $(BUILD_DIR)/$(BUILD_TARGET).hex
	$(JFLASH) -d $(JLINKDEV) -f $<

settings:
	python3 extensions/utils/settings.py prototype

jloadee:
	python3 extensions/utils/settings.py generate_hex keys --bsp $(BSP) --output extensions/utils/keys.hex
	$(JFLASH) -d $(JLINKDEV) -f extensions/utils/keys.hex
	@rm extensions/utils/keys.hex

	python3 extensions/utils/settings.py generate_hex settings --bsp $(BSP) --output extensions/utils/settings.hex
	$(JFLASH) -d $(JLINKDEV) -f extensions/utils/settings.hex
	@rm extensions/utils/settings.hex

jloadeek:
	python3 extensions/utils/settings.py generate_hex keys --bsp $(BSP) --output extensions/utils/keys.hex
	$(JFLASH) -d $(JLINKDEV) -f extensions/utils/keys.hex
	@rm extensions/utils/keys.hex

jloadees:
	python3 extensions/utils/settings.py generate_hex settings --bsp $(BSP) --output extensions/utils/settings.hex
	$(JFLASH) -d $(JLINKDEV) -f extensions/utils/settings.hex
	@rm extensions/utils/settings.hex

format:
	@-astyle --options=extensions/utils/.astyle application/*.h,*.c | grep -i "^Formatted" | cat

format_bsp:
	@-astyle --options=extensions/utils/.astyle --exclude=bsp_$(BSP)/mcu_drivers bsp_$(BSP)/*.h,*.c | grep -i "^Formatted" | cat

format_extensions:
	@-astyle --options=extensions/utils/.astyle extensions/*.h,*.c | grep -i "^Formatted" | cat
