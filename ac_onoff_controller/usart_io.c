/*
 * usart_io.c
 *
 * Created: 2023/08/22 17:32:13
 *  Author: qwpmb
 */ 

#include <avr/io.h>
#include "usart_io.h"
#include "util.h"

const uint8_t ASCII_OFFSET = 48;
const uint8_t HEX_OFFSET = 7;

void init_usart(){
	DDRD |= (1 << PORTD1);
	UBRR0H = (unsigned char)(BAUD_UBRR >> 8);
	UBRR0L = (unsigned char)(BAUD_UBRR);
	UCSR0C = 0x06;
	UCSR0B = 0x08;
}

void send_byte_usart(unsigned char data){
	while (!(UCSR0A & 0x20));
	UDR0 = data;
}

void send_str_usart(char *str){
	while(1){
		char c = *str;
		if(!*str++) return;
		send_byte_usart(c);
	}
}

#define STRLEN (8)

void send_int8_t_decimal(int8_t digit){
	int8_t digit_abs = ABS(digit);
	int8_t digit_col_cnt = digit_abs;
	char str[STRLEN];
	int8_t col = 0;
	for(int i = 0; i < STRLEN; i++){
		str[i] = '\0';
	}
	
	while (digit_col_cnt > 0){
		col++;
		digit_col_cnt /= 10;
	}
	
	if(col == 0){
		send_byte_usart('0');
	}else{
		int8_t prefix=0;
		if ( digit < 0 ){
			prefix=1;
			str[0] = '-';
		}
		for(int i = 0; i < col; i++){
			unsigned char c = (digit_abs % 10) + ASCII_OFFSET;
			str[col - i + prefix - 1] = c;
			digit_abs /= 10;
		}
	}
	send_str_usart(str);
}

void send_uint8_t_hex(uint8_t digit){
	char str[STRLEN];
	for(int i = 0; i < STRLEN; i++){
		str[i] = '\0';
	}
	str[0] = '0';
	str[1] = 'x';
	for(int i = 3; i >= 2; i--){
		unsigned char c = (digit % 16) + ASCII_OFFSET;
		str[i] = c >= 58 ? c + HEX_OFFSET : c;
		digit /= 16;
	}
	send_str_usart(str);
}