#include "SMSModule.h"
#include "ModuleController.h"
#include "PDUClasses.h"
#include "InteropStream.h"

void SMSModule::Setup()
{
  Settings = GetController()->GetSettings();

  // будем смотреть этот пин на предмет наличия питания у модуля NEOWAY
  pinMode(NEOWAY_VCCIO_CHECK_PIN,INPUT);
  
  // запускаем наш сериал
  NEOWAY_SERIAL.begin(NEOWAY_BAUDRATE);
   
  // настройка модуля тут
 }
void SMSModule::SendToNeoway(const String& s, bool addNewLine)
{
  neowayAnswer = F("");
  currentCommand = s;
  
  #ifdef NEOWAY_DEBUG_MODE
    Serial.println("Send the \"" + s + "\" command to NEOWAY...");
  #endif
  
  NEOWAY_SERIAL.write(s.c_str(),s.length());
  
  if(addNewLine)
    NEOWAY_SERIAL.write(String(NEWLINE).c_str());
    
  NEOWAY_SERIAL.flush();
}
bool SMSModule::IsNeowayAnswerCompleted(const String& s, bool& isOkAnswer)
{
  if(s.indexOf(F("OK")) != -1)
  {
    isOkAnswer = true;
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("IsNeowayAnswerCompleted, answer is OK!");
    #endif
    return true;
  }

  if(s.indexOf(F("ERROR")) != - 1)
  {
    isOkAnswer = false;
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("IsNeowayAnswerCompleted, answer is ERROR!");
    #endif
    return true;
  }

  return false;
}
bool SMSModule::IsNeowayReady()
{
    
    SendToNeoway(F("AT+CPAS"));
    
    bool isOkAnswer;
    FetchNeowayAnswer(isOkAnswer);

    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("Check NEOWAY ready, received: \"" + neowayAnswer + "\"");
    #endif

    return isOkAnswer && neowayAnswer.indexOf(F("+CPAS: 0")) != -1;
    
}
void SMSModule::FetchNeowayAnswer(bool& isOkAnswer)
{
  unsigned long timeout_guard = millis() + 2000; 
  while(1)
  {
    // Вот здесь можем зависнуть, поэтому делаем таймаут
    if(millis() >= timeout_guard) // за две секунды не приняли ничего
    {
      #ifdef NEOWAY_DEBUG_MODE
        Serial.println("FetchNeowayAnswer - TIMEOUT REACHED!");
      #endif
      
      isOkAnswer = false;
      break;
    }
    
    while(NEOWAY_SERIAL.available())
      neowayAnswer += (char) NEOWAY_SERIAL.read();

    if(IsNeowayAnswerCompleted(neowayAnswer, isOkAnswer))
    {
      #ifdef NEOWAY_DEBUG_MODE
        Serial.println("Answer received successfully, continue to work...");
      #endif
      
      break;
    }
    
  } // while 1
}
void SMSModule::ProcessQueuedWindowCommand(uint16_t dt)
{
    if(!queuedWindowCommand.length()) // а нет команды на управление окнами
    {
      queuedTimer = 0; // обнуляем таймер
      return;
    }

    queuedTimer += dt;
    if(queuedTimer < 3000) // не дёргаем чаще, чем раз в три секунды
      return;

    queuedTimer = 0; // обнуляем таймер ожидания

    //ModuleController* c = GetController();
    //CommandParser* cParser = c->GetCommandParser();
  
    // для начала проверяем, в каком состоянии у нас окна
      //Command cmd;
      //if(cParser->ParseCommand(F("CTGET=STATE|WINDOW|ALL"), c->GetControllerID(), cmd))
      if(ModuleInterop.QueryCommand(ctGET,F("STATE|WINDOW|ALL"),false))
      {
        #ifdef NEOWAY_DEBUG_MODE
          Serial.println("CTGET=STATE|WINDOW|ALL command parsed, process it...");
        #endif
    
        //ModuleInterop.Clear(); // очищаем ответ, который будет после вызова команды ProcessModuleCommand
        //cmd.SetInternal(false); // говорим, что команда - как бы от юзера, контроллер после выполнения команды перейдёт в ручной режим
       // cmd.SetIncomingStream(&ModuleInterop); // говорим, чтобы модуль плевался ответами в класс взаимодействия между модулями
      //  c->ProcessModuleCommand(cmd,false);

        // теперь проверяем ответ. Если окна не в движении - нам вернётся OPEN или CLOSED последним параметром.
        // только в этом случае мы можем исполнять команду
        String streamAnswer = ModuleInterop.GetData();
        streamAnswer.trim();
        int16_t idx = streamAnswer.lastIndexOf(PARAM_DELIMITER);
        if(idx != -1)
        {
          String state = streamAnswer.substring(idx+1,streamAnswer.length());
          
              if(state == STATE_OPEN || state == STATE_CLOSED)
              {
                // окна не двигаются, можем отправлять команду
                 //if(cParser->ParseCommand(queuedWindowCommand, c->GetControllerID(), cmd))
                 if(ModuleInterop.QueryCommand(ctSET,queuedWindowCommand,false))
                 {
           
                  // команда разобрана, можно выполнять
                    queuedWindowCommand = F(""); // очищаем команду, нам она больше не нужна
                   // ModuleInterop.Clear(); // очищаем ответ, который будет после вызова команды ProcessModuleCommand
                   // cmd.SetInternal(false); // говорим, что команда - как бы от юзера, контроллер после выполнения команды перейдёт в ручной режим
                  //  cmd.SetIncomingStream(&ModuleInterop); // говорим, чтобы модуль плевался ответами в класс взаимодействия между модулями
                  //  c->ProcessModuleCommand(cmd,false);

                    // всё, команда выполнена, когда окна не находились в движении
                 } // if
                
              } // if(state == STATE_OPEN || state == STATE_CLOSED)
              
         } // if(idx != -1)
        
      } // if(cParser->ParseCommand(F("CTGET=STATE|WINDOW|ALL")
  
}
void SMSModule::ParseIncomingSMS(const String& sms)
{
  #ifdef NEOWAY_DEBUG_MODE
    Serial.println("ParseIncomingSMS(\"" + sms + "\")");
  #endif

  bool shouldSendSMS = false;

  PDUIncomingMessage message = PDU.Decode(sms);
  if(message.IsDecodingSucceed)
  {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("Message decoded, try to check sender number and find right commands...");
    #endif

    if(message.SenderNumber != Settings->GetSmsPhoneNumber()) // с неизвестного номера пришло СМС
    {
     #ifdef NEOWAY_DEBUG_MODE
      Serial.println("Message received from unknown number: " + message.SenderNumber + ", skip it...");
    #endif
     return;
    }

    // ищем команды
    int16_t idx = message.Message.indexOf(SMS_OPEN_COMMAND); // открыть окна
    if(idx != -1)
    {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("WINDOWS->OPEN command found, execute it...");
    #endif

        // открываем окна
        // сохраняем команду на выполнение тогда, когда окна будут открыты или закрыты - иначе она не отработает
        queuedWindowCommand = F("STATE|WINDOW|ALL|OPEN");
        shouldSendSMS = true;
    }
    
    idx = message.Message.indexOf(SMS_CLOSE_COMMAND); // закрыть окна
    if(idx != -1)
    {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("WINDOWS->CLOSE command found, execute it...");
    #endif

      // закрываем окна
      // сохраняем команду на выполнение тогда, когда окна будут открыты или закрыты - иначе она не отработает
      queuedWindowCommand = F("STATE|WINDOW|ALL|CLOSE");
      shouldSendSMS = true;
    }
    
    idx = message.Message.indexOf(SMS_AUTOMODE_COMMAND); // перейти в автоматический режим работы
    if(idx != -1)
    {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("Automatic mode command found, execute it...");
    #endif

    // переводим контроллер в автоматический режим работы
      if(ModuleInterop.QueryCommand(ctSET, F("STATE|MODE|AUTO"),false))
      {
        #ifdef NEOWAY_DEBUG_MODE
          Serial.println("CTSET=STATE|MODE|AUTO command parsed, process it...");
        #endif
    
      }

      if(ModuleInterop.QueryCommand(ctSET, F("WATER|MODE|AUTO"),false))
      {
        #ifdef NEOWAY_DEBUG_MODE
          Serial.println("CTSET=WATER|MODE|AUTO command parsed, process it...");
        #endif
    
      }    

      shouldSendSMS = true;
    }

    idx = message.Message.indexOf(SMS_WATER_ON_COMMAND); // включить полив
    if(idx != -1)
    {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("Water ON command found, execute it...");
    #endif

    // включаем полив
      if(ModuleInterop.QueryCommand(ctSET, F("WATER|ON"),false))
      {
        #ifdef NEOWAY_DEBUG_MODE
          Serial.println("CTSET=WATER|ON command parsed, process it...");
        #endif
    
       shouldSendSMS = true;
      }
    }

    idx = message.Message.indexOf(SMS_WATER_OFF_COMMAND); // выключить полив
    if(idx != -1)
    {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("Water OFF command found, execute it...");
    #endif

    // выключаем полив
      if(ModuleInterop.QueryCommand(ctSET, F("WATER|OFF"),false))
      {
        #ifdef NEOWAY_DEBUG_MODE
          Serial.println("CTSET=WATER|OFF command parsed, process it...");
        #endif
    
        shouldSendSMS = true;
      }

    }

           
    idx = message.Message.indexOf(SMS_STAT_COMMAND); // послать статистику
    if(idx != -1)
    {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("STAT command found, execute it...");
    #endif

      // посылаем статистику вызвавшему номеру
      SendStatToCaller(message.SenderNumber);

      // возвращаемся, поскольку нет необходимости посылать СМС с ответом ОК - вместо этого придёт статистика
      return;
    }
    
  }
  else
  {
  #ifdef NEOWAY_DEBUG_MODE
    Serial.println("Message decoding ERROR!");
  #endif
  }

  if(shouldSendSMS) // надо послать СМС с ответом "ОК"
    SendSMS(OK_ANSWER);
  
}

void SMSModule::Update(uint16_t dt)
{ 

  if(needToWaitTimer > 0) // надо ждать следующей команды
  {
    needToWaitTimer -= dt;
    return;
  }

  needToWaitTimer = 0; // сбрасываем таймер ожидания

  // проверяем питание на модуле
  if(digitalRead(NEOWAY_VCCIO_CHECK_PIN) != HIGH)
  {
    isModuleReady = false;
    currentOperation = opIdle;
    needToWaitTimer = 5000; // проверим ещё раз через пять секунд

    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("NEOWAY NOT FOUND!");
    #endif
    return;
  }

  bool isOkAnswer; // флаг ответа - OK или ERROR

  if(!isModuleReady) // модуль не готов к работе
  {
    currentOperation = opIdle; // начинаем опрос сначала
  }

  switch(currentOperation)
  {
     case opIdle: // ничего ещё не делали
      currentOperation = opCheckReady; // проверяем готовность модуля к работе
      isModuleReady = IsNeowayReady();
      if(!isModuleReady)
      {
        needToWaitTimer = 2000; // повторим через пару секунд запрос о готовности
        currentOperation = opIdle;
        return;
      }
     break;

     case opCheckReady: // проверили регистрацию, можно работать дальше
      if(isModuleReady) // модуль готов к работе, выключаем эхо
      {
        currentOperation = opEchoOff;
        SendToNeoway(F("ATE0")); // выключаем эхо
        FetchNeowayAnswer(isOkAnswer); // ждём ответа модуля
        if(!isOkAnswer)
        {
          #ifdef NEOWAY_DEBUG_MODE
            Serial.println("ECHO OFF ERROR!"); // не срослось, повторим через пару секунд
          #endif
          
          currentOperation = opCheckReady;
          needToWaitTimer = 2000;  
        }
        
      } // if(isModuleReady)
     break;

     case opEchoOff:
      if(isModuleReady)
      {
        // эхо выключили, работаем дальше - включаем ООН
        currentOperation = opAONEnable;
        SendToNeoway(F("AT+CLIP=1")); // включаем АОН
        FetchNeowayAnswer(isOkAnswer); // ждём ответа модуля
        
        if(!isOkAnswer)
        {
          #ifdef NEOWAY_DEBUG_MODE
            Serial.println("AT+CLIP=1 ERROR!"); // не срослось, повторим через пару секунд
          #endif
          currentOperation = opEchoOff;
          needToWaitTimer = 2000;  
        }
      } // if
     break;

     case opAONEnable:
      if(isModuleReady)
      {
        // АОН включили, работаем дальше - настраиваем кодировку
        currentOperation = opSetPDUEncoding;
        SendToNeoway(F("AT+CMGF=0")); // включаем формат PDU
        FetchNeowayAnswer(isOkAnswer); // ждём ответа модуля
        if(!isOkAnswer)
        {
          #ifdef NEOWAY_DEBUG_MODE
            Serial.println("AT+CMGF=0 ERROR!"); // не срослось, повторим через пару секунд
          #endif
          currentOperation = opAONEnable;
          needToWaitTimer = 2000;  
        }

      } // if
     break; 

     case opSetPDUEncoding:
      if(isModuleReady)
      {
        // кодировку настроили, работаем дальше - настраиваем вывод входящих смс прямо на экран
        currentOperation = opSetSMSOutput;
        SendToNeoway(F("AT+CNMI=2,2")); // включаем вывод входящих смс прямо на экран
        FetchNeowayAnswer(isOkAnswer); // ждём ответа модуля
        if(!isOkAnswer)
        {
          #ifdef NEOWAY_DEBUG_MODE
            Serial.println("AT+CNMI=2,2 ERROR!"); // не срослось, повторим через пару секунд
          #endif
          
          currentOperation = opSetPDUEncoding;
          needToWaitTimer = 2000;  
        }
      } // if
     break;

     case opSetSMSOutput:
      if(isModuleReady)
      {
        // вывод смс настроили, работаем дальше - запрашиваем регистрацию модуля у оператора
        currentOperation = opCheckRegistration;
        SendToNeoway(F("AT+CREG?")); // запрашиваем регистрацию
        FetchNeowayAnswer(isOkAnswer); // ждём ответа модуля
        if(!isOkAnswer)
        {
          #ifdef NEOWAY_DEBUG_MODE
            Serial.println("AT+CREG? ERROR!"); // не срослось, повторим через пару секунд
          #endif
          
          currentOperation = opSetSMSOutput;
          needToWaitTimer = 2000;  
        }

      } // if
     break;

     case opCheckRegistration: // получили данные о регистрации модуля в сети, проверим их
     
        if(neowayAnswer.indexOf(F("+CREG: 0,1")) != -1) // регистрация у оператора успешна
        {
          isModuleRegistered = true;
          #ifdef NEOWAY_DEBUG_MODE
            Serial.println("NEOWAY registered in GSM! Wait for incoming data...");
          #endif
          
          currentOperation = opWaitForIncomingData; // ждём входящих данных из потока
        }
        else
        {
          #ifdef NEOWAY_DEBUG_MODE
            Serial.println("NEOWAY not registered, try again a little bit later!");
          #endif
          
          needToWaitTimer = 3000; // через сколько секунд повторим запрос?
          // повторяем запрос о регистрации ещё раз
          isModuleRegistered = false;
          currentOperation = opSetSMSOutput; // повторим запрос о регистрации чуть позже, при следующем вызове Update
        }
     break;

     case opWaitForIncomingData: // ждём любых входящих данных
      if(!isModuleRegistered)
      {
        #ifdef NEOWAY_DEBUG_MODE
          Serial.println("NEOWAY not registered, wait for registration succeeded...");
        #endif
        
        needToWaitTimer = 2000;
        currentOperation = opSetSMSOutput; // начинаем запрос о регистрации сначала
      }
      else
      {
        // модуль зарегистрирован, можем работать с любыми входящими данными
        ProcessQueuedWindowCommand(dt); // проверяем, есть ли у нас команда для окон на исполнение
        
          while(NEOWAY_SERIAL.available())
              incomingData += (char) NEOWAY_SERIAL.read();

         // проверяем, чего пришло в поток от модуля
         int16_t smsIdx = incomingData.indexOf(F("+CMT:"));
         int16_t ringIdx = incomingData.indexOf(F("+CLIP:"));
         
         if(smsIdx != -1)
         {
           // возможно, есть полностью принятое СМС-сообщение. Надо искать два символа \n - если они есть в потоке - значит SMS принято полностью
           String sms = incomingData.substring(smsIdx,incomingData.length());

           int16_t idx = sms.indexOf(F("\n"));
           if(idx != -1)
           {
              // нашли первый перевод строки
              sms = sms.substring(idx+1); // убираем первую строку
              
              idx = sms.indexOf(F("\n"));
              if(idx != -1)
              {
                // нашли полноценное SMS, надо с ним работать
                sms = sms.substring(0,idx); // выщемляем SMS
                sms.trim();
                
                #ifdef NEOWAY_DEBUG_MODE
                  Serial.println("SMS received: \"" + sms + "\"");
                #endif

                // очищаем данные, они больше не понадобятся
                incomingData = F("");

                // удаляем СМС из входящих
                SendToNeoway(F("AT+CMGD=0,4")); // команда "Удалить все СМС"
                FetchNeowayAnswer(isOkAnswer); // ждём ответа модуля

                // тут работаем с СМС
                ParseIncomingSMS(sms);
              } // if
           } // if
         } // if
         else if(ringIdx != -1)
         {
            // входящий звонок, проверяем, приняли ли мы конец строки?
            String ring = incomingData.substring(ringIdx+8); // пропускаем команду +CLIP:, пробел и открывающую кавычку "
            
            
            int16_t idx = ring.indexOf(F("\n"));
            if(idx != -1)
            {
                // звонок прошёл, номер принят надо смотреть, от кого
                idx = ring.indexOf("\"");
                if(idx != -1)
                {
                  ring = ring.substring(0,idx);
                }
                ring.trim();
                
                if(ring.length() > 0 && ring[0] != '+') // добавляем + к началу номера при необходимости
                  ring = String(F("+")) + ring;
                  
                #ifdef NEOWAY_DEBUG_MODE
                  Serial.println("RING DETECTED: \"" + ring + "\"");
                #endif

                // очищаем данные, они нам уже не понадобятся
                incomingData = F("");

                // кладём трубку
                SendToNeoway(F("ATH")); // команда "Положить трубку"
                FetchNeowayAnswer(isOkAnswer); // ждём ответа модуля

                // тут работаем с входящим звонком
                SendStatToCaller(ring); // посылаем статистику вызвавшему
                
            } // if
         } // RING
        
      } // else
     break; // opWaitForIncomingData

     case opWaitForSMSSendComplete: // ждём отсыла СМС

        // сюда мы попали через пару секунд после отправки СМС.
        FetchNeowayAnswer(isOkAnswer);
        
        if(isOkAnswer)
        {
          #ifdef NEOWAY_DEBUG_MODE
            Serial.println("SMS SEND OK :)))");
          #endif
        }
        else
        {
         #ifdef NEOWAY_DEBUG_MODE
          Serial.println("SMS SEND ERROR :(((");
         #endif
        }

        // переводим в режим ожидания входящих данных
        currentOperation = opWaitForIncomingData;
        
     break; // opWaitForSMSSendComplete

     
  } // switch
  
  
}
void SMSModule::SendStatToCaller(const String& phoneNum)
{
  #ifdef NEOWAY_DEBUG_MODE
    Serial.println("Try to send stat SMS to " + phoneNum + "...");
  #endif

  if(phoneNum != Settings->GetSmsPhoneNumber()) // не наш номер
  {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("NOT RIGHT NUMBER: " + phoneNum + "!");
    #endif
    
    return;
  }

  ModuleController* c = GetController();
  AbstractModule* stateModule = c->GetModuleByID(F("STATE"));

  if(!stateModule)
  {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("Unable to find STATE module registered!");
    #endif
    
    return;
  }


  // получаем температуры
  OneState* os1 = stateModule->State.GetState(StateTemperature,0);
  OneState* os2 = stateModule->State.GetState(StateTemperature,1);

  String sms;

   if(os1)
  {
    Temperature* insideTemp = (Temperature*) os1->Data;
  
    sms += T_INDOOR; // сообщение
    if(insideTemp->Value != NO_TEMPERATURE_DATA)
      sms += *insideTemp;
    else
      sms += SMS_NO_DATA;
      
    sms += NEWLINE;
    
  } // if 

  if(os2)
  {
    Temperature* outsideTemp = (Temperature*) os2->Data;
  
    sms += T_OUTDOOR;
    if(outsideTemp->Value != NO_TEMPERATURE_DATA)
      sms += *outsideTemp;
    else
      sms += SMS_NO_DATA;
    
    sms += NEWLINE;
  } // if


  // тут получаем состояние окон
  if(ModuleInterop.QueryCommand(ctGET,F("STATE|WINDOW|0"),true))
  {

    sms += W_STATE;

    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("Command CTGET=STATE|WINDOW|0 parsed, execute it...");
    #endif

    String streamAnswer = ModuleInterop.GetData(); // получили ответ от другого модуля
    streamAnswer.trim();
    int16_t idx = streamAnswer.lastIndexOf(PARAM_DELIMITER);
    if(idx != -1)
    {
      String state = streamAnswer.substring(idx+1,streamAnswer.length());
      if(state == STATE_OPEN || state == STATE_OPENING)
        sms += W_OPEN;
      else
        sms += W_CLOSED;
    }

     sms += NEWLINE;
 
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("Receive answer from STATE: \"" + streamAnswer + "\"");
    #endif
  }
    // получаем состояние полива
  if(ModuleInterop.QueryCommand(ctGET,F("WATER"),true))
  {
    sms += WTR_STATE;

    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("Command CTGET=WATER parsed, execute it...");
    #endif

    String streamAnswer = ModuleInterop.GetData(); // получили ответ от другого модуля
    streamAnswer.trim();
    int16_t idx = streamAnswer.lastIndexOf(PARAM_DELIMITER);
    if(idx != -1)
    {
      streamAnswer = streamAnswer.substring(0,idx);
      idx = streamAnswer.lastIndexOf(PARAM_DELIMITER);
      if(idx != -1)
      {
        String state = streamAnswer.substring(idx+1,streamAnswer.length());
        if(state == STATE_ON)
          sms += WTR_ON;
        else
          sms += WTR_OFF;
      }
    }
    
    
  }

  // тут отсылаем SMS
  SendSMS(sms);

}
void SMSModule::SendSMS(const String& sms)
{
  #ifdef NEOWAY_DEBUG_MODE
    Serial.println("Send SMS  (\"" + sms + "\")");
  #endif

  String num = Settings->GetSmsPhoneNumber();
  if(num.length() < 1)
  {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println("No phone number saved in controller!");
    #endif
    
    return;
  }
  
  PDUOutgoingMessage pduMessage = PDU.Encode(num,sms,true);
  String commandToSend = F("AT+CMGS=");commandToSend += String(pduMessage.MessageLength);

  #ifdef NEOWAY_DEBUG_MODE
    Serial.println("commandToSend = \"" + commandToSend + "\"");
  #endif

  if(isModuleRegistered)
  {
  
      // посылаем данные модулю
      SendToNeoway(commandToSend);
    
      // ждём приглашения
      while(1)
      {
        if(NEOWAY_SERIAL.available())
        {
          char ch = (char) NEOWAY_SERIAL.read();
          
          if(ch == '\r' || ch == '\n') // ждём данных с новой строки
            continue;
            
          if(ch == '>') // дождались приглашения, можем посылать дальше
          {
            #ifdef NEOWAY_DEBUG_MODE
              Serial.println("> found, continue sending...");
            #endif
            
            break;
          }
          else
          {
            #ifdef NEOWAY_DEBUG_MODE
              Serial.println(String((byte)ch) + " found - bad symbol!");
            #endif
            return; // не тот символ!
          }
        }
      } // while(1)
    
      // посылаем само СМС
      #ifdef NEOWAY_DEBUG_MODE
        Serial.println("Send message = \"" + pduMessage.Message + "\"");
      #endif
      
      SendToNeoway(pduMessage.Message,false);
      
      NEOWAY_SERIAL.write(0x1A); // посылаем символ окончания посыла
    
      // ждём данных пару секунд, они будут получены в следующем вызове Update
      currentOperation = opWaitForSMSSendComplete;
      needToWaitTimer = NEOWAY_WAIT_FOR_SMS_SEND_COMPLETE;

  } // if(isModuleRegistered)
  else
  {
     // модуль не зарегистрирован в сети
  }

}
bool  SMSModule::ExecCommand(const Command& command)
{
  ModuleController* c = GetController();
  String answer = UNKNOWN_COMMAND;
  bool answerStatus = false; 
  
  if(command.GetType() == ctSET) 
  {
      answerStatus = false;
      answer = NOT_SUPPORTED;
  }
  else
  if(command.GetType() == ctGET) //получить статистику
  {

    String t = command.GetRawArguments();
    t.toUpperCase();
    if(t == GetID()) // нет аргументов
    {
      answerStatus = false;
      answer = PARAMS_MISSED;
    }
    else
    if(t == STAT_COMMAND) // запросили данные статистики
    {
      SendStatToCaller(Settings->GetSmsPhoneNumber()); // посылаем статистику на указанный номер телефона
      answerStatus = true;
      answer = STAT_COMMAND; answer += PARAM_DELIMITER; answer += REG_SUCC;
    }
    else
    {
      // неизвестная команда
    } // else
    
  } // if
 
 // отвечаем на команду
    SetPublishData(&command,answerStatus,answer); // готовим данные для публикации
    c->Publish(this);
    
  return answerStatus;
}

