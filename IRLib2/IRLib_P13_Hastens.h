/* IRLib_P13_Hastens.h
 * Part of IRLib Library for Arduino receiving, decoding, and sending
 * infrared signals. See COPYRIGHT.txt and LICENSE.txt for more information.
 */
/*
 * This is for Hästens bed from the 90's
 * Risto Helkiö
 * 20.1.2021
  *
 * The header is mark(2900) space(415).
 * The header is followed by 17 bit code (6 action, 2 code (A/B/C), 10 subaction)
 * A "1" is mark (2900) space(415)
 * A "0" is mark(415) space(2900)

  Hästens-sängyn IR-protokolla
  
  sekvenssissä 18 merkkiä
  toinen merkki mark 2900 microsecs, space 415 microsecs  '1'
  toinen merkki mark 415 microsecs, space 2900 microsecs  '0'

  101011 10 0011000000
    |     |      |______ 10 merkkiä, toimintakoodin tarkennus (0011000000, 0000110000, 0000001100, 0000000011)
    |     |_____________ 2 merkkiä, kaukosäätimen koodi (A = 11, B = 10, C = 00)
    |___________________ 6 merkkiä, toimintokoodi (legsShake = 101110, flatten = 101010, headShake = 101011, up/down = 111010)



 */

#ifndef IRLIB_PROTOCOL_13_H
#define IRLIB_PROTOCOL_13_H
#define IR_SEND_PROTOCOL_13		case 13: IRsendHastens::send(data); break;
#define IR_DECODE_PROTOCOL_13	if(IRdecodeHastens::decode()) return true;
#ifdef IRLIB_HAVE_COMBO
	#define PV_IR_DECODE_PROTOCOL_13 ,public virtual IRdecodeHastens
	#define PV_IR_SEND_PROTOCOL_13   ,public virtual IRsendHastens
#else
	#define PV_IR_DECODE_PROTOCOL_13  public virtual IRdecodeHastens
	#define PV_IR_SEND_PROTOCOL_13    public virtual IRsendHastens
#endif

#ifdef IRLIBSENDBASE_H
class IRsendHastens: public virtual IRsendBase {
public:
    void send(uint32_t data) {
      enableIROut(38);
      //enableIROut(36);
      //Serial.println("Sending Hästens protocol");
      //Serial.println(data,HEX);
      for (uint8_t i = 0; i < 18; i++) {
        if (data & 0x20000) {
          mark(2900);  space(415);
        } else {
          mark(415);  space(2900);
        };
        data <<= 1;
      };
    };
};
#endif  //IRLIBSENDBASE_H

#ifdef IRLIBDECODEBASE_H
class IRdecodeHastens: public virtual IRdecodeBase {
  public:
    bool decode(void) {
      IRLIB_ATTEMPT_MESSAGE(F("Hastens"));
      if (recvGlobal.decodeLength != 36) return RAW_COUNT_ERROR;
      if (!MATCH(recvGlobal.decodeBuffer[1],2900))  return HEADER_MARK_ERROR(2900);
      if (!MATCH(recvGlobal.decodeBuffer[2],415))  return HEADER_SPACE_ERROR(415);

      // no mark or space should be long - a new message should have started
      for (int i = 1; i < recvGlobal.decodeLength; i++) {
        if (recvGlobal.decodeBuffer[i] > 4000) return HEADER_SPACE_ERROR(recvGlobal.decodeBuffer[i]);
      }

      // get bits
      uint32_t data;
      data=0;
      for (int i = 1; i < recvGlobal.decodeLength; i += 2) {
        if (i == (recvGlobal.decodeLength - 1)){
          // last char, just look at the length of the second last bit, last space
          if (MATCH(recvGlobal.decodeBuffer[i], 2900)){
            // it is a one
            data = (data << 1) | 1;
          } else if (MATCH(recvGlobal.decodeBuffer[i], 415)){
            // it is a zero
            data <<= 1;
          } else {
            return false;
          }
        } else {
          // normal chars look at the lenghts of marks and spaces
          if (MATCH(recvGlobal.decodeBuffer[i], 2900) and MATCH(recvGlobal.decodeBuffer[i+1], 415)){
            // it is a one
            data = (data << 1) | 1;
          } else if (MATCH(recvGlobal.decodeBuffer[i], 415) and MATCH(recvGlobal.decodeBuffer[i+1], 2900)){
            // it is a zero
            data <<= 1;
          } else {
            return false;
          }
        }
      }
      bits = 36;
      value = data;
      protocolNum = HASTENS;
      return true;
    }
};

#endif //IRLIBDECODEBASE_H

#define IRLIB_HAVE_COMBO

#endif //IRLIB_PROTOCOL_13_H
