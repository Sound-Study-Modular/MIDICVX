/*
 * midi.c
 *
 * Created: 1/16/2017 11:47:34 AM
 *  Author: Janis Zaharans
 */ 

#include "main.h"


// Extracts MIDI message type from status byte
midi_type_t Midi_GetType(uint8_t status)
{
	if( (status < 0x80) ||
		(status == 0xF4) ||
		(status == 0xF5) ||
		(status == 0xF9) ||
		(status == 0xFD)) 
	{
		// Data bytes and undefined type
		return MIDI__INVALID;
	} else if(status < 0xF0) {
		return ((midi_type_t)(status & 0xF0));		// Channel message. Remove channel nibble
	} 
	
	return (midi_type_t)status;
}

// Returns true if message contains channel data
uint8_t Midi_IsChannelMsg(midi_type_t type)
{
	return (type == MIDI__NOTE_OFF				||
			type == MIDI__NOTE_ON				||
			type == MIDI__CONTROL_CHANGE		||
			type == MIDI__AFTERTOUCH_POLY		||
			type == MIDI__AFTERTOUCH_CHANNEL	||	
			type == MIDI__PITCH_BEND			||
			type == MIDI__PROGRAM_CHANGE);
}	

// Extracts channel data from status byte
uint8_t Midi_GetChannel(uint8_t status)
{
	return (status & 0x0F) + 1;
}

void Midi_Init(midi_parser_t *midi_parser)
{
	midi_parser->data_idx = 0;
	midi_parser->expected_data_size = 0;
	midi_parser->running_status = 0;
}


uint8_t Midi_Parse(midi_parser_t *midi_parser, uint8_t data)
{
	
	if(midi_parser->data_idx == 0) {
		
		midi_parser->data[0] = data;
		
		// Check for running status (data sent with same status byte)
		if(Midi_IsChannelMsg(Midi_GetType(midi_parser->running_status))) {
			
			// If new data is not status byte, start new message with running status
			if(data < 0x80) {
				midi_parser->data[0] = midi_parser->running_status;
				midi_parser->data[1] = data;
				midi_parser->data_idx = 1;
			}
		}
		
		
		// Parse 1 byte messages or set expected data byte size
		switch(Midi_GetType(midi_parser->data[0]))
		{
			// 1 byte messages
			case MIDI__CLOCK:
			case MIDI__START:
			case MIDI__CONTINUE:
			case MIDI__STOP:
			case MIDI__ACTIVE_SENSING:
			case MIDI__SYETEM_RESET:
			midi_parser->msg.type = midi_parser->data[0];
			midi_parser->msg.channel = 0;
			midi_parser->msg.data1 = 0;
			midi_parser->msg.data2 = 0;
			midi_parser->msg.valid = 1;
			
			midi_parser->data_idx = 0;
			midi_parser->expected_data_size = 0;
			return 1;
			break;
			
			// 3 byte messages
			case MIDI__NOTE_ON:
			case MIDI__NOTE_OFF:
			case MIDI__CONTROL_CHANGE:
			case MIDI__PITCH_BEND:
			midi_parser->expected_data_size = 3;
			break;
			
			// Invalid messages
			case MIDI__INVALID:
			default:
			midi_parser->data_idx = 0;
			midi_parser->expected_data_size = 0;
			midi_parser->running_status = MIDI__INVALID;
			return 0;
			break;
		}
		
		
		// Check if message received
		if(midi_parser->data_idx >= (midi_parser->expected_data_size - 1)) {
			// Reception complete
			midi_parser->msg.type		= Midi_GetType(midi_parser->data[0]);
			midi_parser->msg.channel	= Midi_GetChannel(midi_parser->data[0]);
			midi_parser->msg.data1		= midi_parser->data[1];
			midi_parser->msg.data2		= 0;
			
			midi_parser->data_idx = 0;
			midi_parser->expected_data_size = 0;
			midi_parser->msg.valid = 1;
			
			return 1;
		} else {
			// Wait for more data
			midi_parser->data_idx++;
		}
	} else {
		
		
		// Test if this is status byte
		if(data >= 0x80) {
			
			// Reception of status bytes in the middle of uncompleted messages 
			// are allowed only for interleaved Real Time message or EOX
			switch(data) {
				case MIDI__CLOCK:
				case MIDI__START:
				case MIDI__CONTINUE:
				case MIDI__STOP:
				case MIDI__ACTIVE_SENSING:
				case MIDI__SYETEM_RESET:
				midi_parser->msg.type = (midi_type_t)data;
				midi_parser->msg.channel = 0;
				midi_parser->msg.data1 = 0;
				midi_parser->msg.data2 = 0;
				midi_parser->msg.valid = 1;
				
				return 1;
				break;
				
				case 0xF7:
					midi_parser->data_idx = 0;
					midi_parser->expected_data_size = 0;
					midi_parser->running_status = MIDI__INVALID;
					
					return 0;
					break;
				
				default:
					break;
			}
		}
			
			
		// Append data to pending message
		midi_parser->data[midi_parser->data_idx] = data;
			
		// Check if message has reached expected size
		if(midi_parser->data_idx >= (midi_parser->expected_data_size - 1)) {
				
			// Reception complete
			midi_parser->msg.type = Midi_GetType(midi_parser->data[0]);
				
			if(Midi_IsChannelMsg(midi_parser->msg.type))
				midi_parser->msg.channel = Midi_GetChannel(midi_parser->data[0]);
			else
				midi_parser->msg.channel = 0;
					
			midi_parser->msg.data1 = midi_parser->data[1];
			midi_parser->msg.data2 = midi_parser->expected_data_size == 3 ? midi_parser->data[2] : 0;
				
				
			// Reset parser
			midi_parser->data_idx = 0;
			midi_parser->expected_data_size = 0;
				
			midi_parser->msg.valid = 1;
				
			// Activate running status 
			switch(midi_parser->msg.type) {
				case MIDI__NOTE_ON:
				case MIDI__NOTE_OFF:
				case MIDI__CONTROL_CHANGE:
				case MIDI__PITCH_BEND:
					midi_parser->running_status = midi_parser->data[0];
					break;
					
				default:
					midi_parser->running_status = MIDI__INVALID;
					break;
			}
				
				
			return 1;
		} else {
			// Wait for more data
			midi_parser->data_idx++;
		}
			
	}
	
	return 0;
}


void Midi_ExecuteMessage(midi_msg_t *msg)
{
	static uint8_t clk_cnt = 0;
	static uint8_t clk_state = 0;
	
	// If note on with zero velocity, change it to note off message
	if(msg->type == MIDI__NOTE_ON && msg->data2 == 0)
		msg->type = MIDI__NOTE_OFF;
		
	
	switch(msg->type) {
			case MIDI__CLOCK:
			/* External transport input owns the former CLOCK OUT jack. */
			if(clk_state) {
				clk_cnt++;
				if(clk_cnt >= 6)
					clk_cnt = 0;
			}
			break;
			
		case MIDI__START:
			clk_state = 1;
			clk_cnt = 0;
			break;
			
		case MIDI__CONTINUE:
			clk_state = 1;
			break;
			
		case MIDI__STOP:
			clk_state = 0;
			break;
			
		case MIDI__NOTE_OFF:
			Playback_NoteOff(msg);
			break;

		case MIDI__NOTE_ON:
			Playback_NoteOn(msg);
			break;
			
		case MIDI__PITCH_BEND:
			if(g_midi_ch[0] == g_midi_ch[1]) {
				MidiCv_PitchBend(0, (msg->data2<<7) | msg->data1);
				MidiCv_PitchBend(1, (msg->data2<<7) | msg->data1);
			} else {
				MidiCv_PitchBend((msg->channel == g_midi_ch[0] ? 0 : 1), (msg->data2<<7) | msg->data1);
			}
			break;
			
		case MIDI__CONTROL_CHANGE:
			if(msg->channel == g_midi_ch[0]) {
				MidiCv_ControlChange(msg->data1, msg->data2);
			}
			break;
		
		default:
			break;
					
	} 
}
