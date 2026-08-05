#include "main.h"

#define MIDI_NOTE_COUNT       128U
#define NO_NOTE               0xFFU
#define ARP_GATE_GAP_MS        2U

static uint8_t held_velocity[MIDI_NOTE_COUNT];
static uint8_t held_channel[MIDI_NOTE_COUNT];
static uint8_t held_count;
static uint8_t held_order[MIDI_NOTE_COUNT];
static uint8_t held_order_count;
static playback_mode_t playback_mode;
static uint8_t arp_current_note;
static uint8_t arp_output_note;
static uint8_t arp_direction_up;
static uint8_t random_state;
static uint8_t gate_pending;
static uint32_t gate_on_time;

static void Playback_ClearHeld(void)
{
    uint8_t note;

    for(note = 0U; note < MIDI_NOTE_COUNT; note++) {
        held_velocity[note] = 0U;
        held_channel[note] = 0U;
    }

    held_count = 0U;
    held_order_count = 0U;
    arp_current_note = NO_NOTE;
}


static void Playback_OrderAppend(uint8_t note)
{
    if(held_order_count < MIDI_NOTE_COUNT) {
        held_order[held_order_count] = note;
        held_order_count++;
    }
}

static void Playback_OrderRemove(uint8_t note)
{
    uint8_t i;

    for(i = 0U; i < held_order_count; i++) {
        if(held_order[i] == note) {
            for(; (uint8_t)(i + 1U) < held_order_count; i++) {
                held_order[i] = held_order[i + 1U];
            }
            held_order_count--;
            return;
        }
    }
}

static uint8_t Playback_FindNextOrderPlayed(uint8_t after_note)
{
    uint8_t i;

    if(held_order_count == 0U) {
        return NO_NOTE;
    }

    if(after_note == NO_NOTE) {
        return held_order[0U];
    }

    for(i = 0U; i < held_order_count; i++) {
        if(held_order[i] == after_note) {
            i++;
            if(i >= held_order_count) {
                i = 0U;
            }
            return held_order[i];
        }
    }

    /* The previous note may have been released between ticks. */
    return held_order[0U];
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

/* Arpeggiator output is intentionally fixed to CV1/Gate1. */
static void Playback_ArpSilence(void)
{
    GATE1_LOW();
    gate_pending = 0U;
    arp_output_note = NO_NOTE;
}

static void Playback_ArpStartNote(uint8_t note, uint8_t velocity)
{
    GATE1_LOW();
    DAC_WriteNote(0U, note);

    /* Preserve the stock velocity-to-MOD behavior for generated notes. */
    if(velocity != 0U) {
        /* MidiCv owns configuration of MOD output, so do not force it here. */
    }

    arp_output_note = note;
    gate_on_time = g_time + ARP_GATE_GAP_MS;
    gate_pending = 1U;
}

static uint8_t Playback_FindLowest(void)
{
    uint8_t note;

    for(note = 0U; note < MIDI_NOTE_COUNT; note++) {
        if(held_velocity[note] != 0U) {
            return note;
        }
    }

    return NO_NOTE;
}

static uint8_t Playback_FindHighest(void)
{
    int16_t note;

    for(note = 127; note >= 0; note--) {
        if(held_velocity[(uint8_t)note] != 0U) {
            return (uint8_t)note;
        }
    }

    return NO_NOTE;
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

static uint8_t Playback_FindNextDown(uint8_t before_note)
{
    int16_t note;
    int16_t start;

    if(held_count == 0U) {
        return NO_NOTE;
    }

    start = (before_note == NO_NOTE) ? 127 : ((int16_t)before_note - 1);

    for(note = start; note >= 0; note--) {
        if(held_velocity[(uint8_t)note] != 0U) {
            return (uint8_t)note;
        }
    }

    for(note = 127; note > start; note--) {
        if(held_velocity[(uint8_t)note] != 0U) {
            return (uint8_t)note;
        }
    }

    return NO_NOTE;
}

static uint8_t Playback_FindPingPong(void)
{
    uint8_t next_note;

    if(held_count <= 1U) {
        return Playback_FindLowest();
    }

    if(arp_current_note == NO_NOTE) {
        arp_direction_up = 1U;
        return Playback_FindLowest();
    }

    if(arp_direction_up != 0U) {
        next_note = Playback_FindNextUp(arp_current_note);
        if(next_note <= arp_current_note) {
            arp_direction_up = 0U;
            next_note = Playback_FindNextDown(arp_current_note);
        }
    } else {
        next_note = Playback_FindNextDown(arp_current_note);
        if(next_note >= arp_current_note) {
            arp_direction_up = 1U;
            next_note = Playback_FindNextUp(arp_current_note);
        }
    }

    return next_note;
}

static uint8_t Playback_RandomByte(void)
{
    /* Small nonzero xorshift generator: sufficient for note selection. */
    random_state ^= (uint8_t)(random_state << 3);
    random_state ^= (uint8_t)(random_state >> 5);
    random_state ^= (uint8_t)(random_state << 1);
    if(random_state == 0U) {
        random_state = 0xA7U;
    }
    return random_state;
}

static uint8_t Playback_FindRandom(void)
{
    uint8_t target;
    uint8_t note;
    uint8_t candidate = NO_NOTE;
    uint8_t choices = held_count;

    if(held_count == 0U) {
        return NO_NOTE;
    }
    if(held_count == 1U) {
        return Playback_FindLowest();
    }

    /* Exclude the previous output whenever another held note exists. */
    if(arp_current_note != NO_NOTE && held_velocity[arp_current_note] != 0U) {
        choices--;
    }

    target = (uint8_t)(Playback_RandomByte() % choices);

    for(note = 0U; note < MIDI_NOTE_COUNT; note++) {
        if(held_velocity[note] == 0U || note == arp_current_note) {
            continue;
        }
        candidate = note;
        if(target == 0U) {
            return candidate;
        }
        target--;
    }

    return (candidate != NO_NOTE) ? candidate : Playback_FindLowest();
}

void Playback_Init(void)
{
    Playback_ClearHeld();
    playback_mode = PLAYBACK_MODE_LIVE;
    arp_output_note = NO_NOTE;
    arp_direction_up = 1U;
    random_state = 0xA7U;
    gate_pending = 0U;
    gate_on_time = 0U;
}

void Playback_Process(void)
{
    if(gate_pending != 0U && (int32_t)(g_time - gate_on_time) >= 0) {
        GATE1_HIGH();
        gate_pending = 0U;
    }
}

void Playback_SetMode(playback_mode_t mode)
{
    if(mode >= PLAYBACK_MODE_COUNT || playback_mode == mode) {
        return;
    }

    Playback_ArpSilence();
    MidiCv_Reset();
    GATE2_LOW();
    Playback_ClearHeld();

    playback_mode = mode;
    arp_direction_up = 1U;
    random_state ^= (uint8_t)g_time;
}

playback_mode_t Playback_GetMode(void)
{
    return playback_mode;
}

void Playback_NextMode(void)
{
    playback_mode_t next = (playback_mode_t)(playback_mode + 1U);
    if(next >= PLAYBACK_MODE_COUNT) {
        next = PLAYBACK_MODE_LIVE;
    }
    Playback_SetMode(next);
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
        Playback_OrderAppend(note);
    }
    held_velocity[note] = msg->data2;
    held_channel[note] = msg->channel;
    random_state ^= (uint8_t)(note + msg->data2 + g_time);

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
        Playback_OrderRemove(note);
        if(held_count != 0U) {
            held_count--;
        }
    }

    if(playback_mode == PLAYBACK_MODE_LIVE) {
        Playback_LiveNoteOff(msg);
    } else if(held_count == 0U) {
        Playback_ArpSilence();
        arp_current_note = NO_NOTE;
    }
}

void Playback_TransportTick(void)
{
    uint8_t next_note = NO_NOTE;

    if(playback_mode == PLAYBACK_MODE_LIVE) {
        return;
    }

    if(held_count == 0U) {
        Playback_ArpSilence();
        arp_current_note = NO_NOTE;
        return;
    }

    switch(playback_mode) {
        case PLAYBACK_MODE_ARP_UP:
            next_note = Playback_FindNextUp(arp_current_note);
            break;

        case PLAYBACK_MODE_ARP_DOWN:
            next_note = Playback_FindNextDown(arp_current_note);
            break;

        case PLAYBACK_MODE_ARP_PINGPONG:
            next_note = Playback_FindPingPong();
            break;

        case PLAYBACK_MODE_ARP_RANDOM:
            next_note = Playback_FindRandom();
            break;

        case PLAYBACK_MODE_ARP_ORDER_PLAYED:
            next_note = Playback_FindNextOrderPlayed(arp_current_note);
            break;

        default:
            return;
    }

    if(next_note == NO_NOTE) {
        return;
    }

    Playback_ArpStartNote(next_note, held_velocity[next_note]);
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
    uint8_t note = Playback_FindLowest();
    return (note == NO_NOTE) ? -1 : (int8_t)note;
}

int8_t Playback_GetHighestHeld(void)
{
    uint8_t note = Playback_FindHighest();
    return (note == NO_NOTE) ? -1 : (int8_t)note;
}
