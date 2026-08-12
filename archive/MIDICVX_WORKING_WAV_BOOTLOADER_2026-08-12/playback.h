#ifndef PLAYBACK_H_
#define PLAYBACK_H_

#include <stdint.h>
#include "midi.h"

typedef enum
{
    PERFORMANCE_MODE_LIVE = 0,
    PERFORMANCE_MODE_MONO_ARP,
    PERFORMANCE_MODE_DUAL_ARP,
    PERFORMANCE_MODE_COUNT
} performance_mode_t;

typedef enum
{
    ARP_MODE_UP = 0,
    ARP_MODE_DOWN,
    ARP_MODE_PINGPONG,
    ARP_MODE_RANDOM,
    ARP_MODE_ORDER_PLAYED,
    ARP_MODE_COUNT
} arp_mode_t;

typedef enum
{
    PLAYBACK_OUTPUT_CV1_GATE1 = 0,
    PLAYBACK_OUTPUT_CV2_GATE2,
    PLAYBACK_OUTPUT_COUNT
} playback_output_t;

void Playback_Init(void);
void Playback_LoadSplitSetting(void);
void Playback_SetSplitNote(uint8_t note, uint8_t save_to_eeprom);
uint8_t Playback_GetSplitNote(void);
void Playback_Process(void);
void Playback_SettingsProcess(void);
void Playback_SaveSettingsNow(void);
void Playback_PanicReset(void);
void Playback_SetPerformanceMode(performance_mode_t mode, uint8_t save_to_eeprom);
performance_mode_t Playback_GetPerformanceMode(void);
void Playback_NextPerformanceMode(void);
void Playback_SetArpEnabled(uint8_t enabled);
uint8_t Playback_IsArpEnabled(void);
void Playback_ToggleArp(void);
void Playback_SetArpMode(arp_mode_t mode);
arp_mode_t Playback_GetArpMode(void);
void Playback_NextArpMode(void);
void Playback_SetArpOutput(playback_output_t output);
playback_output_t Playback_GetArpOutput(void);
void Playback_NoteOn(const midi_msg_t *msg);
void Playback_NoteOff(const midi_msg_t *msg);
void Playback_TransportTick(void);
uint8_t Playback_GetHeldCount(void);
uint8_t Playback_IsNoteHeld(uint8_t note);
uint8_t Playback_GetVelocity(uint8_t note);
int8_t Playback_GetLowestHeld(void);
int8_t Playback_GetHighestHeld(void);

#endif
