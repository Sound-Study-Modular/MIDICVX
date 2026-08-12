/*
 * midi_cv.h
 *
 * Created: 1/17/2017 10:25:43 AM
 *  Author: Janis Zaharans
 */ 


#ifndef MIDI_CV_H_
#define MIDI_CV_H_


#define MIDI_CV_VELO_CLEAR_ON_RELEASE	1


enum midi_cv_voice_enum{
	MIDI_CV_VOICE__1	= 0,
	MIDI_CV_VOICE__2	= 1
};

enum midi_cv_out_mode_enum {
	MIDI_CV_OUT_MODE__MOD		= 0,
	MIDI_CV_OUT_MODE__VELOCITY	= 1,	
};


void MidiCv_Init(void);
void MidiCv_Reset(void);
void MidiCv_SetVoices(uint8_t voices);
void MidiCv_NoteOn(uint8_t note, uint8_t velocity);
void MidiCv_NoteOnSingle(uint8_t channel, uint8_t note, uint8_t velocity);
void MidiCv_NoteOff(uint8_t note);
void MidiCv_NoteOffSingle(uint8_t channel, uint8_t note);
void MidiCv_PitchBend(uint8_t channel, uint16_t pitchbend);
void MidiCv_ControlChange(uint8_t control, uint8_t value);
void MidiCv_Config(midi_msg_t *msg, uint8_t mode);
void MidiCv_ConfigSave(void);
void MidiCv_ConfigLoad(void);

#endif /* MIDI_CV_H_ */