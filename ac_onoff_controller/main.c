/*
* aconoffcontroller_board_test.c
*
* Created: 2024/08/09 17:37:28
* Author : qwpmb
*/

#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdbool.h>
#include "i2c_dht20.h"
#include "usart_io.h"
#include "ac_switch_servo.h"
#include "debug.h"
#include "powersave.h"
#include "indicator.h"
#include "config.h"

void init();
void main_routine();



uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
void get_mcusr(void) \
__attribute__((naked)) \
__attribute__((section(".init3")));
void get_mcusr(void)
{
	mcusr_mirror = MCUSR;
	MCUSR = 0;
	wdt_disable();
}

int main(void)
{
	#ifdef EBUG
	init_usart();
	DEBUG_PRINT_STR("MCU starts as DEBUG mode.\n");
	#endif //EBUG
	init();
	run_main_sleep_alternation(main_routine);
}


void init(){
	init_servo_pwm();
	init_i2c();
	init_indicators();
	indicate(POWERON, false);
	indicate(NOLED, false);
}


float calc_discomfort_index(const float temperature, const float humidity){
	return 0.81 * temperature + 0.01 * humidity * (0.99 * temperature - 14.3) + 46.3;
}

int8_t schmitt_trigger_if_switch_ac(int8_t *switch_on, const float discomfort, const float high_thresh, const float low_thresh){
	/*
	* Determine if A/C should be switched (true) or not (false).
	*/
	if (discomfort > high_thresh && *switch_on == false){
		*switch_on = true;
		return true;
		}else if (discomfort < low_thresh && *switch_on == true){
		*switch_on = false;
		return true;
	}
	return false;
}

int8_t switch_status = false;
void main_routine(void){
	DEBUG_PRINT_STR("Running a main routine.\n");
	float temp, hum, discomfort;
	int8_t success_read_temp_hum = get_temp_hum(&temp, &hum);
	if(success_read_temp_hum != 0){
		indicate(ERR01, false);
		_delay_ms(3000);
		indicate(NOLED, false);
	}
	
	discomfort = calc_discomfort_index(temp, hum);
	int8_t toggle_switch = schmitt_trigger_if_switch_ac(&switch_status, discomfort, HIGH_THRESH, LOW_THRESH);
	if(toggle_switch){
		indicate(PRESSING_AC_SWITCH, false);
		enable_servo_pow();
		_delay_ms(HOLD_ANGLE_TIME_MSEC);		// Wait until the servo is well powered.
		rotateto(TARGET_DEG, SERVO0);
		_delay_ms(HOLD_ANGLE_TIME_MSEC);		// Hold the A/C switch for a while.
		rotateto(RESET_DEG, SERVO0);
		_delay_ms(HOLD_ANGLE_TIME_MSEC);		// Ensure the servo motor angle resets.
		disable_servo_pow();
		indicate(NOLED, false);
	}
	
	// DEBUG logs
	DEBUG_PRINT_STR("temp, hum, discomfort = ");
	DEBUG_PRINT_DEC((int8_t)temp); DEBUG_PRINT_STR(", ");
	DEBUG_PRINT_DEC((int8_t)hum); DEBUG_PRINT_STR(", ");
	DEBUG_PRINT_DEC((int8_t)discomfort); DEBUG_PRINT_STR("\n");
	DEBUG_PRINT_STR("swich status "); DEBUG_PRINT_DEC(switch_status);
	DEBUG_PRINT_STR(", toggled? "); DEBUG_PRINT_DEC(toggle_switch); DEBUG_PRINT_STR("\n");
}
