// Подключение бибилиотек
#include <stdint.h>				// Коротние название int.
#include <avr/io.h>				// Названия регистров и номера бит.
#include <avr/interrupt.h>		// Прерывания.
#include <avr/wdt.h>			// Сторожевой собак.

#include "timers.h"				// Таймеры.
#include "macros.h"				// Макросы.
#include "pinout.h"				// Начначение выводов контроллера.

#include "i2c.h"				// I2C (TWI).
#include "oled.h"				// OLED дисплей.
#include "display.h"			// Экраны БК.

#include "uart.h"				// UART.
#include "bkdata.h"				// Структура с данными.
#include "configuration.h"		// Настройки.
#include "dadm.h"				// Датчик аварийного давления масла.
#include "eeprom.h"				// Чтение и запись EEPROM.
#include "functions.h"			// Вспомогательные функции.
#include "buttons.h"			// Кнопки.

// Основной счетчик времени,
// увеличивается по прерыванию на единицу каждую 1мс.
volatile uint8_t MainTimer = 0;

// Счетчики времени.
uint16_t LCDTimer = 0;
int16_t AlarmBoxTimer = 0;
uint16_t ButtonsTimer = 0;
uint16_t RideTimerMs = 0;
uint8_t FuelTimer = 0;

// Прототипы локальных функций.
static void setup();
static void power_on();
static void loop();
static void timers_loop();

#ifdef DEBUG_MODE
	static void debug_mode();
#endif

int main() {
	setup();
	while(1) {
		loop();
	}
	return 0;
}

// Инициализация при включении 
static void setup() {
	wdt_enable(WDTO_500MS);	// Сторожевой собак на 500 мс.
	cli();
		timers_init();		// Настройка таймеров.
		i2c_init();			// Настройка I2C.
		uart_init();
	sei();

	oled_init(0x3c, 60, 0);			// Настройка OLED I2C.
	
	SET_PIN_MODE_OUTPUT(LED_PIN);	// Вывод со светодиодом.
	SET_PIN_LOW(LED_PIN);

	SET_PIN_MODE_INPUT(INT_IGN_PIN);	// Проверка ЗЗ.
	SET_PIN_LOW(INT_IGN_PIN);

	#ifdef INT_OIL_PIN			// ДАДМ
		SET_PIN_MODE_INPUT(INT_OIL_PIN);
		SET_PIN_LOW(INT_OIL_PIN);
		test_oil_pressure(0);
	#endif

	buttons_init();		// Настройка входов для кнопок.

	#ifdef DEBUG_MODE
		debug_mode();
	#endif
	
	//update_eeprom(1);
	// Считываем данные из EEPROM.
	read_eeprom();

	power_on();
}

static void power_on() {
	while (DataStatus != 2) {
		timers_loop();
		if (LCDTimer >= 2000) {
			LCDTimer = 1000;
			draw_lcd_no_signal();
		}
	}
	// Отображение экрана приветствия
	draw_init();
}

// Основной цикл
static void loop() {
	timers_loop();

	#ifdef DEBUG_MODE
		debug_mode();
	#endif

	if (BK.ScreenChange) {
		if (LCDTimer >= 10 && oled_ready()) {
			LCDTimer = 0;
			draw_screen_change();
			buttons_clear();
		}
		return;
	}
	
	if (DataStatus == 2) {
		check_ce_errors();	// Проверка ошибок CE.

		if (LCDTimer >= 100) {
			LCDTimer = 0;
			button_action();

			switch (BK.ScreenMode) {
				case 0:		// Основной экран.
					draw_lcd_main();
					break;
				case 1:		// Второй экран.
					//draw_lcd_second();
					draw_finish();
					break;
				case 2:		// Экран замера разгона.
					draw_lcd_acceleration();
					break;
				case 3:		// Ошибки CE.
					draw_lcd_ce_errors();
					break;
			}
			buttons_clear(); // Сброс необработанных состояний.
			PIN_TOGGLE(LED_PIN);
		}
		DataStatus = 0;
	}
	if (LCDTimer >= 3000) {
		LCDTimer = 1000;
		draw_lcd_no_signal();
	}
}

static void timers_loop() {
	wdt_reset();		// Сброс сторожевого таймера.

	// Временно выключаем прерывания, чтобы забрать значение счетчика.
	uint8_t TimerAdd = 0;
	cli();
		TimerAdd = MainTimer;
		MainTimer = 0;
	sei();

	#ifdef INT_OIL_PIN
		test_oil_pressure(1);		// Проверка ДАДМ.
	#endif

	// Счетчики времени.
	if (TimerAdd) {
		LCDTimer += TimerAdd;
		ButtonsTimer += TimerAdd;
		RideTimerMs += TimerAdd;
		FuelTimer += TimerAdd;

		if (RideTimerMs >= 1000) {
			BK.RideTimer++;
			RideTimerMs -= 1000;
		}

		AlarmBoxTimer += TimerAdd;
		if (AlarmBoxTimer > 800) {AlarmBoxTimer = -800;}
	}

	if (ButtonsTimer >= 25) {		// Обработка кнопок.
		ButtonsTimer = 0;
		buttons_update();
	}
	if (FuelTimer >= 100) {			// Подсчет израсходованного топлива.
		build_fuel(FuelTimer);
		FuelTimer = 0;
	}
}

#ifdef DEBUG_MODE
	static void debug_mode() {
		SET_PIN_HIGH(INT_IGN_PIN);

		DataStatus = 2;

		//BK.ScreenMode = 3;

		BK.CountCE[0] = 1;	// Ошибки CE.
		BK.CountCE[2] = 2;
		BK.CountCE[5] = 3;
		BK.CountCE[9] = 4;
		BK.CountCE[11] = 5;
		BK.CountCE[12] = 6;
		BK.CountCE[16] = 7;
		BK.CountCE[20] = 8;

		//return;
		EcuData[1] = 0x09;		// RPM x1.
		EcuData[2] = 0x80;

		EcuData[3] = 0x07;		// ДАД x64.
		EcuData[4] = 0x80;

		EcuData[5] = 0x13;		// Напряжение сети x400.
		EcuData[6] = 0x10;

		EcuData[7] = 0x01;		// Температура ОЖ x4.
		EcuData[8] = 0x70;

		EcuData[9] = 0x02;		// УОЗ x32.
		EcuData[10] = 0xcc;

		EcuData[13] = 0x00;		// УОЗ ДД x32.
		EcuData[14] = 0x00;

		EcuData[18] = 0xac;		// TPS x2.

		EcuData[19] = 0x01;		// Напряжение x400.
		EcuData[20] = 0x5c;

		EcuData[23] = 0x00;		// Ошибки CE.
		EcuData[24] = 0x00;
		EcuData[25] = 0x00;
		EcuData[26] = 0x00;

		EcuData[27] = 0xc7;		// IAC x2.

		EcuData[29] = 0x0B;		// Скорость x32.
		EcuData[30] = 0xE0;

		EcuData[34] = 0x3e;		// Расход топлива гц x256.
		EcuData[35] = 0xe6;

		EcuData[36] = 0x00;		// Температура воздуха x4.
		EcuData[37] = 0x85; //^ 0xff;

		EcuData[52] = 0xff;		// Лямбда коррекция x512.
		EcuData[53] = 0x0b ^ 0xff;

		EcuData[62] = 0x17;		// AFR x128.
		EcuData[63] = 0x5a;

		EcuData[81] = 0x5e;		// Inj Duty x2.

		EcuData[84] = 0xb6;		// Fan PWM x2.

		BK.DistDay = 151000;
		BK.DistAll = 23800000;

		BK.FuelDay = 14400;
		BK.FuelAll = 2104000;
	}
#endif

// Прерывание при совпадении регистра сравнения OCR0A на таймере 0 каждую 1мс. 
ISR (TIMER0_COMPA_vect) {
	TCNT0 = 0;		// Ручной сброс счетчика.
	MainTimer++;
}

/*
ScreenMode
	Номер активного экрана:
		0 - Основной,
		1 - Вспомогательный,
		2 - Разгон,
		3 - Ошибки CE.
*/