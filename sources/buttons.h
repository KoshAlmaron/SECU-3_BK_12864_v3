// Кнопки.

#ifndef _BUTTONS_H_
	#define _BUTTONS_H_

	void buttons_init();
	void button_action();
	void buttons_clear();
	void buttons_update();

	extern uint8_t ButtonState[4];

#endif