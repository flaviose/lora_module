##############################################################################
# Definitions for the STM32 L073 board
##############################################################################


#-----------------------------------------------------------------------------
# Compilation flags
#-----------------------------------------------------------------------------
#MCU compilation flags
MCU_FLAGS ?= -mcpu=cortex-m0plus -mabi=aapcs -mthumb

BOARD_C_DEFS =  \
	-DSTM32L071xx

BOARD_C_DEFS +=  \
	-DUSE_FULL_LL_DRIVER

BOARD_LDSCRIPT = bsp_l0/mcu_drivers/core/STM32L0xx/stm32l071rz_flash.ld

JLINKDEV = STM32L071RZ

#-----------------------------------------------------------------------------
# Hardware-specific sources
#-----------------------------------------------------------------------------
BOARD_C_SOURCES = \
	bsp_l0/mcu_drivers/core/STM32L0xx/system_stm32l0xx.c

BOARD_C_SOURCES += \
	bsp_l0/smtc_hal/smtc_hal_watchdog.c\
	bsp_l0/smtc_hal/smtc_hal_spi.c\
	bsp_l0/smtc_hal/smtc_hal_rng.c\
	bsp_l0/smtc_hal/smtc_hal_eeprom.c\
	bsp_l0/smtc_hal/smtc_hal_gpio.c\
	bsp_l0/smtc_hal/smtc_hal_uart.c\
	bsp_l0/smtc_hal/smtc_hal_lp_timer.c\
	bsp_l0/smtc_hal/smtc_hal_rtc.c\
	bsp_l0/smtc_hal/smtc_hal_trace.c\
	bsp_l0/smtc_hal/smtc_hal_mcu.c\
	bsp_l0/smtc_hal/smtc_hal_i2c.c\
	bsp_l0/smtc_hal/smtc_hal_adc.c\
	bsp_l0/mcu_drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_adc.c \
	bsp_l0/mcu_drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_i2c.c \
	bsp_l0/mcu_drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_usart.c \
	bsp_l0/mcu_drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_rcc.c \
	bsp_l0/mcu_drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_lptim.c \
	bsp_l0/mcu_drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_rtc.c \
	bsp_l0/mcu_drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_dma.c \
	bsp_l0/mcu_drivers/STM32L0xx_HAL_Driver/Src/stm32l0xx_ll_utils.c \
	extensions/common/smtc_modem_hal/smtc_modem_hal.c

BOARD_ASM_SOURCES =  \
	bsp_l0/mcu_drivers/core/STM32L0xx/startup_stm32l071xx.s

BOARD_C_INCLUDES =  \
	-Ibsp_l0/mcu_drivers/core/STM32L0xx\
	-Ibsp_l0/mcu_drivers/cmsis/Core/Include\
	-Ibsp_l0/mcu_drivers/cmsis/Device/ST/STM32L0xx/Include\
	-Ibsp_l0/mcu_drivers/STM32L0xx_HAL_Driver/Inc\
	-Ibsp_l0/mcu_drivers/STM32L0xx_HAL_Driver/Inc/Legacy

BOARD_C_INCLUDES +=  \
	-Ibsp_l0/smtc_hal

#STM32l0 is slower than STM32L4 and wake up delay is longer so radio planner shall have a bigger delay (wake up time approc 4ms, stm lp timer delay = 4ms, and a security margin of 2 ms)
LBM_FLAGS="-DRP_MARGIN_DELAY=10"
