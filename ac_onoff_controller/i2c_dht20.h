/*
* i2c_dht20.h
*
* Created: 2024/08/21 19:16:53
*  Author: qwpmb
*/


#ifndef I2C_DHT20_H_
#define I2C_DHT20_H_

typedef struct {
	uint8_t state;
	uint8_t hum[3];
	uint8_t temp[3];
	uint8_t crc;
}DHT20_RECV;

void init_i2c();
int8_t get_temp_hum(float*, float*);



#endif /* I2C_DHT20_H_ */