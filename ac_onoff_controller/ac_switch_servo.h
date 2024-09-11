/*
 * ac_switch_servo.h
 *
 * Created: 2024/09/08 17:20:07
 *  Author: qwpmb
 */ 


#ifndef AC_SWITCH_SERVO_H_
#define AC_SWITCH_SERVO_H_

#define SERVO_POW_DDR (DDRB)
#define SERVO_POW_PORT (PORTB)
#define SERVO_POW_BIT (1 << 2)
#define SERVO0_DDR (DDRB)
#define SERVO0_PORT (PORTB)
#define SERVO0_SIGNAL_BIT (1 << 1)
#define SERVO0_COMPARE_REGISTER (OCR1A)
#define SERVO1_DDR (DDRC)
#define SERVO1_PORT (PORTC)
#define SERVO1_SIGNAL_BIT (1 << 0)

/*
 * Servo 0 timers:
 * - OCR1A
*/

typedef enum {
	SERVO0,
	SERVO1,
}SERVO;

void init_servo_pwm();
void rotateto(const int deg, SERVO servo_idx);
void enable_servo_pow();
void disable_servo_pow();


#endif /* AC_SWITCH_SERVO_H_ */