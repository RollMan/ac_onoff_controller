#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include "powersave.h"
#include "debug.h"

#ifdef EBUG
#include <util/delay.h>
#endif //EBUG

/*
	Sleep time is 56 [sec.] = 8 [s] \times 7;
 */
const int SLEEP_CNT = 7;

void init_powersave(){
	cli();
	
	// Disable unnecessary hardwares
	power_adc_disable();
	power_spi_disable();
	
	// Init the sleep confiugration
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sei();
}

void establish_wdt(){
	cli();
	wdt_reset();
	MCUSR &= ~(1<<WDRF);
	WDTCSR |= (1 << WDCE | 1 << WDE);
	WDTCSR = (1 << WDIE) | (1<<WDP3) | (1 << WDP0);
	sei();
}

void revoke_wdt(){
	cli();
	wdt_reset();
	MCUSR &= ~(1 << WDRF);
	WDTCSR |= (1 << WDCE | 1 << WDE);
	WDTCSR = 0x00;
	sei();
}

void turn_into_sleep(){
	DEBUG_PRINT_STR("Start sleeping.\n");
	int wdt_cnt = 0;
	while(wdt_cnt < SLEEP_CNT){
		DEBUG_PRINT_STR("Start a new sleeping: ");
		DEBUG_PRINT_DEC(wdt_cnt);
		DEBUG_PRINT_STR("\n");
		#ifdef EBUG
		_delay_ms(500);			// wait for serial send.
		#endif //EBUG
		cli();
		sleep_enable();
		sleep_bod_disable();
		establish_wdt();
		sei();
		sleep_cpu();
		DEBUG_PRINT_STR("Stop the sleep.\n");
		wdt_cnt++;
	}
	sleep_disable();
	wdt_disable();
	DEBUG_PRINT_STR("Sleep counts satisfied.\n");
}

void run_main_sleep_alternation(void (*fn)()){
	init_powersave();
	while(1){
		fn();
		turn_into_sleep();
	}
}

ISR(WDT_vect){
	DEBUG_PRINT_STR("Interrupt triggered by WDT which was jumped from WDT_vect.\n");
}