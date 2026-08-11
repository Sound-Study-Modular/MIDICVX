#include "main.h"

#define MIDI_NOTE_COUNT 128U
#define NO_NOTE 0xFFU
#define ARP_GATE_GAP_MS 2U
#define DEFAULT_SPLIT_NOTE 36U  /* Lowest arp note: C1 when middle C (60) is labeled C3 */
#define SPLIT_EEPROM_MAGIC 0xB8U
#define SETTINGS_SAVE_DELAY_MS 2000UL

typedef struct {
    uint8_t current_note;
    uint8_t direction_up;
    uint8_t random_state;
    uint8_t gate_pending;
    uint8_t gate_active;
    uint32_t gate_on_time;
    uint32_t gate_off_time;
} arp_engine_t;

static uint8_t held_velocity[MIDI_NOTE_COUNT];
static uint8_t held_channel[MIDI_NOTE_COUNT];
/* ARP 1 Voice uses physically separate pools. A note can never belong to both. */
static uint8_t bass_velocity[MIDI_NOTE_COUNT];
static uint8_t arp_velocity[MIDI_NOTE_COUNT];
static uint8_t held_order[MIDI_NOTE_COUNT];
static uint8_t held_count;
static uint8_t held_order_count;
static performance_mode_t performance_mode;
static arp_mode_t arp_mode;
static playback_output_t compatibility_output;
static arp_engine_t engine[PLAYBACK_OUTPUT_COUNT];
static uint32_t last_transport_tick;
static uint32_t transport_interval_ms;
static uint8_t split_note = DEFAULT_SPLIT_NOTE;
static uint8_t settings_dirty;
static uint32_t settings_save_due;

static uint8_t IsTwoVoice(void)
{
    return ((VOICE_SW_PORT_IN & VOICE_SW_PIN) == 0U);
}

static void GateLow(playback_output_t out)
{
    if(out == PLAYBACK_OUTPUT_CV2_GATE2) GATE2_LOW();
    else GATE1_LOW();
}

static void GateHigh(playback_output_t out)
{
    if(out == PLAYBACK_OUTPUT_CV2_GATE2) GATE2_HIGH();
    else GATE1_HIGH();
}

static void WriteNote(playback_output_t out, uint8_t note)
{
    DAC_WriteNote((out == PLAYBACK_OUTPUT_CV2_GATE2) ? 1U : 0U, note);
}

static void EngineReset(playback_output_t out)
{
    /* Reset sequence history without inheriting a previous arp step.
     * Fixed nonzero seeds make a mode/algorithm restart deterministic while
     * RANDOM still evolves normally after the first selection. */
    engine[out].current_note = NO_NOTE;
    engine[out].direction_up = 1U;
    engine[out].random_state = (out == PLAYBACK_OUTPUT_CV1_GATE1) ? 0xA7U : 0x5DU;
    engine[out].gate_pending = 0U;
    engine[out].gate_active = 0U;
    engine[out].gate_on_time = 0U;
    engine[out].gate_off_time = 0U;
}

static void SilenceOutput(playback_output_t out)
{
    GateLow(out);
    engine[out].gate_pending = 0U;
    engine[out].gate_active = 0U;
    engine[out].current_note = NO_NOTE;
}

static void SilenceArps(void)
{
    SilenceOutput(PLAYBACK_OUTPUT_CV1_GATE1);
    SilenceOutput(PLAYBACK_OUTPUT_CV2_GATE2);
}

static void ClearHeld(void)
{
    uint8_t n;
    for(n = 0U; n < MIDI_NOTE_COUNT; ++n) {
        held_velocity[n] = 0U;
        held_channel[n] = 0U;
        bass_velocity[n] = 0U;
        arp_velocity[n] = 0U;
    }
    held_count = 0U;
    held_order_count = 0U;
}

static void OrderAppend(uint8_t note)
{
    if(held_order_count < MIDI_NOTE_COUNT) held_order[held_order_count++] = note;
}

static void OrderRemove(uint8_t note)
{
    uint8_t i;
    for(i = 0U; i < held_order_count; ++i) {
        if(held_order[i] == note) {
            for(; (uint8_t)(i + 1U) < held_order_count; ++i) held_order[i] = held_order[i + 1U];
            --held_order_count;
            return;
        }
    }
}

static uint8_t LowestHeld(void)
{
    uint8_t n;
    for(n = 0U; n < MIDI_NOTE_COUNT; ++n) if(held_velocity[n] != 0U) return n;
    return NO_NOTE;
}

static uint8_t HighestHeld(void)
{
    int16_t n;
    for(n = 127; n >= 0; --n) if(held_velocity[(uint8_t)n] != 0U) return (uint8_t)n;
    return NO_NOTE;
}

static uint8_t RankOf(uint8_t note)
{
    uint8_t n, rank = 0U;
    for(n = 0U; n < MIDI_NOTE_COUNT; ++n) {
        if(held_velocity[n] == 0U) continue;
        if(n == note) return rank;
        ++rank;
    }
    return NO_NOTE;
}

static uint8_t InPool(playback_output_t out, uint8_t note)
{
    uint8_t rank;
    uint8_t lower_count;

    if(note >= MIDI_NOTE_COUNT || held_velocity[note] == 0U) return 0U;
    if(performance_mode == PERFORMANCE_MODE_LIVE) return 0U;

    /* MONO ARP: one side of the learned split is live and the other side is
     * the single arpeggiator. The front-panel 1/2 VOICE switch mirrors it. */
    if(performance_mode == PERFORMANCE_MODE_MONO_ARP) {
        if(IsTwoVoice() == 0U) {
            /* 1 VOICE: low = live CV1, high = arp CV2. */
            return (out == PLAYBACK_OUTPUT_CV2_GATE2 && note >= split_note);
        }
        /* 2 VOICE: low = arp CV1, high = live CV2. */
        return (out == PLAYBACK_OUTPUT_CV1_GATE1 && note < split_note);
    }

    /* DUAL ARP + 1 VOICE uses the learned keyboard split: each hand owns an
     * independent arp pool. */
    if(IsTwoVoice() == 0U) {
        if(out == PLAYBACK_OUTPUT_CV1_GATE1) return (note < split_note);
        return (note >= split_note);
    }

    /* DUAL ARP + 2 VOICE preserves beta8.4 dynamic lower/upper assignment. */
    rank = RankOf(note);
    if(rank == NO_NOTE) return 0U;
    lower_count = (uint8_t)((held_count + 1U) / 2U);
    if(out == PLAYBACK_OUTPUT_CV1_GATE1) return (rank < lower_count);
    return (rank >= lower_count);
}

static uint8_t PoolCount(playback_output_t out)
{
    uint8_t n, count = 0U;
    for(n = 0U; n < MIDI_NOTE_COUNT; ++n) if(InPool(out, n)) ++count;
    return count;
}

static uint8_t PoolLowest(playback_output_t out)
{
    uint8_t n;
    for(n = 0U; n < MIDI_NOTE_COUNT; ++n) if(InPool(out, n)) return n;
    return NO_NOTE;
}

static uint8_t PoolHighest(playback_output_t out)
{
    int16_t n;
    for(n = 127; n >= 0; --n) if(InPool(out, (uint8_t)n)) return (uint8_t)n;
    return NO_NOTE;
}

static uint8_t NextUp(playback_output_t out, uint8_t after)
{
    uint16_t n, start = (after == NO_NOTE) ? 0U : (uint16_t)after + 1U;
    for(n = start; n < MIDI_NOTE_COUNT; ++n) if(InPool(out, (uint8_t)n)) return (uint8_t)n;
    for(n = 0U; n < start && n < MIDI_NOTE_COUNT; ++n) if(InPool(out, (uint8_t)n)) return (uint8_t)n;
    return NO_NOTE;
}

static uint8_t NextDown(playback_output_t out, uint8_t before)
{
    int16_t n, start = (before == NO_NOTE) ? 127 : (int16_t)before - 1;
    for(n = start; n >= 0; --n) if(InPool(out, (uint8_t)n)) return (uint8_t)n;
    for(n = 127; n > start; --n) if(InPool(out, (uint8_t)n)) return (uint8_t)n;
    return NO_NOTE;
}

static uint8_t NextOrder(playback_output_t out, uint8_t after)
{
    uint8_t i, start = 0U;
    if(PoolCount(out) == 0U) return NO_NOTE;
    if(after != NO_NOTE) {
        for(i = 0U; i < held_order_count; ++i) if(held_order[i] == after) { start = (uint8_t)(i + 1U); break; }
    }
    for(i = start; i < held_order_count; ++i) if(InPool(out, held_order[i])) return held_order[i];
    for(i = 0U; i < start && i < held_order_count; ++i) if(InPool(out, held_order[i])) return held_order[i];
    return PoolLowest(out);
}

static uint8_t RandomByte(playback_output_t out)
{
    uint8_t x = engine[out].random_state;
    x ^= (uint8_t)(x << 3); x ^= (uint8_t)(x >> 5); x ^= (uint8_t)(x << 1);
    if(x == 0U) x = (out == PLAYBACK_OUTPUT_CV1_GATE1) ? 0xA7U : 0x5DU;
    engine[out].random_state = x;
    return x;
}

static uint8_t NextRandom(playback_output_t out)
{
    uint8_t n, target, choices = PoolCount(out), candidate = NO_NOTE;
    if(choices == 0U) return NO_NOTE;
    if(choices == 1U) return PoolLowest(out);
    if(engine[out].current_note != NO_NOTE && InPool(out, engine[out].current_note)) --choices;
    target = (uint8_t)(RandomByte(out) % choices);
    for(n = 0U; n < MIDI_NOTE_COUNT; ++n) {
        if(!InPool(out, n) || n == engine[out].current_note) continue;
        candidate = n;
        if(target-- == 0U) return n;
    }
    return candidate;
}

static uint8_t NextPingPong(playback_output_t out)
{
    uint8_t next;
    if(PoolCount(out) <= 1U) return PoolLowest(out);
    if(engine[out].current_note == NO_NOTE) { engine[out].direction_up = 1U; return PoolLowest(out); }
    if(engine[out].direction_up) {
        next = NextUp(out, engine[out].current_note);
        if(next <= engine[out].current_note) { engine[out].direction_up = 0U; next = NextDown(out, engine[out].current_note); }
    } else {
        next = NextDown(out, engine[out].current_note);
        if(next >= engine[out].current_note) { engine[out].direction_up = 1U; next = NextUp(out, engine[out].current_note); }
    }
    return next;
}

static uint8_t SelectNext(playback_output_t out)
{
    uint8_t pool_count = PoolCount(out);

    /* Continuous-clock ARP rule:
     * - 0 notes: this arp voice is silent.
     * - 1 note: repeat that exact note on EVERY transport tick, regardless
     *   of the selected algorithm.
     * - 2+ notes: run the selected arp algorithm normally.
     *
     * In MONO ARP, the live side never enters an arp pool, so it keeps
     * normal held-key gate behavior through UpdateMonoLiveVoice().
     */
    if(pool_count == 0U) return NO_NOTE;
    if(pool_count == 1U) return PoolLowest(out);

    switch(arp_mode) {
        case ARP_MODE_UP: return NextUp(out, engine[out].current_note);
        case ARP_MODE_DOWN: return NextDown(out, engine[out].current_note);
        case ARP_MODE_PINGPONG: return NextPingPong(out);
        case ARP_MODE_RANDOM: return NextRandom(out);
        case ARP_MODE_ORDER_PLAYED: return NextOrder(out, engine[out].current_note);
        default: return NO_NOTE;
    }
}

static uint32_t ArpGateWidthMs(void)
{
    uint32_t width;

    /* Use a 50% duty-cycle gate whenever the incoming clock period is known.
     * The minimum keeps fast clocks usable; the default makes the first pulse
     * visible and musically useful before a second clock edge has arrived. */
    if(transport_interval_ms == 0U) return 50U;
    width = transport_interval_ms / 2U;
    if(width < 8U) width = 8U;
    return width;
}

static void StartArpNote(playback_output_t out, uint8_t note)
{
    GateLow(out);
    WriteNote(out, note);
    engine[out].current_note = note;
    engine[out].gate_on_time = g_time + ARP_GATE_GAP_MS;
    engine[out].gate_off_time = engine[out].gate_on_time + ArpGateWidthMs();
    engine[out].gate_pending = 1U;
    engine[out].gate_active = 0U;
}

static uint8_t HighestHeldBelowSplit(void)
{
    int16_t note;
    if(split_note == 0U) return NO_NOTE;
    for(note = (int16_t)split_note - 1; note >= 0; --note)
        if(held_velocity[(uint8_t)note] != 0U) return (uint8_t)note;
    return NO_NOTE;
}

static uint8_t HighestHeldAtOrAboveSplit(void)
{
    int16_t note;
    for(note = 127; note >= (int16_t)split_note; --note)
        if(held_velocity[(uint8_t)note] != 0U) return (uint8_t)note;
    return NO_NOTE;
}

static void UpdateMonoLiveVoice(void)
{
    uint8_t note;
    playback_output_t out;

    if(performance_mode != PERFORMANCE_MODE_MONO_ARP) return;

    if(IsTwoVoice() == 0U) {
        /* MONO ARP / 1 VOICE: live bass below split on CV1/GATE1. */
        out = PLAYBACK_OUTPUT_CV1_GATE1;
        note = HighestHeldBelowSplit();
    } else {
        /* MONO ARP / 2 VOICE: mirrored; live high hand on CV2/GATE2. */
        out = PLAYBACK_OUTPUT_CV2_GATE2;
        note = HighestHeldAtOrAboveSplit();
    }

    if(note == NO_NOTE) {
        GateLow(out);
    } else {
        WriteNote(out, note);
        GateHigh(out);
    }
}

static void ResetArpRouting(void)
{
    SilenceArps();
    EngineReset(PLAYBACK_OUTPUT_CV1_GATE1);
    EngineReset(PLAYBACK_OUTPUT_CV2_GATE2);
    if(performance_mode == PERFORMANCE_MODE_MONO_ARP) UpdateMonoLiveVoice();
}

static void LiveNoteOn(const midi_msg_t *msg)
{
    if(g_midi_ch[0] == g_midi_ch[1]) MidiCv_NoteOn(msg->data1, msg->data2);
    else MidiCv_NoteOnSingle((msg->channel == g_midi_ch[0]) ? 0U : 1U, msg->data1, msg->data2);
}

static void LiveNoteOff(const midi_msg_t *msg)
{
    if(g_midi_ch[0] == g_midi_ch[1]) MidiCv_NoteOff(msg->data1);
    else MidiCv_NoteOffSingle((msg->channel == g_midi_ch[0]) ? 0U : 1U, msg->data1);
}

static void RestoreLiveState(void)
{
    uint8_t i;
    midi_msg_t msg;
    MidiCv_Reset();
    msg.type = MIDI__NOTE_ON;
    for(i = 0U; i < held_order_count; ++i) {
        uint8_t note = held_order[i];
        if(held_velocity[note] == 0U) continue;
        msg.channel = held_channel[note]; msg.data1 = note; msg.data2 = held_velocity[note];
        LiveNoteOn(&msg);
    }
}

static uint8_t SettingsCrc8(const midicvx_settings_v1_t *settings)
{
    uint8_t crc = 0U;
    uint8_t i;
    const uint8_t *data = (const uint8_t *)settings;

    for(i = 0U; i < (uint8_t)sizeof(midicvx_settings_v1_t); ++i) {
        uint8_t byte = (i == 3U) ? 0U : data[i];
        uint8_t bit;
        crc ^= byte;
        for(bit = 0U; bit < 8U; ++bit)
            crc = (crc & 0x80U) ? (uint8_t)((crc << 1U) ^ 0x07U) : (uint8_t)(crc << 1U);
    }
    return crc;
}

void Playback_SaveSettingsNow(void)
{
    midicvx_settings_v1_t settings;
    uint8_t i;

    settings.magic = MIDICVX_SETTINGS_MAGIC;
    settings.version = MIDICVX_SETTINGS_VERSION;
    settings.length = (uint8_t)sizeof(midicvx_settings_v1_t);
    settings.crc8 = 0U;
    settings.performance_mode = (uint8_t)performance_mode;
    settings.arp_mode = (uint8_t)arp_mode;
    settings.split_note = split_note;
    settings.flags = 0U;
    for(i = 0U; i < (uint8_t)sizeof(settings.reserved); ++i) settings.reserved[i] = 0U;
    settings.crc8 = SettingsCrc8(&settings);

    eeprom_update_block(&settings,
                        (void *)(uintptr_t)MIDICVX_EEPROM_SETTINGS_BASE,
                        sizeof(settings));
    settings_dirty = 0U;
}

static void MarkSettingsDirty(void)
{
    settings_dirty = 1U;
    settings_save_due = g_time + SETTINGS_SAVE_DELAY_MS;
}

void Playback_SettingsProcess(void)
{
    if(settings_dirty != 0U && (int32_t)(g_time - settings_save_due) >= 0)
        Playback_SaveSettingsNow();
}

void Playback_LoadSplitSetting(void)
{
    midicvx_settings_v1_t settings;
    uint8_t valid = 0U;

    eeprom_read_block(&settings,
                      (const void *)(uintptr_t)MIDICVX_EEPROM_SETTINGS_BASE,
                      sizeof(settings));

    if(settings.magic == MIDICVX_SETTINGS_MAGIC &&
       settings.version == MIDICVX_SETTINGS_VERSION &&
       settings.length == (uint8_t)sizeof(midicvx_settings_v1_t) &&
       settings.crc8 == SettingsCrc8(&settings) &&
       settings.performance_mode < PERFORMANCE_MODE_COUNT &&
       settings.arp_mode < ARP_MODE_COUNT &&
       settings.split_note < MIDI_NOTE_COUNT) {
        performance_mode = (performance_mode_t)settings.performance_mode;
        arp_mode = (arp_mode_t)settings.arp_mode;
        split_note = settings.split_note;
        valid = 1U;
    }

    if(valid == 0U) {
        /* One-time migration from beta8.5's separate EEPROM fields. */
        uint8_t magic = eeprom_read_byte((const uint8_t *)MIDICVX_EEPROM_SPLIT_MAGIC);
        uint8_t stored = eeprom_read_byte((const uint8_t *)MIDICVX_EEPROM_SPLIT_NOTE);
        uint8_t check = eeprom_read_byte((const uint8_t *)MIDICVX_EEPROM_SPLIT_CHECK);
        uint8_t mode_magic = eeprom_read_byte((const uint8_t *)MIDICVX_EEPROM_MODE_MAGIC);
        uint8_t stored_mode = eeprom_read_byte((const uint8_t *)MIDICVX_EEPROM_MODE);
        uint8_t stored_arp = eeprom_read_byte((const uint8_t *)MIDICVX_EEPROM_ARP_MODE);
        uint8_t mode_check = eeprom_read_byte((const uint8_t *)MIDICVX_EEPROM_MODE_CHECK);

        split_note = (magic == SPLIT_EEPROM_MAGIC &&
                      check == (uint8_t)(stored ^ 0xFFU) &&
                      stored < MIDI_NOTE_COUNT) ? stored : DEFAULT_SPLIT_NOTE;

        if(mode_magic == MIDICVX_MODE_EEPROM_MAGIC &&
           mode_check == (uint8_t)(stored_mode ^ stored_arp ^ 0xA5U) &&
           stored_mode < PERFORMANCE_MODE_COUNT && stored_arp < ARP_MODE_COUNT) {
            performance_mode = (performance_mode_t)stored_mode;
            arp_mode = (arp_mode_t)stored_arp;
        }
        Playback_SaveSettingsNow();
    }

    settings_dirty = 0U;
}

void Playback_SetSplitNote(uint8_t note, uint8_t save_to_eeprom)
{
    if(note >= MIDI_NOTE_COUNT) return;
    split_note = note;

    if(save_to_eeprom != 0U) {
        /* Split learning is an explicit user save, so commit immediately. */
        Playback_SaveSettingsNow();
    }
}

uint8_t Playback_GetSplitNote(void)
{
    return split_note;
}

void Playback_Init(void)
{
    ClearHeld();
    last_transport_tick = 0U;
    transport_interval_ms = 0U;
    performance_mode = PERFORMANCE_MODE_LIVE;
    arp_mode = ARP_MODE_UP;
    compatibility_output = PLAYBACK_OUTPUT_CV1_GATE1;
    settings_dirty = 0U;
    settings_save_due = 0U;
    engine[0].random_state = 0xA7U; engine[1].random_state = 0x5DU;
    EngineReset(PLAYBACK_OUTPUT_CV1_GATE1); EngineReset(PLAYBACK_OUTPUT_CV2_GATE2);
}

void Playback_PanicReset(void)
{
    /* Emergency runtime recovery. Keep the selected mode, algorithm, split,
     * and EEPROM settings, but discard every active MIDI/playback state. */
    GATE1_LOW();
    GATE2_LOW();
    MidiCv_Reset();
    ClearHeld();
    EngineReset(PLAYBACK_OUTPUT_CV1_GATE1);
    EngineReset(PLAYBACK_OUTPUT_CV2_GATE2);
    last_transport_tick = 0U;
    transport_interval_ms = 0U;
}

void Playback_Process(void)
{
    playback_output_t out;
    static uint8_t last_voice_switch = 0xFFU;
    uint8_t current_voice_switch = IsTwoVoice();

    if(last_voice_switch == 0xFFU) {
        last_voice_switch = current_voice_switch;
    } else if(current_voice_switch != last_voice_switch) {
        last_voice_switch = current_voice_switch;

        /* The physical switch is a routing modifier in every performance
         * mode. Rebuild held-note output immediately so a switch flip cannot
         * leave a stale gate or wait for another MIDI event. */
        if(performance_mode == PERFORMANCE_MODE_LIVE) {
            RestoreLiveState();
        } else {
            ResetArpRouting();
        }
    }

    if(performance_mode == PERFORMANCE_MODE_LIVE) return;
    for(out = PLAYBACK_OUTPUT_CV1_GATE1; out < PLAYBACK_OUTPUT_COUNT; out = (playback_output_t)(out + 1U)) {
        if(engine[out].gate_pending && (int32_t)(g_time - engine[out].gate_on_time) >= 0) {
            GateHigh(out);
            engine[out].gate_pending = 0U;
            engine[out].gate_active = 1U;
        }
        if(engine[out].gate_active && (int32_t)(g_time - engine[out].gate_off_time) >= 0) {
            GateLow(out);
            engine[out].gate_active = 0U;
        }
    }
}

void Playback_SetPerformanceMode(performance_mode_t mode, uint8_t save_to_eeprom)
{
    if(mode >= PERFORMANCE_MODE_COUNT || mode == performance_mode) return;

    /* Mode transitions are hard musical boundaries: drop both gates, cancel
     * pending retriggers, reset sequence history, then rebuild only the state
     * that belongs to the destination mode. Held MIDI notes themselves are
     * intentionally preserved. */
    SilenceArps();
    MidiCv_Reset();
    EngineReset(PLAYBACK_OUTPUT_CV1_GATE1);
    EngineReset(PLAYBACK_OUTPUT_CV2_GATE2);
    last_transport_tick = 0U;
    transport_interval_ms = 0U;
    performance_mode = mode;

    if(performance_mode == PERFORMANCE_MODE_LIVE) {
        RestoreLiveState();
    } else if(performance_mode == PERFORMANCE_MODE_MONO_ARP) {
        UpdateMonoLiveVoice();
    }

    if(save_to_eeprom != 0U) MarkSettingsDirty();
}

performance_mode_t Playback_GetPerformanceMode(void) { return performance_mode; }

void Playback_NextPerformanceMode(void)
{
    performance_mode_t next = (performance_mode_t)(performance_mode + 1U);
    if(next >= PERFORMANCE_MODE_COUNT) next = PERFORMANCE_MODE_LIVE;
    Playback_SetPerformanceMode(next, 1U);
}

/* Compatibility helpers retained for older code paths. */
void Playback_SetArpEnabled(uint8_t enabled)
{
    Playback_SetPerformanceMode(enabled ? PERFORMANCE_MODE_MONO_ARP : PERFORMANCE_MODE_LIVE, 1U);
}
uint8_t Playback_IsArpEnabled(void) { return (performance_mode != PERFORMANCE_MODE_LIVE); }
void Playback_ToggleArp(void) { Playback_NextPerformanceMode(); }

void Playback_SetArpMode(arp_mode_t mode)
{
    if(mode >= ARP_MODE_COUNT || mode == arp_mode) return;

    /* Algorithm changes restart both arp voices from step zero. This means
     * UP begins at the lowest note, DOWN at the highest, PING-PONG ascending,
     * ORDER PLAYED at the first held key, and RANDOM has no previous note. */
    arp_mode = mode;
    ResetArpRouting();
    last_transport_tick = 0U;
    transport_interval_ms = 0U;
    MarkSettingsDirty();
}
arp_mode_t Playback_GetArpMode(void) { return arp_mode; }
void Playback_NextArpMode(void)
{
    arp_mode_t next;
    if(performance_mode == PERFORMANCE_MODE_LIVE) return;
    next = (arp_mode_t)(arp_mode + 1U); if(next >= ARP_MODE_COUNT) next = ARP_MODE_UP;
    Playback_SetArpMode(next);
}
void Playback_SetArpOutput(playback_output_t output) { if(output < PLAYBACK_OUTPUT_COUNT) compatibility_output = output; }
playback_output_t Playback_GetArpOutput(void) { return compatibility_output; }

void Playback_NoteOn(const midi_msg_t *msg)
{
    uint8_t note;
    if(msg == 0) return;
    note = msg->data1; if(note >= MIDI_NOTE_COUNT) return;
    if(msg->data2 == 0U) { Playback_NoteOff(msg); return; }

    SplitLearn_OnNoteOn(note);

    if(held_velocity[note] == 0U) { ++held_count; OrderAppend(note); }
    held_velocity[note] = msg->data2;
    held_channel[note] = msg->channel;
    if(note < split_note) {
        bass_velocity[note] = msg->data2;
        arp_velocity[note] = 0U;
    } else {
        arp_velocity[note] = msg->data2;
        bass_velocity[note] = 0U;
    }

    if(performance_mode == PERFORMANCE_MODE_LIVE) {
        LiveNoteOn(msg);
        return;
    }

    if(performance_mode == PERFORMANCE_MODE_MONO_ARP) {
        if(IsTwoVoice() == 0U) {
            if(note < split_note) UpdateMonoLiveVoice();
            else EngineReset(PLAYBACK_OUTPUT_CV2_GATE2);
        } else {
            if(note >= split_note) UpdateMonoLiveVoice();
            else EngineReset(PLAYBACK_OUTPUT_CV1_GATE1);
        }
        return;
    }

    /* DUAL ARP: split-hand routing resets only the affected hand; dynamic
     * routing must rebuild both pools because every rank can change. */
    if(IsTwoVoice() == 0U) {
        EngineReset((note < split_note) ? PLAYBACK_OUTPUT_CV1_GATE1 : PLAYBACK_OUTPUT_CV2_GATE2);
    } else {
        EngineReset(PLAYBACK_OUTPUT_CV1_GATE1);
        EngineReset(PLAYBACK_OUTPUT_CV2_GATE2);
    }
}

void Playback_NoteOff(const midi_msg_t *msg)
{
    uint8_t note;
    if(msg == 0) return;
    note = msg->data1; if(note >= MIDI_NOTE_COUNT) return;

    if(held_velocity[note] != 0U) {
        held_velocity[note] = 0U;
        held_channel[note] = 0U;
        bass_velocity[note] = 0U;
        arp_velocity[note] = 0U;
        OrderRemove(note);
        if(held_count) --held_count;
    }

    if(performance_mode == PERFORMANCE_MODE_LIVE) {
        LiveNoteOff(msg);
        return;
    }

    if(performance_mode == PERFORMANCE_MODE_MONO_ARP) {
        if(IsTwoVoice() == 0U) {
            if(note < split_note) UpdateMonoLiveVoice();
            else {
                EngineReset(PLAYBACK_OUTPUT_CV2_GATE2);
                if(PoolCount(PLAYBACK_OUTPUT_CV2_GATE2) == 0U) SilenceOutput(PLAYBACK_OUTPUT_CV2_GATE2);
            }
        } else {
            if(note >= split_note) UpdateMonoLiveVoice();
            else {
                EngineReset(PLAYBACK_OUTPUT_CV1_GATE1);
                if(PoolCount(PLAYBACK_OUTPUT_CV1_GATE1) == 0U) SilenceOutput(PLAYBACK_OUTPUT_CV1_GATE1);
            }
        }
        return;
    }

    if(IsTwoVoice() == 0U) {
        playback_output_t out = (note < split_note) ? PLAYBACK_OUTPUT_CV1_GATE1 : PLAYBACK_OUTPUT_CV2_GATE2;
        EngineReset(out);
        if(PoolCount(out) == 0U) SilenceOutput(out);
    } else {
        EngineReset(PLAYBACK_OUTPUT_CV1_GATE1);
        EngineReset(PLAYBACK_OUTPUT_CV2_GATE2);
        if(held_count == 0U) SilenceArps();
    }
}

void Playback_TransportTick(void)
{
    uint8_t note;
    uint32_t now = g_time;

    if(last_transport_tick != 0U) {
        uint32_t measured = now - last_transport_tick;
        if(measured != 0U) transport_interval_ms = measured;
    }
    last_transport_tick = now;

    if(performance_mode == PERFORMANCE_MODE_LIVE) return;

    if(performance_mode == PERFORMANCE_MODE_MONO_ARP) {
        UpdateMonoLiveVoice();
        if(IsTwoVoice() == 0U) {
            note = SelectNext(PLAYBACK_OUTPUT_CV2_GATE2);
            if(note == NO_NOTE) SilenceOutput(PLAYBACK_OUTPUT_CV2_GATE2);
            else StartArpNote(PLAYBACK_OUTPUT_CV2_GATE2, note);
        } else {
            note = SelectNext(PLAYBACK_OUTPUT_CV1_GATE1);
            if(note == NO_NOTE) SilenceOutput(PLAYBACK_OUTPUT_CV1_GATE1);
            else StartArpNote(PLAYBACK_OUTPUT_CV1_GATE1, note);
        }
        return;
    }

    /* DUAL ARP: both outputs advance from independent pools on every tick. */
    note = SelectNext(PLAYBACK_OUTPUT_CV1_GATE1);
    if(note == NO_NOTE) SilenceOutput(PLAYBACK_OUTPUT_CV1_GATE1);
    else StartArpNote(PLAYBACK_OUTPUT_CV1_GATE1, note);

    note = SelectNext(PLAYBACK_OUTPUT_CV2_GATE2);
    if(note == NO_NOTE) SilenceOutput(PLAYBACK_OUTPUT_CV2_GATE2);
    else StartArpNote(PLAYBACK_OUTPUT_CV2_GATE2, note);
}

uint8_t Playback_GetHeldCount(void) { return held_count; }
uint8_t Playback_IsNoteHeld(uint8_t note) { return (note < MIDI_NOTE_COUNT && held_velocity[note] != 0U); }
uint8_t Playback_GetVelocity(uint8_t note) { return (note < MIDI_NOTE_COUNT) ? held_velocity[note] : 0U; }
int8_t Playback_GetLowestHeld(void) { uint8_t n = LowestHeld(); return (n == NO_NOTE) ? -1 : (int8_t)n; }
int8_t Playback_GetHighestHeld(void) { uint8_t n = HighestHeld(); return (n == NO_NOTE) ? -1 : (int8_t)n; }
