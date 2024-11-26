##############################################################################
# Main makefile for basic_modem
##############################################################################
-include extensions/common/makefiles/app_printing.mk
-include application/app_options.mk


#-----------------------------------------------------------------------------
# default action: print help
#-----------------------------------------------------------------------------

ifeq ($(BSP),nc)
BSP = l0
endif

ifeq ($(REGION),nc)
REGION = EU_868
endif

ifeq ($(TARGET_RADIO),nc)
TARGET_RADIO = sx1261
endif

default:
	$(MAKE) app TARGET_RADIO=$(TARGET_RADIO) $(MTHREAD_FLAG)

help:
	$(call echo_help_b, "----------------------------- Compilation ----------------------------------")
	$(call echo_help, " * make                            : build application and lib")
	$(call echo_help, "")
	$(call echo_help_b, "-------------------------------- Clean -------------------------------------")
	$(call echo_help, " * make clean                      : clean application and lbm")
	$(call echo_help, " * make clean_app                  : clean application")
	$(call echo_help, " * make clean_lbm                  : clean lbm")
	$(call echo_help, "")
	$(call echo_help_b, "---------------------- Optional build parameters ---------------------------")
	$(call echo_help, " * MCU=xxx                         : choose which mcu will be used:(default is l0)")
	$(call echo_help, " *                                  - l0")
	$(call echo_help, " *                                  - l4")
	$(call echo_help, " * TARGET_RADIO=xxx                : choose which radio will be used:(default is sx1261)")
	$(call echo_help, " *                                  - sx128x")
	$(call echo_help, " *                                  - lr1110")
	$(call echo_help, " *                                  - lr1120")
	$(call echo_help, " *                                  - lr1121")
	$(call echo_help, " *                                  - sx1261")
	$(call echo_help, " *                                  - sx1262")
	$(call echo_help, " *                                  - sx1268")
	$(call echo_help, " *                                  - sx1272")
	$(call echo_help, " *                                  - sx1276")
	$(call echo_help, " * REGION=xxx                      : choose which region should be compiled:(default: EU_868)")
	$(call echo_help, " *                                  - AS_923")
	$(call echo_help, " *                                  - AU_915")
	$(call echo_help, " *                                  - CN_470")
	$(call echo_help, " *                                  - CN_470_RP_1_0")
	$(call echo_help, " *                                  - EU_868")
	$(call echo_help, " *                                  - IN_865")
	$(call echo_help, " *                                  - KR_920")
	$(call echo_help, " *                                  - RU_864")
	$(call echo_help, " *                                  - US_915")
	$(call echo_help, " *                                  - WW_2G4 (to be used only for lr1120 and sx128x targets)")
	$(call echo_help, " *                                  - ALL")
	$(call echo_help, " * CRYPTO=xxx                      : choose which crypto should be compiled (default: SOFT)")
	$(call echo_help, " *                                  - SOFT")
	$(call echo_help, " *                                  - LR11XX (only for lr1110 and lr1120 targets)")
	$(call echo_help, " *                                  - LR11XX_WITH_CREDENTIALS (only for lr1110 and lr1120 targets)")
	$(call echo_help, " * LBM_TRACE=yes/no                : choose to enable or disable modem trace print (default: trace is ON)")
	$(call echo_help, " * APP_TRACE=yes/no                : choose to enable or disable application trace print (default: trace is ON)")
	$(call echo_help_b, "-------------------- Optional makefile parameters --------------------------")
	$(call echo_help, " * MULTITHREAD=no                  : Disable multithreaded build")
	$(call echo_help, " * VERBOSE=yes                     : Increase build verbosity")
	$(call echo_help, " * SIZE=yes                        : Display size for all objects")
	$(call echo_help, " * DEBUG=yes                       : Compile library and application with debug symbols")

#-----------------------------------------------------------------------------
# Makefile include selection
#-----------------------------------------------------------------------------
ifeq ($(TARGET_RADIO),lr1110)
-include extensions/common/makefiles/app_lr11xx.mk
endif

ifeq ($(TARGET_RADIO),lr1120)
-include extensions/common/makefiles/app_lr11xx.mk
endif

ifeq ($(TARGET_RADIO),lr1121)
-include extensions/common/makefiles/app_lr11xx.mk
endif

ifeq ($(TARGET_RADIO),sx1261)
-include extensions/common/makefiles/app_sx126x.mk
endif

ifeq ($(TARGET_RADIO),sx1262)
-include extensions/common/makefiles/app_sx126x.mk
endif

ifeq ($(TARGET_RADIO),sx1268)
-include extensions/common/makefiles/app_sx126x.mk
endif

ifeq ($(TARGET_RADIO),sx128x)
-include extensions/common/makefiles/app_sx128x.mk
endif

ifeq ($(TARGET_RADIO),sx1272)
-include extensions/common/makefiles/app_sx127x.mk
endif

ifeq ($(TARGET_RADIO),sx1276)
-include extensions/common/makefiles/app_sx127x.mk
endif

#-----------------------------------------------------------------------------
-include application/application.mk
-include extensions/common/makefiles/app_common.mk

.PHONY: clean help

#-----------------------------------------------------------------------------
# Clean
#-----------------------------------------------------------------------------
clean:
	$(MAKE) -C $(LORA_BASICS_MODEM) clean_all $(MTHREAD_FLAG)
	-rm -rf $(APPBUILD_ROOT)*

clean_app:
	-rm -rf $(APPBUILD_ROOT)*

clean_lbm:
	$(MAKE) -C $(LORA_BASICS_MODEM) clean_all $(MTHREAD_FLAG)


#-----------------------------------------------------------------------------
# Application Compilation
#-----------------------------------------------------------------------------

#-- Generic -------------------------------------------------------------------
app:
ifneq ($(CRYPTO),SOFT)
ifneq ($(LBM_NB_OF_STACK),1)
	$(call echo_error, "----------------------------------------------------------")
	$(call echo_error, "More than one stack compiled: only soft crypto can be used")
	$(call echo_error, "Please use CRYPTO=SOFT option")
	$(call echo_error, "----------------------------------------------------------")
	exit 1
endif
ifneq ($(TARGET_RADIO),lr1110)
ifneq ($(TARGET_RADIO),lr1120)
ifneq ($(TARGET_RADIO),lr1121)
	$(call echo_error, "------------------------------------------------------------")
	$(call echo_error, "sx126x, sx127x and sx128x radio tagets: only soft crypto can be used")
	$(call echo_error, "Please use CRYPTO=SOFT option")
	$(call echo_error, "------------------------------------------------------------")
	exit 1
endif
endif
endif
endif
	$(MAKE) app_build

-include extensions/utils/tools.mk
