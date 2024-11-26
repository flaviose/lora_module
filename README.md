# LBM4 Hello World

## Usage

1. Clone the hello_world repositiory including submodules
2. Move to the root directory of the project in WSL
3. Run make to build the project. Options are described in make help
4. Flash the binary with make jload
5. Flash the keys and settings with make jloadee. The default keys and settings can be found in application/settings.yml
6. Register the device with an LNS and monitor the uart output

## Hardware compatibility

| MCU         | Radio  | Runs | Current |   |
|-------------|--------|------|---------|---|
| STM30L071RZ | SX1261 | Yes  | 2.45uA  |   |
| STM32L451RE | SX1261 | Yes  | ?       |   |
| STM30L071RZ | LR1121 | Yes  | 3uA     |   |
