//Obsługa TWI
//Plik 		twi.cpp

#include "twi.h"

//Inicjalizacja z ustawieniem prędkości transmisji
void twiInit(uint32_t speed)
{
	//Włącz TWI
	TWCR = (1<<TWEA) | (1<<TWEN);

	//Ustaw prędkość transmisji
	uint8_t prescaler = 0;
	speed = (F_CPU/speed/100-16)/2;

	while(speed > 255)
	{
		prescaler++;
		speed=speed/4;
	}

	TWSR = (TWSR & ((1<<TWPS1) | (1<<TWPS0))) | prescaler;
	TWBR = speed;
}

//Wyślij start
void twiStart()
{
	TWCR = (1<<TWINT)|(1<<TWSTA)| (1<<TWEN);	//wyślij
	while (!(TWCR & (1<<TWINT)));	//czekaj
}

//Wyślij stop
void twiStop()
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);	//wyślij
	while(TWCR & (1<<TWSTO));	//czekaj
}

//Wyślij adres
void twiAddress(uint8_t data)
{
	TWDR = data;	//wyślij
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));	//czekaj
}

//Wyślij dane
void twiWrite(uint8_t data)
{
	TWDR = data;	//wyślij
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));	//czekaj
}

//Odczytaj dane
uint8_t twiReadACK()
{
	TWCR = (1<<TWEA) | (1<<TWINT) | (1<<TWEN);	//odbierz
	while (!(TWCR & (1<<TWINT)));	//czekaj

	return TWDR; //zwróć wynik
}

//Odczytaj i wyg. NACK
uint8_t twiReadNACK()
{
	TWCR = (1<<TWINT) | (1<<TWEN);	//odbierz
	while (!(TWCR & (1<<TWINT)));	//czekaj

	return TWDR;	//zwróć wynik
}
