#include "WiFiModule.h"
#include "ModuleController.h"
#include "InteropStream.h"

bool WiFiModule::IsKnownAnswer(const String& line)
{
  return ( line == F("OK") || line == F("ERROR") || line == F("FAIL") || line == F("SEND OK"));
}
void WiFiModule::ProcessAnswerLine(const String& line)
{
  // получаем ответ на команду, посланную модулю
  #ifdef WIFI_DEBUG
    Serial.print(F("<== Receive \"")); Serial.print(line); Serial.println(F("\" answer from ESP-01..."));
  #endif

  switch(currentAction)
  {
    case wfaWantReady:
    {
      // ждём ответа "ready" от модуля
      if(line == F("ready")) // получили
      {
        #ifdef WIFI_DEBUG
          Serial.println(F("[OK] => ESP-01 restarted."));
       #endif
       actionsQueue.pop(); // убираем последнюю обработанную команду
       currentAction = wfaIdle;
      }
    }
    break;

    case wfaEchoOff: // выключили эхо
    {
      if(IsKnownAnswer(line))
      {
        #ifdef WIFI_DEBUG
          Serial.println(F("[OK] => ECHO OFF processed."));
        #endif
       actionsQueue.pop(); // убираем последнюю обработанную команду     
       currentAction = wfaIdle;
      }
    }
    break;

    case wfaCWMODE: // перешли в смешанный режим
    {
      if(IsKnownAnswer(line))
      {
        #ifdef WIFI_DEBUG
          Serial.println(F("[OK] => SoftAP mode is ON."));
        #endif
       actionsQueue.pop(); // убираем последнюю обработанную команду     
       currentAction = wfaIdle;
      }
      
    }
    break;

    case wfaCWSAP: // создали точку доступа
    {
      if(IsKnownAnswer(line))
      {
        #ifdef WIFI_DEBUG
          Serial.println(F("[OK] => access point created."));
        #endif
       actionsQueue.pop(); // убираем последнюю обработанную команду     
       currentAction = wfaIdle;
      }
      
    }
    break;

    case wfaCIPMODE: // установили режим работы сервера
    {
      if(IsKnownAnswer(line))
      {
        #ifdef WIFI_DEBUG
          Serial.println(F("[OK] => TCP-server mode now set to 0."));
        #endif
       actionsQueue.pop(); // убираем последнюю обработанную команду     
       currentAction = wfaIdle;
      }
      
    }
    break;

    case wfaCIPMUX: // разрешили множественные подключения
    {
      if(IsKnownAnswer(line))
      {
        #ifdef WIFI_DEBUG
          Serial.println(F("[OK] => Multiple connections allowed."));
        #endif
       actionsQueue.pop(); // убираем последнюю обработанную команду     
       currentAction = wfaIdle;
      }
      
    }
    break;

    case wfaCIPSERVER: // запустили сервер
    {
       if(IsKnownAnswer(line))
      {
        #ifdef WIFI_DEBUG
          Serial.println(F("[OK] => TCP-server started."));
        #endif
       actionsQueue.pop(); // убираем последнюю обработанную команду     
       currentAction = wfaIdle;
      }
     
    }
    break;

    case wfaCWJAP: // законнектились к роутеру
    {
       if(IsKnownAnswer(line))
      {
        #ifdef WIFI_DEBUG
          Serial.println(F("[OK] => connected to the router."));
        #endif
       actionsQueue.pop(); // убираем последнюю обработанную команду     
       currentAction = wfaIdle;
      }
      
    }
    break;

    case wfaCWQAP: // отсоединились от роутера
    {
      if(IsKnownAnswer(line))
      {
        #ifdef WIFI_DEBUG
          Serial.println(F("[OK] => disconnected from router."));
        #endif
       actionsQueue.pop(); // убираем последнюю обработанную команду     
       currentAction = wfaIdle;
      }
     
    }
    break;


    case wfaCIPSEND: // надо отослать данные клиенту
    {
            
      if(line == F(">")) //IsKnownAnswer(line)) // дождались приглашения
      {
        #ifdef WIFI_DEBUG
          Serial.println(F("SENDING THE DATA!"));
        #endif
        
        actionsQueue.pop(); // убираем последнюю обработанную команду     
        currentAction = wfaIdle;
      }
    }
    break;

    case wfaACTUALSEND: // отослали ли данные?
    {
      if(IsKnownAnswer(line)) // дождались приглашения
      {
        #ifdef WIFI_DEBUG
        Serial.println(F("DATA SENT!"));
        #endif
        actionsQueue.pop(); // убираем последнюю обработанную команду     
        currentAction = wfaIdle;
      }
      
    }
    break;

    case wfaCIPCLOSE: // закрыли соединение
    {
      if(IsKnownAnswer(line)) // дождались приглашения
      {
        #ifdef WIFI_DEBUG
        Serial.println(F("Client connection closed."));
        #endif
        actionsQueue.pop(); // убираем последнюю обработанную команду     
        currentAction = wfaIdle;
      }
      
      
    }
    break;

    case wfaIdle:
    {
      // здесь может придти запрос от сервера
      if(line.startsWith(F("+IPD")))
      {
         // пришёл запрос от сервера, сохраняем его
         waitForQueryCompleted = true; // ждём конца запроса
         httpQuery = F("");
      } // if

      if(waitForQueryCompleted) // ждём всего запроса, он нам может быть и не нужен
      {
          httpQuery += line;
          httpQuery += NEWLINE;

          if( /*line.endsWith(F(",CLOSED")) ||*/ !line.length() 
         // || line.lastIndexOf(F("HTTP/1.")) != -1 // нашли полную строку 
          )
          {
            waitForQueryCompleted = false; // уже не ждём запроса
            ProcessQuery(); // обрабатываем запрос
          }
      } // if
    }
    break;
  } // switch

  
  
}
void WiFiModule::ProcessQuery()
{
  
  int idx = httpQuery.indexOf(F(",")); // ищем первую запятую после +IPD
  const char* ptr = httpQuery.c_str();
  ptr += idx+1;
  // перешли за запятую, парсим ID клиента
  connectedClientID = F("");
  while(*ptr != ',')
  {
    connectedClientID += (char) *ptr;
    ptr++;
  }
  while(*ptr != ':')
    ptr++; // перешли на начало данных
  
  ptr++; // за двоеточие

  String str = ptr; // сохраняем запрос для разбора
  idx = str.indexOf(F(" ")); // ищем пробел
  
  if(idx != -1)
  {
    // переходим на URI
    str = str.substring(idx+2);

    idx = str.indexOf(F(" ")); // ищем пробел опять
    if(idx != -1)
    {
      // выщемляем URI
      requestedURI = str.substring(0,idx);
    } // if 
     ProcessURIRequest(); // обрабатываем запрос от клиента
  } // if 
   
}
void WiFiModule::ProcessURIRequest()
{
 #ifdef WIFI_DEBUG
 Serial.print(F("Client ID = "));
 Serial.println(connectedClientID);
 Serial.println(F("Requested URI: "));
 Serial.println(requestedURI);
#endif

 // тут парсим, какой запрос, и отсылаем ответ клиенту
 TEMP_DATA_TO_SEND = F("<h1>Hello from Arduino MEGA!</h1>");
 TEMP_DATA_TO_SEND += F("REQUESTED: ");
 TEMP_DATA_TO_SEND += requestedURI + F("<br/>");

 COMMAND_TYPE cType = ctUNKNOWN;
 if(requestedURI.startsWith(F("CTGET=")))
  cType = ctGET;
 else
   if(requestedURI.startsWith(F("CTSET=")))
  cType = ctSET;

 if(cType != ctUNKNOWN) // надо получить данные с контроллера
 {
    InteropStream streamI;
    streamI.SetController(GetController());
  
    String command = requestedURI.substring(6);
    if(streamI.QueryCommand(cType,command,false))
    {
      TEMP_DATA_TO_SEND += F("ANSWER: ");
      TEMP_DATA_TO_SEND += streamI.GetData();
    } // if
 } // if
 
 dataToSendLength = TEMP_DATA_TO_SEND.length();


 // данные подготовлены, отсылаем их клиенту в следующем вызове Update
 actionsQueue.push_back(wfaCIPCLOSE);
 actionsQueue.push_back(wfaACTUALSEND);
 actionsQueue.push_back(wfaCIPSEND);
}

void WiFiModule::Setup()
{
  // настройка модуля тут
  Settings = GetController()->GetSettings();

  waitForQueryCompleted = false;
  WaitForDataWelcome = false; // не ждём приглашения

  // настраиваем то, что мы должны сделать
  currentAction = wfaIdle; // свободны, ничего не делаем
  
  if(Settings->GetWiFiState() & 0x01) // коннектимся к роутеру
    actionsQueue.push_back(wfaCWJAP); // коннектимся к роутеру совсем в конце
  else
    actionsQueue.push_back(wfaCWQAP); // отсоединяемся от роутера
    
  actionsQueue.push_back(wfaCIPSERVER); // сервер поднимаем в последнюю очередь
  actionsQueue.push_back(wfaCIPMUX); // разрешаем множественные подключения
  actionsQueue.push_back(wfaCIPMODE); // устанавливаем режим работы
  actionsQueue.push_back(wfaCWSAP); // создаём точку доступа
  actionsQueue.push_back(wfaCWMODE); // // переводим в смешанный режим
  actionsQueue.push_back(wfaEchoOff); // выключаем эхо
  actionsQueue.push_back(wfaWantReady); // надо получить ready от модуля


  // поднимаем сериал
  WIFI_SERIAL.begin(WIFI_BAUDRATE);

}
void WiFiModule::SendCommand(const String& command, bool addNewLine)
{
  #ifdef WIFI_DEBUG
    Serial.print(F("==> Send the \"")); Serial.print(command); Serial.println(F("\" command to ESP-01..."));
  #endif

  WIFI_SERIAL.write(command.c_str(),command.length());
  
  if(addNewLine)
  {
    WIFI_SERIAL.write(String(NEWLINE).c_str());
  }
      
}
void WiFiModule::ProcessQueue()
{
  if(currentAction != wfaIdle) // чем-то заняты, не можем ничего делать
    return;

    size_t sz = actionsQueue.size();
    if(!sz) // в очереди ничего нет
      return;
      
    currentAction = actionsQueue[sz-1]; // получаем очередную команду

    // смотрим, что за команда
    switch(currentAction)
    {
      case wfaWantReady:
      {
        // надо рестартовать модуль
      #ifdef WIFI_DEBUG
        Serial.println(F("Restart the ESP-01..."));
      #endif
      SendCommand(F("AT+RST"));
      }
      break;

      case wfaEchoOff:
      {
        // выключаем эхо
      #ifdef WIFI_DEBUG
        Serial.println(F("Disable echo..."));
      #endif
      SendCommand(F("ATE0"));
      }
      break;

      case wfaCWMODE:
      {
        // переходим в смешанный режим
      #ifdef WIFI_DEBUG
        Serial.println(F("Go to SoftAP mode..."));
      #endif
      SendCommand(F("AT+CWMODE=3"));
      }
      break;

      case wfaCWSAP: // создаём точку доступа
      {

      #ifdef WIFI_DEBUG
        Serial.println(F("Creating the access point..."));
      #endif
      
        String com = F("AT+CWSAP=\"");
        com += Settings->GetStationID();
        com += F("\",\"");
        com += Settings->GetStationPassword();
        com += F("\",8,4");
        
        SendCommand(com);
        
      }
      break;

      case wfaCIPMODE: // устанавливаем режим работы сервера
      {
      #ifdef WIFI_DEBUG
        Serial.println(F("Set the TCP server mode to 0..."));
      #endif
      SendCommand(F("AT+CIPMODE=0"));
      
      }
      break;

      case wfaCIPMUX: // разрешаем множественные подключения
      {
      #ifdef WIFI_DEBUG
        Serial.println(F("Allow the multiple connections..."));
      #endif
      SendCommand(F("AT+CIPMUX=1"));
        
      }
      break;

      case wfaCIPSERVER: // запускаем сервер
      {  
      #ifdef WIFI_DEBUG
        Serial.println(F("Starting TCP-server..."));
      #endif
      SendCommand(F("AT+CIPSERVER=1,80"));
      
      }
      break;

      case wfaCWQAP: // отсоединяемся от точки доступа
      {  
      #ifdef WIFI_DEBUG
        Serial.println(F("Disconnect from router..."));
      #endif
      SendCommand(F("AT+CWQAP"));
      
      }
      break;

      case wfaCWJAP: // коннектимся к роутеру
      {
      #ifdef WIFI_DEBUG
        Serial.println(F("Connecting to the router..."));
      #endif
        String com = F("AT+CWJAP=\"");
        com += Settings->GetRouterID();
        com += F("\",\"");
        com += Settings->GetRouterPassword();
        com += F("\"");
        SendCommand(com);    
      }
      break;

      case wfaCIPSEND: // надо отослать данные клиенту
      {
      #ifdef WIFI_DEBUG
        Serial.println(F("Sending data command to the client..."));
      #endif
        String command = F("AT+CIPSEND=");
        command += connectedClientID;
        command += F(",");
        command += String(dataToSendLength);
        WaitForDataWelcome = true; // выставляем флаг, что мы ждём >
        SendCommand(command);       
      }
      break;

      case wfaACTUALSEND: // отсылаем данные клиенту
      {
      #ifdef WIFI_DEBUG
        Serial.println(F("Sending data to the client..."));
      #endif
        WIFI_SERIAL.write(TEMP_DATA_TO_SEND.c_str(),dataToSendLength);
        TEMP_DATA_TO_SEND = F("");
      }
      break;

      case wfaCIPCLOSE: // закрываем соединение с клиентом
      {
       #ifdef WIFI_DEBUG
        Serial.println(F("Closing client connection..."));
      #endif
        String command = F("AT+CIPCLOSE=");
        command += connectedClientID;
        SendCommand(command);
      }
      break;

      case wfaIdle:
      {
        // ничего не делаем
      }
      break;
      
    } // switch
}
void WiFiModule::Update(uint16_t dt)
{ 
  UNUSED(dt);
  
  ProcessQueue();

}
bool  WiFiModule::ExecCommand(const Command& command)
{
  ModuleController* c = GetController();
  String answer = NOT_SUPPORTED;
  bool answerStatus = false;

  if(command.GetType() == ctSET) // установка свойств
  {
    uint8_t argsCnt = command.GetArgsCount();
    if(argsCnt > 0)
    {
      String t = command.GetArg(0);
      if(t == WIFI_SETTINGS_COMMAND) // установить настройки вай-фай
      {
        if(argsCnt > 5)
        {
          int shouldConnectToRouter = command.GetArg(1).toInt();
          String routerID = command.GetArg(2);
          String routerPassword = command.GetArg(3);
          String stationID = command.GetArg(4);
          String stationPassword = command.GetArg(5);

          bool shouldReastartAP = Settings->GetStationID() != stationID ||
          Settings->GetStationPassword() != stationPassword;


          Settings->SetWiFiState(shouldConnectToRouter);
          Settings->SetRouterID(routerID);
          Settings->SetRouterPassword(routerPassword);
          Settings->SetStationID(stationID);
          Settings->SetStationPassword(stationPassword);
          
          if(!routerID.length())
            Settings->SetWiFiState(0); // не коннектимся к роутеру

          Settings->Save(); // сохраняем настройки

          if(Settings->GetWiFiState() & 0x01) // коннектимся к роутеру
            actionsQueue.push_back(wfaCWJAP); // коннектимся к роутеру совсем в конце
          else
            actionsQueue.push_back(wfaCWQAP); // отсоединяемся от роутера

          if(shouldReastartAP) // надо пересоздать точку доступа
          {
            actionsQueue.push_back(wfaCIPSERVER); // сервер поднимаем в последнюю очередь
            actionsQueue.push_back(wfaCIPMUX); // разрешаем множественные подключения
            actionsQueue.push_back(wfaCIPMODE); // устанавливаем режим работы
            actionsQueue.push_back(wfaCWSAP); // создаём точку доступа
          }
           
          
          answerStatus = true;
          answer = t; answer += PARAM_DELIMITER; answer += REG_SUCC;
        }
        else
          answer = PARAMS_MISSED; // мало параметров
        
      } // WIFI_SETTINGS_COMMAND
    }
    else
      answer = PARAMS_MISSED; // мало параметров
  } // SET
  else
  if(command.GetType() == ctGET) // чтение свойств
  {
    uint8_t argsCnt = command.GetArgsCount();
    if(argsCnt > 0)
    {
      String t = command.GetArg(0);
      
      if(t == IP_COMMAND) // получить данные об IP
      {
        if(currentAction != wfaIdle) // не можем ответить на запрос немедленно
          answer = BUSY;
        else
        {
        #ifdef WIFI_DEBUG
         Serial.println("Request for IP info...");
        #endif
        
        
        SendCommand(F("AT+CIFSR"));
        // поскольку у нас serialEvent не основан на прерываниях, на самом-то деле (!),
        // то мы должны получить ответ вот прямо вот здесь, и разобрать его.


        String line; // тут принимаем данные до конца строки
        String apCurrentIP;
        String stationCurrentIP;
        bool  apIpDone = false;
        bool staIpDone = false;
        

        char ch;
        while(1)
        { 
          if(apIpDone && staIpDone) // получили оба IP
            break;
            
          while(WIFI_SERIAL.available())
          {
            ch = WIFI_SERIAL.read();
        
            if(ch == '\r')
              continue;
            
            if(ch == '\n')
            {
              // получили строку, разбираем её
                if(line.startsWith(F("+CIFSR:APIP"))) // IP нашей точки доступа
                 {
                    #ifdef WIFI_DEBUG
                      Serial.println(F("AP IP found, parse..."));
                    #endif
            
                   int idx = line.indexOf("\"");
                   if(idx != -1)
                   {
                      apCurrentIP = line.substring(idx+1);
                      idx = apCurrentIP.indexOf("\"");
                      if(idx != -1)
                        apCurrentIP = apCurrentIP.substring(0,idx);
                      
                   }
                   else
                    apCurrentIP = F("0.0.0.0");

                    apIpDone = true;
                 } // if(line.startsWith(F("+CIFSR:APIP")))
                 else
                  if(line.startsWith(F("+CIFSR:STAIP"))) // IP нашей точки доступа, назначенный роутером
                 {
                    #ifdef WIFI_DEBUG
                      Serial.println(F("STA IP found, parse..."));
                    #endif
            
                   int idx = line.indexOf("\"");
                   if(idx != -1)
                   {
                      stationCurrentIP = line.substring(idx+1);
                      idx = stationCurrentIP.indexOf("\"");
                      if(idx != -1)
                        stationCurrentIP = stationCurrentIP.substring(0,idx);
                      
                   }
                   else
                    stationCurrentIP = F("0.0.0.0");

                    staIpDone = true;
                 } // if(line.startsWith(F("+CIFSR:STAIP")))
             
              line = F("");
            } // ch == '\n'
            else
            {
                  line += ch;
            }
        
         if(apIpDone && staIpDone) // получили оба IP
            break;
 
          } // while
          
        } // while(1)
        


        #ifdef WIFI_DEBUG
          Serial.println("IP info requested.");
        #endif

        answerStatus = true;
        answer = t; answer += PARAM_DELIMITER;
        answer += apCurrentIP;
        answer += PARAM_DELIMITER;
        answer += stationCurrentIP;
        } // else not busy
      } // IP_COMMAND
    }
    else
      answer = PARAMS_MISSED; // мало параметров
  } // GET

  SetPublishData(&command,answerStatus,answer); // готовим данные для публикации
  c->Publish(this);

  return answerStatus;
}

