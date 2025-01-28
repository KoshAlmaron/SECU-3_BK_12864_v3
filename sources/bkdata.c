#include <stdint.h>			// Коротние название int.

#include "bkdata.h"			// Свой заголовок.

// Инициализация структуры
BK_t BK = {
	.DistRide = 0,
	.DistDelta = 0,
	.FuelRide = 0,
	.FuelDelta = 0,
	.DistDay = 0,
	.FuelDay = 0,
	.DistAll = 0,
	.FuelAll = 0,
	.RideTimer = 0,
	.EngineHours = 0,
	.FuelConsumptionRatio = 100,
	.BrightMode = 0,
	.BrightLCD = {0},
	.BrightPWM = {0},
	.ScreenMode = 0,
	.ScreenChange = 0,
	.CountCE = {0},
	.StatusCE = 0
};
