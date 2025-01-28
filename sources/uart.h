// UART.

#ifndef _UART_H_
	#define _UART_H_

	// Скорость передачи UART для процессора 1284 - 115200,
	// 							  для 644 - 57600
	#define UART_BAUD_RATE 115200UL
	// Значения регистров для настройки скорости UART.
	// Вычисляется требуемое значение по формуле:
	//	UBBR = F_CPU / (16 * baudrate) - 1		для U2X=0
	//  UBBR = F_CPU / (8 * baudrate) - 1		для U2X=1

	// Максимальный размера пакета данных от SECU.
	#define MAX_DATA_SIZE 112

	extern uint8_t EcuData[MAX_DATA_SIZE];
	extern volatile uint8_t DataSize;
	extern volatile uint8_t DataStatus;
	extern int8_t DataShift;

	void uart_init();

	// Спецсимволы в пакете данных
	#define FOBEGIN  0x40       // '@'  Начало исходящего пакета
	#define FIOEND   0x0D       // '\r' Конец пакета
	#define FESC     0x0A       // '\n' Символ подмены байта (FESC)
	// Измененные байты, которые были в пакете и совпадали со сцецбайтами
	#define TFOBEGIN 0x82       // Измененный FOBEGIN
	#define TFIOEND  0x83       // Измененный FIOEND
	#define TFESC    0x84       // Измененный FESC
	
#endif