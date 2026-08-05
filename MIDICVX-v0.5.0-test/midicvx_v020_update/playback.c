#include "main.h"

void Playback_Init(void)
{
    /* Pass-through foundation: no playback state yet. */
}

void Playback_NoteOn(uint8_t midi_channel, uint8_t note, uint8_t velocity)
{
    /* Preserve the original firmware's exact voice/channel routing. */
    if (g_midi_ch[0] == g_midi_ch[1]) {
        MidiCv_NoteOn(note, velocity);
    } else {
        MidiCv_NoteOnSingle((midi_channel == g_midi_ch[0]) ? 0 : 1,
                            note,
                            velocity);
    }
}

void Playback_NoteOff(uint8_t midi_channel, uint8_t note)
{
    /* Preserve the original firmware's exact voice/channel routing. */
    if (g_midi_ch[0] == g_midi_ch[1]) {
        MidiCv_NoteOff(note);
    } else {
        MidiCv_NoteOffSingle((midi_channel == g_midi_ch[0]) ? 0 : 1,
                             note);
    }
}
