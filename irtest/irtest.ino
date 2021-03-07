/*
  ir.ino

  Infrared testing

  18.1.2021 / Risto Helkiö

  Board: Arduino SAMD (32-bits ARM Cortex-M0+) Boards -> Adafruit Circuit Playground Express
  Port: /dev/cu.usbmodem1432401 (Adafruit Circuit Playground Express)

  example:
  https://learn.adafruit.com/infrared-transmit-and-receive-on-circuit-playground-express-in-c-plus-plus-2


*/

/*
  IR sending & receiving relies on the IR-library of Adafruit Circuit Playground Express.
  Library can be found in /Arduino/libraries/Adafruit_circuit_playground/utility

  Hästens bed is added as protocol 13 (IRLib_P13_Hastens.h)

*/

/*
  Hästens-sängyn IR-protokolla
  
  sekvenssissä 18 merkkiä
  toinen merkki mark 2900 microsecs, space 415 microsecs  '1'
  toinen merkki mark 415 microsecs, space 2900 microsecs  '0'

  101011 10 0011000000
    |     |      |______ 10 merkkiä, toimintakoodin tarkennus (0011000000, 0000110000, 0000001100, 0000000011)
    |     |_____________ 2 merkkiä, kaukosäätimen koodi (A = 11, B = 10, C = 00)
    |___________________ 6 merkkiä, toimintokoodi (legsShake = 101110, flatten = 101010, headShake = 101011, up/down = 111010)


  static const char AFlat[]         PROGMEM = "101010110000000000";
  static const char AHeadUp[]       PROGMEM = "111010110011000000";
  static const char AHeadDown[]     PROGMEM = "111010110000110000";
  static const char ALegsUp[]       PROGMEM = "111010110000001100";
  static const char ALegsDown[]     PROGMEM = "111010110000000011";
  static const char AHeadShake1[]   PROGMEM = "101011110011000000";
  static const char AHeadShake2[]   PROGMEM = "101011110000110000";
  static const char AHeadShake3[]   PROGMEM = "101011110000001100";
  static const char AHeadShakeOff[] PROGMEM = "101011110000000011";
  static const char ALegsShake1[]   PROGMEM = "101110110011000000";
  static const char ALegsShake2[]   PROGMEM = "101110110000110000";
  static const char ALegsShake3[]   PROGMEM = "101110110000001100";
  static const char ALegsShakeOff[] PROGMEM = "101110110000000011";
  // POSITION B

*/





// Debugging
//#define _debug
//#define IRLIB_TRACE

// send or receive
#define SENDING true

#include <Adafruit_CircuitPlayground.h>
#include "hastens-codes.h"

/* Infrared_Read.ino Example sketch for IRLib2 and Circuit Playground Express
   Illustrates how to receive an IR signal, decode it and print
   information about it to the serial monitor.
*/

#if !defined(ARDUINO_SAMD_CIRCUITPLAYGROUND_EXPRESS)
  #error "Infrared support is only for the Circuit Playground Express, it doesn't work with the Classic version"
#endif


uint32_t codes[13];       // code set in use
uint32_t code;            // current code
bool head;                // head selected?
bool prev_head;           // previous head selector
bool head_unknown = true; // head selector not initialized yet


void setup() {
  CircuitPlayground.begin();

  #ifdef _debug
    Serial.begin(115200);
    while(!Serial) { } // Wait for serial to initialize.
  #endif

  if (SENDING) {
    #ifdef _debug
      Serial.println("Ready to send IR");
    #endif
  } else {
    CircuitPlayground.irReceiver.enableIRIn(); // Start the receiver
    #ifdef _debug
      Serial.println("Ready to receive IR signals");
    #endif
  }
  // use D0/A6 to indicate IR to trigger oscilloscope
  // use D1/A7 to indicate IR to trigger oscilloscope
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);

  // turn off pixels
  CircuitPlayground.clearPixels();

  // select hard-coded code set
  for (int i = 0; i < 13; i++){
    //codes[i] = a_codes[i];  // RISTO
    codes[i] = c_codes[i];  // ANITA
  }
  
}

// capacitive touch loops
void loopTOUCH(){
  #define CAP_SAMPLES      20   // Number of samples to take for a capacitive touch read.
  uint16_t CAP_THRESHOLD = 800;  // Threshold for a capacitive touch (higher = less sensitive).

  int capPins[7] = { 0, 1, 2, 3, 6, 9, 10 };
  int stripPins[7] = { 3, 4, 2, 1, 6, 7, 8 };
  for( int i = 0; i < 7; i++){
    int k = CircuitPlayground.readCap(capPins[i], CAP_SAMPLES);
    if (k >= CAP_THRESHOLD) {
      CircuitPlayground.strip.setPixelColor(stripPins[i], CircuitPlayground.colorWheel(256/10*i));
      //Serial.println(i);
    } else {
      CircuitPlayground.strip.setPixelColor(stripPins[i], 0);
    }
  }

  // Light up the pixels.
  CircuitPlayground.strip.show();
}

void set_head_pixels() {
  CircuitPlayground.clearPixels();
  int pins[4] = { 1, 0, 8, 9};
  uint32_t color = CircuitPlayground.colorWheel(0); // wheel 0..255
  for (int i = 0; i < 4 ; i++) {
    CircuitPlayground.strip.setPixelColor(pins[i], color);
  }
  CircuitPlayground.strip.show();
}
void set_legs_pixels() {
  CircuitPlayground.clearPixels();
  int pins[2] = { 4, 5 };
  uint32_t color = CircuitPlayground.colorWheel(0); // wheel 0..255
  for (int i = 0; i < 2; i++) {
    CircuitPlayground.strip.setPixelColor(pins[i], color);
  }
  CircuitPlayground.strip.show();
}


void loop() {

  if (SENDING) {

    // **** Send loop ******


    #define PROTOCOL HASTENS
    #define BITS 18

    // select head or legs based on slide switch position
    head = CircuitPlayground.slideSwitch();
    if (head_unknown or (head != prev_head)) {
      if (head)
        set_head_pixels();
      else
        set_legs_pixels();
      prev_head = head;
      head_unknown = false;
    }

    // Left button press
    if (CircuitPlayground.leftButton()) {
      CircuitPlayground.playTone(700, 50);
      digitalWrite(0, LOW);
      if (head) {
        code = codes[HEAD_UP];
      } else {
        code = codes[LEGS_UP];
      }
      while (CircuitPlayground.leftButton()) {
        //wait until button released
        CircuitPlayground.irSend.send(PROTOCOL, code, BITS); delay(20);
        CircuitPlayground.irSend.send(PROTOCOL, code, BITS); delay(20);
      }
      digitalWrite(0, HIGH);
    }

    // Right button press
    if (CircuitPlayground.rightButton()) {
      CircuitPlayground.playTone(350, 50);
      digitalWrite(0, LOW);
      if (head) {
        code = codes[HEAD_DOWN];
      } else {
        code = codes[LEGS_DOWN];
      }
      while (CircuitPlayground.rightButton()) {
        //loop until button released
        CircuitPlayground.irSend.send(PROTOCOL, code, BITS); delay(20);
        CircuitPlayground.irSend.send(PROTOCOL, code, BITS); delay(20);
      }
      digitalWrite(0, HIGH);
    }
  } else {
    // **** Receive loop ******

    //Continue looping until you get a complete signal received
    if (CircuitPlayground.irReceiver.getResults()) {
      //CircuitPlayground.irDecoder.decode();           //Decode it
      //CircuitPlayground.irDecoder.dumpResults(false);  //Now print results. Use false for less detail
      CircuitPlayground.irDecoder.dumpResultsRH(true);  //Now print results. Use false for less detail
      CircuitPlayground.irReceiver.enableIRIn();      //Restart receiver
    }
  }
}
