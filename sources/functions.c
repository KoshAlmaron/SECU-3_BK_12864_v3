// Подключение бибилиотек
#include <stdint.h>			// Коротние название int.

#include "functions.h"		// Свой заголовок.
#include "uart.h"			// UART.
#include "macros.h"			// Макросы.
#include "configuration.h"	// Настройки.
#include "bkdata.h"				// Структура с данными.

// Возвращает байт из массива.
uint8_t get_byte(uint8_t i) {
	return EcuData[i];
}

// Сборка int из двух байт
int16_t build_int(uint8_t i) { // 4
	// Для незадействованных параметров
	if (EcuData[i + 1] == 0xff) {
		if (EcuData[i] == 0x7f || EcuData[i] == 0xff) {
			return 0;
		}
	}
	
	int16_t Value = 0;
	uint8_t *pValue = (uint8_t*)&Value;
	*pValue = EcuData[i + 1];  
	*(pValue + 1) = EcuData[i];
	return Value;
}

// Сборка unsigned int из двух байт
uint16_t build_unsigned_int(uint8_t i) { 
	uint16_t Value = 0;
	uint8_t *pValue = (uint8_t*)&Value;
	*pValue = EcuData[i + 1];  
	*(pValue + 1) = EcuData[i];
	return Value;
}

// Расчет скорости ТС (x32).
uint8_t build_speed(uint8_t i) { // 11
	uint16_t Speed = build_unsigned_int(i);	

	// Для SECU версии от 23.03.2023 изменен порядок расчета
	// скорости, пробега и израсходованного топлива.
	if (DataSize <= 91) {
		if (Speed != 0 && Speed != 65535) {
			Speed = (uint32_t) (312500 * 3600) / (SPEED_SENSOR_COUNT * Speed);
		}
	}
	Speed = CONSTRAIN(Speed >> 5, 0, 255);
	return Speed;
}

// Расчет израсходованного топлива
void build_fuel(uint8_t TimeMs) {
	static uint32_t FuelBuffer = 0;

	FuelBuffer += build_unsigned_int(34 - DataShift) * TimeMs * BK.FuelConsumptionRatio / 100;
	if (FuelBuffer >= 4096000) {
		FuelBuffer -= 4096000;
		BK.FuelRide++;
	}
}

// Разбор блока ошибок CE
void check_ce_errors() {
	// Предыдущее состояние
	static uint32_t PrevCE = 0;

	uint8_t BitsCount = 16;
	if (DataSize >= 74) {BitsCount = MAX_CE_BITS_COUNT;}

	// Собираем все байты CE в один кусок
	uint32_t CE = 0;
	uint8_t *pValue = (uint8_t*) &CE;
	*pValue = EcuData[26];  
	*(pValue + 1) = EcuData[25];
	*(pValue + 2) = EcuData[24];
	*(pValue + 3) = EcuData[23];

	//BK.StatusCE = 0;
	for (uint8_t i = 0; i < BitsCount; i++ ) {
		// Проверяем изменение бита CE с 0 на 1
		if (BITREAD(CE, i) && !BITREAD(PrevCE, i)) {
			BK.CountCE[i] = MIN(99, BK.CountCE[i] + 1);
		}
		// Считаем количество ошибок CE в данный момент.
		if (i != 15 && BITREAD(CE, i)) {
			BK.StatusCE = MIN(255, BK.StatusCE + 1);
		}
	}
	PrevCE = CE;
}