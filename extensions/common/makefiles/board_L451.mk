##############################################################################
# Definitions for the STM32 L476 board
##############################################################################

#-----------------------------------------------------------------------------
# Compilation flags
#-----------------------------------------------------------------------------

#MCU compilation flags
MCU_FLAGS ?= -mcpu=cortex-m4 -mthumb -mabi=aapcs -mfpu=fpv4-sp-d16 -mfloat-abi=hard

BOARD_C_DEFS =  \
	-DUSE_HAL_DRIVER \
	-DSTM32L451xx

BOARD_LDSCRIPT = bsp_l4/mcu_drivers/core/STM32L4xx/stm32l451xx_flash.ld

JLINKDEV = STM32L451RE

#-----------------------------------------------------------------------------
# Hardware-specific sources
#-----------------------------------------------------------------------------
BOARD_C_SOURCES = \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rng.c\
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_lptim.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rtc_ex.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_iwdg.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c \
	bsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c \
	extensions/common/smtc_modem_hal/smtc_modem_hal.c\
	bsp_l4/mcu_drivers/core/STM32L4xx/system_stm32l4xx.c\
	bsp_l4/smtc_hal/smtc_hal_flash.c\
	bsp_l4/smtc_hal/smtc_hal_gpio.c\
	bsp_l4/smtc_hal/smtc_hal_mcu.c\
	bsp_l4/smtc_hal/smtc_hal_rtc.c\
	bsp_l4/smtc_hal/smtc_hal_rng.c\
	bsp_l4/smtc_hal/smtc_hal_spi.c\
	bsp_l4/smtc_hal/smtc_hal_lp_timer.c\
	bsp_l4/smtc_hal/smtc_hal_trace.c\
	bsp_l4/smtc_hal/smtc_hal_uart.c\
	bsp_l4/smtc_hal/smtc_hal_watchdog.c

BOARD_ASM_SOURCES =  \
	bsp_l4/mcu_drivers/core/STM32L4xx/startup_stm32l451xx.s

BOARD_C_INCLUDES =  \
	-Ibsp_l4/mcu_drivers/core/STM32L4xx\
	-Ibsp_l4/mcu_drivers/cmsis/Core/Include\
	-Ibsp_l4/mcu_drivers/cmsis/Device/ST/STM32L4xx/Include\
	-Ibsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Inc\
	-Ibsp_l4/mcu_drivers/STM32L4xx_HAL_Driver/Inc/Legacy\
	-Iextensions/common/smtc_modem_hal\
	-I.\
	-Ibsp_l4/mcu_drivers/core\
	-Iextensions/common/smtc_modem_hal\
	-Ibsp_l4/smtc_hal
