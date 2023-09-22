/*
 * utils.h
 *
 * Created: 2023/08/22 17:33:37
 *  Author: qwpmb
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#define FOSC (F_CPU)
#define BAUD (1200)
#define BAUD_UBRR (51)
#define PWMOUT_DDR (DDRB)
#define PWMOUT_BIT (1 << 2)

#define HIGH_THRESH (79)
#define LOW_THRESH (78)
#define SLEEP_CNT (38)
/*
 * The number of watchdog-powered sleep count.
 * Because the watchdog cycle is 8.0s (1024k clock div),
 * The actual sleep time (or the discomfort index check cycle)
 * will be:
 * sleep_time = SLEEP_CNT * 8 [sec].
 *
 * e.g. SLEEP_CNT should be 37.5 ~ 38 when 5 min. sleep desired.
*/
#define RESET_DEG (-65)
#define TARGET_DEG (-30)


#define ABS(val)        val < 0 ? -val : val
#define true (1)
#define false (0)


#endif /* CONFIG_H_ */