/*
 * midi.h
 *
 * Created: 1/16/2017 11:47:44 AM
 *  Author: Janis Zaharans
 */ 


#ifndef MIDI_H_
#define MIDI_H_



typedef enum
{
	MIDI__INVALID			= 0x00,
	
	MIDI__NOTE_OFF			= 0x80,
	MIDI__NOTE_ON			= 0x90,
	MIDI__AFTERTOUCH_POLY	= 0xA0,		// Not used
	MIDI__CONTROL_CHANGE	= 0xB0,
	MIDI__PROGRAM_CHANGE	= 0xC0,		// Not used
	MIDI__AFTERTOUCH_CHANNEL= 0xD0,		// Not used
	MIDI__PITCH_BEND		= 0xE0,
	MIDI__SYSTEM_EXCLUSIVE	= 0xF0,		// Not used
	
	// System Real Time
	MIDI__CLOCK				= 0xF8,
	MIDI__START				= 0xFA,
	MIDI__CONTINUE			= 0xFB,
	MIDI__STOP				= 0xFC,
	MIDI__ACTIVE_SENSING	= 0xFE,
	MIDI__SYETEM_RESET		= 0xFF,
	
} midi_type_t;

typedef struct
{
	uint8_t		channel;
	midi_type_t	type;
	uint8_t		data1;
	uint8_t		data2;
	uint8_t		valid;
} midi_msg_t;

typedef struct
{
	uint8_t		running_status;
	uint8_t		data[3];
	uint8_t		data_idx;
	uint8_t		expected_data_size;
	
	midi_msg_t	msg;
	
} midi_parser_t;




void	Midi_Init(midi_parser_t *midi_parser);
uint8_t Midi_Parse(midi_parser_t *midi_parser, uint8_t data);
void	Midi_ExecuteMessage(midi_msg_t *msg);



#endif /* MIDI_H_ */