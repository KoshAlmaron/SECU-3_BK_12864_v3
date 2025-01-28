//	Хранение всех необходимых параметров.

#ifndef _BKDATA_H_
	#define _BKDATA_H_

	// Структура для хранения переменных.
	typedef struct BK_t {
		uint32_t DistRide;				// Пройденное расстояние в метрах за время работы.
		uint32_t DistDelta;				// Переменная для обнуления дистанции.
		uint32_t FuelRide;				// Израсходованно топлива за поездку в мл.
		uint32_t FuelDelta;				// Переменная для обнуления топлива.
		uint32_t DistDay;				// Пробег суточный.
		uint32_t FuelDay;				// Расход топлива суточный.
		uint32_t DistAll;				// Пробег общий.
		uint32_t FuelAll;				// Расход топлива общий.
		uint16_t RideTimer;				// Время работы за поездку.
		uint32_t EngineHours;			// Моточасы в секундах.
		uint8_t FuelConsumptionRatio;	// Коэффициент расхода топлива.
		uint8_t BrightMode;				// Режим подсветки.
		uint8_t BrightLCD[3];			// Яркость подсветки дисплея.
		uint8_t BrightPWM[3];			// ШИМ подсветка приборной панели.
		uint8_t ScreenMode;				// Номер активного экрана.
		int8_t ScreenChange;			// Флаг смены экрана.
		uint8_t CountCE[24];			// Количество ошибок CE.
		uint8_t StatusCE;				// Общее кол-во ошибок в данный момент.
	} BK_t;
	
	extern struct BK_t BK;		// Делаем структуру внешней.

#endif


/*

BrightMode
	Режим подсветки, 0 - день, 1 - ночь
BrightLCD
	Яркость подсветки дисплея, 0 - день, 1 - ночь, 2 - буфер для перехода
*/