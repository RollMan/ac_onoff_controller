/*
 * usart_io.h
 *
 * Created: 2023/08/22 17:35:51
 *  Author: qwpmb
 */ 


#ifndef USART_IO_H_
#define USART_IO_H_

#define BAUD (1200)
#define BAUD_UBRR (51)

void init_usart();
void send_byte_usart(unsigned char);
void send_str_usart(char*);
void send_int8_t_decimal(int8_t);
void send_uint8_t_hex(uint8_t);



#endif /* USART_IO_H_ */