/*
* buttons.c
*
* Created: 2024/09/11 18:57:49
*  Author: qwpmb
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "buttons.h"
#include "debug.h"



/*
* BUTTONS
* -------
* SW2 BLUE PD7 PCI23
* SW3 RED PD6 PCI22
* SW4 GREEN PD5 PCI21
* They triggers PCI2.
*/
#define BUTTON_DDR (DDRD)
#define BUTTON_PORT (PORTD)
#define BUTTON_PIN (PIND)
#define BLUE (1 << PORTD7)
#define RED (1 << PORTD6)
#define GREEN (1 << PORTD5)

int8_t new_pin_status = -1;

void init_buttons(){
	DDRD &= ~(BLUE | RED | GREEN);
	PORTD |= (BLUE | RED | GREEN);
	
	PCICR = 0x04;		// Enable PCI2.
	PCMSK2 = 0xE0;
	
	sei();
}

int8_t get_pin_status(){
	int8_t ret = new_pin_status;
	new_pin_status = -1;
	return ret;
}

ISR(PCINT2_vect){
	uint8_t pin_d_status = BUTTON_PIN;
	DEBUG_PRINT_STR("External interrupt triggered.\n");
	if(!(pin_d_status & BLUE)){
		DEBUG_PRINT_STR("The blue button pressed. Setting the current A/C status on.\n");
		new_pin_status = true;
	}
	if(!(pin_d_status & RED)){
		DEBUG_PRINT_STR("The blue button pressed. Setting the current A/C status off.\n");
		new_pin_status = false;
	}
}