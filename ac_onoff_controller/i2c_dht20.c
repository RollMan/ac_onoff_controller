/*
* i2c_dht20.c
*
* Created: 2024/08/09 17:55:19
*  Author: qwpmb
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>
#include "debug.h"
#include "i2c_dht20.h"


uint8_t start_measurement(void);
uint8_t i2c_write_data(uint8_t*, uint8_t);
int8_t i2c_read_data(uint8_t*, uint8_t);

const uint8_t DHT20_SLAVE_ADDRESS = 0x38;
// const uint8_t I2C_WRITE_CONTROL_BIT = 0;
// const uint8_t I2C_READ_CONTROL_BIT = 1;
typedef enum {
	I2C_WRITE_CONTROL_BIT = 0,
	I2C_READ_CONTROL_BIT = 1,
} I2C_SLA_CONTROL_BIT;
#define DHT20_SLAVE_ADDRESS_WITH_RW(RW) ((DHT20_SLAVE_ADDRESS << 1) | RW)
#define WAIT_FOR_TWINT() do { \
	while (!(TWCR & (1 << TWINT))) ;	\
} while(0)

typedef enum {
	SENT_START_COND = 0x08,
	SENT_RESTART_COND = 0x10,
	SENT_SLA_W_ACK = 0x18,
	SENT_SLA_W_NACK = 0x20,
	SENT_DATA_ACK = 0x28,
	SENT_DATA_NACK = 0x30,
	LOST_ARBITRATION = 0x38,
	SENT_SLA_R_ACK = 0x40,
	SENT_SLA_R_NACK = 0x48,
	RECV_DATA_ACK = 0x50,
	RECV_DATA_NACK = 0x58,
} TWI_TX_STATUS;

void init_i2c(){
	// Enable internal pull-ups
	PORTC |= 0x30;
	TWSR |= 0x03;
	TWBR = 0x83;		// SCL 961.54 [Hz] = 1 [MHz] / (16 + 2 * TWBR * prescale[TWPS]) where prescale[TWPS] = prescale[3] = 64 and TWBR = 8
	// all i2c communication are on sync; no interruption are handled.
	// sei(); // allow interrupt
	
}

uint8_t check_twi_status(TWI_TX_STATUS kind){
	TWI_TX_STATUS twi_status = (TWSR & 0xF8);
	int res = twi_status == kind;
	/*
	if (!res) {
		DEBUG_PRINT_STR("The TWI status is not match.\nexpected, actual = ");
		DEBUG_PRINT_HEX(kind);
		DEBUG_PRINT_STR(" ");
		DEBUG_PRINT_HEX(twi_status);
		DEBUG_PRINT_STR("\n");
	}
	*/
	return res;
}

void issue_stop_condition(void){
	TWCR = (TWCR | (1 << TWINT) | (1 << TWSTO) | (1 << TWEN)) & ~(1 << TWSTA);
}

uint8_t issue_start_condition(I2C_SLA_CONTROL_BIT rw){
	TWCR = (TWCR | (1 << TWINT) | (1 << TWSTA) | (1 << TWEN)) & ~(1 << TWSTO);
	WAIT_FOR_TWINT();
	if (!check_twi_status(SENT_START_COND)){
		return 1;
	}
	TWDR = DHT20_SLAVE_ADDRESS_WITH_RW(rw);
	TWCR |= (1 << TWINT) | (1 << TWEN);
	WAIT_FOR_TWINT();
	if (!check_twi_status(rw == I2C_WRITE_CONTROL_BIT ? SENT_SLA_W_ACK : SENT_SLA_R_ACK)){
		issue_stop_condition();
		return 2;
	}
	return 0;
}


uint8_t i2c_write_data(uint8_t *data, uint8_t len){
	uint8_t start_condition_succeeded = issue_start_condition(I2C_WRITE_CONTROL_BIT);
	if (start_condition_succeeded != 0) {
		return start_condition_succeeded;
	}
	
	// Start sending data
	for(int i = 0; i < len; i++){
		TWDR = data[i];
		TWCR = (1 << TWINT) | (1 << TWEN);
		WAIT_FOR_TWINT();
		if (!check_twi_status(SENT_DATA_ACK)){
			if(check_twi_status(SENT_DATA_NACK)) {
				issue_stop_condition();
			}
			return 3;
		}
	}
	issue_stop_condition();
	return 0;
}

// Returns bytes received.
int8_t i2c_read_data(uint8_t *data, uint8_t len){
	uint8_t start_condition_succeeded = issue_start_condition(I2C_READ_CONTROL_BIT);
	if (start_condition_succeeded != 0) {
		return start_condition_succeeded;
	}
	
	int8_t received = 0;
	for(; received < len; received++){
		DEBUG_PRINT_STR("Receiving a byte of ");
		DEBUG_PRINT_DEC(received);
		DEBUG_PRINT_STR("\n");
		// TODO: move following code into another function and use it for sending.
		if (received + 1 == len){
			TWCR = (1 << TWINT) | (1 << TWEN);
		}else{
			TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN);
		}
		WAIT_FOR_TWINT();
		data[received] = TWDR;
		if (check_twi_status(RECV_DATA_NACK)) {
			break;
			} else if (check_twi_status(RECV_DATA_ACK)) {
			continue;
			} else {
			return -1;
		}
	}
	issue_stop_condition();
	return received;
}

uint8_t start_measurement(){
	uint8_t START_COMMAND[] = {0xAC, 0x33, 0x00};
	uint8_t res = i2c_write_data(START_COMMAND, 3);
	_delay_ms(100);
	return res;
}

uint8_t read_dht20(DHT20_RECV *dht20_recv){
	const uint8_t DATASIZE = 7;
	uint8_t recv[DATASIZE];
	int8_t read_data_success	= i2c_read_data(recv, DATASIZE);
	if (read_data_success < 0){
		while(1){
			DEBUG_PRINT_STR("Failed reading data from DHT via I2C. ");
			DEBUG_PRINT_DEC(read_data_success);
			DEBUG_PRINT_BYTE('\n');
			_delay_ms(1000);
		}
	}
	
	dht20_recv->state = recv[0];
	dht20_recv->hum[0] = recv[1] >> 4;
	dht20_recv->hum[1] = (recv[1] << 4) | (recv[2] >> 4);
	dht20_recv->hum[2] = (recv[2] << 4) | (recv[3] >> 4);
	dht20_recv->temp[0] = recv[3] & 0x0f;
	dht20_recv->temp[1] = recv[4];
	dht20_recv->temp[2] = recv[5];
	dht20_recv->crc = recv[6];
	return 0;
}

float shift_20_bits(uint8_t high, uint8_t mid, uint8_t low){
	float fractional_part = (high * powf(2, 16) + mid * powf(2, 8) + low) / pow(2, 20);
	return fractional_part;
}

void convert_dht20_temp_hum(DHT20_RECV *recv_raw, float* hum, float* temp){
	*hum = shift_20_bits(recv_raw->hum[0], recv_raw->hum[1], recv_raw->hum[2]) * (float)100.0;
	*temp = shift_20_bits(recv_raw->temp[0], recv_raw->temp[1], recv_raw->temp[2]) * (float)200.0 - (float)50.0;
}

uint8_t check_crc(uint8_t crc){
	return 0;
}

int8_t get_temp_hum(float* temp, float* hum){
	uint8_t start_success = start_measurement();
	if (start_success != 0){
		DEBUG_PRINT_STR("Failed to issue the start command to DHT20. ");
		DEBUG_PRINT_DEC(start_success);
		DEBUG_PRINT_STR("\n");
		return 1;
	}
	DHT20_RECV recv_raw;
	uint8_t recv_success = read_dht20(&recv_raw);
	if (recv_success != 0){
		DEBUG_PRINT_STR("Failed to read data from DHT20.\n");
		return 2;
	}
	uint8_t crc_success = check_crc(recv_raw.crc);
	if (crc_success != 0){
		DEBUG_PRINT_STR("CRC check failed.\n");
		return 3;
	}
	convert_dht20_temp_hum(&recv_raw, hum, temp);
	return 0;
}