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

*/

// Debugging
//#define _debug
//#define IRLIB_TRACE

#include <Adafruit_CircuitPlayground.h>
#include "hastens-codes.h"

#if !defined(ARDUINO_SAMD_CIRCUITPLAYGROUND_EXPRESS)
  #error "Infrared support is only for the Circuit Playground Express, it doesn't work with the Classic version"
#endif


uint32_t codes[13];           // code set in use
uint32_t code;                // current code
bool current_position;        // current slider position
bool prev_position;           // previous slider position
bool position_unknown = true; // slider position not initialized yet
bool lights_on = true;
uint32_t previous_millis;

#define PROTOCOL HASTENS
#define BITS 18


enum Modes {
  HEAD = 0,
  HEAD_SHAKING = 1,
  LEGS_SHAKING = 2,
  LEGS = 3,
}; 
const int number_of_modes = 4;
int current_mode = HEAD;

enum ShakingSpeed {
  OFF = 0,
  SLOW = 1,
  MEDIUM = 2,
  FAST = 3,
};
const int number_of_speeds = 4;
int current_head_speed = OFF;
int current_legs_speed = OFF;

enum Direction {
  UP = 0,
  DOWN = 1,
};

void setup() {
  CircuitPlayground.begin();

  #ifdef _debug
    Serial.begin(115200);
    while(!Serial) { } // Wait for serial to initialize.
    Serial.println("Ready to send IR");
  #endif

  previous_millis = millis();
  lights_on = true;

}

void set_pixels(){
  CircuitPlayground.clearPixels();
  uint32_t color = 0x101010;
  switch(current_mode) {
    case HEAD:
      CircuitPlayground.strip.setPixelColor((current_position) ? 0 : 9, color);
      break;
    case HEAD_SHAKING:
      if (current_head_speed == SLOW)   color = CircuitPlayground.colorWheel(40);
      if (current_head_speed == MEDIUM) color = CircuitPlayground.colorWheel(25);
      if (current_head_speed == FAST)   color = CircuitPlayground.colorWheel(10);
      CircuitPlayground.strip.setPixelColor((current_position) ? 1 : 8, color);
      break;
    case LEGS_SHAKING:
      if (current_legs_speed == SLOW)   color = CircuitPlayground.colorWheel(40);
      if (current_legs_speed == MEDIUM) color = CircuitPlayground.colorWheel(25);
      if (current_legs_speed == FAST)   color = CircuitPlayground.colorWheel(10);
      CircuitPlayground.strip.setPixelColor((current_position) ? 3 : 6, color);
      break;
    case LEGS:
      CircuitPlayground.strip.setPixelColor((current_position) ? 4 : 5, color);
      break;
  }
  CircuitPlayground.strip.show();
}
void simultaneous_delay(){
  // allow some difference when trying to press buttons simultaneously
  delay(400);
}
void set_headshake_code(enum Direction d){
  if (d == UP){
    current_head_speed++;
  } else {
    current_head_speed--;
  }
  if (current_head_speed < OFF) current_head_speed = OFF;
  if (current_head_speed > FAST) current_head_speed = FAST;
  switch (current_head_speed){
    case OFF:
      code = codes[HEADSHAKE_OFF];
      break;
    case SLOW:
      code = codes[HEADSHAKE_1];
      break;
    case MEDIUM:
      code = codes[HEADSHAKE_2];
      break;
    case FAST:
      code = codes[HEADSHAKE_3];
      break;
  }
}
void set_legsshake_code(enum Direction d){
  if (d == UP){
    current_legs_speed++;
  } else {
    current_legs_speed--;
  }
  if (current_legs_speed < OFF) current_legs_speed = OFF;
  if (current_legs_speed > FAST) current_legs_speed = FAST;
  switch (current_legs_speed){
    case OFF:
      code = codes[LEGSSHAKE_OFF];
      break;
    case SLOW:
      code = codes[LEGSSHAKE_1];
      break;
    case MEDIUM:
      code = codes[LEGSSHAKE_2];
      break;
    case FAST:
      code = codes[LEGSSHAKE_3];
      break;
  }
}
void send_code(uint32_t code) {
  // sends a code to bed, repeat always twice
  CircuitPlayground.irSend.send(PROTOCOL, code, BITS); delay(20);
  CircuitPlayground.irSend.send(PROTOCOL, code, BITS); delay(20);
}
void reset_bed(){
  CircuitPlayground.playTone(2000, 50);
  CircuitPlayground.playTone(1000, 50);
  CircuitPlayground.playTone(750, 50);
  CircuitPlayground.playTone(350, 50);

  for (int i = 0; i < 10 ; i++) {
    CircuitPlayground.strip.setPixelColor(i, CircuitPlayground.colorWheel(0));
  }
  CircuitPlayground.strip.show();
  current_head_speed = 0;
  current_legs_speed = 0;
  for (int i = 0; i < 5; i++) send_code (codes[HEADSHAKE_OFF]);
  delay(200);
  for (int i = 0; i < 5; i++) send_code (codes[LEGSSHAKE_OFF]);
  delay(200);
  for (int i = 0; i < 5; i++) send_code (codes[FLAT]);
  current_mode = HEAD;
}
void read_slider(){
  // select codeset based on slider position
  current_position = CircuitPlayground.slideSwitch();
  if (position_unknown or (current_position != prev_position)) {
    if (current_position){
      // left position - Anita - code set C
      for (int i = 0; i < 13; i++)
        codes[i] = c_codes[i];
    } else {
      // right position - Risto - code set A
      for (int i = 0; i < 13; i++)
        codes[i] = a_codes[i];
    }
    prev_position = current_position;
    position_unknown = false;
    set_pixels();
  }  
}


void loop() {

  // select Anita/Risto
  read_slider();

  // change mode if both buttons pressed simultaneously
  if (CircuitPlayground.leftButton() and CircuitPlayground.rightButton()) {
    CircuitPlayground.playTone(1000, 50);
    CircuitPlayground.playTone(1000, 50);
    current_mode++;
    if ( current_mode >= number_of_modes)
      current_mode = HEAD;
    set_pixels();
  
    unsigned long start_millis = millis();
    while (CircuitPlayground.leftButton() and CircuitPlayground.rightButton()) {
      // loop here until released
      if( millis() - start_millis > 4000) {
        // we have stayed here very long - this must be a reset command
        reset_bed();
      }
    }
    delay(1000);
    set_pixels();
  }

  //// TESTING FLAT
  //if (CircuitPlayground.leftButton() and CircuitPlayground.rightButton()) {
  //  CircuitPlayground.playTone(1000, 50);
  //  CircuitPlayground.playTone(1000, 50);
  //  code = codes[FLAT];
  //  while (CircuitPlayground.leftButton() and CircuitPlayground.rightButton()) {
  //    // loop here until released
  //    send_code(code); 
  //  }
  //}

  // Left button press: lower or slower
  if (CircuitPlayground.leftButton()) {
    simultaneous_delay();
    if (!CircuitPlayground.rightButton()){
      CircuitPlayground.playTone(350, 50);
      switch(current_mode) {
        case HEAD:
          code = codes[HEAD_DOWN];
          break;
        case HEAD_SHAKING:
          set_headshake_code(DOWN);
          break;
        case LEGS:
          code = codes[LEGS_DOWN];
          break;
        case LEGS_SHAKING:
          set_legsshake_code(DOWN);
          break;
      }
      // wait until button released
      while (CircuitPlayground.leftButton()) {
        send_code(code); 
      }
    }
  }

  // Right button press: higher or faster
  if (CircuitPlayground.rightButton()) {
    simultaneous_delay();
    if (!CircuitPlayground.leftButton()) {
      CircuitPlayground.playTone(700, 50);
      switch(current_mode) {
        case HEAD:
          code = codes[HEAD_UP];
          break;
        case HEAD_SHAKING:
          set_headshake_code(UP);
          break;
        case LEGS:
          code = codes[LEGS_UP];
          break;
        case LEGS_SHAKING:
          set_legsshake_code(UP);
          break;
      }
      //wait until button released
      while (CircuitPlayground.rightButton()) {
        send_code(code);
      }
    }
  }

  // blink lights on/off is a shaking mode
  if (current_mode == HEAD_SHAKING or current_mode == LEGS_SHAKING) {
    int period = 1000;
    if (current_mode == HEAD_SHAKING and current_head_speed == SLOW) period /= 4;
    if (current_mode == LEGS_SHAKING and current_legs_speed == SLOW) period /= 4;
    if (current_mode == HEAD_SHAKING and current_head_speed == MEDIUM) period /= 8;
    if (current_mode == LEGS_SHAKING and current_legs_speed == MEDIUM) period /= 8;
    if (current_mode == HEAD_SHAKING and current_head_speed == FAST) period /= 16;
    if (current_mode == LEGS_SHAKING and current_legs_speed == FAST) period /= 16;
    if (millis() - previous_millis >= period) {
      if (lights_on)
        set_pixels();
      else 
        CircuitPlayground.clearPixels();
      lights_on = lights_on ? false : true;
      previous_millis = millis();
    }
  }
}
