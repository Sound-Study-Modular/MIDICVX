/*
 * DIY MIDI CV.c
 *
 * Created: 1/16/2017 10:33:39 AM
 * Author : Janis Zaharans
 */

#include "main.h"

#define BUTTON_LONG_PRESS_MS       700U
#define BUTTON_CONFIG_PRESS_MS    3000U
#define BUTTON_PANIC_PRESS_MS     8000U

#define MODE_LED_BLINK_MS         375U

volatile ring_buffer_t g_midi_buf;
volatile midi_parser_t g_midi_parser;
volatile uint8_t       g_calib_mode;
volatile uint32_t      g_time;
volatile button_t      g_btn;
volatile uint32_t      g_btn_press_time;
volatile uint8_t       g_midi_ch[2] = { 1, 1 };
volatile uint8_t       g_config_mode;
volatile uint32_t      g_midi_clk_timeout;

#define SPLIT_LEARN_WINDOW_MS   2000U
#define SPLIT_LEARN_BLINK_MS     100U

#define ALGO_LED_STEP_MS         100U

static void SplitLearn_Gates(uint8_t on)
{
	if(on != 0U) {
		GATE1_HIGH();
		GATE2_HIGH();
	} else {
		GATE1_LOW();
		GATE2_LOW();
	}
}

typedef struct
{
	uint8_t active;
	uint8_t note_received;
	uint8_t lowest_note;
	uint8_t red_flash_active;
	uint32_t end_time;
	uint32_t red_flash_end;
} split_learn_state_t;

static split_learn_state_t g_split_learn;

static void ArpLed_SetStableState(void);

static uint32_t g_panic_press_time;
static uint8_t g_panic_fired;
static performance_mode_t g_mode_at_button_press;

static void PanicLed_Confirm(void)
{
    uint8_t i;
    /* Use only the red status LED: the yellow LEDs are the actual gate
     * outputs and must remain LOW throughout a panic reset. */
    for(i = 0U; i < 3U; ++i) {
        MIDI_LED_HIGH();
        _delay_ms(70);
        MIDI_LED_LOW();
        _delay_ms(70);
    }
}


static void SplitLearn_Start(void)
{
	g_split_learn.active = 1U;
	g_split_learn.note_received = 0U;
	g_split_learn.lowest_note = 0xFFU;
	g_split_learn.red_flash_active = 0U;
	g_split_learn.end_time = g_time + SPLIT_LEARN_WINDOW_MS;
	SplitLearn_Gates(1U);
}

uint8_t SplitLearn_OnNoteOn(uint8_t note)
{
	if(g_split_learn.active == 0U || note >= 128U) {
		return 0U;
	}

	/* The first valid Note On is the selected lowest ARP note. */
	g_split_learn.active = 0U;
	g_split_learn.note_received = 1U;
	g_split_learn.lowest_note = note;
	g_split_learn.red_flash_active = 0U;

	/* Release the yellow learn indication before acknowledging the key. */
	SplitLearn_Gates(0U);

	/* Blocking pulse cannot be overwritten by the normal ARP LED owner. */
	MIDI_LED_HIGH();
	_delay_ms(90);
	MIDI_LED_LOW();
	_delay_ms(60);

	Playback_SetSplitNote(note, 1U);
	ArpLed_SetStableState();

	return 1U;
}

static void SplitLearn_Process(void)
{
	if(g_split_learn.active == 0U ||
	   (int32_t)(g_time - g_split_learn.end_time) < 0) {
		return;
	}

	/* No note was played during the window: keep the stored split. */
	g_split_learn.active = 0U;
	SplitLearn_Gates(0U);
	ArpLed_SetStableState();
}



typedef struct
{
	uint8_t led_on;
	uint32_t next_time;
} mode_led_state_t;

static mode_led_state_t g_arp_led;

static void ArpLed_Apply(uint8_t on)
{
	if(on != 0U) MIDI_LED_HIGH();
	else MIDI_LED_LOW();
	g_arp_led.led_on = on;
}

static void ArpLed_SetStableState(void)
{
	performance_mode_t mode = Playback_GetPerformanceMode();
	g_arp_led.next_time = g_time + MODE_LED_BLINK_MS;

	if(mode == PERFORMANCE_MODE_LIVE) ArpLed_Apply(0U);
	else if(mode == PERFORMANCE_MODE_MONO_ARP) ArpLed_Apply(1U);
	else ArpLed_Apply(1U);
}

static void ArpLed_Process(void)
{
	performance_mode_t mode = Playback_GetPerformanceMode();

	if(mode == PERFORMANCE_MODE_LIVE) {
		ArpLed_Apply(0U);
		return;
	}

	if(mode == PERFORMANCE_MODE_MONO_ARP) {
		ArpLed_Apply(1U);
		return;
	}

	/* DUAL ARP heartbeat is intentionally free-running, not clock-synced. */
	if((int32_t)(g_time - g_arp_led.next_time) >= 0) {
		ArpLed_Apply((uint8_t)(g_arp_led.led_on == 0U));
		g_arp_led.next_time = g_time + MODE_LED_BLINK_MS;
	}
}

typedef struct
{
	uint8_t active;
	uint8_t toggles_remaining;
	uint8_t led_on;
	uint32_t next_time;
} algo_led_indicator_t;

static algo_led_indicator_t g_algo_led;

static void AlgoLed_Start(arp_mode_t mode)
{
	uint8_t blink_count = (uint8_t)mode + 1U;

	g_algo_led.active = 1U;
	g_algo_led.toggles_remaining = (uint8_t)(blink_count * 2U);
	g_algo_led.led_on = 0U;
	g_algo_led.next_time = g_time;
	MIDI_LED_LOW();
}

static void AlgoLed_Process(void)
{
	if(g_algo_led.active == 0U) {
		return;
	}

	if((int32_t)(g_time - g_algo_led.next_time) < 0) {
		return;
	}

	if(g_algo_led.toggles_remaining == 0U) {
		g_algo_led.active = 0U;
		ArpLed_SetStableState();
		return;
	}

	g_algo_led.led_on = (uint8_t)(g_algo_led.led_on == 0U);
	if(g_algo_led.led_on != 0U) {
		MIDI_LED_HIGH();
	} else {
		MIDI_LED_LOW();
	}

	g_algo_led.toggles_remaining--;
	g_algo_led.next_time = g_time + ALGO_LED_STEP_MS;
}

static void Panic_Reset(void)
{
    /* A normal long hold changes mode at ~700 ms. If the user continues all
     * the way to the panic threshold, restore the mode that was active when
     * the hold began so panic itself never changes saved musical settings. */
    if(Playback_GetPerformanceMode() != g_mode_at_button_press) {
        Playback_SetPerformanceMode(g_mode_at_button_press, 0U);
        Playback_SaveSettingsNow();
    }

    g_split_learn.active = 0U;
    SplitLearn_Gates(0U);
    g_algo_led.active = 0U;
    Playback_PanicReset();
    PanicLed_Confirm();
    ArpLed_SetStableState();
}

int main(void)
{
	uint8_t button_press_consumed = 0U;

	const uint8_t pattern_calib_len[] = { 4, 2 };
	const uint32_t pattern_calib[2][4] = {
		{ 40, 960, 0, 0 },
		{ 40, 200, 40, 720 }
	};
	const uint8_t pattern_config_len[] = { 4, 2 };
	const uint32_t pattern_config[2][4] = {
		{ 40, 460, 0, 0 },
		{ 40, 100, 40, 320 }
	};

	/* Hardware initialization */
	UART_Init();
	DAC_Init();
	DAC_Config();
	Transport_Init();
	MidiCv_Init();
	Playback_Init();
	Switch_Init();
	Button_Init();
	Led_Init();
	Tick_Init();
	Gate_Init();

	/* Initialize Ring Buffer */
	RingBuffer_Init(&g_midi_buf);

	/* Initialize MIDI parser */
	Midi_Init(&g_midi_parser);

	g_btn.port = &BUTTON_PORT_IN;
	g_btn.pin = BUTTON_PIN;

	_delay_ms(10);

	/* Check if first time power-up */
	if(eeprom_read_byte(EEPROM_ADDR__FIRST) != 0xA5) {
		eeprom_write_byte(EEPROM_ADDR__FIRST, 0xA5);
		Dac_MidiCalibSave();
		MidiCv_ConfigSave();
	} else {
		Dac_MidiCalibLoad();
		MidiCv_ConfigLoad();
	}

	Playback_LoadSplitSetting();
	ArpLed_SetStableState();
	g_algo_led.active = 0U;

	/* Enable global interrupts */
	sei();

	/* Learn the lowest ARP note through the normal runtime MIDI path. */
	SplitLearn_Start();

	/* If button held during power-up, enter calibration mode */
	if(BUTTON_STATE()) {
		g_calib_mode = 1U;
	}

	while(1)
	{
		uint8_t special_mode_active;

		Transport_Process();

		/* Apply the physical voice switch before playback evaluates a switch
		 * transition, so held LIVE notes are rebuilt using the new routing. */
		MidiCv_SetVoices((VOICE_SW_PORT_IN & VOICE_SW_PIN) == 0);

		Playback_Process();
		Playback_SettingsProcess();
		SplitLearn_Process();

		special_mode_active = (g_calib_mode != 0U || g_config_mode != 0U);

		/* Eight-second PROGRAM hold: emergency note/gate reset. */
		if(g_panic_fired == 0U &&
		   special_mode_active == 0U &&
		   g_panic_press_time != 0U &&
		   BUTTON_STATE() &&
		   (g_time - g_panic_press_time) >= BUTTON_PANIC_PRESS_MS) {
			Panic_Reset();
			g_panic_fired = 1U;
			button_press_consumed = 1U;
			g_btn_press_time = 0U;
		}

		/* Long hold cycles LIVE -> MONO ARP -> DUAL ARP -> LIVE. */
		if(button_press_consumed == 0U &&
		   special_mode_active == 0U &&
		   g_btn_press_time != 0U &&
		   BUTTON_STATE()) {
			uint32_t held = g_time - g_btn_press_time;
			if(held >= BUTTON_LONG_PRESS_MS && held < BUTTON_CONFIG_PRESS_MS) {
				Playback_NextPerformanceMode();
				ArpLed_SetStableState();
				button_press_consumed = 1U;
				g_btn_press_time = 0U;
			}
		}


		/* Calibration/configuration patterns take ownership of the LED. */
		if(g_calib_mode != 0U) {
			Blinker_Process(
				(uint32_t *)pattern_calib[g_calib_mode - 1U],
				pattern_calib_len[g_calib_mode - 1U]
			);
		} else if(g_config_mode != 0U) {
			Blinker_Process(
				(uint32_t *)pattern_config[g_config_mode - 1U],
				pattern_config_len[g_config_mode - 1U]
			);
		} else if(g_algo_led.active != 0U) {
			AlgoLed_Process();
		} else {
			ArpLed_Process();
		}

		if(g_btn.press) {
			g_btn.press = 0U;
			g_btn_press_time = g_time;
			g_panic_press_time = g_time;
			g_panic_fired = 0U;
			g_mode_at_button_press = Playback_GetPerformanceMode();
			button_press_consumed = 0U;

			if(g_calib_mode == 1U) {
				button_press_consumed = 1U;
				g_calib_mode = 2U;
			} else if(g_calib_mode == 2U) {
				button_press_consumed = 1U;
				g_calib_mode = 0U;
				Dac_MidiCalibSave();
				MidiCv_Reset();
				ArpLed_SetStableState();
			} else if(g_config_mode == 1U) {
				button_press_consumed = 1U;
				g_config_mode = 2U;
			} else if(g_config_mode == 2U) {
				button_press_consumed = 1U;
				g_config_mode = 0U;
				MidiCv_ConfigSave();
				MidiCv_Reset();
				ArpLed_SetStableState();
			}
		}

		if(g_btn.release) {
			uint32_t press_duration = 0U;

			g_btn.release = 0U;

			if(g_btn_press_time != 0U) {
				press_duration = g_time - g_btn_press_time;
			}

			if(button_press_consumed == 0U &&
			   special_mode_active == 0U &&
			   g_btn_press_time != 0U) {

				if(press_duration >= BUTTON_CONFIG_PRESS_MS) {
					/* Preserve the original configuration-mode access. */
					g_config_mode = 1U;
					ArpLed_SetStableState();
				} else if(press_duration >= BUTTON_LONG_PRESS_MS) {
					/* Already handled while button was held. */
				} else if(Playback_IsArpEnabled() != 0U) {
					/* Short presses change algorithms only while ARP is enabled. */
					Playback_NextArpMode();
					AlgoLed_Start(Playback_GetArpMode());
				}
			}

			g_btn_press_time = 0U;
			g_panic_press_time = 0U;
			g_panic_fired = 0U;
			button_press_consumed = 0U;
		}

		if(!RingBuffer_IsEmpty(&g_midi_buf)) {
			if(Midi_Parse(&g_midi_parser, RingBuffer_Read(&g_midi_buf))) {
				if(g_calib_mode != 0U) {
					Dac_MidiCalibration(&g_midi_parser.msg, g_calib_mode);
				} else if(g_config_mode != 0U) {
					MidiCv_Config(&g_midi_parser.msg, g_config_mode);
				} else {
					if(g_midi_parser.msg.channel == 0U ||
					   g_midi_parser.msg.channel == g_midi_ch[0] ||
					   g_midi_parser.msg.channel == g_midi_ch[1]) {
						Midi_ExecuteMessage(&g_midi_parser.msg);
					}
				}
			}
		}
	}
}

ISR(USART_RX_vect)
{
	RingBuffer_Write(&g_midi_buf, UDR0);
}

ISR(TIMER0_COMPA_vect)
{
	g_time++;
	Button_Process(&g_btn);
}
