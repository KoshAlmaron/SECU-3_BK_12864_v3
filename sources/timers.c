#include <avr/io.h>			// Номера бит в регистрах.

#include "timers.h"			// Свой заголовок.

// Прототипы функций.
static void timer_0_init();
static void timer_1_init();
static void timer_2_init();

void timers_init() {
	// Настройка таймера 0 как счетчик времени (1 мс).
	timer_0_init();
	// Настройка таймера 1 на ШИМ.
	timer_1_init();
	// Настройка таймера 2 на ШИМ.
	timer_2_init();	
}

// Настройка таймера 0 канал A как счетчик времени (1 мс) и канал B как ШИМ.
static void timer_0_init() {
	TCCR0A = 0;
	TCCR0B = 0;
	TIMSK0 = 0;
	TCNT0 = 0;
	
	TCCR0B |= (1 << CS01) | (1 << CS00);	// Делитель x64.
	TCCR0A |= (1 << WGM01) | (1 << WGM00);	// Fast PWM (~980гц (250 000 / 255).

	OCR0A = 249;							// Регистр сравнения счетчика времени.
	TIMSK0|= (1 << OCIE0A);					// Прерывание по достижению OCR0A.

	OCR0B = 0;
	//TCCR0A |= (1 << COM0B1);		// Неинвентированный ШИМ на OC0B.
}

// Настройка таймера 1 на ШИМ.
static void timer_1_init() {
	TCCR1A = 0;
	TCCR1B = 0;
	TIMSK1 = 0;
	TCNT1 = 0;

	OCR1A = 0;
	OCR1B = 0;

	TCCR1B |= (1 << CS10) | (1 << CS11);	// Делитель 64, частота 250 000 Гц.
	TCCR1A |= (1 << WGM10);			// Fast PWM 8-bit (~980гц (250 000 / 255).
	TCCR1B |= (1 << WGM12);

	//TCCR1A |= (1 << COM1A1);		// Неинвентированный ШИМ на OC1A.
	//TCCR1A |= (1 << COM1B1);		// Неинвентированный ШИМ на OC1B.
}

// Настройка таймера 2 на ШИМ.
static void timer_2_init() {
	TCCR2A = 0;
	TCCR2B = 0;
	TIMSK2 = 0;
	TCNT2 = 0;

	OCR2A = 0;
	OCR2B = 0;

	TCCR2B |= (1 << CS22);		// Делитель x64.	
	TCCR2A |= (1 << WGM21) | (1 << WGM20);	// Fast PWM (~980гц (250 000 / 255).

	//TCCR2A |= (1 << COM2A1);		// Неинвентированный ШИМ на OC1A.
	//TCCR2A |= (1 << COM2B1);		// Неинвентированный ШИМ на OC1B.
}