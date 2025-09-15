#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>
#include "capacitance.h"
#include "config.h"
#include "uart.h"
#include "setup.h"
#pragma GCC diagnostic ignored "-Woverflow"

/* Math equation for capacitor charging

**The voltage at the capacitor grows with Uc = VCC * (1 - e**(-t/T))
**The voltage 1.3V is reaching 
    1.3 = 5 * (1 - e**(-t/T)) -> 1.3 / 5 = 1 - e**(-t / T)
    0.26 = 1 - e**(-t / T) -> e**(-t / T) = 1 - 0.26
    e**(-t / T) = 0.74 -> -t / T = ln(0.74)
    t = -ln(0.74) * T -> t = 0.3011 * T
** Time constant T = C * R
** C = T / R -> C = t / (0.3011 * R)
** constant H_FAKT = 1 / (0.3011 * 470000) = 7.07 μF or 707 pF

*/
/* Big capacitors (f.ex. C > 100 μF) will be measured with pulse charging method through smaller resistor 680 Ohm
For example: capacitor 1000 μF and resistor 470000 Ohm -> constant T = 470 s 
Capacitor will be charged through 680 Ohm for 10ms (it's one pulse)
if voltage < 300mV = next pulse
if voltage > 300mV finish measurement
300 mV because in analog circuits noises exists in between 10 - 100mV and func is more linear on higher voltage

                          *****EXAMPLE*****

One pulse for t = 10ms , R = 680 Ohm and C = 1000 μF give us: 
T = R * C = 680 * 0.001 = 0.68 s
t/T = 0.01 / 0.68 = 0.147
U1 = Vcc * (1 - e**(-0.01/(680 * C))) -> U1 = 5 * (1 - e**(-0.147)) = 0.683 V or 683mV so close to 680mV

To conclude:
One pulse have length of 10 ms
After every load-pulse the voltage of capacitor is measured
If the voltage is > 300mV calculating capacity by interpolating
values of the RLtab and multiply by load pulses * 10
RLtab - table of results capacitor charging equations like the next one example

                          *****EXAMPLE*****
Uc = Vcc * (1 - e**(-t/(R*C))), where VCC = 5V = 5000mV
t = N * 0.01s
R = 680 Ohm
scailing factor = needed for avoid floating-point numbers
C - ?

Uc = 5000 * (1 - e**(-t/(680 * C)))
Uc / 5000 = 1 - e**(-t/(680 * C))
e**(-t /(680 * c)) = 1 - (Uc / 5000)
-t/(680 * C) = ln(1 - (Uc / 5000))
C = -t/(680 * ln(1 - (Uc / 5000))))
C = ((N * 0.01)/(680 * ln(1 - (Uc / 5000))))) * scaling factor, for Uc = 400mV
C = 1.76368 * 10**-4

*/
/*Table of voltage for 680 Ohm resistor : 300 325 350 375 400 425 450 475 500
 525 550 575 600 625 650 675 700 725 750 775 800 825 850 875 900 925 950 975
 1000 1025 1050 1075 1100 1125 1150 1175 1200 1225 1250 1275 1300 1325 1350 1375
 1400 mV*/

const uint16_t RLtab[] MEM_TEXT  = {23800, 21900, 20300, 18900, 17600, 16600, 15600,
14700, 14000, 13300, 12600, 12000, 11500, 11000, 10600, 10100, 9800, 9400, 9000, 8700,
8400, 8200, 7900, 7600, 7400, 7200, 7000, 6800, 6600, 6400, 6200, 6100, 5900, 5800,
5600, 5500, 5400, 5200, 5100, 5000, 4900, 4800, 4700, 4600, 4500};


uint16_t measure_voltage_mv(uint8_t adc_channel){
	uint16_t voltage_mv = 0;

	for(uint8_t i = 0; i < 10; i++){ // get an avg result of 10 measurement
		voltage_mv += read_adc(adc_channel) * (5000 / 1023.0); //1023 equal 10 bits in ADC (max value)
		// 5000 equals to ADC reference voltage (Vref 5V)
	}

	return voltage_mv / 10;
}


uint8_t find_voltage_index(uint16_t voltage_mv){

  if (voltage_mv < 300){
    return 0; // 1st element in RLtab
  }
  if (voltage_mv > 1400){
    return 44; // 44th element in RLtab
  }
  return (voltage_mv - 300) / 25;
}


void discharge_capacitor(uint8_t active_goldpin){
	if(active_goldpin != CH1_MEASURE_PIN){// if active pin not equal to A1

		if(measure_voltage_mv(CH1_MEASURE_PIN) > 100){ // measuring voltage on channel 1
			DIS_CH_DDR |= (1 << CH1_DIS_CH_PIN); // if it's > 100mV
			DIS_CH_PORT &= ~(1 << CH1_DIS_CH_PIN);

			for(uint8_t i = 0; i < 10; i++){  // discharging capacitor
				_delay_ms(100);
				if(measure_voltage_mv(CH1_MEASURE_PIN) < 50) break;
			}

			DIS_CH_DDR &= ~(1 << CH1_DIS_CH_PIN); // set as output (high impedance)

		}
		if(active_goldpin != CH3_MEASURE_PIN){ //if active pin not equal to A2
			DIS_CH_DDR |= (1 << CH3_DIS_CH_PIN); // if voltage on ch 1 < 100mV
			DIS_CH_PORT &= ~(1 << CH3_DIS_CH_PIN); // using channel 2 for discharging

			for(uint8_t i = 0; i < 10; i++){  // discharging capacitor
				_delay_ms(100);
				if(measure_voltage_mv(CH3_MEASURE_PIN) < 50) break;
			}

			DIS_CH_DDR &= ~(1 << CH3_DIS_CH_PIN); // set as output (high impedance)

		}
	}
}


uint32_t get_micros(void){ // time function
  return TCNT1 * 64; // 1 tick = 0.625 mikro s dla 16 MHz
  //return TCNT1 * (1000000.0 / F_CPU); // = 62.5 ns per tick
}

uint32_t measure_charge_time(uint8_t adc_channel, uint8_t resistor_pin){
  //static uint16_t overflow_count = 0; // buffer overflow handling

  TCNT1 = 0;
  R_DDR |= (1 << resistor_pin);
  R_PORT |= (1 << resistor_pin);
    //R_DDR |= (1 << CH2_RL2_PIN);
    //R_PORT |= (1 << CH2_RL2_PIN);

  discharge_capacitor(adc_channel); // discharging before measuring

  // uint32_t start_time = get_micros(); // measure start time
  // uint32_t timeout = start_time + 10000000; // 10s timeout

  uint16_t target_adc = (400 * 1023) / 5000; // define voltage limit
  uint32_t timeout_ticks = 10000000 * 16; // 10s in tacts
  

  while(TCNT1 < timeout_ticks){
	if (read_adc(adc_channel) >= target_adc) break;
    // uint16_t adc_value = read_adc(adc_channel);

    // if (adc_value >= target_adc) break; // if voltage bigger than limit
    // if (get_micros() >= timeout) break; // time is running out
  }

  if (TCNT1 >= timeout_ticks){
	  uart_send_string("TIMEOUT");
  }
  // uint32_t end_time = get_micros();

  R_PORT &= ~(1 << resistor_pin);
  R_DDR &= ~(1 << resistor_pin);

  // return end_time - start_time;
  return TCNT1 * 64;
}

float measure_capacitance_auto(uint8_t goldpin_number){
  uint8_t measure_pin , r_l_pin, r_h_pin;
  float capacitance;
  uint8_t used_resistor_pin;
  uint16_t voltage_mv = 0;
  uint32_t time_us = 0;

  measure_pin = CH2_MEASURE_PIN;
  r_h_pin = CH2_RH2_PIN;
  r_l_pin = CH2_RL2_PIN;
  // decide which channel will measure
  //switch (goldpin_number)
  //{
  //case 1 : // first channel
  //  measure_pin = CH1_MEASURE_PIN;
  // r_l_pin = CH1_RL1_PIN;
  //  r_h_pin = CH1_RH1_PIN;
  //  break;
  //case 2: // second channel
  //  measure_pin = CH2_MEASURE_PIN;
  //  r_l_pin = CH2_RL2_PIN;
  //  r_h_pin = CH2_RH2_PIN;
  //  break;
  //case 3: // third channel
  //  measure_pin = CH3_MEASURE_PIN;
  //  r_l_pin = CH3_RL3_PIN;
  //  r_h_pin = CH3_RH3_PIN;
  //  break;
  //default:
  //  return 0.0;
  //}
  check_resistors(); // checking resistors of channel 2
  check_pin_state(measure_pin);
  set_others_goldpins_as_gnd(goldpin_number);
  /*-------------------------------------------------------------
    						Calibration of ZERO*/
  //uint32_t zero_time = measure_charge_time(measure_pin, r_h_pin);
  //uint16_t zero_voltage = measure_voltage_mv(measure_pin);
  //uart_send_string("ZERO CALIBRATION: ");
  //uart_send_uint32_t(zero_time);
  //uart_send_string(" us, ");
  //uart_send_uint16_t(zero_voltage);
  //uart_send_string(" mV\r\n");

  //---------------------------------------------------------------

  debug_portc_state("Before measurements\r\n");
  // uint32_t time_us = (measure_charge_time(measure_pin, r_l_pin, measure_pin));// - (zero_time * 0.8);
  time_us = (measure_charge_time(measure_pin, r_h_pin));// - (zero_time * 0.8);

  if(time_us < 10000){
	uart_send_string("Using 470kOhm\r\n");
	used_resistor_pin = r_h_pin;
    //time_us = measure_charge_time(measure_pin, r_h_pin);// - (zero_time * 0.8);
  }
  else if(time_us > 10000000){
	uart_send_string("Using 680 Ohm\r\n");
	used_resistor_pin = r_l_pin;
  	uint16_t pulse_voltage;
  	time_us = measure_pulse_mode(measure_pin, r_l_pin, &pulse_voltage);
  	voltage_mv = pulse_voltage;

  	uart_send_string("After pulse mode: voltage_mv=");
	uart_send_uint16_t(voltage_mv);
	uart_send_string("mV\r\n");
  }

  // - (zero_voltage * 0.8);
  // test_resistor(measure_pin);

  /* -----------------------------------------------------
  filtrates measurement result when capacitor not connected*/

  uart_send_string("Auto-range decision: ");
  uart_send_uint32_t(time_us); // for small capacitors
  if (time_us < 10000){
	  uart_send_string("Using 470 kOhm\r\n");
	  voltage_mv = measure_voltage_mv(measure_pin);
  }
  else{
	  uint16_t pulse_voltage;
	  uart_send_string("Using 680 Ohm\r\n");
  }
  if (voltage_mv < 1){ // voltage detection of capacitor
	  uart_send_string("No capacitor connected");
	  return 0.0;
  }

  if (time_us < 100){ // time charge detection of capacitor
	  uart_send_string("No capacitor connected");
	  return 0.0;
  }

  //if (time_us < 1000 && voltage_mv > 2500){
	  //uart_send_string("NO capacitor connected\r\n");
	  //return 0.0;

  //}
  // -----------------------------------------------------

  uint8_t voltage_index = find_voltage_index(voltage_mv);
  uint16_t table_value = pgm_read_word(&RLtab[voltage_index]);
  debug_portc_state("After measurements\r\n");

  capacitance = (float)time_us / table_value;

  if(capacitance > 10000){
	  capacitance = 0.0;
  }
  /*-----------------------------------------------------------
                          USART DEBUG*/ 
  uart_send_string("GoldPin: ");
  usart_transmit('0' + goldpin_number);
  uart_send_string(", Time: ");
  uart_send_uint32_t(time_us);
  uart_send_string(" us, Voltage: ");
  uart_send_uint16_t(voltage_mv);
  uart_send_string(" mV, Capacitance: ");
  uart_send_float(capacitance);
  uart_send_string(" uF, ");
//-------------------------------------------------------------
  return capacitance;
}

void set_others_goldpins_as_gnd(uint8_t active_goldpin){
  //uint8_t original_ddr = DDRC;
  //uint8_t original_port = PORTC;

  switch(active_goldpin){
    case 1 : // if goldpin 1 A0 is active
      DDRC |= (1 << CH2_MEASURE_PIN) | (1 << CH3_MEASURE_PIN); // A1 and A2 set as output
      PORTC &= ~((1 << CH2_MEASURE_PIN) | (1 << CH3_MEASURE_PIN)); // GND state
      break;
    
    case 2: // if goldpin 2 A1 is active
      DDRC |= (1 << CH1_MEASURE_PIN) | (1 << CH3_MEASURE_PIN); // A1 and A2 set as output
      PORTC &= ~((1 << CH1_MEASURE_PIN) | (1 << CH3_MEASURE_PIN)); // GND state
      break;

    case 3: // if goldpin 3 A2 is active
      DDRC |= (1 << CH1_MEASURE_PIN) | (1 << CH2_MEASURE_PIN); // A1 and A2 set as output
      PORTC &= ~((1 << CH1_MEASURE_PIN) | (1 << CH2_MEASURE_PIN)); // GND state
      break;
  }
  //DDRC = original_ddr;
  //PORTC = original_port;
}
uint32_t measure_pulse_mode(uint8_t measure_pin, uint8_t resistor_pin, uint16_t *voltage_result){
	uint16_t pulse_count = 0;
	uint16_t voltage_mv;

	discharge_capacitor(measure_pin);

	do {
		// 1 pulse = 10ms
		R_DDR |= (1 << resistor_pin);
		R_PORT |= (1 << resistor_pin);
		_delay_ms(10);

		R_PORT &= ~(1 << resistor_pin);
		R_DDR &= ~(1 << resistor_pin);

		pulse_count++;
		voltage_mv = measure_voltage_mv(measure_pin);

		uart_send_string("Pulse ");
		uart_send_uint16_t(pulse_count);
		uart_send_string(", Voltage: ");
		uart_send_uint16_t(voltage_mv);
		uart_send_string("mV\r\n");

	}
	while (voltage_mv < 600 && pulse_count < 2000);

	*voltage_result = voltage_mv;

	uart_send_string("Pulse mode returning: voltage=");
	uart_send_uint16_t(voltage_mv);
	uart_send_string("mV, pulses=");
	uart_send_uint16_t(pulse_count);
	uart_send_string("\r\n");

	return pulse_count * 10000; // 10ms per pulse = time in us
}
