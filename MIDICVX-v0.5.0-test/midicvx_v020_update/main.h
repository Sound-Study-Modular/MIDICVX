/*
 * main.h
 *
 * Created: 1/16/2017 11:47:08 AM
 *  Author: Janis Zaharans
 */ 

#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/power.h>
#include "hardware.h"
#include <util/delay.h>
#include <util/setbaud.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

#include "button.h"

#include "midi.h"
#include "dac.h"

#include "ring_buffer.h"
#include "midi_cv.h"
#include "keyboard.h"
#include "blinker.h"
#include "firmware_info.h"
#include "settings_layout.h"
#include "playback.h"

#define DAC_I2C_ADDR	0xC0	//was 0xC0

#define EEPROM_ADDR__FIRST		0
#define EEPROM_ADDR__MIDI_CH	2
#define EEPROM_ADDR__MIDI_MOD	3
#define EEPROM_ADDR__DAC_CALIB	4



/* --- MACROS --- */
#define SWAP_UINT16(x) (((x) >> 8) | ((x) << 8))


extern volatile uint32_t	g_time;
extern volatile uint8_t		g_midi_ch[2];
extern volatile uint32_t	g_midi_clk_timeout;