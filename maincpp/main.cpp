/*
 * main.c
 *
 * Created: 03.01.2018 22:19:16
 * Author : Dominik Dziemba³a
 */ 
#include <avr/io.h>
#define F_CPU 1000000UL
#include <stdio.h>
#include <util/delay.h> 
#include <avr/interrupt.h>
#include "RTC/rtc.h"
#include "RTC/twi.h"
#include <string.h>
#include  <stdint.h>
#define BAUD 9600
#include <util/setbaud.h>

#define LEDON PORTD |= (1<<PD4);

#define LEDOFF PORTD &= ~(1<<PD4);

#define SPEAKERON PORTB |= (1<<PB1);

#define SPEAKEROFF PORTB &= ~(1<<PB1);


time rtcTime;

int left = 0;
int right = 0;
int leftTSOPOn = 1;
int rightTSOPOn = 1;
time leftSensor[2];
time rightSensor[2];

int length = 1000;
int space = 500;

int amount = 0;
char messageFromPC[70];   
    
volatile unsigned char interruptFlag =0;

char noAlarmsMessage[15] = "no alarms\r\n\0";
char leftParameterMessage[10] = "L OK\r\n\0";
char rightParameterMessage[10] = "R OK\r\n\0";         
char helpMessage[200] = "ca - clear all alarms\r\ncl | cr - clear left/right alarm\r\nsl | sr xx:xx yy:yy - set left/right alarm\r\ntime - show time\r\nst ss:mm:hh.wd.dd.mm.yy - set time and data\r\nsa - show alarms\r\n\0";

volatile unsigned int usart_bufor_ind;
char usart_bufor[200];

/*Funkcja inicjuj¹ca porty i sygnalizuj¹ca dzia³anie uk³adu.*/
void portsInit()
{
	DDRD = 0b00010010;
	DDRB = 0b00000010;
	LEDON;
	_delay_ms(1500);
	LEDOFF;
	_delay_ms(1500);
	LEDON;
	_delay_ms(1500);
	LEDOFF;
	_delay_ms(1500);
	LEDON;
	_delay_ms(1500);
	LEDOFF;
	_delay_ms(1500);
}

/*Funkcja nadaj¹ca dŸwiêk przez g³oœnik.*/
void playSound()
{
	LEDON;
	while(1)
	{
		if(length>0)
		{
			SPEAKERON;
		}
		else
		{
			SPEAKEROFF;
		}
		length = length-space;
		if(length==-1000)
		{
			length = 1000;
		}
		if(!(PINB & (1<<PB6)) && !(PIND & (1<<PD5)) && !(PIND & (1<<PD7)))
		{
			LEDOFF
			SPEAKEROFF
			break;
		}
	}
	
}

/*Funkcja inicjuj¹ca komunikacje z komputerem.*/
void usartInit(void)
{
	 //linkowanie tego pliku musi byæ	//po zdefiniowaniu BAUD	

	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;
	#if USE_2X
	UCSRA |=  (1<<U2X);
	#else
	UCSRA &= ~(1<<U2X);
	#endif

	UCSRC = (1<<URSEL) | (1<<UCSZ1) | (1<<UCSZ0); 
	UCSRB = (1<<TXEN) | (1<<RXEN) | (1<<RXCIE);
}

/*Odbieranie danych z komputera.*/
ISR(USART_RXC_vect)
{		
	char sign = UDR;	
	if(sign==10)
	{
		interruptFlag = 1;
	}
	else
	{
		messageFromPC[amount] = sign;
		amount++;			
	}	
}

/*Wysy³anie odpowiedzi do komputera.*/
ISR(USART_UDRE_vect)
{
	if(usart_bufor[usart_bufor_ind]!= 0)
	{
		UDR = usart_bufor[usart_bufor_ind++];
	}
	else
	{
		UCSRB &= ~(1<<UDRIE);
	}
}

/*£apenie niew³aœciwych przerwañ.*/
ISR(__vector_default)
{

}

/*Funkcja wyœwietlaj¹ca godziny alarmów.*/
void printAlarms()
{
	int i=0;
	if(left==1)
	{
		usart_bufor[i] = 'L';
		usart_bufor[i+1] = ' ';
		usart_bufor[i+2] = ((int)leftSensor[0].hour/10) + 48;
		usart_bufor[i+3] = ((int)leftSensor[0].hour%10) + 48;
		usart_bufor[i+4] = ':';
		usart_bufor[i+5] = ((int)leftSensor[0].min/10) + 48;
		usart_bufor[i+6] = ((int)leftSensor[0].min%10) + 48;
		usart_bufor[i+7] = ' ';
		usart_bufor[i+8] = ((int)leftSensor[1].hour/10) + 48;
		usart_bufor[i+9] = ((int)leftSensor[1].hour%10) + 48;
		usart_bufor[i+10] = ':';
		usart_bufor[i+11] = ((int)leftSensor[1].min/10) + 48;
		usart_bufor[i+12] = ((int)leftSensor[1].min%10) + 48;
		if(right==1)
		{
			usart_bufor[i+13] = '\r';
			usart_bufor[i+14] = '\n';
			i = i + 15;
		}
		else
		{
			i = i +13;
		}
		
	}
	if(right==1)
	{
		usart_bufor[i] = 'R';
		usart_bufor[i+1] = ' ';
		usart_bufor[i+2] = ((int)rightSensor[0].hour/10) + 48;
		usart_bufor[i+3] = ((int)rightSensor[0].hour%10) + 48;
		usart_bufor[i+4] = ':';
		usart_bufor[i+5] = ((int)rightSensor[0].min/10) + 48;
		usart_bufor[i+6] = ((int)rightSensor[0].min%10) + 48;
		usart_bufor[i+7] = ' ';
		usart_bufor[i+8] = ((int)rightSensor[1].hour/10) + 48;
		usart_bufor[i+9] = ((int)rightSensor[1].hour%10) + 48;
		usart_bufor[i+10] = ':';
		usart_bufor[i+11] = ((int)rightSensor[1].min/10) + 48;
		usart_bufor[i+12] = ((int)rightSensor[1].min%10) + 48;
		i = i + 13;
	}
	if(left == 0 && right == 0)
	{
		strcpy(usart_bufor, noAlarmsMessage);
	}
	else
	{
		usart_bufor[i] = 13;
		usart_bufor[i+1] = 10;
		usart_bufor[i+2] = 0;
	}
	while (!(UCSRA & (1<<UDRE)));
	usart_bufor_ind = 0;
	UCSRB |= (1<<UDRIE);
}

/*Funkcja wypisuj¹ca na komputerze wiadomoœæ z mo¿liwymi instrukcjami.*/
void printHelpMessage()
{
	strcpy(usart_bufor, helpMessage);
	while (!(UCSRA & (1<<UDRE)));
	usart_bufor_ind = 0;
	UCSRB |= (1<<UDRIE);	
}

/*Funkcja wypisuj¹ca na komputerze wiadomoœæ o pomyœlnym ustawieniu czasu pracy lewego odbiornika.*/
void printLeftTSOPParameterMessage()
{
	strcpy(usart_bufor, leftParameterMessage);
	while (!(UCSRA & (1<<UDRE)));
	usart_bufor_ind = 0;
	UCSRB |= (1<<UDRIE);
}

/*Funkcja wypisuj¹ca na komputerze wiadomoœæ o pomyœlnym ustawieniu czasu pracy prawego odbiornika.*/
void printRightTSOPParameterMessage()
{
	strcpy(usart_bufor, rightParameterMessage);
	while (!(UCSRA & (1<<UDRE)));
	usart_bufor_ind = 0;
	UCSRB |= (1<<UDRIE);
}

/*Funkcja wypisuj¹ca godzinê na komputerze.*/
void printTime()
{
	rtcTime = rtcGetTime();
	usart_bufor[0] = ((int)rtcTime.hour/10) + 48;
	usart_bufor[1] = ((int)rtcTime.hour%10) + 48;
	usart_bufor[2] = ':';
	usart_bufor[3] = ((int)rtcTime.min/10) + 48;
	usart_bufor[4] = ((int)rtcTime.min%10) + 48;
	usart_bufor[5] = 13;
	usart_bufor[6] = 10;
	usart_bufor[7] = 0;
	while (!(UCSRA & (1<<UDRE)));
	usart_bufor_ind = 0;
	UCSRB |= (1<<UDRIE);
}

/*Funkcja czyszcz¹ca zawartoœæ wiadomoœci odebranej z komputera.*/
void clearString(char* array, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		array[i] = 0;
	}

}

/*Funkcja zapisuj¹ca godziny dzia³ania lewego odbiornika TSOP odebrane z komputera.*/
void checkLeftTSOPParameter(int &i)
{
	if(messageFromPC[i]=='s' && messageFromPC[i+1]=='l')
	{
		int h = (messageFromPC[i+3] - 48) * 10 + (messageFromPC[i+4] - 48);
		int m = (messageFromPC[i+6] - 48) * 10 + (messageFromPC[i+7] - 48);
		int h2 = (messageFromPC[i+9] - 48)*10 + (messageFromPC[i+10] - 48);
		int m2 = (messageFromPC[i+12] - 48) * 10 + (messageFromPC[i+13] - 48);
		if((h>=0 && h<=23) && (m>=0 && m<=59) && (h2>=0 && h2<=23) && (m2>=0 && m2<=59))
		{
			leftSensor[0].hour = (uint8_t)h;
			leftSensor[0].min = (uint8_t)m;
			leftSensor[1].hour = (uint8_t)h2;
			leftSensor[1].min = (uint8_t)m2;
			i = i + 13;
			left = 1;
			printLeftTSOPParameterMessage();
		}		
	}
}

/*Funkcja zapisuj¹ca godziny dzia³ania prawego odbiornika TSOP odebrane z komputera.*/
void checkRightTSOPParameter(int &i)
{
	if(messageFromPC[i]=='s' && messageFromPC[i+1]=='r')
	{
		int h = (messageFromPC[i+3] - 48) * 10 + (messageFromPC[i+4] - 48);
		int m = (messageFromPC[i+6] - 48) * 10 + (messageFromPC[i+7] - 48);
		int h2 = (messageFromPC[i+9] - 48)*10 + (messageFromPC[i+10] - 48);
		int m2 = (messageFromPC[i+12] - 48) * 10 + (messageFromPC[i+13] - 48);
		if((h>=0 && h<=23) && (m>=0 && m<=59) && (h2>=0 && h2<=23) && (m2>=0 && m2<=59))
		{
			rightSensor[0].hour = (uint8_t)h;
			rightSensor[0].min = (uint8_t)m;
			rightSensor[1].hour = (uint8_t)h2;
			rightSensor[1].min = (uint8_t)m2;
			i = i + 13;
			right = 1;
			printRightTSOPParameterMessage();
		}
	}
}

/*Funkcja kasuj¹ca godziny dzia³ania obu odbiorników TSOP.*/
void checkClearAllParameter(int &i)
{
	if(messageFromPC[i]=='c' && messageFromPC[i+1]=='a')
	{
		left = 0;
		leftTSOPOn = 1;
		right = 0;
		rightTSOPOn = 1;
		i = i + 1;
		printAlarms();
	}
}

/*Funkcja kasuj¹ca godziny dzia³ania lewego odbiornika TSOP.*/
void checkClearLeftParameter(int &i)
{
	if(messageFromPC[i]=='c' && messageFromPC[i+1]=='l')
	{
		left = 0;
		leftTSOPOn = 1;
		i = i + 1;
		printAlarms();
	}
}

/*Funkcja kasuj¹ca godziny dzia³ania prawego odbiornika TSOP.*/
void checkClearRightParameter(int &i)
{
	if(messageFromPC[i]=='c' && messageFromPC[i+1]=='r')
	{
		right = 0;
		rightTSOPOn = 1;
		i = i + 1;
		printAlarms();
	}
}

/*Funkcja sprawdzaj¹ca parametr odpowiadaj¹cy za wyœwietlenie czasu.*/
void checkTimeParameter(int &i)
{
	if(messageFromPC[i]=='t' && messageFromPC[i+1]=='i' && messageFromPC[i+2]=='m' && messageFromPC[i+3]=='e')
	{
		i = i + 3;
		printTime();
	}
}

/*Funkcja ustawiaj¹ca czas i datê.*/
void checkSetTimeParameter(int &i)
{
	if(messageFromPC[i]=='s' && messageFromPC[i+1]=='t')
	{
		rtcTime.sec = (uint8_t)((messageFromPC[i+3] - 48) * 10 + (messageFromPC[i+4] - 48));
		rtcTime.min = (uint8_t)((messageFromPC[i+6] - 48) * 10 + (messageFromPC[i+7] - 48));
		rtcTime.hour = (uint8_t)((messageFromPC[i+9] - 48) * 10 + (messageFromPC[i+10] - 48));
		rtcTime.wday = (uint8_t)((messageFromPC[i+12] - 48) * 10 + (messageFromPC[i+13] - 48));
		rtcTime.day = (uint8_t)((messageFromPC[i+15] - 48) * 10 + (messageFromPC[i+16] - 48));
		rtcTime.month = (uint8_t)((messageFromPC[i+18] - 48) * 10 + (messageFromPC[i+19] - 48));
		rtcTime.year = (uint8_t)((messageFromPC[i+21] - 48) * 10 + (messageFromPC[i+22] - 48));
		if((rtcTime.sec>=0 && rtcTime.sec<=59) && (rtcTime.min>=0 && rtcTime.min<=59) && (rtcTime.hour>=0 && rtcTime.hour<=23) && (rtcTime.wday>=1 && rtcTime.wday<=7) && (rtcTime.day>=1 && rtcTime.day<=31) && (rtcTime.month>=1 && rtcTime.month<=12) && (rtcTime.year>=0 && rtcTime.year<=200))
		{
			rtcSetTime(rtcTime);
			i = i + 22;		
			printTime();
		}
	}
}

/*Funkcja sprawdzaj¹ca czy u¿ytkownik wywo³a³ polecenie help*/
void checkHelpParameter(int &i)
{
	if(messageFromPC[i]=='h' && messageFromPC[i+1]=='e' && messageFromPC[i+2]=='l' && messageFromPC[i+3]=='p')
	{
		i = i + 3;
		printHelpMessage();
	}
}

/*Funkcja sprawdzaj¹ca czy u¿ytkownik wywo³a³ polecenie do wyœwietlania godzin alarmów.*/
void checkAlarmsParameter(int &i)
{
	if(messageFromPC[i]=='s' && messageFromPC[i+1]=='a')
	{
		i = i + 1;
		printAlarms();
	}
}


/*Funkcja sprawdzaj¹ca parametry otrzymane z komputera.*/
void checkParameters()
{
	int i;
	for(i=0; i<amount; i++)
	{
		if(messageFromPC[i]!=0)
		{
			checkHelpParameter(i);
			checkLeftTSOPParameter(i);
			checkRightTSOPParameter(i);
			checkClearAllParameter(i);
			checkClearLeftParameter(i);
			checkClearRightParameter(i);
			checkTimeParameter(i);
			checkSetTimeParameter(i);
			checkAlarmsParameter(i);
		}
		else
		{
			break;
		}
	}
}

/*Funkcja sprawdzaj¹ca czas dzia³anie odbiorników TSOP.*/
void checkTSOPTime(time sensorOnTime, time sensorOffTime, int& switchKey)
{
	rtcTime = rtcGetTime();
	if((sensorOnTime.hour<sensorOffTime.hour) || ((sensorOnTime.hour==sensorOffTime.hour) && (sensorOnTime.min<sensorOffTime.min)))
	{
		if((sensorOnTime.hour<=rtcTime.hour) && (rtcTime.hour<sensorOffTime.hour))
		{
			if(sensorOnTime.hour<rtcTime.hour)
			{
				switchKey = 0;
			}
			else if((sensorOnTime.hour==rtcTime.hour) && (sensorOnTime.min<=rtcTime.min))
			{
				switchKey = 0;
			}
			else if((sensorOnTime.hour==rtcTime.hour) && (sensorOnTime.min>rtcTime.min))
			{
				switchKey = 1;
			}
		}
		else if(sensorOffTime.hour<=rtcTime.hour)
		{
			if((rtcTime.hour==sensorOffTime.hour) && (rtcTime.min<sensorOffTime.min))
			{
				switchKey = 0;
			}
			else
			{
				switchKey = 1;
			}
		}
		else if(rtcTime.hour<sensorOnTime.hour)
		{
			switchKey = 1;
		}
	}
	else if((sensorOnTime.hour==leftSensor[1].hour) && (sensorOnTime.min==sensorOffTime.min))
	{
		switchKey = 0;
	}
	else if((sensorOnTime.hour>sensorOffTime.hour) || ((sensorOnTime.hour==sensorOffTime.hour) && (sensorOnTime.min>sensorOffTime.min)))
	{
		if((sensorOffTime.hour<=rtcTime.hour) && (rtcTime.hour<sensorOnTime.hour))
		{
			if(sensorOffTime.hour<rtcTime.hour)
			{
				switchKey = 1;
			}
			else if((sensorOffTime.hour==rtcTime.hour) && (sensorOffTime.min<=rtcTime.min))
			{
				switchKey = 1;
			}
			else if((sensorOffTime.hour==rtcTime.hour) && (sensorOffTime.min>rtcTime.min))
			{
				switchKey = 0;
			}
		}
		else if(sensorOnTime.hour<=rtcTime.hour)
		{
			if((rtcTime.hour==sensorOnTime.hour) && (rtcTime.min<sensorOnTime.min))
			{
				switchKey = 1;
			}
			else
			{
				switchKey = 0;
			}
		}
		else if(rtcTime.hour<sensorOffTime.hour)
		{
			switchKey = 0;
		}
	}
	
}

/*Funkcja wykrywaj¹ca zak³ócenie podczerwieni z podanymi godzinami dzia³ania prawego odbiornika TSOP.*/
void checkRightTSOP()
{
	checkTSOPTime(rightSensor[0], rightSensor[1], rightTSOPOn);
	if((rightTSOPOn && ((PIND & (1<<PD3)))) || ((PIND & (1<<PD2))))
	{
		playSound();
	}
}

/*Funkcja wykrywaj¹ca zak³ócenie podczerwieni z podanymi godzinami dzia³ania lewego odbiornika TSOP.*/
void checkLeftTSOP()
{
	checkTSOPTime(leftSensor[0], leftSensor[1], leftTSOPOn);
	if((leftTSOPOn && ((PIND & (1<<PD2)))) || ((PIND & (1<<PD3))))
	{
		playSound();
	}
}

/*Funkcja wykrywaj¹ca zak³ócenie podczerwieni z podanymi godzinami dzia³ania obu odbiorników TSOP.*/
void checkBothTSOPs()
{
	checkTSOPTime(rightSensor[0], rightSensor[1], rightTSOPOn);
	checkTSOPTime(leftSensor[0], leftSensor[1], leftTSOPOn);
	if((leftTSOPOn && ((PIND & (1<<PD2)))) || (rightTSOPOn && ((PIND & (1<<PD3)))))
	{
		playSound();
	}
}

/*Funkcja wykrywaj¹ca zak³ócenie podczerwieni.*/
void checkNeitherTSOPs()
{
	if ( ((PIND & (1<<PD2))) || ((PIND & (1<<PD3))))
	{
		playSound();
	}
}

/*Funkcja sprawdzaj¹ca czy do odbiorników TSOP s¹ przypisane godziny dzia³ania.*/
void checkTSOPS()
{
	if(left == 0 && right == 0)
	{
		checkNeitherTSOPs();
	}
	else if(left == 1 && right == 0)
	{
		checkLeftTSOP();
	}
	else if(left == 0 && right == 1)
	{
		checkRightTSOP();
	}
	else if(left == 1 && right == 1)
	{
		checkBothTSOPs();
	}

}

int main(void)
{
	portsInit();
	rtcInit();
	usartInit();
	sei();
    while (1) 
    {
		if(interruptFlag)
		{		
			checkParameters();
			interruptFlag = 0; 
			amount = 0;		
			clearString(messageFromPC, 70);
		}
		checkTSOPS();	
    }
}


