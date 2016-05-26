#ifndef PDU_CLASSES_H
#define PDU_CLASSES_H
#include <Arduino.h>

struct PDUOutgoingMessage // исходящее сообщение
{
  int MessageLength; // длина пакета, которую надо вставить в команду AT+CMGS=
  String Message; // само закодированное сообщение в формате UCS2, 16 бит на символ (каждый символ кодируется двумя байтами)
  
};

struct PDUIncomingMessage // входящее сообщение
{
  bool IsDecodingSucceed; // флаг успешности декодирования
  String SMSCenterNumber; // номер телефона СМС-центра, через который прошло сообщение
  String SenderNumber; // телефон, с которого было послано сообщение
  String Message; // текст сообщения в кодировке UTF-8
};


class PDUMessageEncoder // кодировщик сообщений из UTF-8 в UCS2
{
  private:
    unsigned int utf8GetCharSize(unsigned char byte);
    bool utf8ToUInt(const String& bytes, unsigned int& target);
    
    String UTF8ToUCS2(const String& inpString, unsigned int& bytesProcessed);
    String ToHex(int i);
    String EncodePhoneNumber(const String& nm);
  
  public:
    PDUMessageEncoder();
    PDUOutgoingMessage Encode(const String& recipientPhoneNum, const String& utf8Message, bool isFlash); // кодирует сообщение из UTF-8 в UCS2
}; 


class PDUMessageDecoder // декодировщик сообщений из UCS2 в UTF-8
{

  private:

    uint8_t HexToNum(const String& numberS);
    uint8_t MakeNum(char ch);

    String exchangeOctets(const String& src); // меняет соседние октеты местами
    char mapChar(char ch);
    uint8_t DCS_Bits(const String& tp_DCS);
    int UCS2ToUTF8(unsigned long ucs2, unsigned char * utf8);

    String getUTF8From16BitEncoding(const String& ucs2Message);
    String getUTF8From8BitEncoding(const String& ucs2Message);
    String getUTF8From7BitEncoding(const String& ucs2Message, uint16_t trueLength);
  
  public:

    PDUIncomingMessage Decode(const String& ucs2Message, const String& allowedSenderNumber); // декодирует сообщение
    PDUMessageDecoder();
  
}; 

class PDUHelper : public PDUMessageEncoder, public PDUMessageDecoder
{

  public:

  PDUHelper() {}
  
}; 

extern PDUHelper PDU;
#endif
