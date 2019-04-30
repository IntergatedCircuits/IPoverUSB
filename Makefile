########+++++++++++++++++++++++++++++++++++########
##++----  Makefile for IPoverUSB project   ----++##
########+++++++++++++++++++++++++++++++++++########

# optimization
OPT = -O3
DEBUG = 1

TARGET = IPoverUSB

# Build path
#BUILD_DIR = build

# Submodule paths
XPD_DIR = STM32_XPD
USBD_DIR = USBDevice
LWIPDIR = lwIP/src
CONTRIBDIR = lwip-contrib
OS_DIR =

##++----  Included files  ----++##
include $(LWIPDIR)/Filelists.mk

##++----  Target configuration  ----++##

CORE = m4
SERIES = STM32F4
BSP = BSP_$(SERIES)xx

C_DEFS = 


##++----  Build tool binaries  ----++##
BINPATH = /usr/bin
PREFIX = arm-none-eabi-
CC = $(BINPATH)/$(PREFIX)gcc
AS = $(BINPATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(BINPATH)/$(PREFIX)objcopy
AR = $(BINPATH)/$(PREFIX)ar
SZ = $(BINPATH)/$(PREFIX)size
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S


##++----  MCU config  ----++##
CPU = -mcpu=cortex-$(CORE)
MCU = $(CPU) -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16


##++----  Assembler  ----++##
AS_DEFS = 
AS_INCLUDES = 
# assembly sources
AS_SOURCES = $(wildcard $(BSP)/*.s)

# assembler flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections


##++----  Compiler  ----++##
C_STANDARD = -std=gnu11

# All USBDevice classes are built, but only DFU is used
# C includes
C_INCLUDES =  \
-I$(BSP) \
-ICore \
-I$(LWIPDIR)/include \
-I$(USBD_DIR)/Include \
-I$(USBD_DIR)/PDs/STM32_XPD \
-I$(XPD_DIR)/CMSIS/Include \
-I$(XPD_DIR)/CMSIS/Device/ST/$(SERIES)xx/Include \
-I$(XPD_DIR)/$(SERIES)_XPD/inc

# C sources
C_SOURCES =  \
$(wildcard $(BSP)/*.c) \
$(wildcard Core/*.c) \
$(wildcard Core/arch/*.c) \
$(LWIPNOAPPSFILES) \
$(DHCPFILES) \
$(HTTPFILES) \
$(wildcard $(USBD_DIR)/Device/*.c) \
$(wildcard $(USBD_DIR)/Class/CDC/*.c) \
$(wildcard $(USBD_DIR)/Class/DFU/*.c) \
$(wildcard $(XPD_DIR)/$(SERIES)_XPD/src/*.c)

##++----  OS selection  ----++##

ifeq ($(strip $(OS_DIR)),)
# no OS

C_INCLUDES += -IConfig

BUILD_DIR = build_baremetal_$(SERIES)

else ifeq ($(findstring FreeRTOS,$(OS_DIR)),FreeRTOS)
# FreeRTOS

LWIP_OS_PORT = $(CONTRIBDIR)/ports/freertos

ifeq ($(CORE),m0)
PORT_CORE = GCC/ARM_CM0
else ifeq ($(CORE),m3)
PORT_CORE = GCC/ARM_CM3
else ifeq ($(CORE),m4)
PORT_CORE = GCC/ARM_CM4F
else ifeq ($(CORE),m7)
PORT_CORE = GCC/ARM_CM7
endif

C_INCLUDES +=  \
-IConfig_FreeRTOS \
-I$(LWIP_OS_PORT)/include \
-I$(OS_DIR)/include \
-I$(OS_DIR)/portable/$(PORT_CORE)

C_SOURCES +=  \
$(OS_DIR)/portable/MemMang/heap_4.c \
$(wildcard $(LWIP_OS_PORT)/*.c) \
$(wildcard $(OS_DIR)/*.c) \
$(wildcard $(OS_DIR)/portable/Common/*.c) \
$(wildcard $(OS_DIR)/portable/$(PORT_CORE)/*.c) \
$(wildcard Core/os/*.c)

BUILD_DIR = build_FreeRTOS_$(SERIES)

endif


# compiler flags
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections $(C_STANDARD)

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"


##++----  Linker  ----++##
# link script
LDSCRIPT = $(wildcard $(BSP)/*.ld)

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections


##++----  Build the application  ----++##
# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex

# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(AS_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(AS_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR):
	mkdir $@

##++----  Clean  ----++##
clean:
	-rm -fR .dep $(BUILD_DIR)


##++----  Dependencies  ----++##
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

# *** EOF ***
