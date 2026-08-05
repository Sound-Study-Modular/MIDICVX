/*
 * dac.h
 *
 * Created: 1/17/2017 10:59:49 AM
 *  Author: Janis Zaharans
 */ 


#ifndef DAC_H_
#define DAC_H_


void DAC_Config(void);

void DAC_Write(uint8_t dac, uint16_t value);
void DAC_ModWrite(uint16_t value);
void DAC_WritePitch(uint8_t dac, int16_t pitch_bend);
void DAC_WriteNote(uint8_t dac, uint8_t note);
void DAC_Update(void);

void Dac_MidiCalibration(midi_msg_t *msg, uint8_t channel);

void Dac_MidiCalibSave(void);
void Dac_MidiCalibLoad(void);

#endif /* DAC_H_ */