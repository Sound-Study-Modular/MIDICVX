#ifndef PLAYBACK_H_
#define PLAYBACK_H_

#include <stdint.h>
#include "midi.h"

typedef enum
{
    PLAYBACK_MODE_LIVE = 0,
    PLAYBACK_MODE_ARP_UP = 1
} playback_mode_t;

void Playback_Init(void);
void Playback_SetMode(playback_mode_t mode);
playback_mode_t Playback_GetMode(void);
void Playback_ToggleMode(void);

void Playback_NoteOn(const midi_msg_t *msg);
void Playback_NoteOff(const midi_msg_t *msg);
void Playback_TransportTick(void);

uint8_t Playback_GetHeldCount(void);
uint8_t Playback_IsNoteHeld(uint8_t note);
uint8_t Playback_GetVelocity(uint8_t note);
int8_t Playback_GetLowestHeld(void);
int8_t Playback_GetHighestHeld(void);

#endif /* PLAYBACK_H_ */
