## MIDI-CV X firmware build
## Milestone v0.1: stock runtime behavior + future-safe project foundation.

TARGET   := midi_cv_x
MCU      ?= atmega328
ifeq ($(MCU),atmega328p)
  AVR_PART := m328p
else
  AVR_PART := m328
endif

SRCS     := $(wildcard *.c)
BUILD    ?= release
BUILDDIR := build
OBJS     := $(SRCS:%.c=$(BUILDDIR)/%.o)
DEPS     := $(OBJS:.o=.d)

CC       := avr-gcc
OBJCOPY  := avr-objcopy
OBJDUMP  := avr-objdump
SIZE     := avr-size

ifeq ($(BUILD),debug)
  OPT    := -O1 -g2
  DEFS   := -DDEBUG
else
  OPT    := -Os
  DEFS   := -DNDEBUG
endif

CFLAGS := -mmcu=$(MCU) -std=gnu99 $(OPT) $(DEFS) \
          -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums \
          -ffunction-sections -fdata-sections -Wall -Wextra -MMD -MP

# Keep the firmware metadata block even with --gc-sections.
LDFLAGS := -mmcu=$(MCU) -Wl,--gc-sections \
           -Wl,--undefined=g_midicvx_firmware_info \
           -Wl,-Map=$(BUILDDIR)/$(TARGET).map
LDLIBS  := -lm

ELF := $(BUILDDIR)/$(TARGET).elf
HEX := $(BUILDDIR)/$(TARGET).hex
EEP := $(BUILDDIR)/$(TARGET).eep
LSS := $(BUILDDIR)/$(TARGET).lss

# Top 4 KiB is intentionally reserved for a future MIDI/audio bootloader.
FLASH_TOTAL       := 32768
BOOTLOADER_RESERVE:= 4096
APP_MAX           := 28672

AVRDUDE    := avrdude
PROGRAMMER ?= usbasp
PORT       ?= usb
BITCLOCK   ?= 100
AVRDUDE_FLAGS := -p $(AVR_PART) -c $(PROGRAMMER) -P $(PORT) -B $(BITCLOCK)

# Known-good values established on the actual ATmega328 modules.
LFUSE ?= 0xFF
HFUSE ?= 0xDE
EFUSE ?= 0x05

.PHONY: all hex eep lss size check-size flash verify fuses read-fuses clean
.SECONDARY:

all: $(HEX) $(EEP) check-size size
hex: $(HEX)
eep: $(EEP)
lss: $(LSS)

$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: %.c | $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(ELF): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

$(EEP): $(ELF)
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom=alloc,load \
		--change-section-lma .eeprom=0 --no-change-warnings -O ihex $< $@

$(LSS): $(ELF)
	$(OBJDUMP) -h -S $< > $@

check-size: $(ELF)
	@used=`$(SIZE) -A $(ELF) | awk '/\.text|\.data|\.midicvx_info/ {sum += $$2} END {print sum+0}'`; \
	 echo "Application flash used: $$used / $(APP_MAX) bytes (4 KiB reserved)"; \
	 if [ $$used -gt $(APP_MAX) ]; then \
	   echo "ERROR: application overlaps the reserved bootloader area"; exit 1; \
	 fi

size: $(ELF)
	@$(SIZE) --mcu=$(MCU) --format=avr $< 2>/dev/null || $(SIZE) $<

flash: $(HEX)
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:$(HEX):i

verify: $(HEX)
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:v:$(HEX):i

fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
		-U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m

read-fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
		-U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h -U lock:r:-:h

clean:
	rm -rf $(BUILDDIR)

-include $(DEPS)
