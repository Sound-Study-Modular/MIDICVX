/*
 * midi_cv.c
 *
 * Created: 1/17/2017 10:25:23 AM
 *  Author: Janis Zaharans
 */ 

#include "main.h"

static volatile int8_t	midi_highest_note[2], midi_lowest_note[2];
static volatile int16_t midi_pitch_bend;
static volatile uint8_t	midi_mod_wheel;
static volatile uint8_t	midi_voices;
static volatile uint8_t	midi_out_mode = 0; 


volatile uint8_t keyboard[2][16];


void MidiCv_Init(void)
{
	Keyboard_Reset(keyboard[0]);
	Keyboard_Reset(keyboard[1]);
}

void MidiCv_Reset(void)
{
	Keyboard_Reset(keyboard[0]);
	Keyboard_Reset(keyboard[1]);
	
	midi_lowest_note[0] = -1;
	midi_lowest_note[1] = -1;
	midi_highest_note[0] = -1;
	midi_highest_note[1] = -1;
	
	DAC_WriteNote(0, 0);
	DAC_WriteNote(1, 0);
	DAC_ModWrite(0);
}

void MidiCv_SetVoices(uint8_t voices)
{
	midi_voices = voices & 1;
}


void MidiCv_NoteOn(uint8_t note, uint8_t velocity)
{
	int8_t highest_note, lowest_note;
	
	Keyboard_Press(keyboard[0], note);

	// Get highest and lowest pressed note
	highest_note = Keyboard_GetHighest(keyboard[0]);
	lowest_note = Keyboard_GetLowest(keyboard[0]);
	
	
	// New highest note
	if(highest_note != -1 && highest_note != midi_highest_note[0]) {
		midi_highest_note[0] = highest_note;
	
		DAC_WriteNote(1, highest_note);
		
		if(midi_voices == MIDI_CV_VOICE__1)
			DAC_WriteNote(0, highest_note);
	}
	
	if(note == highest_note) {
		
		if(midi_voices == MIDI_CV_VOICE__1) {
			
			if(midi_out_mode == MIDI_CV_OUT_MODE__VELOCITY) {
				DAC_ModWrite(velocity<<5);
				DAC_Update();
			}
			
			GATE1_HIGH();
			GATE2_HIGH();	
			
		} else {// if(highest_note != lowest_note){
			
			if(midi_out_mode == MIDI_CV_OUT_MODE__VELOCITY) {
				DAC_ModWrite(velocity<<5);
				DAC_Update();
			}
			
			GATE2_HIGH();
		}
	}
	
	
	if(midi_voices == MIDI_CV_VOICE__2) {
		// New lowest note
		if(lowest_note != -1 && lowest_note != midi_lowest_note[0]) {
			midi_lowest_note[0] = lowest_note;
		
			DAC_WriteNote(0, lowest_note);
		}
	
		if(note == lowest_note)
		{
			GATE1_HIGH();
		}
	}	
}


void MidiCv_NoteOnSingle(uint8_t channel, uint8_t note, uint8_t velocity)
{
	int8_t highest_note;
	
	Keyboard_Press(keyboard[channel], note);

	// Get highest and lowest pressed note
	highest_note = Keyboard_GetHighest(keyboard[channel]);
	
	// New highest note
	if(highest_note != -1 && highest_note != midi_highest_note[channel]) {
		midi_highest_note[channel] = highest_note;
		
		DAC_WriteNote(channel, highest_note);
	}
	
	if(note == highest_note) {
		if(channel == 0) {
			if(midi_out_mode == MIDI_CV_OUT_MODE__VELOCITY) {
				DAC_ModWrite(velocity<<5);
				DAC_Update();
			}
			GATE1_HIGH();
		} else {
			GATE2_HIGH();
		}
	}
}


void MidiCv_NoteOff(uint8_t note)
{
	int8_t highest_note, lowest_note;
	
	Keyboard_Release(keyboard[0], note);
	
	highest_note = Keyboard_GetHighest(keyboard[0]);
	lowest_note = Keyboard_GetLowest(keyboard[0]);
	
	
	if(midi_voices == MIDI_CV_VOICE__2) {	
		if(highest_note == lowest_note) {
			if(note == midi_lowest_note[0]) {
				GATE2_LOW();
			} else if(note == midi_highest_note[0]) {
				GATE2_LOW();
				
#ifdef MIDI_CV_VELO_CLEAR_ON_RELEASE
				if(midi_out_mode == MIDI_CV_OUT_MODE__VELOCITY) {
					DAC_ModWrite(0);
					DAC_Update();
				}
#endif
				
			}
		}
	}
	
	// New highest note
	if(highest_note != -1) {
		if (highest_note != midi_highest_note[0]) {
			midi_highest_note[0] = highest_note;
			
			DAC_WriteNote(1, highest_note);
			
			if(midi_voices == MIDI_CV_VOICE__1)
				DAC_WriteNote(0, highest_note);
		}
	} else { 
		GATE2_LOW();	
		
#ifdef MIDI_CV_VELO_CLEAR_ON_RELEASE
		if(midi_out_mode == MIDI_CV_OUT_MODE__VELOCITY) {
			DAC_ModWrite(0);
			DAC_Update();
		}
#endif
		
		if(midi_voices == MIDI_CV_VOICE__1)
			GATE1_LOW();
	}
		
		
	if(midi_voices == MIDI_CV_VOICE__2) {	
		// New lowest note
		if(lowest_note != -1) { 
			if(lowest_note != midi_lowest_note[0]) {
				midi_lowest_note[0] = lowest_note;
			
				DAC_WriteNote(0, lowest_note);
			}
		} else {
			GATE1_LOW();
		}
	}
}

void MidiCv_NoteOffSingle(uint8_t channel, uint8_t note)
{
	int8_t highest_note;
	
	Keyboard_Release(keyboard[channel], note);
	
	highest_note = Keyboard_GetHighest(keyboard[channel]);
	
	// New highest note
	if(highest_note != -1) {
		if (highest_note != midi_highest_note[channel]) {
			midi_highest_note[channel] = highest_note;
			
			DAC_WriteNote(channel, highest_note);
		}
	} else {
		if(channel == 0) {
			GATE1_LOW();
			
#ifdef MIDI_CV_VELO_CLEAR_ON_RELEASE
			if(midi_out_mode == MIDI_CV_OUT_MODE__VELOCITY) {
				DAC_ModWrite(0);
				DAC_Update();
			}
#endif
		} else
			GATE2_LOW();
	}
}


void MidiCv_PitchBend(uint8_t channel, uint16_t pitchbend)
{
	midi_pitch_bend = pitchbend - 8192;
	
	DAC_WritePitch(channel, midi_pitch_bend);
}

void MidiCv_ControlChange(uint8_t control, uint8_t value)
{
	if(control == 0x01) {
		midi_mod_wheel = value;
		
		if(midi_out_mode == MIDI_CV_OUT_MODE__MOD) {
			DAC_ModWrite((uint16_t)value<<5);
			DAC_Update();
		}
	}
}



void MidiCv_Config(midi_msg_t *msg, uint8_t mode)
{
	// If note on with zero velocity, change it to note off message
	if(msg->type == MIDI__NOTE_ON && msg->data2 == 0)
		msg->type = MIDI__NOTE_OFF;
	
	
	switch(msg->type) {
		case MIDI__CLOCK:
		case MIDI__START:
		case MIDI__CONTINUE:
		case MIDI__STOP:
		break;
		
		case MIDI__NOTE_OFF: {
			
			if(mode == 1) {
				int8_t low, high;
				Keyboard_Release(keyboard[0], msg->data1);
				
				low = Keyboard_GetLowest(keyboard[0]) % 12;
				high = Keyboard_GetHighest(keyboard[0]) % 12;
				
				if(low != -1) g_midi_ch[0] = low + 1;
				if(high != -1) g_midi_ch[1] = high + 1;
			}
			
		} break;
		
		case MIDI__NOTE_ON: {
			
			if(mode == 1) {
				int8_t low, high;
				
				Keyboard_Press(keyboard[0], msg->data1);
				

				low = Keyboard_GetLowest(keyboard[0]) % 12;
				high = Keyboard_GetHighest(keyboard[0]) % 12;
				
				if(low != -1) g_midi_ch[0] = low + 1;
				if(high != -1) g_midi_ch[1] = high + 1;
			} else if(mode == 2) {
				midi_out_mode = MIDI_CV_OUT_MODE__VELOCITY;
			}
			
		} break;
		
		case MIDI__PITCH_BEND:
		break;
		
		case MIDI__CONTROL_CHANGE: {
			if(msg->data1 == 0x01 && mode == 2) 
				midi_out_mode = MIDI_CV_OUT_MODE__MOD;
		}
		break;
		
		default:
		break;
		
	}
}


void MidiCv_ConfigSave(void)
{
	uint8_t midi_ch = (g_midi_ch[0] & 0x0F) | (g_midi_ch[1]<<4);
	
	eeprom_update_byte((void*)EEPROM_ADDR__MIDI_CH, midi_ch);
	eeprom_update_byte((void*)EEPROM_ADDR__MIDI_MOD, midi_out_mode);
}

void MidiCv_ConfigLoad(void)
{
	uint8_t midi_ch = eeprom_read_byte((void*)EEPROM_ADDR__MIDI_CH);
	g_midi_ch[0] = midi_ch & 0x0F;
	g_midi_ch[1] = midi_ch >> 4;
	
	midi_out_mode = eeprom_read_byte((void*)EEPROM_ADDR__MIDI_MOD);
}