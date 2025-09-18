#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>
#include "resistance.h"
#include "config.h"
#include "uart.h"
#include "setup.h"

// V_ADC = VCC * (R unknown / ( R unknown * R known))
// R_unknown = R_known * ( VCC / V_ADC - 1)
uint8_t voltage_divider(void){
	uint8_t measure_pin , R_known, R_unknown;
	uint16_t voltage_mv = 0;
	float vcc = read_vcc();

	measure_pin = CH2_MEASURE_PIN;
	R_DDR |= (1 << CH2_RK2_PIN);
	R_PORT |= (1 << CH2_RK2_PIN);

	voltage_mv = measure_voltage_mv(measure_pin); // measuring V_ADC
	R_known = 10000;

	if(voltage_mv < 300){

		R_DDR &= ~(1 << CH2_RK2_PIN);
		R_PORT &= ~(1 << CH2_RK2_PIN);

		ON_DDR |= (1 << CH2_RO2_PIN);
		ON_PORT |= (1 << CH2_RO2_PIN);

		voltage_mv =  measure_voltage_mv(measure_pin);
		R_known = 100;

		ON_DDR &= ~(1 << CH2_RO2_PIN);
		ON_PORT &= ~(1 << CH2_RO2_PIN);

	}
	else if(voltage_mv > 3500){

		R_DDR &= ~(1 << CH2_RK2_PIN);
		R_PORT &= ~(1 << CH2_RK2_PIN);

		R_DDR |= (1 << CH2_RM2_PIN);
		R_PORT |= (1 << CH2_RM2_PIN);

		voltage_mv =  measure_voltage_mv(measure_pin);
		R_known = 1000000;

		R_DDR &= ~(1 << CH2_RM2_PIN);
		R_PORT &= ~(1 << CH2_RM2_PIN);
	}

	R_DDR &= ~(1 << CH2_RK2_PIN);
	R_PORT &= ~(1 << CH2_RK2_PIN);

	R_unknown = calculate_resistance(R_known, measure_pin);

	return R_unknown;
}

uint8_t calculate_resistance(uint8_t R_known, uint8_t channel ){

	uint8_t R_unknown;
	uint16_t adc_value = read_adc(channel);
	float vcc = read_vcc();


	R_unknown = R_known * (vcc / adc_value - 1);

	return R_unknown;

}

