	// Начначение выводов контроллера.

#ifndef _PINOUT_H_
	#define _PINOUT_H_

	// Входы для проверки состояний	
	//#define INT_OIL_PIN D, 6		// Датчика аварийного давления масла (Опционально)
	#define INT_IGN_PIN D, 7		// Замка зажигания

	// Кнопки
	#define BUTTON_UP_PIN C, 3		// Вверх (Encoder A).
	#define BUTTON_DOWN_PIN C, 1	// Вниз (Encoder B).
	#define BUTTON_RIGHT_PIN C, 0	// Вправо (Encoder C).
	#define BUTTON_LEFT_PIN C, 2	// Влево (Не используется при управлении энкодером).

	#define LED_PIN B, 5		// Визуализация работы цикла.

#endif

/*
	0 PD0	(RX)			|	Прием данных от SECU.
	1 PD1	(TX)			|	
	2 PD2	(INT0)			|	Speed Chime (опционально).
   ~3 PD3	(OC2B) (INT1)	|	ШИМ управление подсветкой приборной панели.
	4 PD4					|	Выход управления питанием.
   ~5 PD5	(OC0B)			|	Датчик температуры DS18B20.
   ~6 PD6	(OC0A)			|	Вход для проверки состояния ДАДМ.
	7 PD7					|	Вход для проверки состояния замка зажигания.
	8 PB0	(ICP1)			|	
   ~9 PB1	(OC1A)			|
   ~10 PB2	(OC1B) (SS)		|
   ~11 PB3	(OC2A) (MOSI)	|
	12 PB4	(MISO)			|
	13 PB5	(SCL) (LED)		|	

	A0 PC0	(ADC0)			|	Кнопка вправо (Encoder C).
	A1 PC1	(ADC0)			|	Кнопка вниз (Encoder B).
	A2 PC2	(ADC0)			|	Кнопка влево.
	A3 PC3	(ADC0)			|	Кнопка вверх (Encoder A).
	A4 PC4	(ADC0) (SDA)	|	Датчик EGT (опционально).
	A5 PC5	(ADC0) (SCL)	|	Фоторезистор регулировки яркости.
*/