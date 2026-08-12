/*
 * dac.c
 *
 * Created: 1/17/2017 10:59:38 AM
 *  Author: Janis Zaharans
 */ 

#include "main.h"


static volatile int16_t dac_calib[2][10] = { 
		{ 0, 512, 1024, 1536, 2048, 2560, 3072, 3584, 4095 }, 
		{ 0, 512, 1024, 1536, 2048, 2560, 3072, 3584, 4095 }};
			
static volatile uint16_t dac_last_value[2];

void DAC_WriteNote(uint8_t dac, uint8_t note)
 {
	int32_t output;
	int16_t a, b;
	uint8_t oct;
	uint8_t semi;
	
	//offset note by 24
	if(note < 24)
		note = 0;
	else
		note -= 24;
	
	if(note > 96)
		note = 96;
		
	oct = note / 12;
	semi = note % 12;
	
	a = dac_calib[dac][oct];
	b = dac_calib[dac][oct+1];
	
	// Do interpolation
	output = a + (semi * (b - a)) / 12;
	
	// Limit DAC output value
	if(output < 0) output = 0;
	if(output > 4095) output = 4095;
	
	dac_last_value[dac] = output;

	DAC_Write(dac, (uint16_t)output);
	DAC_Update();
}



void DAC_WritePitch(uint8_t dac, int16_t pitch_bend)
{
	int32_t output = dac_last_value[dac] + (pitch_bend/128);
	
	// Limit DAC output value
	if(output < 0) output = 0;
	if(output > 4095) output = 4095;
	
	DAC_Write(dac, (uint16_t)output);
	DAC_Update();
}


#ifdef DIY_HW

void DAC_Config(void)
{
	
}

void DAC_Write(uint8_t dac, uint16_t value)
{
	DAC_PORT &= ~DAC_CS1_PIN;
	
	SPDR = (dac ? 0x80 : 0x00) | 0x10 | ((value>>8) & 0x0F);
	while(!(SPSR & _BV(SPIF)));
	SPDR = value & 0xFF;
	while(!(SPSR & _BV(SPIF)));
	
	DAC_PORT |= DAC_CS1_PIN;
}

void DAC_ModWrite(uint16_t value)
{
	DAC_PORT &= ~DAC_CS2_PIN;

	SPDR = 0x10 | ((value>>8) & 0x0F);
	while(!(SPSR & _BV(SPIF)));
	SPDR = value & 0xFF;
	while(!(SPSR & _BV(SPIF)));

	DAC_PORT |= DAC_CS2_PIN;
}

#else


static volatile uint16_t dac_values[4];

void DAC_Config(void)
{
	// Configure External DAC
	uint8_t dac_config[4*3] = {
		0x40, 0x90, 0x00,		// Internal Ref (2.048V), Gain 1x, Normal mode
		0x42, 0x90, 0x00,
		0x44, 0x90, 0x00,
		0x46, 0x90, 0x00 };
		
	
	I2C_Transmit(DAC_I2C_ADDR, dac_config, 4*3);
}


void DAC_Write(uint8_t dac, uint16_t value)
{
	uint8_t i;
	uint16_t data[4];
	
	dac_values[dac] = value;

	// Prepare data
	for(i=0; i<4; i++) {
		data[i] = SWAP_UINT16(dac_values[i]);
	}

	I2C_Transmit(DAC_I2C_ADDR, (uint8_t*)data, 8);

}

void DAC_ModWrite(uint16_t value)
{
	uint8_t i;
	uint16_t data[4];
	
	dac_values[2] = value;

	// Prepare data
	for(i=0; i<4; i++) {
		data[i] = SWAP_UINT16(dac_values[i]);
	}

	I2C_Transmit(DAC_I2C_ADDR, (uint8_t*)data, 8);
}

#endif

void DAC_Update(void)
{
	DAC_PORT &= ~DAC_LOAD_PIN;
	DAC_PORT |= DAC_LOAD_PIN;
}





void Dac_MidiCalibration(midi_msg_t *msg, uint8_t channel)
{
	static uint8_t	active_oct = 0;
	static int16_t	active_key = 0;
	
	
	// If note on with zero velocity, change it to note off message
	if(msg->type == MIDI__NOTE_ON && msg->data2 == 0)
	msg->type = MIDI__NOTE_OFF;
	
	
	switch(msg->type) {
		case MIDI__CLOCK:
		case MIDI__START:
		case MIDI__CONTINUE:
		case MIDI__STOP:
		break;
		
		case MIDI__NOTE_OFF:
		
		break;
		
		case MIDI__NOTE_ON: {
			int16_t key = msg->data1;
			int16_t real_key = key - 24;
			
			//offset note by 24
			if(key < 24)
				key = 0;
			else
				key -= 24;
			
			uint8_t oct = key / 12;
			uint8_t note = key % 12;
			uint8_t ch = channel;
			
			// If channel greater than 1 (ch1 = cv1, ch2 = cv2)
			if(ch != 1 && ch != 2)
				return;
				
			// Make ch between 0 and 1
			ch -= 1;
			
			if(note == 0 && real_key >= 0) {
				active_oct = oct;
				active_key = key;
				
			} else if(real_key == (active_key + 1)) {
				dac_calib[ch][active_oct]++;
			} else if(real_key == (active_key + 2)) {
				dac_calib[ch][active_oct] += 10;
			} else if(real_key == (active_key - 1)) {
				dac_calib[ch][active_oct]--;
			} else if(real_key == (active_key - 2)) {
				dac_calib[ch][active_oct] -= 10;
			}
			
			// Limit value
			if(dac_calib[ch][active_oct] > 4095)
				dac_calib[ch][active_oct] = 4095;
			else if(dac_calib[ch][active_oct] < 0)
				dac_calib[ch][active_oct] = 0;
			
			DAC_Write(ch, dac_calib[ch][active_oct]);
			DAC_Update();
		} break;
		
		case MIDI__PITCH_BEND:
		break;
		
		case MIDI__CONTROL_CHANGE:
		break;
		
		default:
		break;
		
	}
}



void Dac_MidiCalibSave(void)
{
	eeprom_write_block((void*)dac_calib, (void*)EEPROM_ADDR__DAC_CALIB, sizeof(dac_calib));
}

void Dac_MidiCalibLoad(void)
{
	eeprom_read_block((void*)dac_calib, (void*)EEPROM_ADDR__DAC_CALIB, sizeof(dac_calib));
}