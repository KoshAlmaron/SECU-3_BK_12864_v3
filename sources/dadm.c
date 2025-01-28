// Подключение бибилиотек
#include <stdint.h>			// Коротние название int.
#include <avr/io.h>				// Названия регистров и номера бит.

#include "dadm.h"			// Свой заголовок.
#include "functions.h"		// Вспомогательные функции.
#include "macros.h"			// Макросы.
#include "pinout.h"			// Начначение выводов контроллера.
#include "bkdata.h"			// Структура с данными.

#ifdef INT_OIL_PIN
	extern int16_t AlarmBoxTimer;		// Счетчики времени из main.
	uint8_t OilPressureState = 0;		// Состояние ДАДМ.
	uint8_t AlarmShowed = 0;			// Флаг, что состояние было отображено на экране.

	void test_oil_pressure(uint8_t Mode) {
		if (!Mode) {
			// Если при старте нет напряжения, значит перегорела лампочка
			// или неисправна проводка.
			if (PIN_READ(INT_OIL_PIN)) {OilPressureState = 16;}
			return;
		}
		// Выход при обрыве цепи.
		if (OilPressureState == 16) {return;}
		if (!PIN_READ(INT_OIL_PIN)) {OilPressureState = 8;}
	}


	uint8_t get_oil_pressure_state() {
		// Сброс состояния датчика на экране ошибок CE.
		if (BK.ScreenMode == 3 && OilPressureState == 16) {OilPressureState = 0;}

		uint16_t RPM = build_unsigned_int(1);
		if (RPM > 400 && OilPressureState) {
			// Сброс после отображения аварии и нормализации давления.
			if (AlarmShowed) {
				if (AlarmBoxTimer < 0 &&OilPressureState == 8 && PIN_READ(INT_OIL_PIN)) {
					OilPressureState = 0;
					AlarmShowed = 0;
				}
			}
			else {
				AlarmBoxTimer = 1;
				AlarmShowed = 1;
			}
			return OilPressureState;
		}
		return 0;
	}
#endif
