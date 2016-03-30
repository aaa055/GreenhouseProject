#include "NextionController.h"

char NextionAbstractController::command_buff[NEXTION_COMMAND_BUFFER_LENGTH] = {0};


const char _thsp_FORMAT[] PROGMEM = "thsp=%u";
const char _baud_FORMAT[] PROGMEM = "baud%s=%u";
const char _dim_FORMAT[] PROGMEM = "dim%s=%u";
const char _spa_FORMAT[] PROGMEM = "spa%s=%u";
const char _sendxy_FORMAT[] PROGMEM = "sendxy=%u";
const char _sleep_FORMAT[] PROGMEM = "sleep=%u";
const char _sysvar_FORMAT[] PROGMEM = "sys%u=%lu";
const char _wake_FORMAT[] PROGMEM = "thup=%u";
const char _echo_FORMAT[] PROGMEM = "bkcmd=%u";
const char _page_FORMAT[] PROGMEM = "page %u";
const char _refstop[] PROGMEM = "ref_stop";
const char _refstar[] PROGMEM = "ref_star";

// наши кастомные команды
const char _seg_FORMAT[] PROGMEM = "page0.seg%u.pic=%u";
const char _tmr_FORMAT[] PROGMEM = "va0.val=%u";
const char _wnd_FORMAT[] PROGMEM = "page1.wnd%u.val=%u";
const char _water_FORMAT[] PROGMEM = "page2.water%u.val=%u";
const char _light_FORMAT[] PROGMEM = "page3.light%u.val=%u";
const char _settings_t_FORMAT[] PROGMEM = "page4.t%s%u.pic=%u";
const char _sensor_type_FORMAT[] PROGMEM = "page0.p1.pic=%u";
const char _sensor_desc_FORMAT[] PROGMEM = "page0.p0.pic=%u";



NextionAbstractController::NextionAbstractController() 
{
  workStream = NULL;
  _userData = NULL;
  _onError = NULL;
  _onSuccess = NULL;
  _onPageIDReceived = NULL;
  _onButtonTouch = NULL;
  _onTouch = NULL;
  _onStringReceived = NULL;
  _onNumberReceived = NULL;
  _onSleep = NULL;
  _onWakeUp = NULL;
  _onLaunch = NULL;
  _onUpgrade = NULL;
 
  recvBuff.reserve(NEXTION_COMMAND_BUFFER_LENGTH); // резервируем память под приём команд от Nextion
}
void NextionAbstractController::subscribe(const NextionSubscribeStruct& ss)
{
  _onError = ss.OnError;
  _onSuccess = ss.OnSuccess;
  _onPageIDReceived = ss.OnPageIDReceived;
  _onButtonTouch = ss.OnButtonTouch;
  _onTouch = ss.OnTouch;
  _onStringReceived = ss.OnStringReceived;
  _onNumberReceived = ss.OnNumberReceived;
  _onSleep = ss.OnSleep;
  _onWakeUp = ss.OnWakeUp;
  _onLaunch = ss.OnLaunch;
  _onUpgrade = ss.OnUpgrade;
  
}
void NextionAbstractController::begin(Stream* s, void* userData)
{
  workStream = s;
  _userData = userData;
}
void NextionAbstractController::update()
{
  recvAnswer(); // вычитываем ответ от Nextion
}
void NextionAbstractController::setBaudRate(uint16_t baud, bool setAsDefault)
{
  sprintf_P(command_buff,_baud_FORMAT,setAsDefault ? "s" : "", baud);
  sendCommand(command_buff);  
}
void NextionAbstractController::setBrightness(uint8_t bright, bool setAsDefault)
{
  sprintf_P(command_buff,_dim_FORMAT,setAsDefault ? "s" : "", bright);
  sendCommand(command_buff);   
}
void NextionAbstractController::setFontXSpacing(uint8_t spacing)
{
  sprintf_P(command_buff,_spa_FORMAT,"x", spacing);
  sendCommand(command_buff);    
}
void NextionAbstractController::setFontYSpacing(uint8_t spacing)
{
  sprintf_P(command_buff,_spa_FORMAT,"y", spacing);
  sendCommand(command_buff);    
}
void NextionAbstractController::setSendXY(bool shouldSend)
{
  sprintf_P(command_buff,_sendxy_FORMAT,shouldSend ? 1 : 0);
  sendCommand(command_buff);  
}
void NextionAbstractController::sleep(bool enterSleep)
{
  sprintf_P(command_buff,_sleep_FORMAT,enterSleep ? 1 : 0);
  sendCommand(command_buff);    
}
void NextionAbstractController::setSysVariableValue(uint8_t sysVarNumber,uint32_t val)
{
  sprintf_P(command_buff,_sysvar_FORMAT,sysVarNumber, val);
  sendCommand(command_buff);      
}
void NextionAbstractController::setSleepDelay(uint8_t seconds)
{
  sprintf_P(command_buff,_thsp_FORMAT,seconds);
  sendCommand(command_buff);  
}
void NextionAbstractController::setWakeOnTouch(bool awake)
{
  sprintf_P(command_buff,_wake_FORMAT,awake ? 1 : 0);
  sendCommand(command_buff);    
}
void NextionAbstractController::setEchoMode(NextionEchoMode mode)
{
  sprintf_P(command_buff,_echo_FORMAT,(uint8_t)mode);
  sendCommand(command_buff);      
}
void NextionAbstractController::goToPage(uint8_t pageNum)
{
  sprintf_P(command_buff,_page_FORMAT,pageNum);
  sendCommand(command_buff);        
}
void NextionAbstractController::sendCommand(const char* cmd)
{
  recvAnswer(); // вычитываем ответ от Nextion
  
  if(!workStream)
    return;

    // пишем команду
    workStream->write(cmd);

    // пишем конец пакета - три байта с кодами 0xFF, как написано в документации на дисплей
    static uint8_t endOfPacket[3] = {0xFF,0xFF,0xFF};
    workStream->write(endOfPacket,sizeof(endOfPacket));
  
}
bool NextionAbstractController::gotCommand()
{
  int len = recvBuff.length();
  return (len > 3
  && 0xFF ==(uint8_t) recvBuff[len-1]
  && 0xFF ==(uint8_t) recvBuff[len-2]
  && 0xFF ==(uint8_t) recvBuff[len-3]
  );
  
}
void NextionAbstractController::recvAnswer()
{
  if(!workStream)
    return;

  // получаем ответ от Nextion
  char ch;
  while(workStream->available())
  {
    if(gotCommand()) // если уже есть команда
      processCommand(); // обрабатываем её

      ch = workStream->read();
      recvBuff += ch;
  }

    if(gotCommand()) // если уже есть команда
      processCommand(); // обрабатываем её
  
}
void NextionAbstractController::processCommand()
{
  uint8_t commandType = (uint8_t) recvBuff[0];
  switch(commandType)
  {
    case 0x00: // ret invalid command
    {
      if(_onError)
      _onError(this,etInvalidCommand);
    }
    break;
    
    case 0x01: // ret command finished
    {
      if(_onSuccess)
        _onSuccess(this);
    }
    break;

    case 0x02: // ret invalid component id
    {
      if(_onError)
      _onError(this,etInvalidComponentID);      
    }
    break;

    case 0x03: // ret invalid page id
    {
      if(_onError)
      _onError(this,etInvalidPageID);            
    }
    break;

    case 0x04: // ret invalid picture id
    {
      if(_onError)
      _onError(this,etInvalidPictureID);                  
    }
    break;

    case 0x05: // ret invalid font id
    {      
      if(_onError)
        _onError(this,etInvalidFontID);                  
    }
    break;

    case 0x11: // ret invalid baud
    {
      if(_onError)
        _onError(this,etInvalidBaudRate);                        
    }
    break;

    case 0x65: // ret event touch
    {
      // что лежит в буфере:
      // 0X65+Page ID+Component ID+TouchEvent+End 
      uint8_t pageID = (uint8_t) recvBuff[1];
      uint8_t buttonID = (uint8_t) recvBuff[2];
      bool pressed = 1 == (uint8_t) recvBuff[3];

      if(_onButtonTouch)
        _onButtonTouch(this,pageID,buttonID,pressed);
    }
    break;

    case 0x66: // ret current page id
    {
      // что лежит в буфере:
      // 0X66+Page ID+End
      uint8_t pageID = (uint8_t) recvBuff[1];
      if(_onPageIDReceived)
        _onPageIDReceived(this,pageID);
      
    }
    break;

    case 0x67: // ret event position
    case 0x68: // ret event sleep
    {
      // что лежит в буфере:
      // 0X67+ Coordinate X High-order+Coordinate X Low-order+Coordinate Y High-order+Coordinate Y Low-order+TouchEvent State+End
      uint16_t x = ((uint16_t) recvBuff[1] << 8) | ((uint8_t) recvBuff[2]);
      uint16_t y = ((uint16_t) recvBuff[3] << 8) | ((uint8_t) recvBuff[4]);
      bool pressed = 1 == (uint8_t) recvBuff[5];
      bool inSleep = commandType == 0x68;

      if(_onTouch)
        _onTouch(this,x,y,pressed,inSleep);
      
    }
    break;

    
    case 0x70: // ret string
    {
      recvBuff[recvBuff.length()-3] = '\0'; // маскируем конец пакета
      const char* ptr = recvBuff.c_str();
      ptr++; // переходим на начало данных
      
      if(_onStringReceived)
        _onStringReceived(this,ptr);
    }
    break;

    case 0x71: // ret number
    {
      // что лежит в буфере:
      // 0x71, b1,b2,b3,b4,0xFF,0xFF,0xFF
      int readIdx = recvBuff.length()-4; // переходим на первый значащий байт
      uint32_t num = ((uint32_t)recvBuff[readIdx] << 24) | ((uint32_t)recvBuff[readIdx-1] << 16) | ((uint32_t)recvBuff[readIdx-2] << 8) | recvBuff[readIdx-3];

      if(_onNumberReceived)
        _onNumberReceived(this,num);
      
    }
    break;

    case 0x86: // enter sleep mode
    {
      if(_onSleep)
        _onSleep(this);
    }
    break;

    case 0x87: // wake up
    {
      if(_onWakeUp)
        _onWakeUp(this); 
    }
    break;

    case 0x88: // ret event launched
    {
      if(_onLaunch)
        _onLaunch(this); 
    }
    break;

    case 0x89: // ret event upgraded
    {
      if(_onUpgrade)
        _onUpgrade(this);
    }
    break;
    
  } // switch

  recvBuff = ""; // очищаем буфер
}

/////////////////////////////////////////////////////////////////////////
// Реализация нашего кастомного управления Nextion
/////////////////////////////////////////////////////////////////////////
NextionController::NextionController() : NextionAbstractController()
{
  
}
void NextionController::setSegmentInfo(uint8_t segNum,uint8_t charStartAddress)
{
  sprintf_P(command_buff,_seg_FORMAT,segNum, (charStartAddress + segNum));  
  sendCommand(command_buff);
}
void NextionController::setWaitTimerInterval(uint16_t val)
{
  sprintf_P(command_buff,_tmr_FORMAT,val);
  sendCommand(command_buff);
}
void NextionController::notifyWindowState(bool isWindowsOpen)
{
  sprintf_P(command_buff,_wnd_FORMAT,0,isWindowsOpen ? 1 : 0);
  sendCommand(command_buff);        
}
void NextionController::notifyWindowMode(bool isAutoMode)
{
  sprintf_P(command_buff,_wnd_FORMAT,1,isAutoMode ? 1 : 0);
  sendCommand(command_buff);          
}
void NextionController::notifyWaterState(bool isWaterOn)
{
  sprintf_P(command_buff,_water_FORMAT,0,isWaterOn ? 1 : 0);
  sendCommand(command_buff);          
}
void NextionController::notifyWaterMode(bool isAutoMode)
{
  sprintf_P(command_buff,_water_FORMAT,1,isAutoMode ? 1 : 0);
  sendCommand(command_buff);          
}
void NextionController::notifyLightState(bool isLightOn)
{
  sprintf_P(command_buff,_light_FORMAT,0,isLightOn ? 1 : 0);
  sendCommand(command_buff);          
}
void NextionController::notifyLightMode(bool isAutoMode)
{
  sprintf_P(command_buff,_light_FORMAT,1,isAutoMode ? 1 : 0);
  sendCommand(command_buff);          
}
uint8_t NextionController::fillEmptySpaces(uint8_t pos_written)
{
 while(pos_written < NEXTION_CHAR_PLACES)
 {
  setSegmentInfo(pos_written,EMPTY_CELLS_START_ADDRESS);
  pos_written++;
 }
    return pos_written;
}
void NextionController::doShowSettingsTemp(uint8_t temp,const char* which, uint8_t offset)
{
  sprintf_P(command_buff,_refstop);
  sendCommand(command_buff);
  uint8_t decimals = temp/10;
  uint8_t ones = temp%10;

  // для первой цифры у нас позиция индикатора 0
  sprintf_P(command_buff,_settings_t_FORMAT,which,0,DIGITS_START_ADDRESS+decimals*NEXTION_CHAR_PLACES + offset);
  sendCommand(command_buff);

  // для второй цифры у нас позиция индикатора 1
  sprintf_P(command_buff,_settings_t_FORMAT,which,1,DIGITS_START_ADDRESS+ones*NEXTION_CHAR_PLACES + 1 + offset);
  sendCommand(command_buff); 

  sprintf_P(command_buff,_refstar);
  sendCommand(command_buff);
}
void NextionController::showOpenTemp(uint8_t temp)
{
 doShowSettingsTemp(temp);
}
void NextionController::showCloseTemp(uint8_t temp)
{
  doShowSettingsTemp(temp,"close",4);
}
void NextionController::showLuminosity(long lum)
{
  sprintf_P(command_buff,_refstop);
  sendCommand(command_buff);
  
  sprintf_P(command_buff,_sensor_type_FORMAT,10);
  sendCommand(command_buff); // показываем лампу как тип датчика

  sprintf_P(command_buff,_sensor_desc_FORMAT,13);
  sendCommand(command_buff); // показываем надпись "Освещенность"

  uint8_t pos_written = showNumber(lum); // сколько позиций записано?

  // теперь пишем знак lux
  if(pos_written < NEXTION_CHAR_PLACES)
  {
    setSegmentInfo(pos_written,LUX_START_ADDRESS); // написали знак lux
    pos_written++;
  }

 // добиваем всё пустыми символами
 fillEmptySpaces(pos_written);
 
 sprintf_P(command_buff,_refstar);
 sendCommand(command_buff);
  
}
void NextionController::showHumidity(const Humidity& h)
{
  sprintf_P(command_buff,_refstop);
  sendCommand(command_buff);

  sprintf_P(command_buff,_sensor_type_FORMAT,9);
  sendCommand(command_buff); // показываем каплю как тип датчика

  sprintf_P(command_buff,_sensor_desc_FORMAT,12);
  sendCommand(command_buff); // показываем надпись "Влажность"

  uint8_t pos_written = showNumber(h.Value); // сколько позиций записано?

  // теперь пишем запятую
  if(pos_written < NEXTION_CHAR_PLACES)
  {
    setSegmentInfo(pos_written,DOT_START_ADDRESS); // запятую написали
    pos_written++;
  }
  // пишем значение после запятой
  pos_written = showNumber(h.Fract,pos_written,h.Fract < 10);

  // теперь пишем знак процента
  if(pos_written < NEXTION_CHAR_PLACES)
  {
    setSegmentInfo(pos_written,PERCENT_START_ADDRESS); // написали знак процента
    pos_written++;
  }

 // добиваем всё пустыми символами
 fillEmptySpaces(pos_written);
 
  sprintf_P(command_buff,_refstar);
  sendCommand(command_buff);
  
}
void NextionController::showTemperature(const Temperature& t)
{
  sprintf_P(command_buff,_refstop);
  sendCommand(command_buff);

  sprintf_P(command_buff,_sensor_type_FORMAT,11);
  sendCommand(command_buff); // показываем градусник как тип датчика

  sprintf_P(command_buff,_sensor_desc_FORMAT,14);
  sendCommand(command_buff); // показываем надпись "Температура"

  uint8_t pos_written = showNumber(t.Value); // сколько позиций записано?

  // теперь пишем запятую
  if(pos_written < NEXTION_CHAR_PLACES)
  {
    setSegmentInfo(pos_written,DOT_START_ADDRESS); // запятую написали
    pos_written++;
  }
  // пишем значение после запятой
  pos_written = showNumber(t.Fract,pos_written,t.Fract < 10);

  // теперь пишем знак градуса
  if(pos_written < NEXTION_CHAR_PLACES)
  {
    setSegmentInfo(pos_written,CELSIUS_START_ADDRESS); // написали знак градуса
    pos_written++;
  }

  // добиваем всё пустыми символами
  fillEmptySpaces(pos_written);
 
  sprintf_P(command_buff,_refstar);
  sendCommand(command_buff);

}
uint8_t NextionController::showNumber(long num,uint8_t segNum,bool addLeadingZero)
{

  if(segNum >= NEXTION_CHAR_PLACES) // плохой, негодный индекс
    return segNum;
    
  uint8_t written = segNum;
  // максимальное число у нас, которое мы можем отобразить - занимает NEXTION_CHAR_PLACES позиций.
  // максимальный буфер, который мы можем занять: знак 'минус' + ведущий ноль + длина представления long в виде строки.
  // однако, мы не отображаем показания более чем 7 символов, т.к. их некуда вместить.
  // следовательно, буфер у нас будет из "7 + минус + ведущий ноль + знак конца строки" = 10 символов.

   // для начала проверим, удовлетворяет ли переданным условиям число?
   if(abs(num) > 65535) // 5 символов плюс возможный минус плюс возможный ведущий ноль - итого 7
    return written; // ничего не можем писать

    bool isNegative = num < 0;
    if(isNegative)
      num = -num;

    static char buff[20] = {0}; // наш буфер для записи значений
    sprintf(buff,"%s%u",(addLeadingZero ? "0" : ""), (unsigned int)num);

    // буфер сформирован, смотрим, не отрицательное ли число
 if(isNegative)
  {
    // минус начинается с адреса MINUS_START_ADDR
    setSegmentInfo(written,MINUS_START_ADDR);
    written++;
  }

   // теперь проходимся по всем цифрам в массиве и пишем их в дисплей
   const char* ptr = buff;
   while(*ptr && written < NEXTION_CHAR_PLACES) // пока не достигли конца сегментов или конца строки
   {
    uint8_t digInfo = *ptr - '0';
    ptr++;
    setSegmentInfo(written,DIGITS_START_ADDRESS + digInfo*NEXTION_CHAR_PLACES);
    written++;
   } // while

 return written;     
}



