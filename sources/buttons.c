#include <stdint.h>				// Коротние название int.
#include <avr/io.h>				// Названия регистров и номера бит.

#include "buttons.h"			// Свой заголовок.
#include "macros.h"				// Макросы.
#include "pinout.h"				// Начначение выводов контроллера.
#include "bkdata.h"				// Структура с данными.
#include "configuration.h"		// Настройки.

/*
	Состояние кнопок:
		0 - не нажата,
		1..99 - ожидание после нажатия,
		100 - событие обработано,
		101..149 - откат после срабатывания,
		201 - короткое нажатие,
		202 - длинное нажатие.
*/
uint8_t ButtonState[4] = {};	//Ввер/внизх, вправо/влево

// Прототипы функций.
static void button_read(uint8_t State, uint8_t N);

void buttons_init() {
	// Настраиваем выводы для кнопок/энкодера.
	SET_PIN_MODE_INPUT(BUTTON_UP_PIN);
	SET_PIN_HIGH(BUTTON_UP_PIN);
	SET_PIN_MODE_INPUT(BUTTON_DOWN_PIN);
	SET_PIN_HIGH(BUTTON_DOWN_PIN);
	SET_PIN_MODE_INPUT(BUTTON_RIGHT_PIN);
	SET_PIN_HIGH(BUTTON_RIGHT_PIN);
	#ifndef ENCODER_CONTROL
		SET_PIN_MODE_INPUT(BUTTON_LEFT_PIN);
		SET_PIN_HIGH(BUTTON_LEFT_PIN);
	#endif
}

void button_action() {
	// Вправо/влево короткое смена экрана
	if (ButtonState[2] == 201) {
		BK.ScreenChange = 1;
	}
	if (ButtonState[3] == 201) {
		BK.ScreenChange = -1;
	}
}


void buttons_clear() {
	if (PIN_READ(BUTTON_UP_PIN) && ButtonState[0] > 200) {ButtonState[0] = 100;}
	if (PIN_READ(BUTTON_DOWN_PIN) && ButtonState[1] > 200) {ButtonState[1] = 100;}
	if (PIN_READ(BUTTON_RIGHT_PIN) && ButtonState[2] > 200) {ButtonState[2] = 100;}
	#ifndef ENCODER_CONTROL
		if (PIN_READ(BUTTON_LEFT_PIN) && ButtonState[3] > 200) {ButtonState[3] = 100;}
	#endif
}

// Вызов каждые 25 мс.
void buttons_update() {
	button_read(0, PIN_READ(BUTTON_UP_PIN));
	button_read(1, PIN_READ(BUTTON_DOWN_PIN));
	button_read(2, PIN_READ(BUTTON_RIGHT_PIN));
	#ifndef ENCODER_CONTROL
		button_read(3, PIN_READ(BUTTON_LEFT_PIN));
	#endif
}

static void button_read(uint8_t N, uint8_t State) {
	if (ButtonState[N] < 100) {
		if (!State) {
			ButtonState[N]++;
			if (ButtonState[N] >= 60) {ButtonState[N] = 202;}	// Длиное нажатие.
		}
		else {
			if (ButtonState[N] >= 2) {ButtonState[N] = 201;}	// Короткое нажатие.
		}
		return;
	}

	if (ButtonState[N] < 150 && State) {
		ButtonState[N]++;
		if (ButtonState[N] >= 108) {
			ButtonState[N] = 0;		// Сброс состояния кнопки.
		}
		return;		
	}
}
