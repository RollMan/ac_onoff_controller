#include <avr/io.h>
#include <avr/interrupt.h>
#include "ac_switch_servo.h"
#include "debug.h"

#define WRITE16B(p, data) \
do { \
	unsigned char sreg; \
	sreg = SREG; \
	cli(); \
	p = data; \
	SREG = sreg; \
} \
while(0)


void init_servo_pwm(){
	SERVO_POW_DDR |= SERVO_POW_BIT;
	SERVO0_DDR |= SERVO0_SIGNAL_BIT;
	SERVO1_DDR |= SERVO1_SIGNAL_BIT;

	TCCR1A = 0b10000010;  // Disable OC1B Enable OC1A (PB1).
	TCCR1B = 0b00011001;  // no pre-scaler. Fast PWM, TOP OCR1A, Update OCR1x at BOTTOM.
	TCCR1C = 0;
	WRITE16B(ICR1, 20000);
	WRITE16B(OCR1A, 20000);  // PWM cycle 20 ms. (prescale) / (clk) * OCR0A = (pwm_cycle) where prescale = 1, clk = 1MHz, pwm_cycle = 20ms.
}

void enable_servo_pow(){
	SERVO_POW_PORT |= SERVO_POW_BIT;
}

void disable_servo_pow(){
	SERVO_POW_PORT &= ~SERVO_POW_BIT;
}

unsigned int deg_to_clkcycle(int deg){
	unsigned int clk_cycle	= (unsigned int)(13 * deg + 1224);
	return clk_cycle;
}


void rotateto(const int deg, SERVO servo_idx){
	uint16_t cycle = deg_to_clkcycle(deg);
	switch(servo_idx){
		case SERVO0:
		WRITE16B(SERVO0_COMPARE_REGISTER, cycle);
		break;
		case SERVO1:
		DEBUG_PRINT_STR("Servo1 is not in this version.\n");
		break;
		default:
		DEBUG_PRINT_STR("Unexpected servo index refered. ");
		DEBUG_PRINT_DEC(servo_idx);
		DEBUG_PRINT_STR("\n");
	}
}