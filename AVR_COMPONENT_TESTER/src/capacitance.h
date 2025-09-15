#include <stdint.h>

void discharge_capacitor(uint8_t active_goldpin);
uint32_t measure_charge_time(uint8_t adc_channel, uint8_t resistor_pin);
uint16_t measure_voltage_mv(uint8_t adc_channel);
uint8_t find_voltage_index(uint16_t voltage_mv);
float measure_capacitance();
float calculate_capacitance();
float measure_capacitance_auto(uint8_t goldpin_number); /* switch structure : pin auto choose for any goldpin
preventing choosing wrong GND pin */ 
void set_others_goldpins_as_gnd(uint8_t active_goldpin); /* to be sure, that other pins are gnd*/
uint32_t get_micros(void); // for 16 MHz it's 0.0625 micro s
uint32_t measure_pulse_mode(uint8_t measure_pin, uint8_t resistor_pin, uint16_t *voltage_result);
