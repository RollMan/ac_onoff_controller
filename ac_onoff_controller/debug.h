/*
* debug.h
*
* Created: 2024/09/06 12:21:16
*  Author: qwpmb
*/


#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef EBUG
#include "usart_io.h"
#define DEBUG_PRINT_STR(msg) send_str_usart(msg)
#define DEBUG_PRINT_DEC(dec) send_int8_t_decimal(dec)
#define DEBUG_PRINT_BYTE(chr) send_byte_usart(chr)
#define DEBUG_PRINT_HEX(hex) send_uint8_t_hex(hex)
#else
#define DEBUG_PRINT_STR(msg) do {} while(0)
#define DEBUG_PRINT_DEC(dec) do {} while(0)
#define DEBUG_PRINT_BYTE(chr)do {} while(0)
#define DEBUG_PRINT_HEX(hex) do {} while(0)
#endif



#endif /* DEBUG_H_ */