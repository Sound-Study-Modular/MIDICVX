/*
 * hardware.h
 *
 * Created: 1/16/2017 10:50:57 AM
 *  Author: Janis Zaharans
 */ 


#ifndef HARDWARE_H_
#define HARDWARE_H_

#define DIY_HW		1

#define F_CPU		16000000UL
#define BAUD		31250


#ifdef DIY_HW

#define MIDI_LED_PORT		PORTC
#define MIDI_LED_DDR		DDRC
#define MIDI_LED_PIN		(1<<3)

#define MIDI_PORT			PORTD
#define MIDI_DDR			DDRD
#define MIDI_RX_PIN			(1<<0)

#define GATE1_PORT			PORTC
#define GATE1_DDR			DDRC
#define GATE1_PIN			(1<<2)

#define GATE2_PORT			PORTC
#define GATE2_DDR			DDRC
#define GATE2_PIN			(1<<4)

#define CLK_OUT_PORT		PORTC
#define CLK_OUT_DDR			DDRC
#define CLK_OUT_PIN			(1<<1)

#define BUTTON_PORT			PORTC
#define BUTTON_DDR			DDRC
#define BUTTON_PORT_IN		PINC
#define BUTTON_PIN			(1<<5)

#define VOICE_SW_PORT		PORTD
#define VOICE_SW_DDR		DDRD
#define VOICE_SW_PORT_IN	PIND
#define VOICE_SW_PIN		(1<<2)


// --- DAC ---
#define DAC_PORT			PORTB
#define DAC_DDR				DDRB
#define DAC_MOSI_PIN		(1<<3)
#define DAC_MISO_PIN		(1<<4)
#define DAC_SCK_PIN			(1<<5)
#define DAC_LOAD_PIN		(1<<0)
#define DAC_CS1_PIN			(1<<2)
#define DAC_CS2_PIN			(1<<1)

#else

#define MIDI_LED_PORT		PORTB
#define MIDI_LED_DDR		DDRB
#define MIDI_LED_PIN		(1<<1)

#define MIDI_PORT			PORTD
#define MIDI_DDR			DDRD
#define MIDI_RX_PIN			(1<<0)

#define GATE1_PORT			PORTD
#define GATE1_DDR			DDRD
#define GATE1_PIN			(1<<2)

#define GATE2_PORT			PORTD
#define GATE2_DDR			DDRD
#define GATE2_PIN			(1<<6)

#define CLK_OUT_PORT		PORTC
#define CLK_OUT_DDR			DDRC
#define CLK_OUT_PIN			(1<<1)

#define BUTTON_PORT			PORTD
#define BUTTON_DDR			DDRD
#define BUTTON_PORT_IN		PIND
#define BUTTON_PIN			(1<<4)

#define VOICE_SW_PORT		PORTD
#define VOICE_SW_DDR		DDRD
#define VOICE_SW_PORT_IN	PIND
#define VOICE_SW_PIN		(1<<5)

// --- DAC ---
#define DAC_PORT			PORTC
#define DAC_DDR				DDRC
#define DAC_SDA_PIN			(1<<4)
#define DAC_SCL_PIN			(1<<5)
#define DAC_LOAD_PIN		(1<<3)

#endif




#define MIDI_LED_HIGH()		MIDI_LED_PORT |= MIDI_LED_PIN
#define MIDI_LED_LOW()		MIDI_LED_PORT &= ~MIDI_LED_PIN
#define MIDI_LED_TOGGLE()	MIDI_LED_PORT ^= MIDI_LED_PIN

#define GATE1_HIGH()		GATE1_PORT |= GATE1_PIN
#define GATE1_LOW()			GATE1_PORT &= ~GATE1_PIN

#define GATE2_HIGH()		GATE2_PORT |= GATE2_PIN
#define GATE2_LOW()			GATE2_PORT &= ~GATE2_PIN

#define BUTTON_STATE()		((BUTTON_PORT_IN & BUTTON_PIN) == 0)




void UART_Init(void);
void ClockOut_Init(void);
void Switch_Init(void);
void Button_Init(void);
void Led_Init(void);
void DAC_Init(void);
void Tick_Init(void);
void Gate_Init(void);

#endif /* HARDWARE_H_ */