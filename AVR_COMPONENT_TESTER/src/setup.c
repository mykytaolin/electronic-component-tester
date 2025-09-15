#include "config.h"
#include "setup.h"
#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"

void init_adc() {
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // turn on ADC and init frequancy factor 128
  ADMUX = (1 << REFS0); // set 5V AVcc as reference voltage

  // turn on digital ouput
  DIDR0 = (1 << ADC0D) | (1 << ADC1D) | (1 << ADC2D);
  // turn of pull up
  PORTC &= ~((1 << PC0) | (1 << PC1) | (1 << PC2));
}

void timer_init(void){ // timer cofiguration 16 MHz
	TCCR1B = (1 << CS10) | (0 << CS11) | (1 << CS12); //table 15-5 15.11 datasheet
	TCNT1 = 0; // reset counter
	//TIMSK1 = 0;
}

void pins_init(void){

  R_DDR = 0x00;
  R_PORT = 0x00;

}

uint16_t read_adc(uint8_t channel){
	ADMUX = (1 << REFS0) | (channel & 0x07);
	ADCSRA |= (1 << ADSC);

	while(ADCSRA & (1 << ADSC));
	return ADC;

}
// function that filtrates noise (avg final result)
uint16_t read_adc_avg(uint8_t channel, uint8_t sample){

	uint32_t sum = 0;

	for(uint8_t i = 0; i < sample; i++){
		sum += read_adc(channel);
		_delay_ms(100);
	}
	return sum / sample;
}

void test_resistor(uint8_t resistor_pin){
	R_DDR |= (1 << resistor_pin);
	R_PORT |= (1 << resistor_pin);

	_delay_ms(1000);

	uart_send_string("Resistor test: PIN ");
	uart_send_uint16_t(resistor_pin);
	uart_send_string("Acrive PIN \r\n");

	R_DDR &= ~ (1 << resistor_pin);
	R_PORT &= ~ (1 << resistor_pin);
}
void test_connections(uint8_t measure_pin){
	uint8_t original_ddr = DDRC;
	uint8_t original_port = PORTC;

	DDRC = 0x00;
	PORTC = 0x00;

	DDRC |= (1 << measure_pin);
	PORTC |= (1 << measure_pin);

	_delay_ms(1);

	DDRC &= ~(1 << measure_pin);
	PORTC |= (1 << measure_pin);

	_delay_us(10);

	if(PINC & (1 << measure_pin)) {
		uart_send_string("Capacitor Not Connected\r\n");
	}
	else{
		uart_send_string("Capacitor connected\r\n");
	}

	DDRC = original_ddr;
	PORTC = original_port;

	_delay_us(10);
}

void debug_portc_state(const char* label){

	uart_send_string(label);
	uart_send_string(": DDRC= ");
	uart_send_uint16_t(DDRC);
	uart_send_string(", PORTC= ");
	uart_send_uint16_t(PORTC);
	uart_send_string(", PINC= ");
	uart_send_uint16_t(PINC);
	uart_send_string("\r\n");

}
void test_only_connections(){
	uart_send_string(" Testing only connection detection: ");

	for(uint8_t i = 0; i < 3; i++){
		uart_send_string("Pin ");
		uart_send_uint16_t(i);
		uart_send_string(": ");
		test_connections(i);
		_delay_ms(1000);
	}
}

void check_pin_state(uint8_t pin){
	DDRC &= ~(1 << pin);
	PORTC &= ~(1 << pin);
	_delay_us(10);

	uart_send_string("Pin ");
	uart_send_uint16_t(pin);
	uart_send_string(" state: ");

	if(PINC & (1 << pin)){
		uart_send_string("High (not connected)\r\n");
	}
	else{
		uart_send_string("Low (connected to GND)\r\n");
	}
}

void check_resistors(){
	uart_send_string("Testing resistors\r\n");
	//checking 680 Ohm resistor
	R_DDR |= (1 << CH2_RL2_PIN);
	R_PORT |= (1 << CH2_RL2_PIN);
	_delay_ms(100);
	uart_send_string("680 Ohm resistor is ON\r\n");

	R_PORT &= ~(1 << CH2_RL2_PIN);
	R_DDR &= ~(1 << CH2_RL2_PIN);

	//checking 470 kOhm resistor
	R_DDR |= (1 << CH2_RH2_PIN);
	R_PORT |= (1 << CH2_RH2_PIN);
	_delay_ms(100);
	uart_send_string("470 kOhm resistor is ON\r\n");

	R_PORT &= ~(1 << CH2_RH2_PIN);
	R_DDR &= ~(1 << CH2_RH2_PIN);

}
