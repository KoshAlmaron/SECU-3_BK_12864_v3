// Подключение бибилиотек
#include <stdint.h>			// Коротние название int.
#include <stdio.h>			// Стандартная библиотека ввода/вывода.
#include <avr/pgmspace.h>	// Работа с PROGMEM.

#include "blocks.h"			// Свой заголовок.
#include "macros.h"			// Макросы.
#include "functions.h"		// Вспомогательные функции.
#include "oled.h"			// OLED дисплей.
#include "fonts.h"			// Шрифты.
#include "pictograms.h"		// Пиктограммамы.
#include "configuration.h"	// Настройки.
#include "uart.h"			// UART.
#include "bkdata.h"			// Структура с данными.
#include "dadm.h"			// Датчик аварийного давления масла.
#include "pinout.h"			// Начначение выводов контроллера.
#include "ce_errors.h"		// Список ошибок CE.
#include "buttons.h"		// Кнопки.

extern int16_t AlarmBoxTimer;		// Счетчики времени из main.

// Строка для вывода на экран.
char Buffer[8] = {0};
const char NoSignal[] PROGMEM = "NO_SIGNAL";
const char NoErrors[] PROGMEM = "NO_ERRORS";

const char TravelTime[] PROGMEM = "TRAVEL_TIME";
const char TravelDist[] PROGMEM = "DISTANCE";
const char TravelFuel[] PROGMEM = "FUEL_CONSUMED";
const char TravelSpeed[] PROGMEM = "AVERAGE_SPEED";

void draw_statistics() {
	#define ROW_SPACE_FINISH 16
	#define ROW_SHIFT 5

	BK.RideTimer = 6160;
	BK.DistRide = 190950;
	BK.FuelRide = 10750;

	uint8_t Row = 0;
	uint8_t y = 0;
	oled_set_font(u8g2_font_helvB08_tr);
	// Строка 1 - Время поездки в минутах
	Row = 1;
	snprintf(Buffer, 6, "%3u.%1u", MIN(999, BK.RideTimer / 60), ((BK.RideTimer % 60) * 10) / 60);
	y = (Row - 1) * ROW_SPACE_FINISH + ROW_SHIFT;
	oled_print_string_f(1, y, TravelTime, 11);
	oled_print_string(100, y, Buffer, 5);

	// Строка 2 - Пройденное расстояние
	Row = 2;
	snprintf(Buffer, 7, "%4lu.%1lu", MIN(9999, BK.DistRide / 1000), ((BK.DistRide % 1000) * 10) / 1000);
	y = (Row - 1) * ROW_SPACE_FINISH + ROW_SHIFT;
	oled_print_string_f(1, y, TravelDist, 8);
	oled_print_string(94, y, Buffer, 6);

	// Строка 3 - Израсходовано топлива
	Row = 3;
	snprintf(Buffer, 6, "%3lu.%1lu", MIN(999, BK.FuelRide / 1000), ((BK.FuelRide % 1000) * 10) / 1000);
	y = (Row - 1) * ROW_SPACE_FINISH + ROW_SHIFT;
	oled_print_string_f(1, y, TravelFuel, 13);
	oled_print_string(100, y, Buffer, 5);

	// Строка 4 - Средняя скорость
	Row = 4;
	uint32_t SpeedTmp = (BK.DistRide * 36 / 10);
	snprintf(Buffer, 6, "%3lu.%1lu", MIN(999, SpeedTmp / BK.RideTimer), MIN(9, ((SpeedTmp % BK.RideTimer) * 10) / BK.RideTimer));
	y = (Row - 1) * ROW_SPACE_FINISH + ROW_SHIFT;
	oled_print_string_f(1, y, TravelSpeed, 13);
	oled_print_string(100, y, Buffer, 5);
}

void draw_ce_errors() {
	#define ROWS_ON_SCREEN 4
	#define ROW_SPACE_CE 5
	static uint8_t SkipRow = 0;		// Пропуск строк для перелистывания.
	uint8_t	Row = 0;				// Номер строки на экране.
	uint8_t	RowCount = 0;			// Общее кол-во сторок.

	// Сброс начальной строки при переключении экрана.
	if (ButtonState[2] == 201 || ButtonState[3] == 201) {SkipRow = 0;}


	oled_set_font(u8g2_font_helvB08_tr);
	for (uint8_t i = 0; i < MAX_CE_BITS_COUNT; i++) {
		if (BK.CountCE[i] > 0) {
			if (Row < ROWS_ON_SCREEN && RowCount >= SkipRow) {
				BK.CountCE[i] = MIN(99, BK.CountCE[i]);
				snprintf(Buffer, 3, "%2u", BK.CountCE[i]);
				uint8_t y = Row * (ROW_SPACE_CE + 8) + 4;
				oled_print_string_f(1, y, (char*)pgm_read_word(&(CEItemsArray[i])), 16);
				oled_print_string(116, y, Buffer, 3);
				Row += 1;
			}
			RowCount++;
		}
	}

	// Есть строки выше.
	if (SkipRow) {oled_draw_xbmp(72 - 5, 64 - 8, Prev_bits, Prev_width, Prev_height);}
	// Смещение вверх.
	if (ButtonState[0] == 201 && SkipRow) {SkipRow--;}
	if (ROWS_ON_SCREEN + SkipRow < RowCount) {
		// Есть строки ниже.
		oled_draw_xbmp(56 - 5, 64 - 8, Next_bits, Next_width, Next_height);
		// Смещение вниз.
		if (ButtonState[1] == 201) {SkipRow++;}
	}


	// Если нет ни одной ошибки
	if (!Row) {oled_print_string_f(29, 28, NoErrors, 9);}

	BK.StatusCE = 0;
}

void draw_error_box() {
	// Рамка при появлении ошибок CE
	if (BK.StatusCE > 0 && AlarmBoxTimer > 0) {
		oled_draw_frame(0, 0, 128, 64);
		BK.StatusCE = 0;
	}

    // Проверка ДАДМ.
	#ifdef INT_OIL_PIN
		if (get_oil_pressure_state() && AlarmBoxTimer > 0) {
			oled_draw_box(0, 0, 128, 64, 1);
		}
	#endif
}

void draw_no_signal() {
	oled_set_font(u8g2_font_helvB08_tr);
	oled_print_string_f(29, 28, NoSignal, 9);
}

void draw_battery_f(uint8_t x, uint8_t y) {
	uint16_t BAT = build_unsigned_int(5);	// x400
	BAT = CONSTRAIN(BAT, 0, 50 * 400);

	oled_draw_xbmp(x + 2, y + 2, BatteryL_bits, BatteryL_width, BatteryL_height);
	oled_set_font(u8g2_font_helvB10_tn);
	snprintf(Buffer, 5, "%2u.%1u", BAT / 400, ((BAT % 400) * 10) / 400);

	oled_print_string(x + 10, y + 5, Buffer, 4);

	if (AlarmBoxTimer > 0) {
		if (BAT < BATT_VOLT_MIN || BAT > BATT_VOLT_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}
}

void draw_battery_h(uint8_t x, uint8_t y) {
	uint16_t BAT = build_unsigned_int(5);	// x400
	BAT = CONSTRAIN(BAT, 0, 50 * 400);

	oled_draw_xbmp(x + 2, y + 1, BatteryS_bits, BatteryS_width, BatteryS_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 5, "%2u.%1u", BAT / 400, ((BAT % 400) * 10) / 400);
	oled_print_string(x + 18, y + 2, Buffer, 4);

	if (AlarmBoxTimer > 0) {
		if (BAT < BATT_VOLT_MIN || BAT > BATT_VOLT_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 9, 1);
		}
	}
}

void draw_inj_duty_f(uint8_t x, uint8_t y) {
	uint8_t InjDuty = get_byte(81) >> 1;	// x2

	oled_draw_xbmp(x + 2, y + 3, InjDuty_bits, InjDuty_width, InjDuty_height);
	oled_set_font(u8g2_font_pxplusibmvga8_tn);

	snprintf(Buffer, 4, "%3u", InjDuty);
	oled_print_string(x + 12, y + 4, Buffer, 3);
}

void draw_O2_sensor_f(uint8_t x, uint8_t y, uint8_t Channel) {
	// Тип датчика кислорода, 0 - УДК, 1 - ШДК.
	uint8_t LambdaType;
	uint8_t ByteNumber = 62 - DataShift;
	switch (Channel) {
		case 0:	// Lambda 1.
			oled_draw_xbmp(x + 6, y + 15, Symbol_I_bits, Symbol_I_width, Symbol_I_height);
			break;
		case 1:	// Lambda 2.
			ByteNumber = 93;
			oled_draw_xbmp(x + 6, y + 14, Symbol_II_bits, Symbol_II_width, Symbol_II_height);
			break;
		case 2:	// Mix.
			ByteNumber = 97;
			oled_draw_xbmp(x + 6, y + 3, Symbol_mix_bits, Symbol_mix_width, Symbol_mix_height);
			break;
		case 3:	// Target.
			ByteNumber = 99;
			oled_draw_xbmp(x + 6, y + 15, Symbol_T_bits, Symbol_T_width, Symbol_T_height);
			break;
	}

	uint16_t AFR = build_unsigned_int(ByteNumber);	// x128
	if (AFR > 0 || Channel) {
		// Если есть показания АФР, значит показывать надо АФР.
		LambdaType = 1;
		AFR = CONSTRAIN(AFR, 0, 30 * 128);
		snprintf(Buffer, 5, "%2u.%1u", AFR / 128, ((AFR % 128) * 10) / 128);
	}
	else {
		// Иначе показываем напряжение.
		LambdaType = 0;
		AFR = build_int(19);	// x400
		AFR = CONSTRAIN(AFR, 0, 9 * 400);
		snprintf(Buffer, 5, "%1u.%02u", AFR / 400, ((AFR % 400) * 100) / 400);
	}

	oled_draw_xbmp(x + 2, y + 2, Lambda_L_bits, Lambda_L_width, Lambda_L_height);
	oled_set_font(u8g2_font_helvB10_tn);
	oled_print_string(x + 11, y + 5, Buffer, 4);

	if (AlarmBoxTimer > 0) {
		if (LambdaType == 1) {
			if (AFR < LAMBDA_AFR_MIN || AFR > LAMBDA_AFR_MAX) {
				oled_draw_box(x + 1, y + 1, 40, 19,  1);
			}
		}
		else {
			if (AFR < UDK_VOLT_MIN || AFR > UDK_VOLT_MAX) {
				oled_draw_box(x + 1, y + 1, 40, 19, 1);
			}
		}
	}
}

void draw_O2_sensor_h(uint8_t x, uint8_t y, uint8_t Channel) {
	// Тип датчика кислорода, 0 - УДК, 1 - ШДК.
	uint8_t LambdaType = 0;
	uint8_t ByteNumber = 62 - DataShift;
	switch (Channel) {
		case 0:	// Lambda 1.
			oled_draw_xbmp(x + 2, y + 1, Lambda_1_S_bits, Lambda_1_S_width, Lambda_1_S_height);
			break;
		case 1:	// Lambda 2.
			ByteNumber = 93;
			oled_draw_xbmp(x + 2, y + 1, Lambda_2_S_bits, Lambda_2_S_width, Lambda_2_S_height);
			break;
		case 2:	// Mix.
			ByteNumber = 97;
			oled_draw_xbmp(x + 2, y + 1, Lambda_mix_S_bits, Lambda_mix_S_width, Lambda_mix_S_height);
			break;
		case 3:	// Target.
			ByteNumber = 99;
			oled_draw_xbmp(x + 2, y + 1, Lambda_target_S_bits, Lambda_target_S_width, Lambda_target_S_height);
			break;
	}

	uint16_t AFR = build_unsigned_int(ByteNumber);	// x128
	if (AFR > 0 || Channel) {
		// Если есть показания АФР, значит показывать надо АФР.
		LambdaType = 1;
		AFR = CONSTRAIN(AFR, 0, 30 * 128);
		snprintf(Buffer, 5, "%2u.%1u", AFR / 128, ((AFR % 128) * 10) / 128);
	}
	else {
		// Иначе показываем напряжение.
		LambdaType = 0;
		AFR = build_int(19);	// x400
		AFR = CONSTRAIN(AFR, 0, 9 * 400);
		snprintf(Buffer, 5, "%1u.%02u", AFR / 400, ((AFR % 400) * 100) / 400);
	}

	oled_set_font(u8g2_font_haxrcorp4089_tn);
	oled_print_string(x + 20, y + 2, Buffer, 4);

	if (AlarmBoxTimer > 0) {
		if (LambdaType == 1) {
			if (AFR < LAMBDA_AFR_MIN || AFR > LAMBDA_AFR_MAX) {
				oled_draw_box(x + 1, y + 1, 40, 9, 1);
			}
		}
		else {
			if (AFR < UDK_VOLT_MIN || AFR > UDK_VOLT_MAX) {
				oled_draw_box(x + 1, y + 1, 40, 9, 1);
			}
		}
	}
}

void draw_fan_pwm_f(uint8_t x, uint8_t y) {
	uint8_t FanPWM = get_byte(84) >> 1;		// x2

	oled_draw_xbmp(x + 1, y + 2, FAN_bits, FAN_width, FAN_height);
	oled_set_font(u8g2_font_helvB12_tn);

	snprintf(Buffer, 4, "%3u", FanPWM);
	oled_print_string(x + 12, y + 4, Buffer, 3);
}

void draw_angle_h(uint8_t x, uint8_t y) {
	int16_t ANGLE = build_unsigned_int(9);		// x32
	ANGLE = CONSTRAIN(ANGLE, 0, 50 * 32);
	int16_t DDANGLE = build_int(13);	// x32

	oled_draw_xbmp(x + 2, y + 1, Angle_bits, Angle_width, Angle_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 5, "%2u.%1u", ANGLE / 32, ((ANGLE % 32) * 10) / 32);
	oled_print_string(x + 17, y + 2, Buffer, 4);
	oled_draw_xbmp(x + 37, y + 2, Cels_bits, Cels_width, Cels_height);

	if (AlarmBoxTimer > 0) {
		if (DDANGLE > 0) {
			oled_draw_box(x + 1, y + 1, 40, 9, 1);
		}
	}
}

void draw_speed_f(uint8_t x, uint8_t y) {
	uint16_t Speed =  build_speed(29 - DataShift);	// x32
	int16_t DDANGLE = build_int(13);	// x32

	oled_draw_xbmp(x + 0, y + 2, Speed_bits, Speed_width, Speed_height);
	oled_set_font(u8g2_font_helvB12_tn);

	snprintf(Buffer, 4, "%3u", Speed);
	oled_print_string(x + 12, y + 4, Buffer, 3);

	if (AlarmBoxTimer > 0) {
		if (DDANGLE > 0) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}
}

void draw_rpm_f(uint8_t x, uint8_t y) {
	uint16_t RPM = build_unsigned_int(1);
	RPM = (RPM / 10) * 10;
	RPM = CONSTRAIN(RPM, 0, 9999);

	oled_set_font(u8g2_font_helvB12_tn);
	snprintf(Buffer, 5, "%4u", RPM);
	oled_print_string(x + 2, y + 4, Buffer, 4);

	if (AlarmBoxTimer > 0) {
		if (RPM > RPM_LIMIT) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}
}

void draw_fuel_burned_f(uint8_t x, uint8_t y) {
	oled_draw_xbmp(x + 2, y + 3, FuelCanister_bits, FuelCanister_width, FuelCanister_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 6, "%3lu.%1lu", MIN(999, (BK.FuelDay + BK.FuelRide) / 1000), \
								 (((BK.FuelDay + BK.FuelRide) % 1000) * 10) / 1000);
	oled_print_string(x + 14, y + 2, Buffer, 5);

	snprintf(Buffer, 7, "%4lu", MIN(9999, (BK.FuelAll + BK.FuelRide) / 1000));
	oled_print_string(x + 16, y + 11, Buffer, 6);
}

void draw_airtemp_h(uint8_t x, uint8_t y, uint8_t Channel) {
	uint8_t ByteNumber = 36 - DataShift;
	if (Channel == 1) {
		ByteNumber = 101;
		oled_draw_pixel(x + 2, y + 2);
		oled_draw_pixel(x + 2, y + 4);
	}

	int16_t AIRTEMP = build_int(ByteNumber);	// x4
	AIRTEMP = CONSTRAIN(AIRTEMP, -99 * 4, 200 * 4);

	oled_draw_xbmp(x + 4, y + 1, AirTempS_bits, AirTempS_width, AirTempS_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 6, "%3i.%1u", AIRTEMP / 4, ABS(((AIRTEMP % 4) * 10) / 4));
	oled_print_string(x + 11, y + 2, Buffer, 5);
	oled_draw_xbmp(x + 37, y + 2, Cels_bits, Cels_width, Cels_height);

	if (AlarmBoxTimer > 0) {
		if (AIRTEMP < AIR_TEMP_MIN || AIRTEMP > AIR_TEMP_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 9, 1);
		}
	}
}

void draw_lambda_corr_h(uint8_t x, uint8_t y, uint8_t Channel) {
	uint8_t ByteNumber = 52 - DataShift;
	if (Channel == 1) {ByteNumber = 95;}
	int16_t AFR_CORR = build_int(ByteNumber) * 100; // x512
	AFR_CORR = CONSTRAIN(AFR_CORR, -50 * 512, 50 * 512);

	oled_draw_xbmp(x + 2, y + 1, LambdaCorrS_bits, LambdaCorrS_width, LambdaCorrS_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 6, "%3i.%1u", AFR_CORR / 512, ABS(((AFR_CORR % 512) * 10) / 512));
	if (AFR_CORR > 0) {Buffer[0] = '+';}
	oled_print_string(x + 14, y + 2, Buffer, 5);

	if (AlarmBoxTimer > 0) {
		if (AFR_CORR < LAMBDA_CORR_MIN || AFR_CORR > LAMBDA_CORR_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 9, 1);
		}
	}
}

void draw_afc_f(uint8_t x, uint8_t y) {
	uint16_t AFC = 0;
	uint16_t TAFC = 0;

	// Средний расход суточный
	if (BK.DistDay + BK.DistRide > 1000) {
		AFC = (uint32_t) (BK.FuelDay + BK.FuelRide)* 100 / ((BK.DistDay + BK.DistRide) >> 7);		// x128
	}
	// Средний расход общий
	if (BK.DistAll + BK.DistRide > 1000) {
		TAFC = (uint32_t) (BK.FuelAll + BK.FuelRide) * 100 / ((BK.DistAll + BK.DistRide) >> 7);	// x128
	}

	oled_draw_xbmp(x + 3, y + 3, AFC_bits, AFC_width, AFC_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 7, "%3u.%02u", AFC / 128, ((AFC % 128) * 100) / 128);
	oled_print_string(x + 7, y + 2, Buffer, 7);

	snprintf(Buffer, 7, "%3u.%02u", TAFC / 128, ((TAFC % 128) * 100) / 128);
	oled_print_string(x + 7, y + 11, Buffer, 7);
}

void draw_trottle_f(uint8_t x, uint8_t y) {
	uint8_t TPS = get_byte(18);				// x2
	uint8_t IAC = get_byte(27 - DataShift);	// x2

	uint8_t StrSize = 5;
	uint16_t Offset = 0;
	if (TPS > 2) {
		snprintf(Buffer, 6, "%3u.%1u", TPS / 2, ((TPS % 2) * 10) / 2);
		oled_draw_xbmp(x + 2, y + 4, Trottle_bits, Trottle_width, Trottle_height);
		if (TPS == 200) {
			StrSize = 3;
			Offset = 7;
		}
	}
	else {
		snprintf(Buffer, 6, "%3u.%1u", IAC / 2, ((IAC % 2) * 10) / 2);
		oled_draw_xbmp(x + 2, y + 4, IAC_bits, IAC_width, IAC_height);
		if (IAC == 200) {
			StrSize = 3;
			Offset = 7;
		}
	}
	
	oled_set_font(u8g2_font_helvB08_tr);
	oled_print_string(x + 8 + Offset, y + 6, Buffer, StrSize);
}

void draw_map_f(uint8_t x, uint8_t y) {
	uint16_t MAP = build_unsigned_int(3) >> 6;	// 64
	MAP = CONSTRAIN(MAP, 0, 999);

	if (AlarmBoxTimer > 0) {
		if (MAP > OVERBOOST_LIMIT) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}

	oled_draw_xbmp(x + 2, y + 2, MapL_bits, MapL_width, MapL_height);
	oled_set_font(u8g2_font_pxplusibmvga8_tn);
	snprintf(Buffer, 4, "%3u", MAP);
	oled_print_string(x + 14, y + 5, Buffer, 3);
}

void draw_distance_f(uint8_t x, uint8_t y) {
	oled_draw_xbmp(x + 1, y + 2, Road_bits, Road_width, Road_height);
	oled_set_font(u8g2_font_haxrcorp4089_tn);

	snprintf(Buffer, 7, "%4lu.%1lu", MIN(9999, (BK.DistDay + BK.DistRide) / 1000), \
								 (((BK.DistDay + BK.DistRide) % 1000) * 10) / 1000);
	oled_print_string(x + 9, y + 2, Buffer, 6);

	snprintf(Buffer, 7, "%6lu", MIN(999999, (BK.DistAll + BK.DistRide) / 1000));
	oled_print_string(x + 5, y + 11, Buffer, 6);
}

void draw_ff_fc_f(uint8_t x, uint8_t y) {
	uint8_t Speed = build_speed(29 - DataShift);
	uint16_t FuelFlow = build_unsigned_int(34 - DataShift);  // x256

	// Мгновенный расход л/ч (x64).
	// [Частота] * [3600 с] / [16000 имп] * FuelConsumptionRatio / 100
	FuelFlow = (uint32_t) FuelFlow * 9 * BK.FuelConsumptionRatio / 16000;	// x64

	// Режим отображения в зависимости от скорости, л/ч или л/100км.
	oled_draw_xbmp(x + 2, y + 2, LETTER_F_bits, LETTER_F_width, LETTER_F_height);
	if (Speed <= 3) {
		oled_draw_xbmp(x + 2, y + 11, LETTER_F_bits, LETTER_F_width, LETTER_F_height);
	}
	else {
		FuelFlow = (FuelFlow * 100) / Speed;	// x64
		oled_draw_xbmp(x + 2, y + 11, LETTER_C_bits, LETTER_C_width, LETTER_C_height);
	}
	
	FuelFlow = CONSTRAIN(FuelFlow, 0, 99 * 64);
	oled_set_font(u8g2_font_helvB12_tn);
	snprintf(Buffer, 5, "%2u.%1u", FuelFlow / 64, ((FuelFlow % 64) * 10) / 64);
	oled_print_string(x + 8, y + 4, Buffer, 4);
}

void draw_water_temp_f(uint8_t x, uint8_t y) {
	int16_t TEMP = build_int(7) >> 2; // x4
	TEMP = CONSTRAIN(TEMP, -50, 150);
	
	// Рисуем пиктограмму параметра и знач градуса.
	oled_draw_xbmp(x + 2, y + 3, WaterTempL_bits, WaterTempL_width, WaterTempL_height);
	oled_draw_xbmp(x + 37, y + 4, Cels_bits, Cels_width, Cels_height);

	snprintf(Buffer, 4, "%3i", TEMP);
	oled_set_font(u8g2_font_pxplusibmvga8_tn);
	oled_print_string(x + 11, y + 5, Buffer, 3);

	// Рисуем прямоугольник, при выходе параметра за допустимые пределы.
	if (AlarmBoxTimer > 0) {
		if (TEMP < WATER_TEMP_MIN || TEMP > WATER_TEMP_MAX) {
			oled_draw_box(x + 1, y + 1, 40, 19, 1);
		}
	}
}

/*

	//char BufferT[] = "+01234-5678.9";
	//char BufferT[] = "ABCDEFGHIJKLMNO";
	//char BufferT[] = "PQRSTUVWXYZ----";
	//char_shift(BufferT, 15);
	//oled_print_string(0, 42, Buffer, 15);s
*/
