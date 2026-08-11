TARGET := midicvx_bootloader
MCU ?= atmega328p

ifeq ($(MCU),atmega328p)
  AVR_PART := m328p
else ifeq ($(MCU),atmega328)
  AVR_PART := m328
else
  $(error MCU must be atmega328p or atmega328)
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

# 4 KiB boot section at byte 0x7000. BOOTSZ1:0=00, BOOTRST=0 (programmed).
# Preserve the project's 16 MHz clock and BOD settings.
LFUSE ?= 0xFF
HFUSE ?= 0xD8
EFUSE ?= 0xFD

APP_HEX ?= ../build/midi_cv_x.hex

.PHONY: all clean size verify-hex install verify-install fuses read-fuses wav edge-test

all: $(TARGET).hex size

$(TARGET).elf: bootloader.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

size: $(TARGET).elf
	@used=`$(SIZE) -A $< | awk '/\.text|\.data/ {sum += $$2} END {print sum+0}'`; \
	 echo "Bootloader flash used: $$used / 4096 bytes"; \
	 if [ $$used -gt 4096 ]; then echo "ERROR: bootloader exceeds 4 KiB"; exit 1; fi

verify-hex:
	@test -f "$(APP_HEX)" || (echo "ERROR: application HEX not found: $(APP_HEX)"; exit 1)
	@test -f "$(TARGET).hex" || (echo "ERROR: bootloader HEX not found; run make first"; exit 1)

# IMPORTANT: install both images in ONE flash session. A normal ISP chip erase
# would otherwise erase whichever image was already present.
install: all verify-hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
	  -U flash:w:$(APP_HEX):i \
	  -U flash:w:$(TARGET).hex:i
	@echo "Flash images installed. Now run: make fuses MCU=$(MCU)"

verify-install: verify-hex
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
	  -U flash:v:$(APP_HEX):i \
	  -U flash:v:$(TARGET).hex:i

fuses:
	@echo "Programming 4 KiB boot section + BOOTRST for $(MCU)"
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
	  -U lfuse:w:$(LFUSE):m \
	  -U hfuse:w:$(HFUSE):m \
	  -U efuse:w:$(EFUSE):m

read-fuses:
	$(AVRDUDE) $(AVRDUDE_FLAGS) \
	  -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

wav:
	@test -f "$(APP_HEX)" || (echo "ERROR: application HEX not found: $(APP_HEX)"; exit 1)
	python3 make_update_wav.py "$(APP_HEX)" --version 1.0.0 -o MIDICVX_v1.0.0.wav

edge-test:
	python3 make_edge_test_wav.py -o MIDICVX_audio_input_test.wav

clean:
	rm -f $(TARGET).elf $(TARGET).hex $(TARGET).map MIDICVX_v1.0.0.wav MIDICVX_audio_input_test.wav
