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

// Phototransitor connected currently with 20kOHM to Ground.
#define PHO 9

// Debug LED
#define LED 3


// Deep Sleep
#include <avr/sleep.h>
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

// Batterielevel
int battery_Level = 0;

volatile bool blinkit;
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
char payload2[6] = "DD0007";
const int max_payload_size = 32;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

void setup() {
    pinMode(PHO, INPUT);
    pinMode(PIR, INPUT);
    sbi(GIMSK,PCIE0); // Turn on Pin Change interrupt
    sbi(PCMSK0,PCINT0); // Which pins are affected by the interrupt
}

void loop() {
  turnONRF24();
  sendRF24();
  //sendBatLevel();
  turnOFFRF24();

  
  ddsleep();
  while (digitalRead(PIR) == LOW) {
  ddsleep();
  }
  radio.powerUp();
  
}

void blinkLed()
{
  if (blinkit) {
    digitalWrite(LED, HIGH);
    delay(500);
    blinkit = false;
  }
  else
  {
    digitalWrite(LED, LOW); 
    delay(500);   
    blinkit = true;
  }
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
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setCRCLength(RF24_CRC_8);
  radio.setAutoAck(1); // Ensure autoACK is enabled
  radio.setRetries(15,15); // Max delay between retries & number of retries
  radio.openWritingPipe(pipes[1]); // Write to device address '2Node'
  radio.openReadingPipe(1,pipes[0]); // Read on pipe 1 for device address '1Node'
  radio.startListening(); // Start listening
}


void sendRF24() {
  radio.stopListening(); // First, stop listening so we can talk.
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
  // if (blinkit) { blinkit = false; }
  // else { blinkit = true; }
}

void sendBatLevel() {
  batLevel();
  radio.stopListening(); // First, stop listening so we can talk.
  radio.write( &payload, sizeof(payload) );
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
  // if (blinkit) { blinkit = false; }
  // else { blinkit = true; }
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
  /// THIS is a working routine for ATTINY84 with 1 MHZ.


  ADMUX =
            (0 << REFS1) |     // Sets ref. voltage to Vcc, bit 1   
            (0 << REFS0) |     // Sets ref. voltage to Vcc, bit 0
            (0 << MUX5)  |     // use ADC1 for input (PA1), MUX bit 5
            (0 << MUX4)  |     // use ADC1 for input (PA1), MUX bit 4
            (0 << MUX3)  |     // use ADC1 for input (PA1), MUX bit 3
            (0 << MUX2)  |     // use ADC1 for input (PA1), MUX bit 2
            (0 << MUX1)  |     // use ADC1 for input (PA1), MUX bit 1
            (1 << MUX0);       // use ADC1 for input (PA1), MUX bit 0

  ADCSRA = 
            (1 << ADEN)  |     // Enable ADC 
            (1 << ADPS2) |     // set prescaler to 16, bit 2 
            (0 << ADPS1) |     // set prescaler to 16, bit 1 
            (0 << ADPS0);      // set prescaler to 16, bit 0 
  ADCSRB = 
            (1 << ADLAR);      // left shift result (for 8-bit values)
  //        (0 << ADLAR);      // right shift result (for 10-bit values)
}


void batLevel() // does not work yet
{
 initADC();

  while(1)
  {

    ADCSRA |= (1 << ADSC);         // start ADC measurement
    while (ADCSRA & (1 << ADSC) ); // wait till conversion complete 

    if (ADCH > 128) {  payload = 65; }        // ADC input voltage is more than half of VCC } 
    else if (ADCH > 100) { payload = 66; }    // B
    else if (ADCH > 72)  { payload = 67; }    // C
    else if (ADCH > 44)  { payload = 68; }    // D
    else { payload = 70; } // ADC input voltage is less than half of VCC   
   return 0;
  }
}
