-include extensions/common/makefiles/common_options.mk

#-----------------------------------------------------------------------------
# Global configuration options
#-----------------------------------------------------------------------------

# Prefix for all binaries names
APP_NAME = hello_world

# Target board (l0 or l4)
BSP ?= l0

# Target radio (sx128x, lr1110, lr1120, lr1121, sx1261, sx1262, sx1268, sx1272 or sx1276)
TARGET_RADIO ?= sx1261

# Regions to be included in lbm lib (AS_923, AU_915, CN_470, CN_470P_1_0, EU_868, IN_865, KR_920, RU_864, US_915, WW_2G4 or ALL)
REGION ?= EU_868

# Allow fuota (take more RAM, due to read_modify_write feature) and force lbm build with fuota
ALLOW_FUOTA ?= no
FUOTA_VERSION ?= 1

# USE LBM Store and forward (take more RAM on STML4, due to read_modify_write feature)
ALLOW_STORE_AND_FORWARD ?= no

#TRACE
LBM_TRACE ?= no
APP_TRACE ?= yes

# LR11xx option to use crc
USE_LR11XX_CRC_SPI ?= no

# Allow relay 
ALLOW_RELAY_RX ?= no
ALLOW_RELAY_TX ?= no

#-----------------------------------------------------------------------------
# LBM options management
#-----------------------------------------------------------------------------

# CRYPTO Management
CRYPTO ?= SOFT

# Multistack 
# Note: if more than one stack is used, more ram will be used for nvm context saving due to read_modify_write feature
LBM_NB_OF_STACK ?= 1

# Add any lbm build options (ex: LBM_BUILD_OPTIONS ?= LBM_CLASS_B=yes REGION=ALL)
LBM_BUILD_OPTIONS ?=

#-----------------------------------------------------------------------------
# Optimization
#-----------------------------------------------------------------------------

# Application build optimization
APP_OPT = -Os

# Lora Basics Modem build optimization
LBM_OPT = -Os

#-----------------------------------------------------------------------------
# Debug
#-----------------------------------------------------------------------------

# Compile library and application with debug symbols
APP_DEBUG ?= no

# Debug optimization (will overwrite APP_OPT and LBM_OPT values in case DEBUG is set)
DEBUG_APP_OPT ?= -O0
DEBUG_LBM_OPT ?= -O0

#-----------------------------------------------------------------------------
# Makefile Configuration options
#-----------------------------------------------------------------------------

# Use multithreaded build (make -j)
MULTITHREAD ?= yes

# Print each object file size
SIZE ?= no

# Save memory usage to log file
LOG_MEM ?= yes

# Verbosity
VERBOSE ?= no