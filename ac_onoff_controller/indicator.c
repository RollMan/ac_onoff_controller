/*
* indicator.c
*
* Created: 2024/09/11 15:38:48
*  Author: qwpmb
*/
#include <avr/io.h>
#include <util/delay.h>
#include "indicator.h"

#define LED_DDR (DDRD)
#define LED_PORT (PORTD)
#define GREEN (PORTD4)
#define RED (PORTD3)
#define WHITE (PORTD2)

#define BVGREEN (1 << 2)
#define BVRED (1 << 1)
#define BVWHITE (1 << 0)



void init_indicators(){
	LED_DDR |= (1 << GREEN) | (1 << RED) | (1 << WHITE);
}

void green_led(uint8_t on){
	if(on){
		LED_PORT |= (1 << GREEN);
		}else{
		LED_PORT &= ~(1 << GREEN);
	}
}

void red_led(uint8_t on){
	if(on){
		LED_PORT |= (1 << RED);
		}else{
		LED_PORT &= ~(1 << RED);
	}
}
void white_led(uint8_t on){
	if(on){
		LED_PORT |= (1 << WHITE);
		}else{
		LED_PORT &= ~(1 << WHITE);
	}
}

void light_as(uint8_t x){
	/*
	x: uint8_t
	Least 3 bits represents if green, red or white led turns on.
	*/
	LED_PORT = (LED_PORT & 0xe3) | ((0x07 & x) << 2);
}

void lights_off(){
	light_as(0);
}

void indicate(LED_PATTERN pattern, uint8_t halt){
	switch (pattern){
		case POWERON:
		while(1){
			light_as(BVGREEN | BVRED);
			_delay_ms(500);
			light_as(0);
			_delay_ms(500);
			light_as(BVGREEN | BVRED);
			_delay_ms(500);
			light_as(0);
			_delay_ms(500);
			if(!halt) break;
		}
		break;
		case PRESSING_AC_SWITCH:
		light_as(BVRED | BVRED | BVWHITE);
		break;
		case CURRENT_STATUS_AC_OFF:
		light_as(BVGREEN);
		break;
		case CURRENT_STATUS_AC_ON:
		light_as(BVRED);
		break;
		case ERR01:
		while(1){
			light_as(BVWHITE | BVRED);
			if(!halt) break;
		}
		break;
		case ERR02:
		while(1){
			light_as(BVWHITE | BVGREEN);
			if(!halt) break;
		}
		break;
		case ERR03:
		while(1){
			light_as(BVWHITE);
			if(!halt) break;
		}
		break;
		case NOLED:
		default:
		while(1){
			light_as(0);
			if (!halt) break;
		}
		break;
	}
}