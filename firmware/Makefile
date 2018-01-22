# Esptool firmware download options
PORT			:= /dev/ttyUSB0
BAUD			:= 921600

# Directory structure
BUILDDIR		:= build
SRCDIR			:= src
OBJDIR			:= obj

SRCS			:= $(wildcard  $(SRCDIR)/*.c)
OBJS			:= $(addprefix $(OBJDIR)/,$(notdir $(SRCS:.c=.o)))
DEPS			:= $(addprefix $(OBJDIR)/,$(notdir $(SRCS:.c=.d)))

# SDK / Compiler settings
SDK_DIR			:= /opt/esp-open-sdk/sdk
CC			:= xtensa-lx106-elf-gcc
LD			:= xtensa-lx106-elf-gcc
SIZE			:= xtensa-lx106-elf-size
READELF			:= xtensa-lx106-elf-readelf
ESPTOOL			:= esptool.py
CFLAGS			:= -O2 -fdata-sections -ffunction-sections -Wpointer-arith -Wall -Wno-parentheses -Wno-implicit-function-declaration -fno-inline-functions -nostdlib -mlongcalls -mtext-section-literals -DICACHE_FLASH
LDLIBS			:= -nostdlib -Wl,--gc-sections -Wl,--no-check-sections -u call_user_start -Wl,-static -Wl,--start-group -lc -lgcc -lhal -lphy -lpp -lnet80211 -lwpa -lmain -llwip -lcrypto $(OBJS) -Wl,--end-group
LDFLAGS			:= -O2 -T$(SDK_DIR)/ld/eagle.app.v6.ld
INCLUDES		:= -I $(SDK_DIR)/include -I $(SDK_DIR)/include/json -I $(SRCDIR)/

# Project settings
TARGET			:= main
FLASH_FREQ		:= 80m
FLASH_MODE		:= dio
FLASH_SIZE		:= 32m
FLASH_OPT		:= --flash_freq $(FLASH_FREQ) --flash_mode $(FLASH_MODE) --flash_size $(FLASH_SIZE)

TARGET_ELF		:= $(BUILDDIR)/$(TARGET).elf
TARGET_FLASH		:= $(BUILDDIR)/$(TARGET)-0x00000.bin
TARGET_IROM		:= $(BUILDDIR)/$(TARGET)-0x10000.bin

all: $(TARGET_FLASH) $(TARGET_IROM)
	@echo Compilation successful
	@echo Use \'make flash\' to flash firmware

$(TARGET_FLASH) $(TARGET_IROM): $(TARGET_ELF)
	@mkdir -p $(BUILDDIR)
	$(ESPTOOL) elf2image $(FLASH_OPT) $^ --output $(BUILDDIR)/$(TARGET)-

$(TARGET_ELF): $(OBJS)
	@mkdir -p $(BUILDDIR)
	$(LD) -L$(SDK_DIR)/lib $(LDFLAGS) $(LDLIBS) -o $@
		
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

$(OBJDIR)/%.d: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -MM $< -MF $@

flash: all
	$(ESPTOOL) --port $(PORT) --baud $(BAUD) write_flash $(FLASH_OPT) 0x00000 $(TARGET_FLASH) 0x10000 $(TARGET_IROM)

flash_init_data:
	$(ESPTOOL) --port $(PORT) --baud $(BAUD) write_flash 0x3fc000 $(SDK_DIR)/bin/esp_init_data_default.bin

.PHONY: sizeinfo
sizeinfo: all
	$(SIZE) $(TARGET_ELF)

.PHONY: ram_usage
ram_usage: all
	$(READELF) -sW $(TARGET_ELF) | awk '$$4 == "OBJECT" { print }' | sort -k 3 -n

.PHONY: clean
clean:
	$(RM) -r $(OBJDIR)
	$(RM) -r $(BUILDDIR)

-include $(DEPS)
