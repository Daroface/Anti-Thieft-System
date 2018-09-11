//Obsługa TWI
//Plik 		twi.h

#ifndef TWI_H_
#define TWI_H_

#include <avr/io.h>

//Inicjalizacja z ustawieniem prędkości transmisji
void twiInit(uint32_t speed);

//Wyślij start/stop
void twiStart();
void twiStop();

//Wyślij adres/dane
void twiAddress(uint8_t data);
void twiWrite(uint8_t data);

//Odczytaj dane z lub bez NACK
uint8_t twiReadACK();
uint8_t twiReadNACK();

#endif
