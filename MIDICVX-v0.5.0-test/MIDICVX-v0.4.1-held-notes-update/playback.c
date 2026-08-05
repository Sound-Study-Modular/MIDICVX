#include "main.h"

#define MIDI_NOTE_COUNT 128U

static uint8_t held_velocity[MIDI_NOTE_COUNT];
static uint8_t held_count;

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

void Playback_Init(void)
{
    uint8_t note;

    for(note = 0U; note < MIDI_NOTE_COUNT; note++) {
        held_velocity[note] = 0U;
    }

    held_count = 0U;
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

    /* MIDI Note On with velocity zero is treated as Note Off. */
    if(msg->data2 == 0U) {
        Playback_NoteOff(msg);
        return;
    }

    if(held_velocity[note] == 0U) {
        held_count++;
    }
    held_velocity[note] = msg->data2;

    /* v0.4.1 remains exact live pass-through; transport does not play notes yet. */
    Playback_LiveNoteOn(msg);
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
        if(held_count != 0U) {
            held_count--;
        }
    }

    Playback_LiveNoteOff(msg);
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
