/*
* ac_onoff_controller.c
*
* Created: 2023/07/22 14:23:53
* Author : qwpmb
*/

#include <avr/common.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#include "DHT.h"
#include "usart_io.h"
#include "config.h"

volatile int wdt_cnt = 0;

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

ISR(WDT_vect){
	sleep_disable();
	wdt_cnt++;
	sleep_enable();
}

void init_power_configuration(void){
	power_adc_disable();
	power_spi_disable();
	power_twi_disable();
}

unsigned int deg_to_clkcycle(int deg){
	unsigned int clk_cycle	= (unsigned int)(13 * deg + 1224);
	return clk_cycle;
}

void init_watchdogtimer(){
	cli();
	wdt_reset();
	MCUSR &= ~(1<<WDRF);
	WDTCSR |= (1 << WDCE | 1 << WDE);
	WDTCSR = (1 << WDIE) | (1<<WDP3) | (1 << WDP0);
	sei();
}


void revoke_watchdog_timer(){
	cli();
	wdt_reset();
	MCUSR &= ~(1<<WDRF);
	WDTCSR = 0x00;
	sei();
}

void turn_into_power_save(){
	wdt_cnt = 0;
	while(wdt_cnt < SLEEP_CNT){
		send_str_usart("going into powersave.");
		//_delay_ms(5000);  // wait until USART send completes.
		init_watchdogtimer();
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_enable();
		cli();
		sleep_bod_disable();
		sei();
		sleep_cpu();
		send_str_usart("waking up from powersave and checking the count.\r\n");
		send_int8_t_decimal(wdt_cnt); send_byte_usart(' '); send_int8_t_decimal(SLEEP_CNT);
		send_byte_usart('\r'); send_byte_usart('\n');
	}
	send_str_usart("desired sleep count satisfied.\r\n");
	sleep_disable();
	revoke_watchdog_timer();
}

void toggle_ac_switch(const int reset_deg, const int target_deg){
	unsigned int reset_cycle = deg_to_clkcycle(reset_deg);
	unsigned int target_cycle = deg_to_clkcycle(target_deg);
	OCR1B = target_cycle;
	_delay_ms(1500);
	OCR1B = reset_cycle;
	_delay_ms(1500);
}

float calc_discomfort_index(const float temperature, const float humidity){
	return 0.81 * temperature + 0.01 * humidity * (0.99 * temperature - 14.3) + 46.3;
}

int schmitt_trigger_if_switch_ac(int *switch_on, const float discomfort, const float high_thresh, const float low_thresh){
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

void timer16_write_ocr1a(uint16_t p, unsigned int i){
	unsigned char sreg;
	sreg = SREG;
	cli();
	OCR1A = i;
	SREG = sreg;
}

void timer16_write_ocr1b(uint16_t p, unsigned int i){
	unsigned char sreg;
	sreg = SREG;
	cli();
	OCR1B = i;
	SREG = sreg;
}


void init_timer16(){
	PWMOUT_DDR |= PWMOUT_BIT;
	TCCR1A = 0b00100010;  // Disable OC1A. Enable OC1B.
	TCCR1B = 0b00011001;  // no pre-scaler. Fast PWM, TOP OCR1A, Update OCR1x at BOTTOM.
	TCCR1C = 0;
	ICR1 = 20000;
	timer16_write_ocr1a(OCR1A, 20000);  // PWM cycle 20 ms. (prescale) / (clk) * OCR0A = (pwm_cycle) where prescale = 1, clk = 1MHz, pwm_cycle = 20ms.
}


int main(void)
{
	if(MCUSR & _BV(WDRF)){            // If a reset was caused by the Watchdog Timer...
		MCUSR &= ~_BV(WDRF);                 // Clear the WDT reset flag
		WDTCSR |= (_BV(WDCE) | _BV(WDE));   // Enable the WD Change Bit
		WDTCSR = 0x00;                      // Disable the WDT
	}
	init_timer16();
	init_usart();
	init_power_configuration();

	_delay_ms(1000);
	send_str_usart("MCU restarting.\r\n");
	
	DDRB |= 0x04;
	PORTB |= 0x04;

	int8_t dht11_res;
	int8_t *temp = &(int8_t){0}, *hum = &(int8_t){0};
	int switch_status = false;
	while (1)
	{
		/*
		send_int8_t_decimal(deg);
		send_byte_usart('\r');
		send_byte_usart('\n');
		*/
		
		// Temperature
		dht11_res = dht_GetTempUtil(temp, hum);
		if (dht11_res != 0){
			
			send_byte_usart('e');
			send_int8_t_decimal(dht11_res);
			}else{
			float discomfort = calc_discomfort_index(*temp, *hum);
			int toggle_switch = schmitt_trigger_if_switch_ac(&switch_status, discomfort, HIGH_THRESH, LOW_THRESH);
			if (toggle_switch){
				send_str_usart("toggling...\r\n");
				toggle_ac_switch(RESET_DEG, TARGET_DEG);
			}

			
			send_int8_t_decimal(*temp);
			send_byte_usart(' ');
			send_int8_t_decimal(*hum);
			send_byte_usart(' ');
			send_int8_t_decimal(discomfort);
			send_byte_usart(' ');
			send_int8_t_decimal(toggle_switch);
			send_byte_usart(' ');
			send_int8_t_decimal(switch_status);
			

		}
		turn_into_power_save();
	}
}

