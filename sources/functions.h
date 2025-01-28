// Вспомогательные функции

#ifndef _FUNCTIONS_H_
	#define _FUNCTIONS_H_

	uint8_t get_byte(uint8_t i);
	int16_t build_int(uint8_t i);
	uint16_t build_unsigned_int(uint8_t i);
	uint8_t build_speed(uint8_t i);
	void build_fuel(uint8_t TimeMs);
	void check_ce_errors();
	
#endif