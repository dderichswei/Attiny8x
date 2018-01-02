/*
 * ddtinyPIR
 * 
 * based on Attiny84 
 * 
 * aim: low current consumption
 * 
 * parts needed:
 *  Attiny84 (of course)
 *  NRF24L01+ 
 *  MINIPIR
 * 
 * How to connect the ATTINY84 with the RF24:
 * 
 *     ATtiny24/44/84 Pin map with CE_PIN 8 and CSN_PIN 7
  Schematic provided and successfully tested by Carmine Pastore (https://github.com/Carminepz)
                                       +-\/-+
    nRF24L01  VCC, pin2 --- (--) VCC  1|o   |14 (--) GND --- nRF24L01  GND, pin1
                            (00) PB0  2|    |13 (10) AREF
                            (01) PB1  3|    |12 (09) PA1
                            (11) PB3  4|    |11 (08) PA2 --- nRF24L01   CE, pin3
                            (02) PB2  5|    |10 (07) PA3 --- nRF24L01  CSN, pin4
                            (03) PA7  6|    |9  (06) PA4 --- nRF24L01  SCK, pin5
    nRF24L01 MOSI, pin7 --- (04) PA6  7|    |8  (05) PA5 --- nRF24L01 MISO, pin6
                                       +----+
 * 
 * it only works if you set PIN MAPPING to: Counterclockwise 
 * 
 * 
 * 
 * 
 * 
 */

// PIR
#define PIR 10
    
// Deep Sleep
#include <avr/sleep.h>
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// volatile bool slept;

// Watchdog Sleep
#include <avr/wdt.h>

// nRF24L01 setup
#include "RF24.h"
#define CE_PIN 8          // 8 select Counterclockwise for PIN MAPPING in ARDUINO
#define CSN_PIN 7         // 7 select Counterclockwise for PIN MAPPING in ARDUINO
RF24 radio(CE_PIN, CSN_PIN);

// Topology and Payload
//byte addresses[][6] = {"1Node","2Node"};
unsigned long payload = 0;
char payload2[15] = "DD0001";
const int max_payload_size = 32;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

// Common
int i;

void setup() {
    pinMode(PIR, INPUT);
    sbi(GIMSK,PCIE0); // Turn on Pin Change interrupt
    sbi(PCMSK0,PCINT0); // Which pins are affected by the interrupt
 
}

void loop() {
 // initRF24();
 // sendRF24();


  turnONRF24();

  
  radio.stopListening(); // First, stop listening so we can talk.
  //payload++;
  //payload2=batLevel();
  //radio.write( &payload, sizeof(unsigned long) );
  radio.write( &payload2, sizeof(payload2) );
  radio.startListening(); // Now, continue listening

  unsigned long started_waiting_at = micros(); // Set up a timeout period, get the current microseconds
  boolean timeout = false; // Set up a variable to indicate if a response was received or not

  while ( !radio.available() ){ // While nothing is received
    if (micros() - started_waiting_at > 200000 ){ // If waited longer than 200ms, indicate timeout and exit while loop
      timeout = true;
      break;
    }

  }

  if ( !timeout ){ // Describe the results
    unsigned long got_time; // Grab the response, compare, and send to debugging spew
    radio.read( &got_time, sizeof(unsigned long) );
  }

  turnOFFRF24(); 
  ddsleep();
  while (digitalRead(PIR) == LOW) {
  ddsleep();
    }
  radio.powerUp();
 

}

void turnONRF24() {
  initRF24();
}

void turnOFFRF24() {
  radio.powerDown();
}

void initRF24()
{
  // Setup and configure rf radio
  radio.begin(); // Start up the radio
  radio.setAutoAck(1); // Ensure autoACK is enabled
  radio.setRetries(15,15); // Max delay between retries & number of retries
  //radio.openWritingPipe(addresses[1]); // Write to device address '2Node'
  //radio.openReadingPipe(1,addresses[0]); // Read on pipe 1 for device address '1Node'
  radio.openWritingPipe(pipes[1]); // Write to device address '2Node'
  radio.openReadingPipe(1,pipes[0]); // Read on pipe 1 for device address '1Node'
  
  
  radio.startListening(); // Start listening
}


void sendRF24() {
  radio.stopListening(); // First, stop listening so we can talk.
  payload++;
  radio.write( &payload, sizeof(unsigned long) );
  radio.startListening(); // Now, continue listening

  unsigned long started_waiting_at = micros(); // Set up a timeout period, get the current microseconds
  boolean timeout = false; // Set up a variable to indicate if a response was received or not

  while ( !radio.available() ){ // While nothing is received
    if (micros() - started_waiting_at > 200000 ){ // If waited longer than 200ms, indicate timeout and exit while loop
      timeout = true;
      break;
    }

  }

  if ( !timeout ){ // Describe the results
    unsigned long got_time; // Grab the response, compare, and send to debugging spew
    radio.read( &got_time, sizeof(unsigned long) );
  }
}

void ddsleep() {

//radio.powerDown();
GIMSK |= _BV(PCIE0); // Enable Pin Change Interrupts
PCMSK0 |= _BV(PCINT0); // Use PB3 as interrupt pin
//PCMSK0 |= _BV(PCINT1); // Use PB4 as interrupt pin
ADCSRA &= ~_BV(ADEN); // ADC off
set_sleep_mode(SLEEP_MODE_PWR_DOWN); // replaces above statement
sleep_enable(); // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
sei(); // Enable interrupts
sleep_cpu(); // sleep

cli(); // Disable interrupts
PCMSK0 &= ~_BV(PCINT0); // Turn off PB3 as interrupt pin
//PCMSK0 &= ~_BV(PCINT1); // Turn off PB4 as interrupt pin
sleep_disable(); // Clear SE bit
ADCSRA |= _BV(ADEN); // ADC on

sei(); // Enable interrupts
} // sleep

ISR(PCINT0_vect) {
}

ISR(PCINT1_vect) {
}

// watchdog interrupt
ISR(WDT_vect) 
  {
   wdt_disable();  // disable watchdog
   radio.powerUp();
  }


void myWatchdogEnable(const byte interval) 
  {  
  // radio.powerDown();  
  MCUSR = 0;                          // reset various flags
  WDTCSR |= 0b00011000;               // see docs, set WDCE, WDE
  WDTCSR =  0b01000000 | interval;    // set WDIE, and appropriate delay
  cbi(ADCSRA,ADEN); // Switch Analog to Digital converter OFF
  wdt_reset();
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_mode();            // now goes to Sleep and waits for the interrupt
  sbi(ADCSRA,ADEN);  // Switch Analog to Digital converter ON
  } 


void initADC()
{
  /* this function initialises the ADC 

        ADC Prescaler Notes:
  --------------------

     ADC Prescaler needs to be set so that the ADC input frequency is between 50 - 200kHz.
  
           For more information, see table 17.5 "ADC Prescaler Selections" in 
           chapter 17.13.2 "ADCSRA – ADC Control and Status Register A"
          (pages 140 and 141 on the complete ATtiny25/45/85 datasheet, Rev. 2586M–AVR–07/10)

           Valid prescaler values for various clock speeds
  
       Clock   Available prescaler values
           ---------------------------------------
             1 MHz   8 (125kHz), 16 (62.5kHz)
             4 MHz   32 (125kHz), 64 (62.5kHz)
             8 MHz   64 (125kHz), 128 (62.5kHz)
            16 MHz   128 (125kHz)

           Below example set prescaler to 128 for mcu running at 8MHz
           (check the datasheet for the proper bit values to set the prescaler)
  */

  // 8-bit resolution
  // set ADLAR to 1 to enable the Left-shift result (only bits ADC9..ADC2 are available)
  // then, only reading ADCH is sufficient for 8-bit results (256 values)

  ADMUX =
            (1 << ADLAR) |     // left shift result
            (0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
            (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            (1 << MUX5)  |     // use ADC2 for input (PB4), MUX bit 3
            (0 << MUX4)  |     // use ADC2 for input (PB4), MUX bit 3
            (0 << MUX3)  |     // use ADC2 for input (PB4), MUX bit 3
            (0 << MUX2)  |     // use ADC2 for input (PB4), MUX bit 2
            (0 << MUX1)  |     // use ADC2 for input (PB4), MUX bit 1
            (1 << MUX0);       // use ADC2 for input (PB4), MUX bit 0

  ADCSRA = 
            (1 << ADEN)  |     // Enable ADC 
            (0 << ADPS2) |     // set prescaler to 64, bit 2 
            (1 << ADPS1) |     // set prescaler to 64, bit 1 
            (1 << ADPS0);      // set prescaler to 64, bit 0  
}


int batLevel()
{
  initADC();
  int level = 42;
  uint8_t adc_lobyte; // to hold the low byte of the ADC register (ADCL)
  uint16_t raw_adc;

  while(1)
  {

    ADCSRA |= (1 << ADSC);         // start ADC measurement
    while (ADCSRA & (1 << ADSC) ); // wait till conversion complete 

    // for 10-bit resolution:
    //adc_lobyte = ADCL; // get the sample value from ADCL
    //raw_adc = ADCH<<8 | adc_lobyte;   // add lobyte and hibyte

    if (ADCH > 128)  // ADC input voltage is more than half of the internal 1.1V reference voltage
    {
     level=43;
     
    } else {      // ADC input voltage is less than half of the internal 1.1V reference voltage

     level=41;
    }

  }
 return level;
}

