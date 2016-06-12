#include "WiFiModule.h"
#include "ModuleController.h"
#include "InteropStream.h"

#define WIFI_DEBUG_WRITE(s,ca) { Serial.print(String(F("[CA] ")) + String((ca)) + String(F(": ")));  Serial.println((s)); }
#define CHECK_QUEUE_TAIL(v) { if(!actionsQueue.size()) {Serial.println(F("[QUEUE IS EMPTY!]"));} else { if(actionsQueue[actionsQueue.size()-1]!=(v)){Serial.print(F("NOT RIGHT TAIL, WAITING: ")); Serial.print((v)); Serial.print(F(", ACTUAL: "));Serial.println(actionsQueue[actionsQueue.size()-1]); } } }
#define CIPSEND_COMMAND F("AT+CIPSENDBUF=") // F("AT+CIPSEND=")


bool WiFiModule::IsKnownAnswer(const String& line)
{
  return ( line == F("OK") || line == F("ERROR") || line == F("FAIL") || line.endsWith(F("SEND OK")) || line.endsWith(F("SEND FAIL")));
}

void WiFiModule::ProcessAnswerLine(const String& line)
{
    
  #ifdef WIFI_DEBUG
     WIFI_DEBUG_WRITE(line,currentAction);
  #endif

  // здесь может придти запрос от сервера
  if(line.startsWith(F("+IPD")))
  {
    ProcessQuery(line); // разбираем пришедшую команду
  } // if

  
  switch(currentAction)
  {
    case wfaWantReady:
    {
      // ждём ответа "ready" от модуля
      if(line == F("ready")) // получили
      {
        #ifdef WIFI_DEBUG
          WIFI_DEBUG_WRITE(F("[OK] => ESP restarted."),currentAction);
          CHECK_QUEUE_TAIL(wfaWantReady);
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
          WIFI_DEBUG_WRITE(F("[OK] => ECHO OFF processed."),currentAction);
          CHECK_QUEUE_TAIL(wfaEchoOff);
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
          WIFI_DEBUG_WRITE(F("[OK] => SoftAP mode is ON."),currentAction);
          CHECK_QUEUE_TAIL(wfaCWMODE);
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
          WIFI_DEBUG_WRITE(F("[OK] => access point created."),currentAction);
          CHECK_QUEUE_TAIL(wfaCWSAP);
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
          WIFI_DEBUG_WRITE(F("[OK] => TCP-server mode now set to 0."),currentAction);
          CHECK_QUEUE_TAIL(wfaCIPMODE);
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
          WIFI_DEBUG_WRITE(F("[OK] => Multiple connections allowed."),currentAction);
          CHECK_QUEUE_TAIL(wfaCIPMUX);
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
          WIFI_DEBUG_WRITE(F("[OK] => TCP-server started."),currentAction);
          CHECK_QUEUE_TAIL(wfaCIPSERVER);
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
          WIFI_DEBUG_WRITE(F("[OK] => connected to the router."),currentAction);
          CHECK_QUEUE_TAIL(wfaCWJAP);
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
          WIFI_DEBUG_WRITE(F("[OK] => disconnected from router."),currentAction);
          CHECK_QUEUE_TAIL(wfaCWQAP);
        #endif
       actionsQueue.pop(); // убираем последнюю обработанную команду     
       currentAction = wfaIdle;
      }
     
    }
    break;


    case wfaCIPSEND: // надо отослать данные клиенту
    {
      // wfaCIPSEND плюёт в очередь функция UpdateClients, перед отсылкой команды модулю.
      // значит, мы сами должны разрулить ситуацию, как быть с обработкой этой команды. 
        #ifdef WIFI_DEBUG
          WIFI_DEBUG_WRITE(F("Waiting for \">\"..."),currentAction);
        #endif        
            
      if(line == F(">")) // дождались приглашения
      {
        #ifdef WIFI_DEBUG
          WIFI_DEBUG_WRITE(F("\">\" FOUND, sending the data..."),currentAction);
          CHECK_QUEUE_TAIL(wfaCIPSEND);
        #endif        
        actionsQueue.pop(); // убираем последнюю обработанную команду (wfaCIPSEND, которую плюнула в очередь функция UpdateClients)
        actionsQueue.push_back(wfaACTUALSEND); // добавляем команду на актуальный отсыл данных в очередь     
        currentAction = wfaIdle;
        inSendData = true; // выставляем флаг, что мы отсылаем данные, и тогда очередь обработки клиентов не будет чухаться
      }
      else
      if(line.indexOf(F("FAIL")) != -1 || line.indexOf(F("ERROR")) != -1)
      {
        // передача данных клиенту неудачна, отсоединяем его принудительно
         #ifdef WIFI_DEBUG
          WIFI_DEBUG_WRITE(F("Closing client connection unexpectedly!"),currentAction);
          CHECK_QUEUE_TAIL(wfaCIPSEND);
        #endif 
                
        clients[currentClientIDX].SetConnected(false); // выставляем текущему клиенту статус "отсоединён"
        actionsQueue.pop(); // убираем последнюю обработанную команду (wfaCIPSEND, которую плюнула в очередь функция UpdateClients)
        currentAction = wfaIdle; // переходим в ждущий режим
        inSendData = false;
      }
    }
    break;

    case wfaACTUALSEND: // отослали ли данные?
    {
      // может ли произойти ситуация, когда в очереди есть wfaACTUALSEND, помещенная туда обработчиком wfaCIPSEND,
      // но до Update дело ещё не дошло? Считаем, что нет. Мы попали сюда после функции Update, которая в обработчике wfaACTUALSEND
      // отослала нам пакет данных. Надо проверить результат отсылки.
      if(IsKnownAnswer(line)) // получен результат отсылки пакета
      {
        #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("DATA SENT, go to IDLE mode..."),currentAction);
        // проверяем валидность того, что в очереди
        CHECK_QUEUE_TAIL(wfaACTUALSEND);
        #endif
        actionsQueue.pop(); // убираем последнюю обработанную команду (wfaACTUALSEND, которая в очереди)    
        currentAction = wfaIdle; // разрешаем обработку следующего клиента
        inSendData = false; // выставляем флаг, что мы отправили пакет, и можем обрабатывать следующего клиента
        if(!clients[currentClientIDX].HasPacket())
        {
           // данные у клиента закончились
        #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(String(F("No packets in client #")) + String(currentClientIDX),currentAction);
        #endif

         #ifndef WIFI_TCP_KEEP_ALIVE  // если надо разрывать соединение после отсыла результатов - разрываем его
          if(clients[currentClientIDX].IsConnected())
          {
            #ifdef WIFI_DEBUG
            WIFI_DEBUG_WRITE(String(F("Client #")) + String(currentClientIDX) + String(F(" has no packets, closing connection...")),currentAction);
            #endif
            actionsQueue.push_back(wfaCIPCLOSE); // добавляем команду на закрытие соединения
            inSendData = true; // пока не обработаем отсоединение клиента - не разрешаем посылать пакеты другим клиентам
          } // if
        #endif  
        
        } // if
      
        if(line.indexOf(F("FAIL")) != -1 || line.indexOf(F("ERROR")) != -1)
        {
          // передача данных клиенту неудачна, отсоединяем его принудительно
           #ifdef WIFI_DEBUG
            WIFI_DEBUG_WRITE(F("Closing client connection unexpectedly!"),currentAction);
          #endif 
                  
          clients[currentClientIDX].SetConnected(false);
        }      

      } // if known answer

    }
    break;

    case wfaCIPCLOSE: // закрыли соединение
    {
      if(IsKnownAnswer(line)) // дождались приглашения
      {
        #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Client connection closed."),currentAction);
        CHECK_QUEUE_TAIL(wfaCIPCLOSE);
        #endif
        clients[currentClientIDX].SetConnected(false);
        actionsQueue.pop(); // убираем последнюю обработанную команду     
        currentAction = wfaIdle;
        inSendData = false; // разрешаем обработку других клиентов
      }
    }
    break;

    case wfaIdle:
    {
    }
    break;
  } // switch

  // смотрим, может - есть статус клиента
  int idx = line.indexOf(F(",CONNECT"));
  if(idx != -1)
  {
    // клиент подсоединился
    String s = line.substring(0,idx);
    int clientID = s.toInt();
    if(clientID >= 0 && clientID < MAX_WIFI_CLIENTS)
    {
   #ifdef WIFI_DEBUG
    WIFI_DEBUG_WRITE(String(F("[CLIENT CONNECTED] - ")) + s,currentAction);
   #endif     
      clients[clientID].SetConnected(true);
    }
  } // if
  idx = line.indexOf(F(",CLOSED"));
 if(idx != -1)
  {
    // клиент отсоединился
    String s = line.substring(0,idx);
    int clientID = s.toInt();
    if(clientID >= 0 && clientID < MAX_WIFI_CLIENTS)
    {
   #ifdef WIFI_DEBUG
   WIFI_DEBUG_WRITE(String(F("[CLIENT DISCONNECTED] - ")) + s,currentAction);
   #endif     
      clients[clientID].SetConnected(false);
      
    }
  } // if
  
  
}
void WiFiModule::ProcessQuery(const String& command)
{
  
  int idx = command.indexOf(F(",")); // ищем первую запятую после +IPD
  const char* ptr = command.c_str();
  ptr += idx+1;
  // перешли за запятую, парсим ID клиента
  String connectedClientID = F("");
  while(*ptr != ',')
  {
    connectedClientID += (char) *ptr;
    ptr++;
  }
  ptr++; // за запятую
  String dataLen;
  while(*ptr != ':')
  {
    dataLen += (char) *ptr;
    ptr++; // перешли на начало данных
  }
  
  ptr++; // за двоеточие

  // тут пришла команда, разбираем её
  ProcessCommand(connectedClientID.toInt(),dataLen.toInt(),ptr);
   
}
void WiFiModule::ProcessCommand(int clientID, int dataLen, const char* command)
{
  // обрабатываем команду, пришедшую по TCP/IP
  
 #ifdef WIFI_DEBUG
  WIFI_DEBUG_WRITE(String(F("Client ID = ")) + String(clientID) + String(F("; len= ")) + String(dataLen),currentAction);
  WIFI_DEBUG_WRITE(String(F("Requested command: ")) + String(command),currentAction);
#endif
  
  // работаем с клиентом
  if(clientID >=0 && clientID < MAX_WIFI_CLIENTS)
  {


      if(!*command) // пустой пакет, с переводом строки
        dataLen = 0;

        // теперь нам надо сложить все данные в клиента - как только он получит полный пакет - он подготовит
        // все данные к отправке. Признаком конца команды к контроллеру у нас служит перевод строки \r\n.
        // следовательно, пока мы не получим в любом виде перевод строки - считается, что команда не получена.
        // перевод строки может быть либо получен прямо в данных, либо - в следующем пакете.

        // как только клиент накопит всю команду - он получает данные с контроллера в следующем вызове Update.
        clients[clientID].CommandRequested(dataLen,command); // говорим клиенту, чтобы сложил во внутренний буфер
  } // if
 }
void WiFiModule::Setup()
{
  // настройка модуля тут
  
  Settings = MainController->GetSettings();
  nextClientIDX = 0;
  currentClientIDX = 0;
  inSendData = false;
  
  for(uint8_t i=0;i<MAX_WIFI_CLIENTS;i++)
    clients[i].Setup(i, WIFI_PACKET_LENGTH);

 // waitForQueryCompleted = false;
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
    WIFI_DEBUG_WRITE(String(F("==> Send the \"")) + command + String(F("\" command to ESP...")),currentAction);
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
        WIFI_DEBUG_WRITE(F("Restart the ESP..."),currentAction);
      #endif
      SendCommand(F("AT+RST"));
      //SendCommand(F("AT+GMR"));
      }
      break;

      case wfaEchoOff:
      {
        // выключаем эхо
      #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Disable echo..."),currentAction);
      #endif
      SendCommand(F("ATE0"));
      //SendCommand(F("AT+GMR"));
      //SendCommand(F("AT+CIOBAUD=230400")); // переводим на другую скорость
      }
      break;

      case wfaCWMODE:
      {
        // переходим в смешанный режим
      #ifdef WIFI_DEBUG
       WIFI_DEBUG_WRITE(F("Go to SoftAP mode..."),currentAction);
      #endif
      SendCommand(F("AT+CWMODE_DEF=3"));
      }
      break;

      case wfaCWSAP: // создаём точку доступа
      {

      #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Creating the access point..."),currentAction);
      #endif
      
        String com = F("AT+CWSAP_DEF=\"");
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
        WIFI_DEBUG_WRITE(F("Set the TCP server mode to 0..."),currentAction);
      #endif
      SendCommand(F("AT+CIPMODE=0"));
      
      }
      break;

      case wfaCIPMUX: // разрешаем множественные подключения
      {
      #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Allow the multiple connections..."),currentAction);
      #endif
      SendCommand(F("AT+CIPMUX=1"));
        
      }
      break;

      case wfaCIPSERVER: // запускаем сервер
      {  
      #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Starting TCP-server..."),currentAction);
      #endif
      SendCommand(F("AT+CIPSERVER=1,1975"));
      
      }
      break;

      case wfaCWQAP: // отсоединяемся от точки доступа
      {  
      #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Disconnect from router..."),currentAction);
      #endif
      SendCommand(F("AT+CWQAP"));
      
      }
      break;

      case wfaCWJAP: // коннектимся к роутеру
      {
      #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Connecting to the router..."),currentAction);
      #endif
        String com = F("AT+CWJAP_DEF=\"");
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
       //  WIFI_DEBUG_WRITE(F("ASSERT: wfaCIPSEND in ProcessQueue!"),currentAction);
        #endif
        
      }
      break;

      case wfaACTUALSEND: // дождались приглашения в функции ProcessAnswerLine, она поместила команду wfaACTUALSEND в очередь - отсылаем данные клиенту
      {
            #ifdef WIFI_DEBUG
              WIFI_DEBUG_WRITE(String(F("Sending data to the client #")) + String(currentClientIDX),currentAction);
            #endif
      
            if(clients[currentClientIDX].IsConnected()) // не отвалился ли клиент?
            {
              // клиент по-прежнему законнекчен, посылаем данные
              if(!clients[currentClientIDX].SendPacket(&(WIFI_SERIAL)))
              {
                // если мы здесь - то пакетов у клиента больше не осталось. Надо дождаться подтверждения отсылки последнего пакета
                // в функции ProcessAnswerLine (обработчик wfaACTUALSEND), и послать команду на закрытие соединения с клиентом.
              #ifdef WIFI_DEBUG
              WIFI_DEBUG_WRITE(String(F("All data to the client #")) + String(currentClientIDX) + String(F(" has sent, need to wait for last packet sent..")),currentAction);
              #endif
 
              }
              else
              {
                // ещё есть пакеты, продолжаем отправлять в следующих вызовах Update
              #ifdef WIFI_DEBUG
              WIFI_DEBUG_WRITE(String(F("Client #")) + String(currentClientIDX) + String(F(" has ")) + String(clients[currentClientIDX].GetPacketsLeft()) + String(F(" packets left...")),currentAction);
              #endif
              } // else
            } // is connected
            else
            {
              // клиент отвалится, чистим...
            #ifdef WIFI_DEBUG
              WIFI_DEBUG_WRITE(F("Client disconnected, clear the client data..."),currentAction);
            #endif
              clients[currentClientIDX].SetConnected(false);
            }

      }
      break;

      case wfaCIPCLOSE: // закрываем соединение с клиентом
      {
        if(clients[currentClientIDX].IsConnected()) // только если клиент законнекчен 
        {
          #ifdef WIFI_DEBUG
            WIFI_DEBUG_WRITE(String(F("Closing client #")) + String(currentClientIDX) + String(F(" connection...")),currentAction);
          #endif
          clients[currentClientIDX].SetConnected(false);
          String command = F("AT+CIPCLOSE=");
          command += currentClientIDX; // закрываем соединение
          SendCommand(command);
        }
        
        else
        {
          #ifdef WIFI_DEBUG
            WIFI_DEBUG_WRITE(String(F("Client #")) + String(currentClientIDX) + String(F(" already broken!")),currentAction);
            CHECK_QUEUE_TAIL(wfaCIPCLOSE);
          #endif
          // просто убираем команду из очереди
           actionsQueue.pop();
           currentAction = wfaIdle; // разрешаем обработку следующей команды
           inSendData = false; // разрешаем обработку следующего клиента
        } // else
        
      }
      break;

      case wfaIdle:
      {
        // ничего не делаем

      }
      break;
      
    } // switch
}
void WiFiModule::UpdateClients()
{
  if(currentAction != wfaIdle || inSendData) // чем-то заняты, не можем ничего делать
    return;
    
  // тут ищем, какой клиент сейчас хочет отослать данные

  for(uint8_t idx = nextClientIDX;idx < MAX_WIFI_CLIENTS; idx++)
  { 
    ++nextClientIDX; // переходим на следующего клиента, как только текущему будет послан один пакет

    clients[idx].Update(); // обновляем внутреннее состояние клиента - здесь он может подготовить данные к отправке, например
    
    if(clients[idx].IsConnected() && clients[idx].HasPacket())
    {
      currentAction = wfaCIPSEND; // говорим однозначно, что нам надо дождаться >
      actionsQueue.push_back(wfaCIPSEND); // добавляем команду отсылки данных в очередь
      
    #ifdef WIFI_DEBUG
      WIFI_DEBUG_WRITE(F("Sending data command to the ESP..."),currentAction);
    #endif
  
      // клиент подсоединён и ждёт данных от нас - отсылаем ему следующий пакет
      currentClientIDX = idx; // сохраняем номер клиента, которому будем посылать данные
      String command = CIPSEND_COMMAND;
      command += String(idx);
      command += F(",");
      command += String(clients[idx].GetPacketLength());
      WaitForDataWelcome = true; // выставляем флаг, что мы ждём >

      SendCommand(command);
  
      break; // выходим из цикла
    } // if
    
  } // for
  
  if(nextClientIDX >= MAX_WIFI_CLIENTS) // начинаем обработку клиентов сначала
    nextClientIDX = 0;  
}
void WiFiModule::Update(uint16_t dt)
{ 
  UNUSED(dt);
  
  UpdateClients();
  ProcessQueue();

}
bool  WiFiModule::ExecCommand(const Command& command, bool wantAnswer)
{
  UNUSED(wantAnswer);
  
  PublishSingleton = NOT_SUPPORTED;

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
          int shouldConnectToRouter = atoi(command.GetArg(1));
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
           
          
          PublishSingleton.Status = true;
          PublishSingleton = t; 
          PublishSingleton << PARAM_DELIMITER << REG_SUCC;
        }
        else
          PublishSingleton = PARAMS_MISSED; // мало параметров
        
      } // WIFI_SETTINGS_COMMAND
    }
    else
      PublishSingleton = PARAMS_MISSED; // мало параметров
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
          PublishSingleton = BUSY;
        else
        {
        #ifdef WIFI_DEBUG
         WIFI_DEBUG_WRITE(F("Request for IP info..."),currentAction);
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
                      WIFI_DEBUG_WRITE(F("AP IP found, parse..."),currentAction);
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
                      WIFI_DEBUG_WRITE(F("STA IP found, parse..."),currentAction);
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
          WIFI_DEBUG_WRITE(F("IP info requested."),currentAction);
        #endif

        PublishSingleton.Status = true;
        PublishSingleton = t; 
        PublishSingleton << PARAM_DELIMITER << apCurrentIP << PARAM_DELIMITER << stationCurrentIP;
        } // else not busy
      } // IP_COMMAND
    }
    else
      PublishSingleton = PARAMS_MISSED; // мало параметров
  } // GET

  MainController->Publish(this,command);

  return PublishSingleton.Status;
}

