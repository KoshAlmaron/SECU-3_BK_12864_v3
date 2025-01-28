#include <stdint.h>			// Коротние название int.
#include <avr/interrupt.h>	// Прерывания.

#include "uart.h"			// Свой заголовок.

#define SET_UBRR ((F_CPU / (8UL * UART_BAUD_RATE)) - 1UL)

//  Массив байтов от SECU и флаг успешного получения данных
uint8_t DataOk = 0;
uint8_t EcuData[MAX_DATA_SIZE] = {0};
// Размер пакета данных
volatile uint8_t DataSize = MAX_DATA_SIZE;
// Позиция в буфере.
volatile uint16_t RxBuffPos = 0;
// Признак, что был принят символ подмены байта.
volatile uint8_t MarkerByte = 0;
// Смещение данных после расширения блока CE
int8_t DataShift = 0;

// Статусы данных от SECU:
// 0 - готов к приему,
// 1 - идет прием пакета,
// 2 - пакет принят, ожидание обработки,
// 3 - ошибка приема.
volatile uint8_t DataStatus = 0;

void uart_init() {
	// Сброс регистров настроек, так как загрузчик Arduino может нагадить.
	UCSR0A = 0;
	UCSR0B = 0;
	UCSR0C = 0;

	UCSR0A |= (1 << U2X0);							// Двойная скорость передачи.
	UCSR0C &= ~((1 << UMSEL01) | (1 << UMSEL00));	// Асинхронный режим.
	UCSR0C |= (1 << UCSZ00) | ( 1 << UCSZ01);		// Размер пакета 8 бит.
	UBRR0H = (uint8_t) (SET_UBRR >> 8);				// Настройка скорости.
	UBRR0L = (uint8_t) SET_UBRR;
			
	UCSR0B |= (1 << RXEN0);			// 1 - Только прием.
	UCSR0B |= (1 << RXCIE0);		// Прерывание по завершеию приёма.
}

// Прерывание по окончании приема.
ISR (USART_RX_vect) {
	// Счетчик для определения длины пакета.
	static volatile uint8_t Count = 0;

	uint8_t OneByte = UDR0;		// Получаем байт.
	if (DataStatus > 1) {return;}

	switch (OneByte) {
		case FOBEGIN:	// Принят начальный байт.
			DataStatus = 1;
			RxBuffPos = 0;
			MarkerByte = 0;
			break;
		case FIOEND:	// Принят завершающий байт.
			// Для определения длины пакета необходимо
			// получить 4 пакета одинаковой длины подряд.
			if (Count < 4) {
				if (RxBuffPos == DataSize) {
					Count++;
					if (DataSize < 74) {DataShift = 2;}
				}
				else {
					Count = 0;
					DataSize = RxBuffPos;
				}
				DataStatus = 0;		// Сброс состояния приема.
			}
			// Если длина совпала, то ок.
			else if (RxBuffPos == DataSize) {DataStatus = 2;}
			break;
		case FESC:		// Принят символ подмены байта.
			MarkerByte = 1;
			break;
		default:
		 	if (MarkerByte) {	// Следующий байт после символа подмены.
		 		switch (OneByte) {
		 			case TFOBEGIN:
		 				OneByte = FOBEGIN;
		 				break;
		 			case TFIOEND:
		 				OneByte = FIOEND;
		 				break;
		 			case TFESC:
		 				OneByte = FESC;
		 				break;
		 			default:		// Если ничего не совпало, значит косяк.
		 				DataStatus = 0;
		 				MarkerByte = 0;
		 				return;
		 		}
		 		MarkerByte = 0;
		 	}
			EcuData[RxBuffPos] = OneByte;
			RxBuffPos++;
			// Превышена максимальная длина пакета.
			if (RxBuffPos > MAX_DATA_SIZE) {DataStatus = 0;}
			break;
	}
}