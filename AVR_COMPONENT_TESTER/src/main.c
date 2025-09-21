#include "uart.h"
#include"setup.h"
#include "config.h"
#include <avr/io.h>
#include <util/delay.h>
#include "capacitance.h"


int main(void){
	uint8_t measurement_mode = 0; // 0 - capacitance, 1 - resistance

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
      DDRC &= ~(1 << CHNG_MEASURE_TYPE); // A3 as input
      PORTC |= (1 << CHNG_MEASURE_TYPE); // pull-up turn on

      if(!(PINC & (1 << CHNG_MEASURE_TYPE))){
    	  _delay_ms(50); // debounce

    	  if(!(PINC & (1 << CHNG_MEASURE_TYPE))){
    		  measurement_mode = !measurement_mode; // changing mode
    		  while(!(PINC & (1 << CHNG_MEASURE_TYPE))); // wait till untouched button
    	  }
      }

      if(measurement_mode == 0){
    	 uart_send_string("Mode: Capacitance\r\n");

		 float capacitance = measure_capacitance_auto(2); // measure with second goldpin

		 uart_send_string(" Final capacitance: ");
		 uart_send_float(capacitance);
		 uart_send_string(" uF\r\n");

		 _delay_ms(2000);
      }
      else{
    	 uart_send_string("Mode: Resistance\r\n");

    	 uint8_t resistance = voltage_divider();

    	 uart_send_string(" Final resistance: ");
		 uart_send_float(resistance);
		 uart_send_string(" Ohm\r\n");
		 _delay_ms(2000);
      }

    }
}
