/*
 * usart_io.h
 *
 * Created: 2023/08/22 17:35:51
 *  Author: qwpmb
 */ 


#ifndef USART_IO_H_
#define USART_IO_H_

void init_usart();
void send_byte_usart(unsigned char data);
void send_str_usart(char *str);
void send_int8_t_decimal(int8_t digit);



#endif /* USART_IO_H_ */