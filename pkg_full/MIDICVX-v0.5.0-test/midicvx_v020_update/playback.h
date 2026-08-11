#ifndef PLAYBACK_H_
#define PLAYBACK_H_

#include <stdint.h>

/*
 * MIDICVX playback routing layer.
 *
 * v0.2.0 is deliberately pass-through only: it preserves the stock
 * MIDI-to-CV behavior while giving future arp/sequencer modes one clean
 * insertion point.
 */
void Playback_Init(void);
void Playback_NoteOn(uint8_t midi_channel, uint8_t note, uint8_t velocity);
void Playback_NoteOff(uint8_t midi_channel, uint8_t note);

#endif /* PLAYBACK_H_ */
