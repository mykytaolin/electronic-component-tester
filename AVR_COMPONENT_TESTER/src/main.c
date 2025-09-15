#include "uart.h"
#include"setup.h"
#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include "capacitance.h"


int main(void){

  init_adc();
  timer_init();
  pins_init();
  usart_init(9600);

  uart_send_string("------ Capacitance Test Started ------\r\n");
  uart_send_string("F_CPU");
  uart_send_uint32_t(F_CPU);
  uart_send_string(" Hz\r\n");
  //DDRB = 0xFF;
  test_only_connections();

  while (1) {
    //PORTB ^= (1 << PB0);
	// _delay_ms(500);
    uart_send_string("\r\n----- Measurement -----\r\n");

    float capacitance = measure_capacitance_auto(2); // measure with second goldpin

    uart_send_string(" Final capacitance: ");
    uart_send_float(capacitance);
    uart_send_string(" uF\r\n");

    _delay_ms(2000);

  }
}
