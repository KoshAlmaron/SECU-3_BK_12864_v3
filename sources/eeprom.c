#include <avr/eeprom.h>

#include "eeprom.h"			// Свой заголовок.
#include "bkdata.h"			// Структура с данными.
#include "configuration.h"	// Настройки.

// Чтение EEPROM
void read_eeprom() {
	// Пробег в метрах, расход топлива в мл.
    
	BK.DistDay = eeprom_read_dword((uint32_t*) 0);		// Пробег суточный (0-3).
	BK.DistAll = eeprom_read_dword((uint32_t*) 4);		// Пробег общий (4-7).
	BK.FuelDay = eeprom_read_dword((uint32_t*) 8);		// Топливо суточный (8-11).
	BK.FuelAll = eeprom_read_dword((uint32_t*) 12);	// Топливо всего (12-15).
	BK.EngineHours = eeprom_read_dword((uint32_t*) 16);	// Моточасы (16-19).

    // Подсветка OLED
	BK.BrightLCD[0] = eeprom_read_byte((uint8_t*) 20);
	BK.BrightLCD[1] = eeprom_read_byte((uint8_t*) 21);

	// Подсветка ШИМ
	BK.BrightPWM[0] = eeprom_read_byte((uint8_t*) 22);
	BK.BrightPWM[1] = eeprom_read_byte((uint8_t*) 23);

	// Коэффициент коррекции расхода топлива
	BK.FuelConsumptionRatio = eeprom_read_byte((uint8_t*) 24);
	if (BK.FuelConsumptionRatio > 125 || BK.FuelConsumptionRatio < 75) {
		BK.FuelConsumptionRatio = 100;
	}
}

// Запись EEPROM
void update_eeprom(uint8_t OverWrite) {
	uint32_t DstDay = BK.DistDay + BK.DistRide;
	uint32_t DstAll = BK.DistAll + BK.DistRide;

	uint32_t Fuel = BK.FuelDay + BK.FuelRide;
	uint32_t FuelAll = BK.FuelAll + BK.FuelRide;

	if (OverWrite) {
		#ifdef WRITE_DISTANCE_DAY
			DstDay = WRITE_DISTANCE_DAY;
		#endif
		#ifdef WRITE_DISTANCE_ALL
			DstAll = WRITE_DISTANCE_ALL;
		#endif
		#ifdef WRITE_FUEL_BURNED_DAY
			Fuel = WRITE_FUEL_BURNED_DAY;
		#endif
		#ifdef WRITE_FUEL_BURNED_ALL
			FuelAll = WRITE_FUEL_BURNED_ALL;
		#endif
		#ifdef WRITE_ENGINE_HOURS
			BK.EngineHours = WRITE_ENGINE_HOURS;	
		#endif
		#ifdef WRITE_BRIGHT_LCD_NIGHT
			BK.BrightLCD[1] = WRITE_BRIGHT_LCD_NIGHT;
		#endif
		#ifdef WRITE_BRIGHT_LCD_DAY
			BK.BrightLCD[0] = WRITE_BRIGHT_LCD_DAY;		
		#endif
		#ifdef WRITE_BRIGHT_PWM_NIGHT
			BK.BrightPWM[1] = WRITE_BRIGHT_PWM_NIGHT;
		#endif
		#ifdef WRITE_BRIGHT_PWM_DAY
			BK.BrightPWM[0] = WRITE_BRIGHT_PWM_DAY;
		#endif
	}

    // Пробег суточный
    eeprom_update_dword((uint32_t*) 0, DstDay); // 0-3
    //  Пробег общий
    eeprom_update_dword((uint32_t*) 4, DstAll); // 4-7
    // Топливо суточный
    eeprom_update_dword((uint32_t*) 8, Fuel); // 8-11
    // Топливо всего
    eeprom_update_dword((uint32_t*) 12, FuelAll); // 12-15
    // Моточасы
    eeprom_update_dword((uint32_t*) 16, BK.EngineHours); // 16-19

    // Подсветка OLED
    eeprom_write_byte((uint8_t*) 20, BK.BrightLCD[0]);
    eeprom_write_byte((uint8_t*) 21, BK.BrightLCD[1]);
    // Подсветкой ШИМ
    eeprom_write_byte((uint8_t*) 22, BK.BrightPWM[0]);
    eeprom_write_byte((uint8_t*) 23, BK.BrightPWM[1]);

	// Коэффициент коррекции расхода топлива
	eeprom_write_byte((uint8_t*) 24, BK.FuelConsumptionRatio);
}