#ifndef PLAYBACK_H_
#define PLAYBACK_H_

#include <stdint.h>
#include "midi.h"

/*
 * MIDICVX held-note/playback layer.
 *
 * Velocity 0 means the MIDI note is not currently held. The table is indexed
 * directly by MIDI note number (0..127), so duplicate Note On messages cannot
 * create duplicate entries and Note Off removal is constant-time.
 */
void Playback_Init(void);
void Playback_NoteOn(const midi_msg_t *msg);
void Playback_NoteOff(const midi_msg_t *msg);

uint8_t Playback_GetHeldCount(void);
uint8_t Playback_IsNoteHeld(uint8_t note);
uint8_t Playback_GetVelocity(uint8_t note);
int8_t Playback_GetLowestHeld(void);
int8_t Playback_GetHighestHeld(void);

#endif /* PLAYBACK_H_ */
