#include "main.h"

#define MIDI_NOTE_COUNT 128U
#define NO_NOTE          0xFFU

static uint8_t held_velocity[MIDI_NOTE_COUNT];
static uint8_t held_channel[MIDI_NOTE_COUNT];
static uint8_t held_count;
static playback_mode_t playback_mode;
static uint8_t arp_current_note;
static uint8_t arp_output_note;
static uint8_t arp_output_channel;

static void Playback_ClearHeld(void)
{
    uint8_t note;

    for(note = 0U; note < MIDI_NOTE_COUNT; note++) {
        held_velocity[note] = 0U;
        held_channel[note] = 0U;
    }

    held_count = 0U;
    arp_current_note = NO_NOTE;
}

static void Playback_LiveNoteOn(const midi_msg_t *msg)
{
    if(g_midi_ch[0] == g_midi_ch[1]) {
        MidiCv_NoteOn(msg->data1, msg->data2);
    } else {
        MidiCv_NoteOnSingle((msg->channel == g_midi_ch[0]) ? 0U : 1U,
                            msg->data1,
                            msg->data2);
    }
}

static void Playback_LiveNoteOff(const midi_msg_t *msg)
{
    if(g_midi_ch[0] == g_midi_ch[1]) {
        MidiCv_NoteOff(msg->data1);
    } else {
        MidiCv_NoteOffSingle((msg->channel == g_midi_ch[0]) ? 0U : 1U,
                             msg->data1);
    }
}

static void Playback_OutputNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    if(g_midi_ch[0] == g_midi_ch[1]) {
        MidiCv_NoteOn(note, velocity);
    } else {
        MidiCv_NoteOnSingle((channel == g_midi_ch[0]) ? 0U : 1U,
                            note,
                            velocity);
    }
}

static void Playback_OutputNoteOff(uint8_t channel, uint8_t note)
{
    if(g_midi_ch[0] == g_midi_ch[1]) {
        MidiCv_NoteOff(note);
    } else {
        MidiCv_NoteOffSingle((channel == g_midi_ch[0]) ? 0U : 1U,
                             note);
    }
}

static uint8_t Playback_FindNextUp(uint8_t after_note)
{
    uint16_t note;
    uint16_t start;

    if(held_count == 0U) {
        return NO_NOTE;
    }

    start = (after_note == NO_NOTE) ? 0U : ((uint16_t)after_note + 1U);

    for(note = start; note < MIDI_NOTE_COUNT; note++) {
        if(held_velocity[note] != 0U) {
            return (uint8_t)note;
        }
    }

    for(note = 0U; note < start && note < MIDI_NOTE_COUNT; note++) {
        if(held_velocity[note] != 0U) {
            return (uint8_t)note;
        }
    }

    return NO_NOTE;
}

void Playback_Init(void)
{
    Playback_ClearHeld();
    playback_mode = PLAYBACK_MODE_LIVE;
    arp_output_note = NO_NOTE;
    arp_output_channel = 0U;
}

void Playback_SetMode(playback_mode_t mode)
{
    if(mode != PLAYBACK_MODE_LIVE && mode != PLAYBACK_MODE_ARP_UP) {
        return;
    }

    if(playback_mode == mode) {
        return;
    }

    if(arp_output_note != NO_NOTE) {
        Playback_OutputNoteOff(arp_output_channel, arp_output_note);
        arp_output_note = NO_NOTE;
    }

    MidiCv_Reset();
    GATE1_LOW();
    GATE2_LOW();
    Playback_ClearHeld();
    playback_mode = mode;
}

playback_mode_t Playback_GetMode(void)
{
    return playback_mode;
}

void Playback_ToggleMode(void)
{
    if(playback_mode == PLAYBACK_MODE_LIVE) {
        Playback_SetMode(PLAYBACK_MODE_ARP_UP);
    } else {
        Playback_SetMode(PLAYBACK_MODE_LIVE);
    }
}

void Playback_NoteOn(const midi_msg_t *msg)
{
    uint8_t note;

    if(msg == 0) {
        return;
    }

    note = msg->data1;
    if(note >= MIDI_NOTE_COUNT) {
        return;
    }

    if(msg->data2 == 0U) {
        Playback_NoteOff(msg);
        return;
    }

    if(held_velocity[note] == 0U) {
        held_count++;
    }
    held_velocity[note] = msg->data2;
    held_channel[note] = msg->channel;

    if(playback_mode == PLAYBACK_MODE_LIVE) {
        Playback_LiveNoteOn(msg);
    }
}

void Playback_NoteOff(const midi_msg_t *msg)
{
    uint8_t note;

    if(msg == 0) {
        return;
    }

    note = msg->data1;
    if(note >= MIDI_NOTE_COUNT) {
        return;
    }

    if(held_velocity[note] != 0U) {
        held_velocity[note] = 0U;
        held_channel[note] = 0U;
        if(held_count != 0U) {
            held_count--;
        }
    }

    if(playback_mode == PLAYBACK_MODE_LIVE) {
        Playback_LiveNoteOff(msg);
    } else if(held_count == 0U && arp_output_note != NO_NOTE) {
        Playback_OutputNoteOff(arp_output_channel, arp_output_note);
        arp_output_note = NO_NOTE;
        arp_current_note = NO_NOTE;
    }
}

void Playback_TransportTick(void)
{
    uint8_t next_note;
    uint8_t next_velocity;
    uint8_t next_channel;

    if(playback_mode != PLAYBACK_MODE_ARP_UP) {
        return;
    }

    if(held_count == 0U) {
        if(arp_output_note != NO_NOTE) {
            Playback_OutputNoteOff(arp_output_channel, arp_output_note);
            arp_output_note = NO_NOTE;
        }
        arp_current_note = NO_NOTE;
        return;
    }

    next_note = Playback_FindNextUp(arp_current_note);
    if(next_note == NO_NOTE) {
        return;
    }

    next_velocity = held_velocity[next_note];
    next_channel = held_channel[next_note];

    /* Retrigger each step by releasing the previous generated note first. */
    if(arp_output_note != NO_NOTE) {
        Playback_OutputNoteOff(arp_output_channel, arp_output_note);
    }

    Playback_OutputNoteOn(next_channel, next_note, next_velocity);
    arp_output_note = next_note;
    arp_output_channel = next_channel;
    arp_current_note = next_note;
}

uint8_t Playback_GetHeldCount(void)
{
    return held_count;
}

uint8_t Playback_IsNoteHeld(uint8_t note)
{
    if(note >= MIDI_NOTE_COUNT) {
        return 0U;
    }

    return (held_velocity[note] != 0U);
}

uint8_t Playback_GetVelocity(uint8_t note)
{
    if(note >= MIDI_NOTE_COUNT) {
        return 0U;
    }

    return held_velocity[note];
}

int8_t Playback_GetLowestHeld(void)
{
    uint8_t note;

    for(note = 0U; note < MIDI_NOTE_COUNT; note++) {
        if(held_velocity[note] != 0U) {
            return (int8_t)note;
        }
    }

    return -1;
}

int8_t Playback_GetHighestHeld(void)
{
    int16_t note;

    for(note = 127; note >= 0; note--) {
        if(held_velocity[(uint8_t)note] != 0U) {
            return (int8_t)note;
        }
    }

    return -1;
}
