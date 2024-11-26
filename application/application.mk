APP_C_SOURCES += \
	application/main.c \
	extensions/drivers/mx25/mx25.c \
	extensions/drivers/sht3x/sht3x.c 
	

COMMON_C_INCLUDES += \
	-Iapplication \
	-Iextensions/drivers/mx25 \
	-Iextensions/drivers/sht3x
