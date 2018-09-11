//Obsługa RTC DS1307
//Plik rtc.cpp

#include "RTC/rtc.h"

//--------------------------------------------------
//Zapisz czas do DS1307
void rtcSetBCDTime(BCDTime data)
{
	twiStart();		//początek
	twiAddress(208);//adres DSa
	twiWrite(0);	//adres zapisu

	twiWrite(data.sec);	//zapis
	twiWrite(data.min);
	twiWrite(data.hour);
	twiWrite(data.wday);
	twiWrite(data.day);
	twiWrite(data.month);
	twiWrite(data.year);

	twiStop();	//zakończenie

}

//--------------------------------------------------
//Odczytaj czas z DS1307
BCDTime rtcGetBCDTime()
{
	BCDTime data;

	twiStart();		//początek
	twiAddress(208);//adres DSa
	twiWrite(0);	//adres odczytu

	twiStart();		//początek odczytu
	twiAddress(209);//adres odczytu z DSa

	data.sec = twiReadACK();	//odczyt
	data.min = twiReadACK();
	data.hour = twiReadACK();
	data.wday = twiReadACK();
	data.day = twiReadACK();
	data.month = twiReadACK();
	data.year = twiReadNACK();

	twiStop();	//zakończenie
	return data;

}

//--------------------------------------------------
//Zapisz czas
void rtcSetTime(time data)
{
	rtcSetBCDTime(binToBCD(data));
}

//--------------------------------------------------
//Odczytaj czas
time rtcGetTime()
{
	return BCDToBin(rtcGetBCDTime());
}

//--------------------------------------------------
//Sprawdź czy nie doszło do resetu
bool checkReset()
{
	twiStart();		//początek
	twiAddress(208);//adres DSa
	twiWrite(0);	//adres odczytu

	twiStart();		//początek odczytu
	twiAddress(209);//adres odczytu z DSa

	uint8_t data = twiReadNACK();	//odczyt

	twiStop();	//zakończenie

	if(data == 0x80)
		return true;
	else
		return false;
}

//--------------------------------------------------
//Konwersja
uint8_t binToBCD(uint8_t data)
{
	uint8_t result;

	result = (data / 10) << 4;
	result += (data % 10);

	return result;
}

BCDTime binToBCD(time data)
{
	BCDTime result;

	result.sec = binToBCD(data.sec);
	result.min = binToBCD(data.min);
	result.hour = binToBCD(data.hour);
	result.wday = binToBCD(data.wday);
	result.day = binToBCD(data.day);
	result.month = binToBCD(data.month);
	result.year = binToBCD(data.year);

	return result;
}

uint8_t BCDToBin(uint8_t data)
{
	uint8_t result;

	result = data & 0x0f;
	result += ((data >> 4) /*& 0x03*/) * 10;

	return result;
}

time BCDToBin(BCDTime data)
{
	time result;

	result.sec = BCDToBin(data.sec);
	result.min = BCDToBin(data.min);
	result.hour = BCDToBin(data.hour);
	result.wday = BCDToBin(data.wday);
	result.day = BCDToBin(data.day);
	result.month = BCDToBin(data.month);
	result.year = BCDToBin(data.year);

	return result;
}

//--------------------------------------------------
//Inicjalizacja komunikacji z DS1307
void rtcInit()
{
	//Pull-up na magistali
	set(PORTC, PC5);
	set(PORTC, PC4);

	//Prędkość transmisji
	twiInit(400000);

	//Wstępna konfiguracja DS1307
	bool reset = checkReset();
	if(reset == true)
	{
		time czas;
		czas = rtcGetTime();
		rtcSetTime(czas);
	}

	twiStart();
	twiAddress(208);
	twiWrite(7);
	twiWrite(0b00010000);	//generator 1Hz
	twiStop();

	//Konfiguracja przerwania od czasu
	set(PORTD, PD2);	//Pull-up na INT0
	MCUCR |= (1 << ISC00) | (1 << ISC01);	//Przerwanie przy rosnącym zboczu
	GICR |= (1 << INT0);	//Włącz przerwanie
}

//--------------------------------------------------
//Okrojone funkje do dodawania i odejmowania czasu

//Odejmowanie
void rtcDiff(time* min, time* sub)
{
	//Zmienne przechowujące przeniesienia
	uint8_t carry_sec = 0,
			carry_min = 0,
			carry_hour = 0;

	//Kolejne odejmowania z przeniesieniem

	//Milisekundy
	if(min->msec < sub->msec)
	{
		carry_sec++;
		min->msec += 1000;
	}
	min->msec -= sub->msec;

	//Sekundy
	if(min->sec < (sub->sec + carry_sec))
	{
		carry_min++;
		min->sec += 60;
	}
	min->sec -= (sub->sec + carry_sec);

	//Minuty
	if(min->min < (sub->min + carry_min))
	{
		carry_hour++;
		min->min += 60;
	}
	min->min -= (sub->min + carry_min);

	//Godziny
	if(min->hour < (sub->hour + carry_hour))
	{
		min->hour += 24;
	}
	min->hour -= (sub->hour + carry_hour);

	//TODO można ew. rozszerzyć odejmowanie na pozostałe pola
	//Nie jest to konieczne do poprawnego działania
}

//Dodawanie
void rtcSum(time* element_a, time* element_b)
{
	//Zmienne przechowujące przeniesienia
	uint8_t carry_sec = 0,
			carry_min = 0,
			carry_hour = 0;

	//Kolejne dodawania z przeniesieniem

	//Milisekundy
	if((element_a->msec + element_b->msec) > 999)
	{
		carry_sec++;
		element_a->msec = (element_a->msec + element_b->msec) - 1000;
	}
	else
		element_a->msec += element_b->msec;

	//Sekundy
	if((element_a->sec + element_b->sec + carry_sec) > 59)
	{
		carry_min++;
		element_a->sec = (element_a->sec + element_b->sec + carry_sec) - 60;
	}
	else
		element_a->sec += element_b->sec + carry_sec;

	//Minuty
	if((element_a->min + element_b->min + carry_min) > 59)
	{
		carry_hour++;
		element_a->min = (element_a->min + element_b->min + carry_min) - 60;
	}
	else
		element_a->min += element_b->min + carry_min;

	//Godziny
	if((element_a->hour + element_b->hour + carry_hour) > 23)
		element_a->hour = (element_a->hour + element_b->hour + carry_hour) - 24;

	else
		element_a->hour += element_b->hour + carry_hour;

	//TODO można ew. rozszerzyć dodawnie na pozostałe pola
	//Nie jest to konieczne do poprawnego działania
}
