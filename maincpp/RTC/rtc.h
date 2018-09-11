//Obsługa RTC DS1307
//Plik rtc.h

#include "twi.h"
#include <avr/io.h>

#ifndef RTC_H_
#define RTC_H_

//Przydatne makra
#define set(reg, num) ((reg) |= (1 << (num)))
#define clear(reg, num) ((reg) &= ~(1 << (num)))

//Struktura do przechowywania czasu
struct time
{
	volatile uint16_t msec;
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t wday;
	uint8_t day;
	uint8_t month;
	uint8_t year;
};

//Ta sama struktura nadaje się do czasy w formacie BCD
typedef time BCDTime;

//Zapisz/odczytaj w formacie BCD
void 	rtcSetBCDTime(BCDTime data);	//Zapisz czas
BCDTime rtcGetBCDTime();				//Odczytaj czas

//Zapisz/odczytaj w formacie dziesiętnym
void rtcSetTime(time data);	//Zapisz czas
time rtcGetTime();			//Odczytaj czas

//Sprawdź czy nie doszło do resetu
bool checkReset();

//Konwersja pomiędzy formatami czasu
uint8_t binToBCD(uint8_t data);
BCDTime binToBCD(time data);
uint8_t BCDToBin(uint8_t data);
time BCDToBin(BCDTime data);

//Inicjalizacja komunikacji z DS1307
void rtcInit();

//Okrojone funkje do dodawania i odejmowania czasu
void rtcDiff(time* min, time* sub);		//Odejmowanie
void rtcSum(time* first, time* second);	//Dodawanie

#endif /* RTC_H_ */
