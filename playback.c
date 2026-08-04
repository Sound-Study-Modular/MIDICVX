#include "main.h"
#include "playback.h"

#define PLAYBACK_NOTE_COUNT 128
#define PLAYBACK_BITMAP_SIZE (PLAYBACK_NOTE_COUNT / 8)

static playback_mode_t playback_mode;
static uint8_t held_notes[PLAYBACK_BITMAP_SIZE];
static uint8_t held_velocity[PLAYBACK_NOTE_COUNT];
static int8_t arp_note;
static uint8_t arp_velocity;

static void held_clear(void)
{
    uint8_t i;

    for(i = 0; i < PLAYBACK_BITMAP_SIZE; i++)
        held_notes[i] = 0;

    for(i = 0; i < PLAYBACK_NOTE_COUNT; i++)
        held_velocity[i] = 0;
}

static void held_set(uint8_t note, uint8_t velocity)
{
    held_notes[note >> 3] |= (uint8_t)(1U << (note & 7));
    held_velocity[note] = velocity;
}

static void held_release(uint8_t note)
{
    held_notes[note >> 3] &= (uint8_t)~(1U << (note & 7));
    held_velocity[note] = 0;
}

static uint8_t held_is_set(uint8_t note)
{
    return (held_notes[note >> 3] & (uint8_t)(1U << (note & 7))) != 0;
}

static int8_t held_first(void)
{
    uint8_t note;

    for(note = 0; note < PLAYBACK_NOTE_COUNT; note++) {
        if(held_is_set(note))
            return (int8_t)note;
    }

    return -1;
}

static int8_t held_next(int8_t current)
{
    uint8_t note;

    if(current < 0)
        return held_first();

    for(note = (uint8_t)current + 1; note < PLAYBACK_NOTE_COUNT; note++) {
        if(held_is_set(note))
            return (int8_t)note;
    }

    return held_first();
}

static void live_note_on(const midi_msg_t *msg)
{
    if(g_midi_ch[0] == g_midi_ch[1])
        MidiCv_NoteOn(msg->data1, msg->data2);
    else
        MidiCv_NoteOnSingle((msg->channel == g_midi_ch[0] ? 0 : 1), msg->data1, msg->data2);
}

static void live_note_off(const midi_msg_t *msg)
{
    if(g_midi_ch[0] == g_midi_ch[1])
        MidiCv_NoteOff(msg->data1);
    else
        MidiCv_NoteOffSingle((msg->channel == g_midi_ch[0] ? 0 : 1), msg->data1);
}

void Playback_Init(void)
{
    playback_mode = PLAYBACK_MODE_LIVE;
    arp_note = -1;
    arp_velocity = 0;
    held_clear();
}

void Playback_SetMode(playback_mode_t mode)
{
    if(mode != PLAYBACK_MODE_ARP_UP)
        mode = PLAYBACK_MODE_LIVE;

    Playback_Stop();
    held_clear();
    playback_mode = mode;
}

void Playback_ToggleMode(void)
{
    if(playback_mode == PLAYBACK_MODE_LIVE)
        Playback_SetMode(PLAYBACK_MODE_ARP_UP);
    else
        Playback_SetMode(PLAYBACK_MODE_LIVE);
}

playback_mode_t Playback_GetMode(void)
{
    return playback_mode;
}

void Playback_NoteOn(const midi_msg_t *msg)
{
    if(playback_mode == PLAYBACK_MODE_LIVE) {
        live_note_on(msg);
        return;
    }

    /* Alpha arp uses the first configured MIDI channel as its note pool. */
    if(msg->channel == g_midi_ch[0])
        held_set(msg->data1, msg->data2);
}

void Playback_NoteOff(const midi_msg_t *msg)
{
    if(playback_mode == PLAYBACK_MODE_LIVE) {
        live_note_off(msg);
        return;
    }

    if(msg->channel != g_midi_ch[0])
        return;

    held_release(msg->data1);

    if(held_first() < 0)
        Playback_Stop();
}

void Playback_ClockTick(void)
{
    int8_t next_note;

    if(playback_mode != PLAYBACK_MODE_ARP_UP)
        return;

    next_note = held_next(arp_note);
    if(next_note < 0) {
        Playback_Stop();
        return;
    }

    if(arp_note >= 0)
        MidiCv_NoteOff((uint8_t)arp_note);

    arp_note = next_note;
    arp_velocity = held_velocity[(uint8_t)arp_note];
    if(arp_velocity == 0)
        arp_velocity = 100;

    MidiCv_NoteOn((uint8_t)arp_note, arp_velocity);
}

void Playback_Stop(void)
{
    if(arp_note >= 0)
        MidiCv_NoteOff((uint8_t)arp_note);

    arp_note = -1;
    arp_velocity = 0;
}
