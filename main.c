/*
 * DIY MIDI CV.c
 *
 * Created: 1/16/2017 10:33:39 AM
 * Author : Janis Zaharans
 */ 

#include "main.h"


volatile ring_buffer_t	g_midi_buf;
volatile midi_parser_t	g_midi_parser;
volatile uint8_t		g_calib_mode;
volatile uint32_t		g_time;
volatile button_t		g_btn;
volatile uint32_t		g_btn_press_time;
volatile uint8_t		g_midi_ch[2] = { 1, 1 };
volatile uint8_t		g_config_mode;
volatile uint32_t		g_midi_clk_timeout;


int main(void)
{
	uint32_t led_time = 0;
	
	const uint8_t pattern_calib_len[] = { 4, 2 };
	const uint32_t pattern_calib[2][4] = {{40, 960, 0, 0 }, { 40, 200, 40, 720 }};
	const uint8_t pattern_config_len[] = { 4, 2 };
	const uint32_t pattern_config[2][4] = {{40, 460, 0, 0 }, { 40, 100, 40, 320 }};
	
	
	// Hardware initialization
	UART_Init();
	DAC_Init();
	DAC_Config();
	ClockOut_Init();
	MidiCv_Init();
	Switch_Init();
	Button_Init();
	Led_Init();
	Tick_Init();
	Gate_Init();
	
	// Initialize Ring Buffer
	RingBuffer_Init(&g_midi_buf);
	
	// Initialize MIDI parser
	Midi_Init(&g_midi_parser);
	
	g_btn.port = &BUTTON_PORT_IN;
	g_btn.pin = BUTTON_PIN;
	
	_delay_ms(10);

	// Check if first time power-up
	if(eeprom_read_byte(EEPROM_ADDR__FIRST) != 0xA5) {
		eeprom_write_byte(EEPROM_ADDR__FIRST, 0xA5);
		Dac_MidiCalibSave();
		MidiCv_ConfigSave();
	} 
	// Read EEPROM data
	else {
		Dac_MidiCalibLoad();
		MidiCv_ConfigLoad();
	}
	

	// Enable global interrupts
	sei();
	
	// If button held during power-up, enter calibration mode
	if(BUTTON_STATE()) {
		g_calib_mode = 1;
	}
	
	
    while (1) 
    {
		// Read voice switch
		MidiCv_SetVoices((VOICE_SW_PORT_IN & VOICE_SW_PIN) == 0);
		
		// Blink LED
		if(g_calib_mode) {

			Blinker_Process(pattern_calib[g_calib_mode-1], pattern_calib_len[g_calib_mode-1]);

		} else if(g_config_mode) {
			
			Blinker_Process(pattern_config[g_config_mode-1], pattern_config_len[g_config_mode-1]);
			
		} else {
			if(led_time != 0 && (g_time - led_time) > 5) {
				led_time = 0;
				
				MIDI_LED_LOW();
			}
		}
		
		
		if(g_midi_clk_timeout != 0 && (g_time - g_midi_clk_timeout) > 1000) {
			g_midi_clk_timeout = 0;
			CLK_OUT_PORT &= ~CLK_OUT_PIN;
		}
		
		if(g_btn.press) {
			g_btn.press = 0;
			g_btn_press_time = g_time;
			
			if(g_calib_mode == 1) {
				g_calib_mode = 2;
			} else if(g_calib_mode == 2) {
				g_calib_mode = 0;
				
				Dac_MidiCalibSave();
				MidiCv_Reset();
				MIDI_LED_LOW();
			} else if(g_config_mode == 1) {
				g_config_mode = 2;
				
			} else if(g_config_mode == 2) {
				g_config_mode = 0;	
				
				MidiCv_ConfigSave();
				MidiCv_Reset();
				MIDI_LED_LOW();
			} else {
				MidiCv_Reset();
			}

			//MIDI_LED_HIGH();
		}
		
		if(g_btn.release) {
			g_btn.release = 0;
			g_btn_press_time = 0;
			
			//MIDI_LED_LOW();
			
		}
		
		
		// Button hold
		if(g_btn_press_time != 0 && (g_time - g_btn_press_time) > 3000) {
			g_btn_press_time = 0;
			
			g_config_mode = 1;
		}
		
		if(!RingBuffer_IsEmpty(&g_midi_buf)) {
			if(Midi_Parse(&g_midi_parser, RingBuffer_Read(&g_midi_buf))) {
				
				if(g_midi_parser.msg.channel != 0) {
					led_time = g_time;
					MIDI_LED_HIGH();
				}
				
				if(g_calib_mode) {
					Dac_MidiCalibration(&g_midi_parser.msg, g_calib_mode);
				} else if(g_config_mode) {
					MidiCv_Config(&g_midi_parser.msg, g_config_mode);
				} else {
					if(	g_midi_parser.msg.channel == 0 || 
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