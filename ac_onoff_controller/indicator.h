/*
 * indicator.h
 *
 * Created: 2024/09/11 15:37:35
 *  Author: qwpmb
 */ 


#ifndef INDICATOR_H_
#define INDICATOR_H_

typedef enum {
	POWERON,
	PRESSING_AC_SWITCH,
	CURRENT_STATUS_AC_ON,
	CURRENT_STATUS_AC_OFF,
	ERR01,
	ERR02,
	ERR03,
	NOLED,
} LED_PATTERN;

void init_indicators();
void lights_off();
void indicate(LED_PATTERN, uint8_t);




#endif /* INDICATOR_H_ */