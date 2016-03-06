#include "SMSModule.h"
#include "ModuleController.h"
#include "PDUClasses.h"
#include "InteropStream.h"

bool SMSModule::IsKnownAnswer(const String& line)
{
  return ( line == F("OK") || (line.indexOf(F("ERROR")) != -1) );
}

void SMSModule::Setup()
{
  Settings = mainController->GetSettings();

  // будем смотреть этот пин на предмет наличия питания у модуля NEOWAY
  pinMode(NEOWAY_VCCIO_CHECK_PIN,INPUT);
  
  // запускаем наш сериал
  NEOWAY_SERIAL.begin(NEOWAY_BAUDRATE);

 
  InitQueue(); // инициализируем очередь
   
  // настройка модуля тут
}
void SMSModule::InitQueue()
{
  while(actionsQueue.size() > 0) // чистим очередь 
    actionsQueue.pop();
 
  isModuleRegistered = false;
  waitForSMSInNextLine = false;
  WaitForSMSWelcome = false; // не ждём приглашения
  needToWaitTimer = 0; // сбрасываем таймер
   
  // настраиваем то, что мы должны сделать для начала работы
  currentAction = smaIdle; // свободны, ничего не делаем
  actionsQueue.push_back(smaWaitReg); // ждём регистрации
  actionsQueue.push_back(smaSMSSettings); // настройки вывода SMS
  actionsQueue.push_back(smaPDUEncoding); // формат сообщений
  actionsQueue.push_back(smaAON); // включение АОН
  actionsQueue.push_back(smaEchoOff); // выключение эха
  actionsQueue.push_back(smaCheckReady); // проверка готовности
  
}
void SMSModule::ProcessAnswerLine(const String& line)
{
  // получаем ответ на команду, посланную модулю
  if(!line.length()) // пустая строка, нечего её разбирать
    return;

  #ifdef NEOWAY_DEBUG_MODE
    Serial.print(F("<== Receive \"")); Serial.print(line); Serial.println(F("\" answer from NEOWAY..."));
  #endif


  switch(currentAction)
  {
    case smaCheckReady:
    {
      // ждём ответа "+CPAS: 0" от модуля
          if(line == F("+CPAS: 0")) // получили
          {
            #ifdef NEOWAY_DEBUG_MODE
              Serial.println(F("[OK] => Neoway ready."));
           #endif
           actionsQueue.pop(); // убираем последнюю обработанную команду
           currentAction = smaIdle;
          }
          else
          {
           #ifdef NEOWAY_DEBUG_MODE
              Serial.println(F("[ERR] => Neoway NOT ready, try again later..."));
           #endif
             needToWaitTimer = 2000; // повторим через 2 секунды
          }
    }
    break;

    case smaEchoOff: // выключили эхо
    {
      if(IsKnownAnswer(line))
      {
        #ifdef NEOWAY_DEBUG_MODE
          Serial.println(F("[OK] => ECHO OFF processed."));
        #endif
       actionsQueue.pop(); // убираем последнюю обработанную команду     
       currentAction = smaIdle;
      }
    }
    break;

    case smaAON: // включили АОН
    {
      if(IsKnownAnswer(line))
      {
        if(line == F("OK"))
        {
          #ifdef NEOWAY_DEBUG_MODE
            Serial.println(F("[OK] => AON is ON."));
          #endif
          actionsQueue.pop(); // убираем последнюю обработанную команду     
          currentAction = smaIdle;
        } // if
        else
        {
          // пробуем ещё раз
          needToWaitTimer = 1500; // через некоторое время
          currentAction = smaIdle;
        }
      } // known answer
      
    }
    break;

    case smaPDUEncoding: // кодировка PDU
    {
      if(IsKnownAnswer(line))
      {
        if(line == F("OK"))
        {
          #ifdef NEOWAY_DEBUG_MODE
            Serial.println(F("[OK] => PDU encoding is set."));
          #endif
         actionsQueue.pop(); // убираем последнюю обработанную команду     
         currentAction = smaIdle;
        }
        else
        {
            // пробуем ещё раз
          needToWaitTimer = 1500; // через некоторое время
          currentAction = smaIdle;
        
        }
      }
      
    }
    break;

    case smaSMSSettings: // установили режим отображения входящих SMS сразу в порт
    {
      if(IsKnownAnswer(line))
      {
        if(line == F("OK"))
        {
            #ifdef NEOWAY_DEBUG_MODE
              Serial.println(F("[OK] => SMS settings is set."));
            #endif
           actionsQueue.pop(); // убираем последнюю обработанную команду     
           currentAction = smaIdle;
        }
        else
        {
            // пробуем ещё раз
          needToWaitTimer = 1500; // через некоторое время
          currentAction = smaIdle;           
        }
      }
      
    }
    break;

    case smaWaitReg: // пришёл ответ о регистрации
    {
      if(line.indexOf(F("+CREG: 0,1")) != -1)
      {
        // зарегистрированы в GSM-сети
           isModuleRegistered = true;
            #ifdef NEOWAY_DEBUG_MODE
              Serial.println(F("[OK] => NEOWAY registered in GSM!"));
            #endif
           actionsQueue.pop(); // убираем последнюю обработанную команду     
           currentAction = smaIdle;
      } // if
      else
      {
        // ещё не зарегистрированы
          isModuleRegistered = false;
          needToWaitTimer = 4500; // через некоторое время
          currentAction = smaIdle;
      } // else
    }
    break;

    case smaHangUp: // положили трубку
    {
      if(IsKnownAnswer(line))
      {
             #ifdef NEOWAY_DEBUG_MODE
              Serial.println(F("[OK] => Hang up DONE."));
            #endif
       
           actionsQueue.pop(); // убираем последнюю обработанную команду     
           currentAction = smaIdle;
      } 
      
    }
    break;

    case smaStartSendSMS: // начинаем посылать SMS
    {
            #ifdef NEOWAY_DEBUG_MODE
              Serial.println(F("[OK] => Welcome received, continue sending..."));
            #endif

           actionsQueue.pop(); // убираем последнюю обработанную команду     
           currentAction = smaIdle;
           actionsQueue.push_back(smaSmsActualSend); // добавляем команду на обработку
      
    }
    break;

    case smaSmsActualSend: // отослали SMS
    {
      if(IsKnownAnswer(line))
      {
            #ifdef NEOWAY_DEBUG_MODE
              Serial.println(F("[OK] => SMS sent."));
            #endif
      
       actionsQueue.pop(); // убираем последнюю обработанную команду     
       currentAction = smaIdle;
       actionsQueue.push_back(smaClearAllSMS); // добавляем команду на обработку
      }
    }
    break;

    case smaClearAllSMS: // очистили все SMS
    {
      if(IsKnownAnswer(line))
      {
            #ifdef NEOWAY_DEBUG_MODE
              Serial.println(F("[OK] => saved SMS cleared."));
            #endif
      
       actionsQueue.pop(); // убираем последнюю обработанную команду     
       currentAction = smaIdle;
      }
     
    }
    break;


    case smaIdle:
    {
      if(waitForSMSInNextLine) // дождались входящего SMS
      {
        waitForSMSInNextLine = false;
        ProcessIncomingSMS(line);
      }
      
      if(line.startsWith(F("+CLIP:")))
        ProcessIncomingCall(line);
      else
      if(line.startsWith(F("+CMT:")))
        waitForSMSInNextLine = true;
       
    

    }
    break;
  } // switch  
  
}
void SMSModule::ProcessIncomingSMS(const String& line) // обрабатываем входящее SMS
{
  #ifdef NEOWAY_DEBUG_MODE
  Serial.print(F("SMS RECEIVED: ")); Serial.println(line);
  #endif


  bool shouldSendSMS = false;

  PDUIncomingMessage message = PDU.Decode(line);
  if(message.IsDecodingSucceed)
  {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println(F("Message decoded, check phone number..."));
    #endif

    if(message.SenderNumber != Settings->GetSmsPhoneNumber()) // с неизвестного номера пришло СМС
    {
     #ifdef NEOWAY_DEBUG_MODE
      Serial.print(F("Message received from unknown number: ")); Serial.print(message.SenderNumber); Serial.println(F(", skip it..."));
    #endif
     return;
    }

    #ifdef NEOWAY_DEBUG_MODE
      Serial.println(F("Phone number is OK, continue..."));
    #endif

    // ищем команды
    int16_t idx = message.Message.indexOf(SMS_OPEN_COMMAND); // открыть окна
    if(idx != -1)
    {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println(F("WINDOWS->OPEN command found, execute it..."));
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
      Serial.println(F("WINDOWS->CLOSE command found, execute it..."));
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
      Serial.println(F("Automatic mode command found, execute it..."));
    #endif

      // переводим управление окнами в автоматический режим работы
      if(ModuleInterop.QueryCommand(ctSET, F("STATE|MODE|AUTO"),false))
      {
        #ifdef NEOWAY_DEBUG_MODE
          Serial.println(F("CTSET=STATE|MODE|AUTO command parsed, process it..."));
        #endif
    
      }

      // переводим управление поливом в автоматический режим работы
      if(ModuleInterop.QueryCommand(ctSET, F("WATER|MODE|AUTO"),false))
      {
        #ifdef NEOWAY_DEBUG_MODE
          Serial.println(F("CTSET=WATER|MODE|AUTO command parsed, process it..."));
        #endif
    
      }
     
      // переводим управление досветкой в актоматический режим работы    
      if(ModuleInterop.QueryCommand(ctSET, F("LIGHT|MODE|AUTO"),false))
      {
        #ifdef NEOWAY_DEBUG_MODE
          Serial.println(F("CTSET=LIGHT|MODE|AUTO command parsed, process it..."));
        #endif
    
      }    

      shouldSendSMS = true;
    }

    idx = message.Message.indexOf(SMS_WATER_ON_COMMAND); // включить полив
    if(idx != -1)
    {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println(F("Water ON command found, execute it..."));
    #endif

    // включаем полив
      if(ModuleInterop.QueryCommand(ctSET, F("WATER|ON"),false))
      {
        #ifdef NEOWAY_DEBUG_MODE
          Serial.println(F("CTSET=WATER|ON command parsed, process it..."));
        #endif
    
       shouldSendSMS = true;
      }
    }

    idx = message.Message.indexOf(SMS_WATER_OFF_COMMAND); // выключить полив
    if(idx != -1)
    {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println(F("Water OFF command found, execute it..."));
    #endif

    // выключаем полив
      if(ModuleInterop.QueryCommand(ctSET, F("WATER|OFF"),false))
      {
        #ifdef NEOWAY_DEBUG_MODE
          Serial.println(F("CTSET=WATER|OFF command parsed, process it..."));
        #endif
    
        shouldSendSMS = true;
      }

    }

           
    idx = message.Message.indexOf(SMS_STAT_COMMAND); // послать статистику
    if(idx != -1)
    {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println(F("STAT command found, execute it..."));
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
    Serial.println(F("Message decoding ERROR!"));
  #endif
  }

  if(shouldSendSMS) // надо послать СМС с ответом "ОК"
    SendSMS(OK_ANSWER);


  
}
void SMSModule::ProcessIncomingCall(const String& line) // обрабатываем входящий звонок
{
  // приходит строка вида
  // +CLIP: "79182900063",145,,,"",0
  
   // входящий звонок, проверяем, приняли ли мы конец строки?
    String ring = line.substring(8); // пропускаем команду +CLIP:, пробел и открывающую кавычку "

    int idx = ring.indexOf("\"");
    if(idx != -1)
      ring = ring.substring(0,idx);

    if(ring.length() && ring[0] != '+')
      ring = String(F("+")) + ring;
      
      #ifdef NEOWAY_DEBUG_MODE
          Serial.print(F("RING DETECTED: ")); Serial.println(ring);
      #endif

 
  if(ring != Settings->GetSmsPhoneNumber()) // не наш номер
  {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.print(F("UNKNOWN NUMBER: ")); Serial.print(ring); Serial.println(F("!"));
    #endif

 // добавляем команду "положить трубку"
  actionsQueue.push_back(smaHangUp);
    
    return;
  }

  // отправляем статистику вызвавшему номеру
   SendStatToCaller(ring); // посылаем статистику вызвавшему
  
 // добавляем команду "положить трубку" - она выполнится первой, а потом уже уйдёт SMS
  actionsQueue.push_back(smaHangUp);
 
  
}
void SMSModule::SendCommand(const String& command, bool addNewLine)
{
  #ifdef NEOWAY_DEBUG_MODE
    Serial.print(F("==> Send the \"")); Serial.print(command); Serial.println(F("\" command to NEOWAY..."));
  #endif

  NEOWAY_SERIAL.write(command.c_str(),command.length());
  
  if(addNewLine)
  {
    NEOWAY_SERIAL.write(String(NEWLINE).c_str());
  }
      
}
void SMSModule::ProcessQueue()
{
  if(currentAction != smaIdle) // чем-то заняты, не можем ничего делать
    return;

    size_t sz = actionsQueue.size();
    if(!sz) // в очереди ничего нет
      return;
      
    currentAction = actionsQueue[sz-1]; // получаем очередную команду

    // смотрим, что за команда
    switch(currentAction)
    {
      case smaCheckReady:
      {
        // надо проверить модуль на готовность
      #ifdef NEOWAY_DEBUG_MODE
        Serial.println(F("Check for NEOWAY READY..."));
      #endif
      SendCommand(F("AT+CPAS"));
      //SendCommand(F("AT+IPR=57600"));
      }
      break;

      case smaEchoOff:
      {
        // выключаем эхо
      #ifdef NEOWAY_DEBUG_MODE
        Serial.println(F("Disable echo..."));
      #endif
      SendCommand(F("ATE0"));
      }
      break;

      case smaAON:
      {
        // включаем АОН
      #ifdef NEOWAY_DEBUG_MODE
        Serial.println(F("Turn AON ON..."));
      #endif
      SendCommand(F("AT+CLIP=1"));
      }
      break;

      case smaPDUEncoding: // устанавливаем кодировку сообщений
      {

      #ifdef NEOWAY_DEBUG_MODE
        Serial.println(F("Set PDU encoding..."));
      #endif
      
       SendCommand(F("AT+CMGF=0"));
        
      }
      break;

      case smaSMSSettings: // устанавливаем режим отображения SMS
      {
      #ifdef NEOWAY_DEBUG_MODE
        Serial.println(F("Set SMS output mode..."));
      #endif
      SendCommand(F("AT+CNMI=2,2"));
      
      }
      break;

      case smaWaitReg: // ждём регистрации модуля в сети
      {
     #ifdef NEOWAY_DEBUG_MODE
        Serial.println(F("Check registration status..."));
      #endif
      SendCommand(F("AT+CREG?"));
        
      }
      break;

      case smaHangUp: // кладём трубку
      {
      #ifdef NEOWAY_DEBUG_MODE
        Serial.println(F("Hang up..."));
      #endif
      SendCommand(F("ATH"));
       
      }
      break;

      case smaStartSendSMS: // начало отсылки SMS
      {
        #ifdef NEOWAY_DEBUG_MODE
        Serial.println(F("Start SMS sending..."));
        #endif
        
        SendCommand(commandToSend);
        commandToSend = F("");
      
       
      }
      break;

      case smaSmsActualSend: // отсылаем данные SMS
      {
      #ifdef NEOWAY_DEBUG_MODE
        Serial.println(F("Start sending SMS data..."));
      #endif
      
        SendCommand(smsToSend,false);
        NEOWAY_SERIAL.write(0x1A); // посылаем символ окончания посыла
        smsToSend = F("");
        
        
      }
      break;

      case smaClearAllSMS: // надо очистить все SMS
      {
       #ifdef NEOWAY_DEBUG_MODE
        Serial.println(F("SMS clearance..."));
      #endif
      SendCommand(F("AT+CMGD=0,4"));
       
      }
      break;


      case smaIdle:
      {
        // ничего не делаем
      }
      break;
      
    } // switch
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
     
    needToWaitTimer = 10000; // проверим ещё раз через десять секунд

    #ifdef NEOWAY_DEBUG_MODE
      Serial.println(F("NEOWAY NOT FOUND!"));
    #endif

    InitQueue(); // инициализировали очередь по новой, т.к. у модуля отвалилось питание
    return;
  }
  
  ProcessQueue();
  ProcessQueuedWindowCommand(dt);

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

       if(ModuleInterop.QueryCommand(ctGET,F("STATE|WINDOW|ALL"),false))
      {
        #ifdef NEOWAY_DEBUG_MODE
          Serial.println(F("CTGET=STATE|WINDOW|ALL command parsed, process it..."));
        #endif
    

        // теперь проверяем ответ. Если окна не в движении - нам вернётся OPEN или CLOSED последним параметром.
        // только в этом случае мы можем исполнять команду
        const char* strPtr = PublishSingleton.Text.c_str();
        int16_t idx = PublishSingleton.Text.lastIndexOf(PARAM_DELIMITER);
        if(idx != -1)
        {
          strPtr += idx + 1;
          
              if(strstr(strPtr,String(STATE_OPEN).c_str()) || strstr(strPtr,String(STATE_CLOSED).c_str()))
              {
                // окна не двигаются, можем отправлять команду
                 if(ModuleInterop.QueryCommand(ctSET,queuedWindowCommand,false))
                 {
           
                  // команда разобрана, можно выполнять
                    queuedWindowCommand = F(""); // очищаем команду, нам она больше не нужна

                    // всё, команда выполнена, когда окна не находились в движении
                 } // if
                
              } // if(state == STATE_OPEN || state == STATE_CLOSED)
              
         } // if(idx != -1)
        
      } // if(cParser->ParseCommand(F("CTGET=STATE|WINDOW|ALL")
  
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

  AbstractModule* stateModule = mainController->GetModuleByID(F("STATE"));

  if(!stateModule)
  {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println(F("Unable to find STATE module registered!"));
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
      Serial.println(F("Command CTGET=STATE|WINDOW|0 parsed, execute it..."));
    #endif

    const char* strPtr = PublishSingleton.Text.c_str();
     if(strstr(strPtr,String(STATE_OPEN).c_str()))
        sms += W_OPEN;
      else
        sms += W_CLOSED;


     sms += NEWLINE;
 
    #ifdef NEOWAY_DEBUG_MODE
      Serial.print(F("Receive answer from STATE: ")); Serial.println(PublishSingleton.Text);
    #endif
  }
    // получаем состояние полива
  if(ModuleInterop.QueryCommand(ctGET,F("WATER"),true))
  {
    sms += WTR_STATE;

    #ifdef NEOWAY_DEBUG_MODE
      Serial.println(F("Command CTGET=WATER parsed, execute it..."));
    #endif

    const char* strPtr = PublishSingleton.Text.c_str();
    String sOFF = STATE_OFF;
    if(strstr(strPtr,sOFF.c_str()))
      sms += WTR_OFF;
    else
      sms += WTR_ON;
          
  }

  // тут отсылаем SMS
  SendSMS(sms);

}

void SMSModule::SendSMS(const String& sms)
{
  #ifdef NEOWAY_DEBUG_MODE
    Serial.print(F("Send SMS:  ")); Serial.println(sms);
  #endif

  if(!isModuleRegistered)
  {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println(F("Module not registered!"));
    #endif

    return;
  }

  String num = Settings->GetSmsPhoneNumber();
  if(num.length() < 1)
  {
    #ifdef NEOWAY_DEBUG_MODE
      Serial.println(F("No phone number saved in controller!"));
    #endif
    
    return;
  }
  
  PDUOutgoingMessage pduMessage = PDU.Encode(num,sms,true);
  commandToSend = F("AT+CMGS="); commandToSend += String(pduMessage.MessageLength);

  #ifdef NEOWAY_DEBUG_MODE
    Serial.print(F("commandToSend = ")); Serial.println(commandToSend);
  #endif

  smsToSend = pduMessage.Message; // сохраняем SMS для отправки
  WaitForSMSWelcome = true; // выставляем флаг, что мы ждём >
  actionsQueue.push_back(smaStartSendSMS); // добавляем команду на обработку
  
}

bool  SMSModule::ExecCommand(const Command& command, bool wantAnswer)
{
  UNUSED(wantAnswer);
  
  if(command.GetType() == ctSET) 
  {
      PublishSingleton.Text = NOT_SUPPORTED;
  }
  else
  if(command.GetType() == ctGET) //получить статистику
  {

    String t = command.GetRawArguments();
    t.toUpperCase();
    if(t == GetID()) // нет аргументов
    {
      PublishSingleton.Text = PARAMS_MISSED;
    }
    else
    if(t == STAT_COMMAND) // запросили данные статистики
    {
      SendStatToCaller(Settings->GetSmsPhoneNumber()); // посылаем статистику на указанный номер телефона
    
      PublishSingleton.Status = true;
      PublishSingleton.Text = STAT_COMMAND; PublishSingleton.Text += PARAM_DELIMITER; PublishSingleton.Text += REG_SUCC;
    }
    else
    {
      // неизвестная команда
      PublishSingleton.Text = UNKNOWN_COMMAND;
    } // else
    
  } // if
 
 // отвечаем на команду
    mainController->Publish(this,command);
    
  return PublishSingleton.Status;
}

