# 1 "d:\\DATA\\Nextcloud\\DD-DEVBOX\\Arduino\\Attiny8x\\ddtinyPIR\\ddtinyPIR\\ddtinyPIR.ino"
# 1 "d:\\DATA\\Nextcloud\\DD-DEVBOX\\Arduino\\Attiny8x\\ddtinyPIR\\ddtinyPIR\\ddtinyPIR.ino"
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


// Phototransitor connected currently with 20kOHM to Ground.


// Debug LED



// Deep Sleep
# 47 "d:\\DATA\\Nextcloud\\DD-DEVBOX\\Arduino\\Attiny8x\\ddtinyPIR\\ddtinyPIR\\ddtinyPIR.ino" 2







// Batterielevel
int battery_Level = 0;

volatile bool blinkit;
// volatile bool slept;

// Watchdog Sleep
# 62 "d:\\DATA\\Nextcloud\\DD-DEVBOX\\Arduino\\Attiny8x\\ddtinyPIR\\ddtinyPIR\\ddtinyPIR.ino" 2


// nRF24L01 setup
# 66 "d:\\DATA\\Nextcloud\\DD-DEVBOX\\Arduino\\Attiny8x\\ddtinyPIR\\ddtinyPIR\\ddtinyPIR.ino" 2



RF24 radio(8 /* 8 select Counterclockwise for PIN MAPPING in ARDUINO*/, 7 /* 7 select Counterclockwise for PIN MAPPING in ARDUINO*/);

// Topology and Payload
//byte addresses[][6] = {"1Node","2Node"};
unsigned long payload = 0;
char payload2[6] = "DD0007";
const int max_payload_size = 32;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

void setup() {
    pinMode(9, 0x0);
    pinMode(10, 0x0);
    ((*(volatile uint8_t *)(((uint16_t) &((*(volatile uint8_t *)((0x3B) + 0x20)))))) |= (1 << (4))); // Turn on Pin Change interrupt
    ((*(volatile uint8_t *)(((uint16_t) &((*(volatile uint8_t *)((0x12) + 0x20)))))) |= (1 << (0))); // Which pins are affected by the interrupt
}

void loop() {
  turnONRF24();
  sendRF24();
  //sendBatLevel();
  turnOFFRF24();


  ddsleep();
  while (digitalRead(10) == 0x0) {
  ddsleep();
  }
  radio.powerUp();

}

void blinkLed()
{
  if (blinkit) {
    digitalWrite(3, 0x1);
    delay(500);
    blinkit = 0x0;
  }
  else
  {
    digitalWrite(3, 0x0);
    delay(500);
    blinkit = 0x1;
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
  boolean timeout = 0x0; // Set up a variable to indicate if a response was received or not

  while ( !radio.available() ){ // While nothing is received
    if (micros() - started_waiting_at > 200000 ){ // If waited longer than 200ms, indicate timeout and exit while loop
      timeout = 0x1;
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
  boolean timeout = 0x0; // Set up a variable to indicate if a response was received or not

  while ( !radio.available() ){ // While nothing is received
    if (micros() - started_waiting_at > 200000 ){ // If waited longer than 200ms, indicate timeout and exit while loop
      timeout = 0x1;
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
(*(volatile uint8_t *)((0x3B) + 0x20)) |= (1 << (4)); // Enable Pin Change Interrupts
(*(volatile uint8_t *)((0x12) + 0x20)) |= (1 << (0)); // Use PB3 as interrupt pin
//PCMSK0 |= _BV(PCINT1); // Use PB4 as interrupt pin
(*(volatile uint8_t *)((0x06) + 0x20)) &= ~(1 << (7)); // ADC off
do { (*(volatile uint8_t *)((0x35) + 0x20)) = (((*(volatile uint8_t *)((0x35) + 0x20)) & ~((1 << (3)) | (1 << (4)))) | ((0x02<<3))); } while(0); // replaces above statement
do { (*(volatile uint8_t *)((0x35) + 0x20)) |= (uint8_t)(1 << (5)); } while(0); // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
__asm__ __volatile__ ("sei" ::: "memory"); // Enable interrupts
do { __asm__ __volatile__ ( "sleep" "\n\t" :: ); } while(0); // sleep

__asm__ __volatile__ ("cli" ::: "memory"); // Disable interrupts
(*(volatile uint8_t *)((0x12) + 0x20)) &= ~(1 << (0)); // Turn off PB3 as interrupt pin
//PCMSK0 &= ~_BV(PCINT1); // Turn off PB4 as interrupt pin
do { (*(volatile uint8_t *)((0x35) + 0x20)) &= (uint8_t)(~(1 << (5))); } while(0); // Clear SE bit
(*(volatile uint8_t *)((0x06) + 0x20)) |= (1 << (7)); // ADC on

__asm__ __volatile__ ("sei" ::: "memory"); // Enable interrupts
} // sleep

extern "C" void __vector_2 (void) __attribute__ ((signal,used, externally_visible)) ; void __vector_2 (void) {
}

extern "C" void __vector_3 (void) __attribute__ ((signal,used, externally_visible)) ; void __vector_3 (void) {
}

// watchdog interrupt
extern "C" void __vector_4 (void) __attribute__ ((signal,used, externally_visible)) ; void __vector_4 (void)
  {
   wdt_disable(); // disable watchdog
  }


void myWatchdogEnable(const byte interval)
  {
  // radio.powerDown();
  (*(volatile uint8_t *)((0x34) + 0x20)) = 0; // reset various flags
  (*(volatile uint8_t *)((0x21) + 0x20)) |= 0b00011000; // see docs, set WDCE, WDE
  (*(volatile uint8_t *)((0x21) + 0x20)) = 0b01000000 | interval; // set WDIE, and appropriate delay
  ((*(volatile uint8_t *)(((uint16_t) &((*(volatile uint8_t *)((0x06) + 0x20)))))) &= ~(1 << (7))); // Switch Analog to Digital converter OFF
  __asm__ __volatile__ ("wdr");
  do { (*(volatile uint8_t *)((0x35) + 0x20)) = (((*(volatile uint8_t *)((0x35) + 0x20)) & ~((1 << (3)) | (1 << (4)))) | ((0x02<<3))); } while(0);
  do { do { (*(volatile uint8_t *)((0x35) + 0x20)) |= (uint8_t)(1 << (5)); } while(0); do { __asm__ __volatile__ ( "sleep" "\n\t" :: ); } while(0); do { (*(volatile uint8_t *)((0x35) + 0x20)) &= (uint8_t)(~(1 << (5))); } while(0); } while (0); // now goes to Sleep and waits for the interrupt
  ((*(volatile uint8_t *)(((uint16_t) &((*(volatile uint8_t *)((0x06) + 0x20)))))) |= (1 << (7))); // Switch Analog to Digital converter ON
  }


void initADC()
{
  /// THIS is a working routine for ATTINY84 with 1 MHZ.


  (*(volatile uint8_t *)((0x07) + 0x20)) =
            (0 << 7) | // Sets ref. voltage to Vcc, bit 1   
            (0 << 6) | // Sets ref. voltage to Vcc, bit 0
            (0 << 5) | // use ADC1 for input (PA1), MUX bit 5
            (0 << 4) | // use ADC1 for input (PA1), MUX bit 4
            (0 << 3) | // use ADC1 for input (PA1), MUX bit 3
            (0 << 2) | // use ADC1 for input (PA1), MUX bit 2
            (0 << 1) | // use ADC1 for input (PA1), MUX bit 1
            (1 << 0); // use ADC1 for input (PA1), MUX bit 0

  (*(volatile uint8_t *)((0x06) + 0x20)) =
            (1 << 7) | // Enable ADC 
            (1 << 2) | // set prescaler to 16, bit 2 
            (0 << 1) | // set prescaler to 16, bit 1 
            (0 << 0); // set prescaler to 16, bit 0 
  (*(volatile uint8_t *)((0x03) + 0x20)) =
            (1 << 4); // left shift result (for 8-bit values)
  //        (0 << ADLAR);      // right shift result (for 10-bit values)
}


void batLevel() // does not work yet
{
 initADC();

  while(1)
  {

    (*(volatile uint8_t *)((0x06) + 0x20)) |= (1 << 6); // start ADC measurement
    while ((*(volatile uint8_t *)((0x06) + 0x20)) & (1 << 6) ); // wait till conversion complete 

    if ((*(volatile uint8_t *)((0x05) + 0x20)) > 128) { payload = 65; } // ADC input voltage is more than half of VCC } 
    else if ((*(volatile uint8_t *)((0x05) + 0x20)) > 100) { payload = 66; } // B
    else if ((*(volatile uint8_t *)((0x05) + 0x20)) > 72) { payload = 67; } // C
    else if ((*(volatile uint8_t *)((0x05) + 0x20)) > 44) { payload = 68; } // D
    else { payload = 70; } // ADC input voltage is less than half of VCC   
   return 0;
  }
}
