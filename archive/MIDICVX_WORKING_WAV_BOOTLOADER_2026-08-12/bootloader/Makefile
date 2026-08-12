TARGET := midicvx_bootloader
MCU ?= atmega328p

ifeq ($(MCU),atmega328p)
  AVR_PART := m328p
else
  AVR_PART := m328
endif

CC := avr-gcc
OBJCOPY := avr-objcopy
SIZE := avr-size
AVRDUDE := avrdude

CFLAGS := -mmcu=$(MCU) -DF_CPU=16000000UL -std=gnu99 -Os -Wall -Wextra \
          -ffunction-sections -fdata-sections -fno-split-wide-types
LDFLAGS := -mmcu=$(MCU) -Wl,--gc-sections -Wl,--section-start=.text=0x7000 \
           -Wl,-Map=$(TARGET).map

PROGRAMMER ?= usbasp
BITCLOCK ?= 100
AVRDUDE_FLAGS := -p $(AVR_PART) -c $(PROGRAMMER) -B $(BITCLOCK)

# 4 KiB boot section at 0x7000 + BOOTRST enabled.
LFUSE ?= 0xFF
HFUSE ?= 0xD8
EFUSE ?= 0x05

.PHONY: all clean size flash verify fuses read-fuses

all: $(TARGET).hex size

$(TARGET).elf: bootloader.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

size: $(TARGET).elf
	@used=`$(SIZE) -A $< | awk '/\.text|\.data/ {sum += $$2} END {print sum+0}'`; \
	 echo "Bootloader flash used: $$used / 4096 bytes"; \
	 if [ $$used -gt 4096 ]; then echo "ERROR: bootloader exceeds 4 KiB"; exit 1; fi

flash: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:$(TARGET).hex:i

verify: $(TARGET).hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:v:$(TARGET).hex:i

fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
	  -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m

read-fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
	  -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).map
