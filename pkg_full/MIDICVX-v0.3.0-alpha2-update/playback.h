#ifndef PLAYBACK_H_
#define PLAYBACK_H_

#include "midi.h"

typedef enum
{
    PLAYBACK_MODE_LIVE = 0,
    PLAYBACK_MODE_ARP_UP
} playback_mode_t;

void Playback_Init(void);
void Playback_SetMode(playback_mode_t mode);
void Playback_ToggleMode(void);
playback_mode_t Playback_GetMode(void);
void Playback_ProcessMidiMessage(midi_msg_t *msg);
void Playback_ClockStart(void);
void Playback_ClockStop(void);
void Playback_ClockTick(void);

#endif /* PLAYBACK_H_ */
