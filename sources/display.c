// Подключение бибилиотек
#include <stdint.h>				// Коротние название int.
#include <avr/interrupt.h>		// Прерывания
#include <avr/wdt.h>			// Сторожевой собак.

#include "display.h"		// Свой заголовок.
#include "oled.h"			// OLED дисплей.
#include "blocks.h"			// Блоки данных для экрана.
#include "bkdata.h"			// Структура с данными.
#include "macros.h"			// Макросы.
#include "configuration.h"	// Настройки.


extern volatile uint8_t MainTimer; // Основной счетчики времени из main.

// Экран приветствия
void draw_init() {
	#define STEP_DELAY_LCD 20
	int8_t y = 0;
	int8_t LCDTimer = 0;

	oled_clear_buffer();
	while (1) {
		wdt_reset();		// Сброс сторожевого таймера.
		// Временно выключаем прерывания, чтобы забрать значение счетчика.
		uint8_t TimerAdd = 0;
		cli();
			TimerAdd = MainTimer;
			MainTimer = 0;
		sei();

		// Счетчики времени.
		if (TimerAdd) {
			LCDTimer += TimerAdd;
		}

		if (LCDTimer >= STEP_DELAY_LCD && oled_ready()) {
			LCDTimer = 0;
			if (y < 32) {
				oled_set_clip_window(0, 31 - y, 127, 32 + y);
				draw_lcd_main();
				y += 2;
			}
			else {
				oled_disable_clip_window();
				draw_lcd_main();
				break;
			}
		}
	}
}

// Основной экран
void draw_lcd_main() {
	if (!BK.ScreenChange) {oled_clear_buffer();}

	// Линии разметки
	oled_draw_h_line(0, 21, 128);
	oled_draw_h_line(0, 43, 128);
	oled_draw_v_line(42, 0, 64);
	oled_draw_v_line(85, 0, 64);

	// ========================== Блоки данных ==========================
	draw_ff_fc_f(0, 0);			// Мгновенный расход топлива (F)
	draw_water_temp_f(0, 22);	// Температура ОЖ (F)
    draw_distance_f(0, 44);		// Суточный и общий пробег (F)

    draw_map_f(43, 0);			// ДАД (F)
    draw_trottle_f(43, 22);		// ДПДЗ / РХХ (F)
    draw_afc_f(43, 44);			// Средний расход топлива суточный и общий (F)

    draw_O2_sensor_h(86, 0, 0);		// AFR № 1 (Напряжение ADD_I1) (H)
    draw_lambda_corr_h(86, 10, 0);	// Лямбда коррекция № 1 (H)
    draw_airtemp_h(86, 22, 0);		// Температура воздуха на впуске (H)
    draw_battery_h(86, 32);			// Напряжение сети (H)
    draw_fuel_burned_f(86, 44);		// Израсходованное топливо (F)

	// ========================== Блоки данных ==========================

	draw_error_box();	// Индикация ошибок.
	oled_send_buffer();
}

// Второй экран
void draw_lcd_second() {
	if (!BK.ScreenChange) {oled_clear_buffer();}

	// Линии разметки
	oled_draw_h_line(0, 21, 128);
	oled_draw_h_line(0, 43, 128);
	oled_draw_v_line(42, 0, 64);
	oled_draw_v_line(85, 0, 64);

	// ========================== Блоки данных ==========================
	draw_rpm_f(0, 0);				// Обороты (F)
	draw_speed_f(0, 22);			// Скорость (F)
    draw_water_temp_f(0, 44);		// Температура ОЖ (F)	
	
	draw_map_f(43, 0);				// ДАД (F)	
	draw_airtemp_h(43, 22, 0);		// Температура воздуха на впуске (H)
	draw_angle_h(43, 32);			// Текущий УОЗ (H)
	draw_fan_pwm_f(43, 44);			// Вентилятор охлаждения % ШИМ (F)

	draw_O2_sensor_f(86, 0, 0);		// Напряжение УДК / AFR (F)
	draw_inj_duty_f(86, 22);		// Загрузка форсунок (F)
    draw_battery_f(86, 44);			// Напряжение сети (F)	
	// ========================== Блоки данных ==========================

	draw_error_box();	// Индикация ошибок.
	oled_send_buffer();
}

// Экран замера разгона.
void draw_lcd_acceleration() {
	if (!BK.ScreenChange) {oled_clear_buffer();}

	// Линии разметки
	oled_draw_h_line(86, 21, 42);
	oled_draw_h_line(86, 43, 42);
	oled_draw_v_line(85, 0, 64);


	// ========================== Блоки данных ==========================
	draw_speed_f(86, 0);		// Скорость (F)	

	draw_map_f(86, 22);			// ДАД (F)	

	draw_water_temp_f(86, 44);	// Температура ОЖ (F)	
	// ========================== Блоки данных ==========================

	draw_error_box();	// Индикация ошибок.
	oled_send_buffer();
}

// Экран ошибок CE
void draw_lcd_ce_errors() {
	if (!BK.ScreenChange) {oled_clear_buffer();}
	draw_ce_errors();
	draw_error_box();
	oled_send_buffer();
}

void draw_lcd_no_signal() {
	oled_clear_buffer();
	draw_no_signal();
	oled_send_buffer();
}

// Анимация смены экранов
void draw_screen_change() {
	#ifdef ANIMATION_ENABLE
		static uint8_t y = 1;
		switch (BK.ScreenChange) {
			case 1:
				if (BK.ScreenMode == 3) {BK.ScreenMode = 0;}
				else {BK.ScreenMode += 1;}
				y = 1;
				BK.ScreenChange = 2;
				break;
			case -1:
				if (!BK.ScreenMode) {BK.ScreenMode = 3;}
				else {BK.ScreenMode -= 1;}
				y = 1;
				BK.ScreenChange = -2;
				break;
		}

		if (BK.ScreenChange > 0) {
			oled_set_clip_window(0, 0, 127, y);
			oled_draw_box(0, 0, 128, y, 2);
			oled_draw_h_line(0, y, 127);
		}
		else {
			oled_set_clip_window(0, 63 - y, 127, 63);
			oled_draw_box(0, 63 - y, 128, y + 1, 2);
			oled_draw_h_line(0, 63 - y, 128);
		}
		
		switch (BK.ScreenMode) {
			case 0:		// Основной экран.
				draw_lcd_main();
				break;
			case 1:		// Второй экран.
				draw_lcd_second();
				break;
			case 2:		// Экран замера разгона.
				draw_lcd_acceleration();
				break;
			case 3:		// Ошибки CE.
				draw_lcd_ce_errors();
				break;
		}
		y += ANIMATION_SPEED;
		if (y >= 63) {
			oled_disable_clip_window();
			BK.ScreenChange = 0;
			return;
		}
	#else
		if (BK.ScreenChange > 0) {
			if (BK.ScreenMode == 3) {BK.ScreenMode = 0;}
			else {BK.ScreenMode += 1;}
		}
		else {
			if (!BK.ScreenMode) {BK.ScreenMode = 3;}
			else {BK.ScreenMode -= 1;}
		}
		BK.ScreenChange = 0;
	#endif
}

void draw_finish() {
	if (!BK.ScreenChange) {oled_clear_buffer();}
	draw_statistics();
	oled_send_buffer();
}