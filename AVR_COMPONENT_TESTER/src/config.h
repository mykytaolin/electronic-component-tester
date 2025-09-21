//#include SH1106Wire.h
#include <avr/pgmspace.h>

//SH1106Wire display(0x3C, A4, A5)

#define MCU atmega328p
#define F_CPU 16000000UL

#define LANG_ENGLISH

#define WITH_AUTO_REF // enables reading of internal REF-voltage to get koef., for the capacity measuring
#define REF_C_KORR 12 // corrects the reference Voltage for capacity measurement (<40uF)(mV)
#define REF_L_KORR 40 // corrects the reference Voltage for inductance measurement(mV)
#define C_H_KORR 0 // defines a correction of 0.1% units for big capacitor measureme           t

#define CAP_EMPTY_LEVEL 4 // empty voltage level of capacitor (mV)
#define AUTOSCALE_ADC // enables autoscale ABC (needed VCC and bandgap ref)
#define REF_R_KORR 3 // defines a reference correction value (R stand for resistance)

#define ESR_ZERO 20 // define zero for ESR measurment(1 = 0.01 Ohm)

#define NO_AREF_CAP // enable using 1nF capasitors, also reducing waiting time of AUTOSCALE_ADC
#define PULLUP_DISABLE // this option disable the pull-up Resistors of IO-Ports.
#define ANZ_MESS 25 // this option defines the frequency of reading and accumulating ADC value

#define BAT_POOR 6400 // set the threshhold of minimal level of low battery charge (64V)

#ifndef ADC_PORT
  #define ADC_PORT PORTC // port C (microcontroller)
  #define ADC_DDR DDRC // DATA DIRECTION REGISTER of C port defines pins as input or output
  #define ADC_PIN PINC // need to read input pins status
#endif

/*  #define TP1 0 // reference to pin A0(test point)
#define TP1 1 // reference to pin A1
#define TP1 2 // reference to pin A2
#define TPEXT 3 // A3 outside pin for measurments
#define TPREF 6 // Port pin for 2.5V precision reference used for VCC check (optional)
#define TPBAT 7 // // Port pin for Battery voltage measuring  */

/*
  Definition of used resistors Ohm
  Standard value for R_L is 680 Ohm(low resistor) and 470 kOhm for R_H(high resistor)
*/
/*----------------------------------------------------------------------------------
                    Measurement Channel PIN CONFIGURATION*/
                    
// Channel 1 (GoldPin1)
#define CH1_MEASURE_PIN 0 // PC0 A0

// Channel 2 (GoldPin2)
#define CH2_RO2_PIN 7 // PD7 D7 (100 Ohm)
#define CH2_RK2_PIN 0 // PB0 D8 (10 kOhm)
#define CH2_RM2_PIN 1 // PB1 D9 (1 MOhm)
#define CH2_RL2_PIN 2 // PB2 D10 (680 Ohm)
#define CH2_RH2_PIN 3 // PB3 D11 (470 kOhm)
#define CH2_MEASURE_PIN 1 // PC1 A1

// Channel 3 (GoldPin3)
#define CH3_MEASURE_PIN 2 // PC2 A2

//---------------------------------------------------------------------------------
// discharge pin 
#define CH1_DIS_CH_PIN 0 // PC0 A0
#define CH3_DIS_CH_PIN 2 // PC2 A2
#define DIS_CH_DDR DDRC // DDRD - data direction register C (PC0 - PC5)
// DDR : 0 - input , 1 - output
#define DIS_CH_PORT PORTC // state control 0 - low 0V and 1 - high 5V


#define R_DDR DDRB // DATA DIRECTION REGISTER of port B
// DDRB defines pins are input or output
#define R_PORT PORTB // defines output register for port B

#define ON_DDR DDRD // output/input direction through D port
#define ON_PORT PORTD // defines output register for port D
#define ON_PIN_REG PIND // needed for checking pin stan from port D
#define ON_PIN 7 // power control from PD7

// button that changing measure type
#define CHNG_MEASURE_TYPE 3 // PC3 A3
#define CHNG_MEASURE_DDR DDRC
#define CHNG_MEASURE_PORT PORTC

//#ifndef STRIP_GRID_BOARD
//  #define RST_PIN 0 // reset pin D0
//#else
// #define RST_PIN 17 // A3 reset pin (button)
//#endif

/* ----------------------------------------------------------------------------------------------------------------------------------------------------------------
Ports and pins for LCD HD44780
HW_ - hardware
EN_ - enable
RS_ - register select
-------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
extern const uint16_t RLtab[] PROGMEM;

#define UVCC 5000 // defines the VCC voltage in mV
// used to compensate the voltage drop in charge time (more accurate for condesators with huge ESR)

#define U_SCALE 4 // used for better resolution of readADC resistor measurements(better accuracy for R<10 Ohm)
// #define R_ANZ_MESS 200 // number of measurements needed to final average result
// R_ANZ_MESS reducing noise
#define MAX_MEASSUREMENTS 200 // better when resistance < 10 Ohm but slower 
#define MIN_MEASSUREMENTS 20 // faster but good only with 
#define SLD_PIN 7 // Slide button to change R_ANZ_MESS from 200 to 20 and in the opposite way

/*------------------------------------------
Button that changes measurement resolution
-------------------------------------------*/
#define RES_CH_PIN 6 // RESOLUTION_CHANGE_PIN

/*-----------------------------------------
                  Watchdog
------------------------------------------*/
// While debug turn it off
// #define WDT_enabled
/*-----------------------------------------
            End of basic configuration
------------------------------------------*/

/*-----------------------------------------
The next fragment of code is konfiguration of EEPROM and PROGMEM(programm memory)
EEPROM - can contain memory after after switching off
PROGMEM - flash memory
MEM - Always using EEPROM if USE_EEPROM is turned on
MEM2 - dynamically switching between PROGMEM and EEPROM depanding on free memory in buffer
------------------------------------------*/
#ifdef USE_EEPROM // if defined - use EEPROM if not - use PROGMEM
  #define MEM_TEXT EEMEM 
  /* EEMEM - memory atrybut of commpilator AVR-GCC 
  that that literally saying - put this variable into EEPROM*/ 
  #if E2END > 0X1FF /*E2END - const in the AVR-GCC compilator and it's last address in EEPROM
  0X1FF - 511 in hexadecimal system 
  So we checking if our microcontroller has EEPROM bigger than 512 bytes*/
    #define MEM2_TEXT EEMEM // the same as MEM_TEXT
    #define MEM2_read_byte(a) eeprom_read_byte(a)
    /*alias of eeprom_read_byte(a) function from <avr/eeprom.h> that reads 1 byte from address a */ 
    #define MEM2_read_word(a) eeprom_read_word(a)
    /*alias of eeprom_read_word(a) function from <avr/eeprom.h> that reads 2 byte word from address a*/
    #define lcd_fix2_string(a) lcd_fix_string(a)
    /*alias of eeprom_read_word(a) function from <avr/eeprom.h> that display some text from EEPROM
    --------------------------------------------------------------------------------------------------
    The else statement used when EEPROM is small or unreachable*/
  #else
    /*same functionality but for flash memory*/
    #define MEM2_TEXT PROGMEM
    #define MEM2_read_byte(a) pgm_read_byte(a)
    #define MEM2_read_word(a) pgm_read_word(a)
    #define lcd_fix2_string(a) lcd_pgm_string(a)
    #define use_lcd_pgm // pgm - programm memory
    /*flag that informs compilator that program using flash memory instead of EEPROM*/
  #endif

  #define MEM_read_byte(a) eeprom_read_byte(a)
  /*the same functionality as MEM2 but always using EEPROM*/
  #define MEM_read_word(a) eeprom_read_word(a)
  /*the same functionality as MEM2 but always using EEPROM*/
/*-----------------------------------------------------------------------------------------------------
  The next else statement used in emergency situation 
  - When EEPROM is turned off or commented
  - When compilator can't use EEPROM*/
#else
  #define MEM_TEXT PROGMEM
  #define MEM2_TEXT PROGMEM
  #define MEM_read_byte(a) pgm_read_byte(a)
  #define MEM_read_word(a) pgm_read_byte(a)
  #define MEM2_read_byte(a) pgm_read_byte(a)
  #define MEM2_read_word(a) pgm_read_byte(a)
  #define lcd_fix2_string(a) lcd_pgm_string(a)
  #define use_lcd_pgm
#endif
/*-----------------------------------------------------------------------------------------------------
Offset ****(TO CHANGE)**** that compensate systematic measurement error for RH(kOhm)
Resolution - 0.1 Ohm  3000 defines an offset 300 Ohm*/
#define RH_OFFSET 3000
//Offset ****(TO CHANGE)**** that compensate PCB roots capasity in pF
#define TP2_CP_OFFSET 2
// CABLE_CAP compansating cables capasity in pF(for measuremetn) *****UNCOMMENT_WHEN_CONNECT_CABLES****
//#define CABLE_CAP 3
//-----------------------------------------------------------------------------------------------------

#define PROCESSOR_TYP 328

//-----------------------------------------------------------------------------------------------------
/*Auto selection of right call type
call - full function call (4 Bytes) for big size memory 
rcall - relative call (2 Bytes) for small amount of memory*/ 
//FLASHEND compilator const that mean last address of flash memory
#if FLASHEND > 0X1FFF
// checking if microcontroller has more than 8 kB of flash memory
  #define ACALL call
#else
  #define ACALL rcall
#endif

#if PROCESSOR_TYP == 328
  #define MCU_STATUS_REG MCUCR // defines control register of processor (MCU)
  // https://www.microchip.com/en-us/search?searchQuery=ATmega328&category=ALL&fq=start%3D0%26rows%3D10 ***ATMEGA328 datasheet***
  // ADCSRB - ADC Control and Status Register B
  #define ADC_COMP_CONTROL ADCSRB // defines control register of ADC (Analog-to-digital converter)
  #define TI1_INT_FLAGS TIFR1 // TIFR1 special AVR register which observing status of interruptions generated by Timer1
  // TIFR1 it's 'control subsystem' that informs about overflows, handlings and other staff
  // DEFAULT_BAND_GAP 1070 is *****EXPERIMENTAL VALUE(CAN BE CHANGED)*****
  #define DEFAULT_BAND_GAP 1070 // set value (mV units) of voltage source (bandgap) inbuilted in ATMEGA 328 
  // inbuilted voltage source placed somewhere between 1.07 V - 1.12 V
  #define DEFAULT_RH_FAKT 884 // constant which will used for recounting measurement result of high resistance to more presize

  /*---------------------------------------------------------------------------------------------------
  Config of advanced measurement functions for transistors*/
  #define LONG_HFE // turning on very accurate measurement of coefficient of current amplification of transistor
  // LONG_HFE using (32 byte) long datatype instead of (16 byte) int type 
  // using in functions that measurements hFE in *****common emitter***** configuration
  #define COMMON_COLLECTOR // activating measurement hFE in configuraton *****emitter follower*****
  //----------------------------------------------------------------------------------------------------

  // Pin resistor values of ATmega168
  // need this to compensate some measurement unaccuracy
  #define PIN_RM 200 // internal resistance of measurement PIN to GND ****(CAN BE CHANGED IF FINAL RESULT WILL UNACCURATE)****
  #define PIN_RP 220 // internal resistance of measurement PIN to VCC ****(CAN BE CHANGED IF FINAL RESULT WILL UNACCURATE)****
  #define CC0 36 // const (in pF) it's basic parasytic capasity of measurement system ****(CAN BE CHANGED IF FINAL RESULT WILL UNACCURATE)****
  #define COMP_SLEW1 4000 // factor for correction of big capacities
  #define COMP_SLEW2 180 // factor for correction of small capacities
  //SLEW rate correction val += COMP_SLEW1 / (val + COMP_SLEW2)
  #define C_NULL CC0 + CABLE_CAP + (COMP_SLEW1 / (CC0 + CABLE_CAP + COMP_SLEW2))
  // C_NULL NULL capacity of system (formula for accurate capacity measurement)
  // To get accurate final result - measurement result substract C_NULL
  #define MUX_INT_REF 0x0e // channel for internal voltage 1.1 V (bandgap voltage) 
  // used for comparing voltages and when external source is instable
#endif

/*----------------------------------------------------------------------------------------------------------------
                        RESISTANCE AND CAPACITY MEASUREMENT CORRECTIONS*/
#ifndef REF_R_KORR
  #define REF_R_KORR 0 // if some static slopes appear - ****CHANGE VALUE**** (RESISTANCE MEASUREMENT CORRECTION)
#endif
#ifndef REF_C_KORR
  #define REF_C_KORR 0 // if some static slopes appear - ****CHANGE VALUE**** (CAPACITY MEASUREMENT CORRECTION)
#endif
/*---------------------------------------------------------------------------------------------------------------
                                    POWER_OFF FUNCTION*/
#define LONG_WAIT_TIME 30000 // setting long waiting time (charging) for measurement of huge capacities > 1000 µF
// or huge recistance > 1 MOhm for stable result
#define SHORT_WAIT_TIME 5000 // setting short waiting time for capacitance < 1 µF and recistance < 1 kOhm

#ifdef POWER_OFF
  // if POWER_OFF function is selected, wait 30s
  // if POWER_OFF parameter > 2 - wait 5s
  #if (POWER_OFF + 0) > 2
    #define OFF_WAIT_TIME SHORT_WAIT_TIME
  #else
    #define OFF_WAIT_TIME LONG_WAIT_TIME
  #endif
#else
// if POWER_OFF function is not selected, wait 30s before repeat measurement
  #define OFF_WAIT_TIME LONG_WAIT_TIME
#endif
/*----------------------------------------------------------------------------------------------------------------
                                              ADC CLOCK*/
#define F_ADC 125000 // 125kHz frequency ADC while CPU frequency is 16MHz
#if F_CPU/F_ADC == 128
  #define AUTO_CLOCK_DIV (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) // 24.9.2 ATmega328 documentation 172 row
#endif
//----------------------------------------------------------------------------------------------------------------

//IF MEASUREMENT RESULT WILL BE HIGHER - CHANGE PIN_RP AND PIN_RM TO CALIBRATE
#ifndef PIN_RP
  #define PIN_RP 220 // internal resistance between pin and vcc
#endif

#ifndef PIN_RM
  #define PIN_RM 190 // internal resistance between pin and gnd
#endif

/*---------------------------------------------------------------------------------------------------------------
                                        INHIBITSLEEP MODE*/
// save memory, do not use the sleep mode
#ifdef INHIBIT_SLEEP_MODE
  #define wait_about5ms wait5ms()
  #define wait_about10ms wait10ms()
  #define wait_about20ms wait20ms()
  #define wait_about30ms wait30ms()
  #define wait_about50ms wait50ms()
  #define wait_about100ms wait100ms()
  #define wait_about200ms wait200ms()
  #define wait_about300ms wait300ms()
  #define wait_about400ms wait400ms()
  #define wait_about500ms wait500ms()
  #define wait_about1s wait1s()
  #define wait_about2s wait2s()
  #define wait_about3s wait3s()
  #define wait_about4s wait4s()
#else
  // use sleep mode to optimizing battery consumption for user interface
  // number in brackets = iterations
  #define wait_about5ms sleep_5ms(1)
  #define wait_about10ms sleep_10ms(2)
  #define wait_about20ms sleep_20ms(4)
  #define wait_about30ms sleep_30ms(6)
  #define wait_about50ms sleep_50ms(10)
  #define wait_about100ms sleep_100ms(20)
  #define wait_about200ms sleep_200ms(40)
  #define wait_about300ms sleep_300ms(60)
  #define wait_about400ms sleep_400ms(80)
  #define wait_about500ms sleep_500ms(100)
  #define wait_about1s sleep_1s(200)
  #define wait_about2s sleep_2s(400)
  #define wait_about3s sleep_3s(600)
  #define wait_about4s sleep_4s(800)
#endif
//----------------------------------------------------------------------------------

#undef AUTO_RH
#ifdef WITH_AUTO_REF
  #define AUTO_RH
#else
  #ifdef AUTO_CAL
    #define AUTO_RH
  #endif
#endif

#undef CHECK_CALL
#ifdef WITH_SELFTEST
  // autocheck function is needed
  #define CHECK_CALL
#endif

// next is dynamic calibration and manual using previously defined internal pin resistance 
#ifdef AUTO_CAL
  #define CHECK_CALL // for autotests
  #define RP680PL resis680pl // resistor 680 plus
  #define RP680MI resis680mi // resistor 680 minus
  #define RRpinPL pin_rpl // measure pin plus
  #define RRpinMI pin_rmi // measure pin minus
#else
  #define RP680PL (R_L_VAL + PIN_RP)
  #define RP680MI (R_L_VAL + PIN_RM)
  #define RRpinPL PIN_RP
  #define RRpinMI PIN_RM
#endif

// *****DEFAULT ESR VALUE *****
#ifndef ESR_ZERO 
  #define ESR_ZERO 20
#endif
//-------------------------------
// the processor restart delay for crystal oscillator 16K
#ifndef RESTART_DELAY_TICS
  #define RESTART_DELAY_TICS 16384 // 1s
#endif

//configuration style of pin names 
#if EBC_STYLE == 123
  #undef EBC_STYLE
#endif

//--------------------------------------
// *****TODO OLED ICONS*****

//-----------------------------------

#ifdef COMMON_COLLECTOR // more precize for power transistors, NPN/PNP, hFE < 50, Darlington transistors
// func that configurating measurement of transistor amplification hFE/B
  #if FLASHEND > 0x3fff // if FLASH memory > 16 KB
    #define COMMON_EMITTER // more universal method
  #endif
#else
  #define COMMON_EMITTER
#endif
