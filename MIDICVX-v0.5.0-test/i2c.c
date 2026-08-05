/*
 * i2c.c
 *
 * Created: 1/30/2017 7:08:51 PM
 *  Author: Janis Zaharans
 */ 

#include "main.h"


void I2C_Init(void)
{
	//set SCL to 400kHz
	TWSR = 0x00;
	TWBR = 0x0C;
	//enable TWI
	TWCR = (1<<TWEN);
}

void I2C_Start(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
}

void I2C_Stop(void)
{
	TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);
}

void I2C_Write(uint8_t data)
{
	TWDR = data;
	TWCR = (1<<TWINT)|(1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
}

uint8_t I2C_ReadAck(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA);
	while ((TWCR & (1<<TWINT)) == 0);
	return TWDR;
}

uint8_t I2C_ReadNack(void)
{
	TWCR = (1<<TWINT)|(1<<TWEN);
	while ((TWCR & (1<<TWINT)) == 0);
	return TWDR;
}

uint8_t I2C_GetStatus(void)
{
	uint8_t status;
	//mask status
	status = TWSR & 0xF8;
	return status;
}


uint8_t I2C_Transmit(uint8_t addr, uint8_t* data, uint8_t length)
{
	uint16_t i;
	
	// Send Start
	I2C_Start();
	if(I2C_GetStatus() != 0x08)
		return 0;
		
	// Send Address
	I2C_Write(addr);
	if(I2C_GetStatus() != 0x18)
		return 0;
		
	// Send data
	for(i=0; i<length; i++) {
		I2C_Write(*data++);
		if(I2C_GetStatus() != 0x28)
			return 0;
	}
	
	// Send Stop
	I2C_Stop();
	
	return 1;
}