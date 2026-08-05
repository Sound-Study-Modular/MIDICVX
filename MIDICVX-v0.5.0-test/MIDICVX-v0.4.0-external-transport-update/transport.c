#include "main.h"
#include "transport.h"

#define TRANSPORT_LED_HOLD_MS 25U
#define TRANSPORT_MIN_EDGE_MS  1U

static volatile uint32_t transport_ticks;
static volatile uint8_t transport_tick_pending;
static volatile uint8_t previous_clock_state;
static volatile uint32_t last_edge_time;
static volatile uint8_t have_previous_edge;

static uint32_t transport_led_until;
static uint8_t transport_led_active;

void Transport_Init(void)
{
    uint8_t initial_state;

    /* PC1 becomes a high-impedance input; do not enable its pull-up. */
    CLK_OUT_DDR &= (uint8_t)~CLK_OUT_PIN;
    CLK_OUT_PORT &= (uint8_t)~CLK_OUT_PIN;

    initial_state = ((PINC & CLK_OUT_PIN) != 0U);
    previous_clock_state = initial_state;

    transport_ticks = 0;
    transport_tick_pending = 0;
    last_edge_time = 0;
    have_previous_edge = 0;
    transport_led_until = 0;
    transport_led_active = 0;

    /* Clear any stale PC1 pin-change flag, then enable PCINT9. */
    PCIFR |= _BV(PCIF1);
    PCMSK1 |= _BV(PCINT9);
    PCICR |= _BV(PCIE1);
}

void Transport_Process(void)
{
    uint8_t tick_seen = 0;

    /* Atomically consume the ISR event flag. */
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        if(transport_tick_pending != 0U) {
            transport_tick_pending = 0;
            tick_seen = 1;
        }
    }

    if(tick_seen != 0U) {
        /* Lower status/MIDI LED shows the qualified transport tick. */
        MIDI_LED_HIGH();
        transport_led_until = g_time + TRANSPORT_LED_HOLD_MS;
        transport_led_active = 1;
    }

    if(transport_led_active != 0U &&
       (int32_t)(g_time - transport_led_until) >= 0) {
        MIDI_LED_LOW();
        transport_led_active = 0;
    }
}

uint32_t Transport_GetTickCount(void)
{
    uint32_t value;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        value = transport_ticks;
    }

    return value;
}

uint8_t Transport_ConsumeTick(void)
{
    uint8_t pending;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        pending = transport_tick_pending;
        transport_tick_pending = 0;
    }

    return pending;
}

ISR(PCINT1_vect)
{
    uint8_t current_state = ((PINC & CLK_OUT_PIN) != 0U);

    /* One transport event on each LOW-to-HIGH transition. */
    if(current_state != 0U && previous_clock_state == 0U) {
        uint32_t now = g_time;

        if(have_previous_edge == 0U ||
           (uint32_t)(now - last_edge_time) >= TRANSPORT_MIN_EDGE_MS) {
            last_edge_time = now;
            have_previous_edge = 1;
            transport_ticks++;
            transport_tick_pending = 1;
        }
    }

    previous_clock_state = current_state;
}
