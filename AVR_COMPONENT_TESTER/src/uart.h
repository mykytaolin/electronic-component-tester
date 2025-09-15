#ifndef UART_H
#define UART_H

#include <avr/io.h>

void usart_init(uint16_t baud_rate);
void usart_transmit(unsigned char data);
void uart_send_uint16_t(uint16_t value);
void uart_send_string(const char *str);
void uart_send_float(float value);
void uart_send_uint32_t(uint32_t value);
#endif