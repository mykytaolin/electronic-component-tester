#include "uart.h"
#include <stdlib.h>
#include <util/delay.h>
#include <avr/io.h>
// function wrote according to datasheet ATmega 328P
// https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/DataSheets/ATmega48A-PA-88A-PA-168A-PA-328-P-DS-DS40002061B.pdf

void usart_init(uint16_t baud_rate){
    uint16_t UBRR = (F_CPU / 16 / baud_rate) - 1;
    UBRR0H = (UBRR >> 8); // upper 8 bits
    UBRR0L = UBRR; // lower 8 bits

    UCSR0B = (1 << TXEN0); // transmit enable for UART0
    UCSR0C = (1 << USBS0) | (3 << UCSZ00); // 8 data 2 stop
}

void usart_transmit(unsigned char data){
    // wait for empty transmit buffer
    while(!(UCSR0A & (1 << UDRE0)));

    // put data into buffer, sends data
    UDR0 = data;
}

void uart_send_string(const char *str){

    while(*str){
        usart_transmit(*str++);
    }
}

void uart_send_float(float value){
    char buffer[6];
    dtostrf(value, 6, 2, buffer); // convert float to string
    // dtostrf(value, width, prec, str)
    // value - number, width - min width of output
    // precision - number after the comma
    // str - pointer to character buffer
    uart_send_string(buffer);
}

void uart_send_uint16_t(uint16_t value){
    char buffer[6];
    itoa(value, buffer, 10); // conversion to string
    //itoa(number, place to save result, system)
    uart_send_string(buffer);

}
void uart_send_uint32_t(uint32_t value){
    char buffer[6];
    itoa(value, buffer, 10);
    uart_send_string(buffer);
}
