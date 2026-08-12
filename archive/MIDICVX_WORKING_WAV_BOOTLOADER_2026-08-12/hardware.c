/*
 * hardware.c
 *
 * Created: 1/16/2017 11:47:23 AM
 *  Author: Janis Zaharans
 */ 

#include "main.h"


void UART_Init(void)
{
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;

#if USE_2X
	UCSR0A |= _BV(U2X0);
#else
	UCSR0A &= ~(_BV(U2X0));
#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(RXCIE0);   /* Enable RX */
}

void ClockOut_Init(void)
{
	CLK_OUT_DDR |= CLK_OUT_PIN;
}

void Gate_Init(void)
{
	GATE1_DDR |= GATE1_PIN;
	GATE2_DDR |= GATE2_PIN;
}

void Switch_Init(void)
{
	VOICE_SW_DDR &= ~(VOICE_SW_PIN);
	VOICE_SW_PORT |= VOICE_SW_PIN;
}

void Button_Init(void)
{
	BUTTON_DDR &= ~(BUTTON_PIN);
	BUTTON_PORT |= BUTTON_PIN;
}

void Led_Init(void)
{
	MIDI_LED_DDR |= MIDI_LED_PIN;
}


void DAC_Init(void)
{
	#ifdef DIY_HW
	
	// Set SPI and DAC selectd and load pins as output
	DAC_DDR |= DAC_CS1_PIN | DAC_CS2_PIN | DAC_MOSI_PIN | DAC_SCK_PIN | DAC_LOAD_PIN;
	DAC_PORT |= DAC_CS1_PIN | DAC_CS2_PIN | DAC_LOAD_PIN;
	
	power_spi_enable();
	
	// Configure SPI
	SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0);
	
	#else
	
	power_twi_enable();
	
	DAC_DDR |= DAC_LOAD_PIN;
	DAC_PORT |= DAC_LOAD_PIN;
	
	I2C_Init();
		
	#endif
}


void Tick_Init(void)
{
	power_timer0_enable();
	
	TCCR0A |= _BV(WGM01);
	TCCR0B |= _BV(CS01) | _BV(CS00);
	
	OCR0A = 250;	// 1kHz
	
	TIMSK0 |= _BV(OCIE0A);
}




