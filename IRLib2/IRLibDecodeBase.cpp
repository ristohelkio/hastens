/* IRLibDecodeBase.cpp 
 * Part of IRLib Library for Arduino receiving, decoding, and sending
 * infrared signals. See COPYRIGHT.txt and LICENSE.txt for more information.
 */
/*
 * This module contains the base classes for decoding. You will not create instances
 * of these classes, rather you will use them as base classes in creating derived
 * protocol specific decoders.
 */

#if !defined(ARDUINO_NRF52840_CIRCUITPLAY)

#include "IRLibDecodeBase.h"
#include "IRLibHardware.h"

IRdecodeBase::IRdecodeBase(void) {
  recvGlobal.decoderWantsData=false; //turned on by enableIRIn.
  recvGlobal.decodeBuffer=recvGlobal.recvBuffer;//default buffer
  ignoreHeader=false;
  resetDecoder();
};

/*
 * Reinitialize the decoder clearing out previous data.
 */
void IRdecodeBase::resetDecoder(void) {
  protocolNum= UNKNOWN;
  value=0;
  address=0;
  bits=0;
};
void IRdecodeBase::dumpResults(bool verbose) {
  int i;uint32_t Extent;int interval;
  if((protocolNum>89) || (protocolNum<=LAST_PROTOCOL)) {
    Serial.print(F("Decoded ")); Serial.print(Pnames(protocolNum));
    Serial.print(F("(")); Serial.print(protocolNum,DEC);
    Serial.print(F("): Value:")); Serial.print(value, HEX);
    Serial.print(F(" Adrs:" )); Serial.print(address, HEX);
  };
  Serial.print(F(" ("));  Serial.print(bits, DEC); Serial.print(F(" bits) "));
  if(recvGlobal.didAutoResume) Serial.print(F("Auto Resumed"));
  Serial.println();
  if(!verbose)
    return;
  Serial.print(F("Raw samples(")); Serial.print(recvGlobal.decodeLength, DEC);
  Serial.print(F("): Gap:")); Serial.println(recvGlobal.decodeBuffer[0], DEC);
  Serial.print(F("  Head: m")); Serial.print(recvGlobal.decodeBuffer[1], DEC);
  Serial.print(F("  s")); Serial.println(recvGlobal.decodeBuffer[2], DEC);
  int LowSpace= 32767; int LowMark=  32767;
  int HiSpace=0; int HiMark=  0;
  Extent=recvGlobal.decodeBuffer[1]+recvGlobal.decodeBuffer[2];
  for (i = 3; i < recvGlobal.decodeLength; i++) {
    Extent+=(interval= recvGlobal.decodeBuffer[i]);
    if (i % 2) {
      LowMark=min(LowMark, interval);  HiMark=max(HiMark, interval);
      Serial.print(i/2-1,DEC);  Serial.print(F(":m"));
    } 
    else {
       if(interval>0)LowSpace=min(LowSpace, interval);  HiSpace=max (HiSpace, interval);
       Serial.print(F(" s"));
    }
    Serial.print(interval, DEC);
    int j=i-1;
    if ((j % 2)==1)Serial.print(F("\t"));
    if ((j % 4)==1)Serial.print(F("\t "));
    if ((j % 8)==1)Serial.println();
    if ((j % 32)==1)Serial.println();
  }
  Serial.println();
  Serial.print(F("Extent="));  Serial.println(Extent,DEC);
  Serial.print(F("Mark  min:")); Serial.print(LowMark,DEC);Serial.print(F("\t max:")); Serial.println(HiMark,DEC);
  Serial.print(F("Space min:")); Serial.print(LowSpace,DEC);Serial.print(F("\t max:")); Serial.println(HiSpace,DEC);
  Serial.println();
}

void IRdecodeBase::dumpResultsRH(bool verbose) {
  if (recvGlobal.decodeLength != 36){
    //Serial.print('.');
    return;
  }
  // no mark or space should be long - a new message should have started
  for (int i = 1; i < recvGlobal.decodeLength; i++) {
    if (recvGlobal.decodeBuffer[i] > 4000) return;
  }
  
  // RH modification
  bool puhelias = false;
  // RH modification

  int i;
  uint32_t Extent;
  int interval;

  if (puhelias) Serial.println();
  if(!verbose)
    return;

  //  return;
  if (puhelias) {
    Serial.print(F("Samples(")); Serial.print(recvGlobal.decodeLength, DEC);Serial.print(") ");
    Serial.print(F("): Gap:")); Serial.println(recvGlobal.decodeBuffer[0], DEC);
  }

  int LowSpace=32767;
  int LowMark=32767;
  int HiSpace=0;
  int HiMark=  0;
  //long unsigned shortSum = 0;
  //long unsigned longSum = 0;

  if (puhelias) {
    Serial.println("Buffer contents:");
    for (i = 0; i < recvGlobal.decodeLength; i++) {
      Serial.print(recvGlobal.decodeBuffer[i]); Serial.print(" ");
    }
    Serial.println();
  }
  
  char s[18];
  uint32_t data;
  data=0;
  for (int i = 1; i < recvGlobal.decodeLength; i += 2) {
    if (i == (recvGlobal.decodeLength - 1)){
      // last char, just look at the length of the second last bit, last space
      if ( recvGlobal.decodeBuffer[i] > 2000){
        // it is a one
        data = (data << 1) | 1;
        s[i/2] = '1';
      } else if (recvGlobal.decodeBuffer[i] < 500){
        // it is a zero
        data <<= 1;
        s[i/2] = '0';
      }
       else {
        s[i/2] = '-';
      }
    } else {
      // normal chars look at the lenghts of marks and spaces
      if (( recvGlobal.decodeBuffer[i] > 2000) and (recvGlobal.decodeBuffer[i+1] < 500)){
        // it is a one
        data = (data << 1) | 1;
        s[i/2] = '1';
      } else if ((recvGlobal.decodeBuffer[i] < 500) and (recvGlobal.decodeBuffer[i+1] > 2000)){
        // it is a zero
        data <<= 1;
        s[i/2] = '0';
      } else {
        s[i/2] = '-';
      }
    }
  }
  
  Serial.print(data, HEX); Serial.print(" ");Serial.print(s); 

  // POSITION A
  static const char AFlat[]         PROGMEM = "101010110000000000";
  static const char AHeadUp[]       PROGMEM = "111010110011000000";  // ARVAUS!!!
  static const char AHeadDown[]     PROGMEM = "111010110000110000";
  static const char ALegsUp[]       PROGMEM = "111010110000001100";
  static const char ALegsDown[]     PROGMEM = "111010110000000011";
  static const char AHeadShake1[]   PROGMEM = "101011110011000000"; // 2BCC0
  static const char AHeadShake2[]   PROGMEM = "101011110000110000";
  static const char AHeadShake3[]   PROGMEM = "101011110000001100";
  static const char AHeadShakeOff[] PROGMEM = "101011110000000011"; // 2BC03
  static const char ALegsShake1[]   PROGMEM = "101110110011000000";
  static const char ALegsShake2[]   PROGMEM = "101110110000110000";
  static const char ALegsShake3[]   PROGMEM = "101110110000001100";
  static const char ALegsShakeOff[] PROGMEM = "101110110000000011";
  // POSITION B
  static const char BFlat[]         PROGMEM = "101010100000000000";
  static const char BHeadUp[]       PROGMEM = "111010100011000000";  // ARVAUS!!!
  static const char BHeadDown[]     PROGMEM = "111010100000110000";
  static const char BLegsUp[]       PROGMEM = "111010100000001100";
  static const char BLegsDown[]     PROGMEM = "111010100000000011";
  static const char BHeadShake1[]   PROGMEM = "101011100011000000";
  static const char BHeadShake2[]   PROGMEM = "101011100000110000";
  static const char BHeadShake3[]   PROGMEM = "101011100000001100";
  static const char BHeadShakeOff[] PROGMEM = "101011100000000011";
  static const char BLegsShake1[]   PROGMEM = "101110100011000000";
  static const char BLegsShake2[]   PROGMEM = "101110100000110000";
  static const char BLegsShake3[]   PROGMEM = "101110100000001100";
  static const char BLegsShakeOff[] PROGMEM = "101110100000000011";
  // POSITION C
  static const char CFlat[]         PROGMEM = "101010000000000000";
  static const char CHeadUp[]       PROGMEM = "111010000011000000";  // ARVAUS!!!
  static const char CHeadDown[]     PROGMEM = "111010000000110000";
  static const char CLegsUp[]       PROGMEM = "111010000000001100";
  static const char CLegsDown[]     PROGMEM = "111010000000000011";
  static const char CHeadShake1[]   PROGMEM = "101011000011000000";
  static const char CHeadShake2[]   PROGMEM = "101011000000110000";
  static const char CHeadShake3[]   PROGMEM = "101011000000001100";
  static const char CHeadShakeOff[] PROGMEM = "101011000000000011";
  static const char CLegsShake1[]   PROGMEM = "101110000011000000";
  static const char CLegsShake2[]   PROGMEM = "101110000000110000";
  static const char CLegsShake3[]   PROGMEM = "101110000000001100";
  static const char CLegsShakeOff[] PROGMEM = "101110000000000011";

  if (strcmp(s, AFlat)   == 0)       Serial.print(" == A flat");
  if (strcmp(s, AHeadUp)   == 0)     Serial.print(" == A head up");
  if (strcmp(s, AHeadDown)   == 0)   Serial.print(" == A head down");
  if (strcmp(s, AHeadShake1) == 0)   Serial.print(" == A head shake 1");
  if (strcmp(s, AHeadShake2) == 0)   Serial.print(" == A head shake 2");
  if (strcmp(s, AHeadShake3) == 0)   Serial.print(" == A head shake 3");
  if (strcmp(s, AHeadShakeOff) == 0) Serial.print(" == A head shake OFF");
  if (strcmp(s, ALegsUp)     == 0)   Serial.print(" == A legs up");
  if (strcmp(s, ALegsDown)   == 0)   Serial.print(" == A legs down");
  if (strcmp(s, ALegsShake1) == 0)   Serial.print(" == A legs shake 1");
  if (strcmp(s, ALegsShake2) == 0)   Serial.print(" == A legs shake 2");
  if (strcmp(s, ALegsShake3) == 0)   Serial.print(" == A legs shake 3");
  if (strcmp(s, ALegsShakeOff) == 0) Serial.print(" == A legs shake OFF");

  if (strcmp(s, BFlat)   == 0)       Serial.print(" == B flat");
  if (strcmp(s, BHeadUp)   == 0)     Serial.print(" == B head up");
  if (strcmp(s, BHeadDown)   == 0)   Serial.print(" == B head down");
  if (strcmp(s, BHeadShake1) == 0)   Serial.print(" == B head shake 1");
  if (strcmp(s, BHeadShake2) == 0)   Serial.print(" == B head shake 2");
  if (strcmp(s, BHeadShake3) == 0)   Serial.print(" == B head shake 3");
  if (strcmp(s, BHeadShakeOff) == 0) Serial.print(" == B head shake OFF");
  if (strcmp(s, BLegsUp)     == 0)   Serial.print(" == B legs up");
  if (strcmp(s, BLegsDown)   == 0)   Serial.print(" == B legs down");
  if (strcmp(s, BLegsShake1) == 0)   Serial.print(" == B legs shake 1");
  if (strcmp(s, BLegsShake2) == 0)   Serial.print(" == B legs shake 2");
  if (strcmp(s, BLegsShake3) == 0)   Serial.print(" == B legs shake 3");
  if (strcmp(s, BLegsShakeOff) == 0) Serial.print(" == B legs shake OFF");

  if (strcmp(s, CFlat)   == 0)       Serial.print(" == C flat");
  if (strcmp(s, CHeadUp)   == 0)     Serial.print(" == C head up");
  if (strcmp(s, CHeadDown)   == 0)   Serial.print(" == C head down");
  if (strcmp(s, CHeadShake1) == 0)   Serial.print(" == C head shake 1");
  if (strcmp(s, CHeadShake2) == 0)   Serial.print(" == C head shake 2");
  if (strcmp(s, CHeadShake3) == 0)   Serial.print(" == C head shake 3");
  if (strcmp(s, CHeadShakeOff) == 0) Serial.print(" == C head shake OFF");
  if (strcmp(s, CLegsUp)     == 0)   Serial.print(" == C legs up");
  if (strcmp(s, CLegsDown)   == 0)   Serial.print(" == C legs down");
  if (strcmp(s, CLegsShake1) == 0)   Serial.print(" == C legs shake 1");
  if (strcmp(s, CLegsShake2) == 0)   Serial.print(" == C legs shake 2");
  if (strcmp(s, CLegsShake3) == 0)   Serial.print(" == C legs shake 3");
  if (strcmp(s, CLegsShakeOff) == 0) Serial.print(" == C legs shake OFF");
  Serial.println();

  if (puhelias) {
    //Serial.print(F("Extent (merkkijonon pitus) = "));  Serial.print(Extent,DEC); Serial.println(" microseconds");
    Serial.print(F("Mark  min:")); Serial.print(LowMark,DEC);Serial.print(F("\t max:")); Serial.println(HiMark,DEC);
    Serial.print(F("Space min:")); Serial.print(LowSpace,DEC);Serial.print(F("\t max:")); Serial.println(HiSpace,DEC);
    Serial.println();
  }
}

/* We use a generic routine because most protocols have the same basic structure. 
 * Previous versions of this method would handle protocols with variable marks
 * or variable spaces. However we have discovered that only Sony protocol uses
 * variable marks so we have stripped out that portion of the code. This changes
 * the number of necessary parameters. We no longer need markOne and markZero
 * because they are both the same which we will pass in markData. Note this new
 * version will handle up to 48 bits putting the most significant 16 bits in
 * "this.address" in the least significant 32 bits in "this.data". We could have 
 * allowed for 64 bit but we have not seen generic protocols that large.
 */
bool IRdecodeBase::decodeGeneric(uint8_t expectedLength, 
      uint16_t headMark, uint16_t headSpace,  uint16_t markData, 
      uint16_t spaceOne, uint16_t spaceZero) {
   resetDecoder();//This used to be in the receiver getResults.
// If "expectedLenght" or "headMark" or "headSpace" are zero or if "ignoreHeader"
// is true then don't perform these tests. This is because some protocols need 
// to do their own custom header work.
  uint64_t data = 0;  
  bufIndex_t Max=recvGlobal.decodeLength-1; 
  if (expectedLength) {
    if (recvGlobal.decodeLength != expectedLength) return RAW_COUNT_ERROR;
  }
  if(!ignoreHeader) {
    if (headMark) {
      if (!MATCH(recvGlobal.decodeBuffer[1],headMark)) return HEADER_MARK_ERROR(headMark);
    }
  }
  if (headSpace) {
    if (!MATCH(recvGlobal.decodeBuffer[2],headSpace)) return HEADER_SPACE_ERROR(headSpace);
  }
  offset=3;//skip initial gap plus two header items
  while (offset < Max) {
    if (!MATCH (recvGlobal.decodeBuffer[offset],markData)) return DATA_MARK_ERROR(markData);
    offset++;
    if (MATCH(recvGlobal.decodeBuffer[offset],spaceOne)) {
      data = (data << 1) | 1;
    } 
    else if (MATCH (recvGlobal.decodeBuffer[offset],spaceZero)) {
      data <<= 1;
    } 
    else return DATA_SPACE_ERROR(spaceZero);
    offset++;
  }
  bits = (offset - 1) / 2 -1;//didn't encode stop bit
  // Success
  value = (uint32_t)data;           //low order 32 bits
  address = (uint16_t) (data>>32);  //high order 16 bits
  return true;
}

/* 
 * These MATCH methods used to be macros but we saved nearly
 * 800 bytes of program space by making them actual methods.
 */
 
bool IRdecodeBase::MATCH(int16_t val,int16_t expected){
#ifdef IRLIB_USE_PERCENT
  return (val >= (uint16_t)(expected*(1.0-PERCENT_TOLERANCE/100.0)))
      && (val <= (uint16_t)(expected*(1.0+PERCENT_TOLERANCE/100.0))); 
#else
  return  ABS_MATCH(val,expected,DEFAULT_ABS_TOLERANCE);
#endif
}
bool IRdecodeBase::ABS_MATCH(int16_t val,int16_t expected,int16_t tolerance){
  return  (val >= (expected-tolerance)) && (val <= (expected+tolerance));
}

/*
 * The RC5 and RC6 and similar protocols used phase encoding and leave a 
 * routine to extract zeros and ones. This routine gets one undecoded 
 * level at a time from the raw buffer. personally The RC5/6 decoding 
 * is easier if the data is broken into time intervals.
 * E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
 * successive calls to getRClevel will return MARK, MARK, SPACE.
 * The variables "offset" and "used" are updated to keep track of the 
 * current position. The variable "t1" is the time interval for a single 
 * bit in microseconds. Returns ERROR if the measured time interval is 
 * not a multiple of "t1".
 */
IRdecodeRC::RCLevel IRdecodeRC::getRClevel(uint8_t *used, const uint16_t t1) {
  if (offset >= recvGlobal.decodeLength) {
    // After end of recorded buffer, assume SPACE.
    return SPACE;
  }
  uint16_t width = recvGlobal.decodeBuffer[offset];
  IRdecodeRC::RCLevel val;
  if ((offset) % 2) val=MARK; else val=SPACE;
  uint8_t avail;
  if (MATCH(width, t1)) {
    avail = 1;
  } 
  else if (MATCH(width, 2*t1)) {
    avail = 2;
  } 
  else if (MATCH(width, 3*t1)) {
    avail = 3;
  } else {
    if((ignoreHeader) && (offset==1) && (width<t1)){
      avail =1;
    } else {
      return ERROR;
    }
  }
  (*used)++;
  if (*used >= avail) {
    *used = 0;
    (offset)++;
  }
  return val;   
}

#endif //!defined(__NRF52) 
