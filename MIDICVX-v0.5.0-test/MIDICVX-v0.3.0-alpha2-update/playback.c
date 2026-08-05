#include "main.h"
#include "playback.h"

static playback_mode_t g_playback_mode = PLAYBACK_MODE_LIVE;
static uint8_t g_held_notes[16];
static uint8_t g_arp_note = 0;
static uint8_t g_arp_has_note = 0;
static uint8_t g_last_velocity = 127;

static void Playback_ClearHeldNotes(void)
{
    uint8_t i;
    for(i = 0; i < sizeof(g_held_notes); ++i)
        g_held_notes[i] = 0;

    g_arp_note = 0;
    g_arp_has_note = 0;
}

static void Playback_HoldNote(uint8_t note)
{
    if(note < 128)
        g_held_notes[note >> 3] |= (uint8_t)(1u << (note & 7));
}

static void Playback_ReleaseNote(uint8_t note)
{
    if(note < 128)
        g_held_notes[note >> 3] &= (uint8_t)~(1u << (note & 7));
}

static uint8_t Playback_IsHeld(uint8_t note)
{
    return (uint8_t)((g_held_notes[note >> 3] & (uint8_t)(1u << (note & 7))) != 0);
}

static int16_t Playback_FindNextUp(uint8_t start)
{
    uint16_t i;

    for(i = 0; i < 128; ++i) {
        uint8_t note = (uint8_t)((start + i) & 0x7f);
        if(Playback_IsHeld(note))
            return note;
    }

    return -1;
}

void Playback_Init(void)
{
    g_playback_mode = PLAYBACK_MODE_LIVE;
    Playback_ClearHeldNotes();
}

void Playback_SetMode(playback_mode_t mode)
{
    g_playback_mode = mode;
    Playback_ClearHeldNotes();
    MidiCv_Reset();
}

void Playback_ToggleMode(void)
{
    Playback_SetMode(g_playback_mode == PLAYBACK_MODE_LIVE ?
        PLAYBACK_MODE_ARP_UP : PLAYBACK_MODE_LIVE);
}

playback_mode_t Playback_GetMode(void)
{
    return g_playback_mode;
}

void Playback_ProcessMidiMessage(midi_msg_t *msg)
{
    if(msg == 0)
        return;

    if(g_playback_mode == PLAYBACK_MODE_LIVE) {
        if(msg->type == MIDI__NOTE_ON) {
            if(g_midi_ch[0] == g_midi_ch[1])
                MidiCv_NoteOn(msg->data1, msg->data2);
            else
                MidiCv_NoteOnSingle((msg->channel == g_midi_ch[0] ? 0 : 1), msg->data1, msg->data2);
        }
        else if(msg->type == MIDI__NOTE_OFF) {
            if(g_midi_ch[0] == g_midi_ch[1])
                MidiCv_NoteOff(msg->data1);
            else
                MidiCv_NoteOffSingle((msg->channel == g_midi_ch[0] ? 0 : 1), msg->data1);
        }
        return;
    }

    if(msg->type == MIDI__NOTE_ON) {
        Playback_HoldNote(msg->data1);
        g_last_velocity = msg->data2;
    }
    else if(msg->type == MIDI__NOTE_OFF) {
        Playback_ReleaseNote(msg->data1);
    }
}

void Playback_ClockStart(void)
{
    g_arp_has_note = 0;
}

void Playback_ClockStop(void)
{
    GATE1_LOW();
    GATE2_LOW();
}

void Playback_ClockTick(void)
{
    int16_t next;
    uint8_t start;

    if(g_playback_mode != PLAYBACK_MODE_ARP_UP)
        return;

    start = g_arp_has_note ? (uint8_t)((g_arp_note + 1) & 0x7f) : 0;
    next = Playback_FindNextUp(start);

    if(next < 0) {
        GATE1_LOW();
        GATE2_LOW();
        g_arp_has_note = 0;
        return;
    }

    GATE1_LOW();
    GATE2_LOW();

    g_arp_note = (uint8_t)next;
    g_arp_has_note = 1;

    DAC_WriteNote(0, g_arp_note);
    DAC_WriteNote(1, g_arp_note);

    _delay_ms(1);
    GATE1_HIGH();
    GATE2_HIGH();
}
