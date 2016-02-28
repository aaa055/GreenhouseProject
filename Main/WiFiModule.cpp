#include "WiFiModule.h"
#include "ModuleController.h"
#include "InteropStream.h"

#define WIFI_DEBUG_WRITE(s) Serial.print(s)

WIFIClient::WIFIClient()
{
  isConnected = false;
  isFileOpen = false;
  httpHeaders.reserve(100); // зарезервируем 100 байт под заголовки
  Clear();
}
void WIFIClient::SetConnected(bool c) 
{
  isConnected = c;
  
  if(!c) // ничего не будем отсылать, т.к. клиент отсоединился
    Clear(); // очищаем все статусы
}
String WIFIClient::GetContentType(const String& uri)
{
  String s = uri;
  s.toLowerCase();
  
  if(s.endsWith(F(".html")) || s.endsWith(F(".htm")))
    return F("text/html");
  else
  if(s.endsWith(F(".js")))
    return F("text/javascript");
  else
  if(s.endsWith(F(".png")))
    return F("image/png");
  else
  if(s.endsWith(F(".gif")))
    return F("image/gif");
  else
  if(s.endsWith(F(".jpg")) || s.endsWith(F(".jpeg")))
    return F("image/jpeg");
  else
  if(s.endsWith(F(".css")))
    return F("text/css");

  return F("text/plain");  
}
void WIFIClient::EnsureCloseFile()
{
  if(isFileOpen)
    workFile.close(); // закрываем файл, с которым работали
    
  isFileOpen = false;  
}
void WIFIClient::Clear()
{
  nextPacketLength = 0;
  packetsCount = 0;
  contentLength = 0;
  packetsLeft = 0;
  packetsSent = 0;
  sentContentLength = 0;
  httpHeaders = F("");
  ajaxQueryFound = false;
  dataToSend = F("");

  

  //TODO: Тут закрываем файлы и пр.!
  EnsureCloseFile();
  
}
bool WIFIClient::PrepareFile(const String& fileName,unsigned long& fileSize)
{
  EnsureCloseFile(); // сперва закрываем файл
  
  fileSize = 0;

  if(!parent->CanWorkWithSD()) // не можем работать с SD-модулем, поэтому мимо кассы
  {
  #ifdef WIFI_DEBUG
    WIFI_DEBUG_WRITE(F("No SD module!")); WIFI_DEBUG_WRITE(NEWLINE);
  #endif
    return false; 
  }

  if(!SD.exists(fileName)) // файл не найден
  {
  #ifdef WIFI_DEBUG
  WIFI_DEBUG_WRITE(F("File not found on SD: ")); WIFI_DEBUG_WRITE(fileName); WIFI_DEBUG_WRITE(NEWLINE);
  #endif
    return false;
  }

  workFile = SD.open(fileName);
  if(workFile)
  {
    // файл открыли, подготовили, получили размер
    isFileOpen = true;
    fileSize = workFile.size();
    return true;
  }

  return false;
}
bool WIFIClient::Prepare(const String& uriRequested)
{
  Clear(); // закрываем файлы, очищаем переменные и т.п.
  
  #ifdef WIFI_DEBUG
  WIFI_DEBUG_WRITE(F("Prepare URI request: ")); WIFI_DEBUG_WRITE(uriRequested); WIFI_DEBUG_WRITE(NEWLINE);
  #endif



 COMMAND_TYPE cType = ctUNKNOWN;
 if(uriRequested.startsWith(F("CTGET=")))
  cType = ctGET;
 else
   if(uriRequested.startsWith(F("CTSET=")))
  cType = ctSET;

 if(cType != ctUNKNOWN) // надо получить данные с контроллера
 {
    ajaxQueryFound = true;
    InteropStream streamI;
    streamI.SetController(controller);
  
    String command = uriRequested.substring(6);
    if(streamI.QueryCommand(cType,command,false))
    {
      // можем формировать AJAX-ответ
      dataToSend = F("{");
      dataToSend += F("\"query\": \"");
      dataToSend += uriRequested;
      dataToSend += F("\",\"answer\": \"");
      String dt = streamI.GetData();
      dt.trim(); // убираем перевод строки в конце
      dataToSend += dt;
      dataToSend += F("\"}");      
    } // if
 } // if

  // проверяем, есть ли файл на диске, например
  bool dataFound = true;
  
 if(!ajaxQueryFound)
  {
   // подготавливаем файл для отправки
   dataFound = PrepareFile(uriRequested,contentLength);
  }
  else
  {
    // ответ на запрос AJAX
    // выставляем длину данных, которые надо передать
    contentLength = dataToSend.length();
  }

 
  // формируем заголовки
  httpHeaders = H_HTTP_STATUS;
  if(dataFound)
    httpHeaders += STATUS_200; // данные нашли
  else
  {
    // данные не нашли
    httpHeaders += STATUS_404;
    dataToSend = STATUS_404_TEXT;
    contentLength = dataToSend.length();
  }

  httpHeaders += NEWLINE;
/*
  httpHeaders += H_CONNECTION;
  httpHeaders += NEWLINE;
*/
  // подставляем тип данных
  httpHeaders += H_CONTENT_TYPE;
  if(ajaxQueryFound)
    httpHeaders += F("text/javascript");
  else
    httpHeaders += GetContentType(uriRequested);
    
  httpHeaders += NEWLINE;

// говорим, сколько данных будет идти
  httpHeaders += H_CONTENT_LENGTH;
  httpHeaders += String(contentLength);
  httpHeaders += NEWLINE;

  // добавляем вторую пустую строку
  httpHeaders += NEWLINE;

  // теперь надо вычислить общую длину пересылаемых данных.
  // просто складываем длину заголовков с длиной данных,
  // и в результате получим кол-во байт, которые надо
  // скормить модулю ESP
  contentLength +=  httpHeaders.length();
 
 // вычисляем кол-во пакетов, которые нам надо послать
 if(contentLength < WIFI_PACKET_LENGTH)
  packetsCount = 1;
 else
 {
  packetsCount = contentLength/WIFI_PACKET_LENGTH;
  if((contentLength > WIFI_PACKET_LENGTH) && (contentLength % WIFI_PACKET_LENGTH))
    packetsCount++;
 }

 // говорим, что мы не отослали ещё ни одного пакета
 packetsLeft = packetsCount;

 // вычисляем длину пакета для пересылки
 nextPacketLength = WIFI_PACKET_LENGTH;
 if(contentLength < WIFI_PACKET_LENGTH)
  nextPacketLength = contentLength;

  return HasPacket();
}
bool WIFIClient::SendPacket(Stream* s)
{

 if(!HasPacket()) // нечего больше отсылать
  return false;
  
  #ifdef WIFI_DEBUG
  WIFI_DEBUG_WRITE(F("Send packet # ")); WIFI_DEBUG_WRITE(packetsSent);WIFI_DEBUG_WRITE(NEWLINE);
  #endif  
  //тут отсылаем пакет по Wi-Fi
  // возвращаем false, если больше нечего посылать

  // формируем пакет. для начала смотрим, есть ли у нас ещё неотосланные заголовки
  String str;

  uint16_t headersLen =  httpHeaders.length(); 
  if(headersLen > 0)
  {
    // ещё есть непосланные заголовки, надо их дослать.
    if(headersLen >= nextPacketLength)
    {
       // длина оставшихся к отсылу заголовков больше, чем размер одного пакета.
       // поэтому можем отсылать целиком
       str = httpHeaders.substring(0,nextPacketLength);
       httpHeaders = httpHeaders.substring(nextPacketLength);

       s->write(str.c_str(),nextPacketLength); // пишем данные в поток
       
    }
    else
    {
      // длина оставшихся заголовков меньше, чем длина следующего пакета, поэтому нам надо дочитать
      // необходимое кол-во байт из данных.
      str = httpHeaders;
      httpHeaders = F("");
      uint16_t dataLeft = nextPacketLength - str.length();

      s->write(str.c_str(),str.length()); // пишем данные в поток

      if(dataToSend.length())//ajaxQueryFound)
      {
        // тут вычитываем недостающие байты из данных, сформированных в ответ на AJAX-запрос, или данные страницы. сформированной в памяти
        str = dataToSend.substring(0,dataLeft);
        dataToSend = dataToSend.substring(dataLeft);

        s->write(str.c_str(),str.length()); // пишем данные в поток

      }
      else
      {
        // тут вычитываем данные из файла, длиной dataLeft
        if(isFileOpen) // если файл открыт
        {
          for(uint16_t i=0;i<dataLeft;i++)
            s->write(workFile.read()); // пишем данные в поток
            //str += (char) workFile.read();
        }
      }
    } // else
   
  } // if
  else
  {
    // у нас идёт работа с данными уже
    if(dataToSend.length())//ajaxQueryFound)
    {
        // тут дочитываем недостающие байты из данных, сформированных в ответ на AJAX-запрос, или данные страницы. сформированной в памяти
      str = dataToSend.substring(0,nextPacketLength);
      dataToSend = dataToSend.substring(nextPacketLength);

      s->write(str.c_str(),str.length()); // пишем данные в поток
    }
    else
    {
      // тут вычитываем данные из файла, длиной nextPacketLength
      if(isFileOpen) // если файл открыт
      {
        for(uint16_t i=0;i<nextPacketLength;i++)
          s->write(workFile.read()); // пишем данные в поток
          //str += (char) workFile.read();
      }
    }
  }
 // вычисляем, сколько осталось пакетов
 packetsLeft--;
 packetsSent++;
 sentContentLength += nextPacketLength;

 // вычисляем длину следующего пакета
 if((contentLength - sentContentLength) >= WIFI_PACKET_LENGTH)
   nextPacketLength = WIFI_PACKET_LENGTH;
 else
  nextPacketLength = (contentLength - sentContentLength);
  
  return HasPacket(); // если ещё есть пакеты - продолжаем отсылать
}
uint16_t WIFIClient::GetPacketLength()
{
  return nextPacketLength;
}








bool WiFiModule::IsKnownAnswer(const String& line)
{
  return ( line == F("OK") || line == F("ERROR") || line == F("FAIL") || line == F("SEND OK") || line == F("SEND FAIL"));
}

void WiFiModule::ProcessAnswerLine(const String& line)
{
  // получаем ответ на команду, посланную модулю
  #ifdef WIFI_DEBUG
     WIFI_DEBUG_WRITE(line);WIFI_DEBUG_WRITE(NEWLINE);
    //Serial.print(F("<== Receive \"")); Serial.print(line); Serial.println(F("\" answer from ESP-01..."));
    WIFI_DEBUG_WRITE("[CA] " + String(currentAction));WIFI_DEBUG_WRITE(NEWLINE);
  #endif

  // здесь может придти запрос от сервера
  if(line.startsWith(F("+IPD")))
  {
     // пришёл запрос от сервера, сохраняем его
     waitForQueryCompleted = true; // ждём конца запроса
     httpQuery = F(""); // сбрасываем запрос
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
  } // if waitForQueryCompleted



  
  switch(currentAction)
  {
    case wfaWantReady:
    {
      // ждём ответа "ready" от модуля
      if(line == F("ready")) // получили
      {
        #ifdef WIFI_DEBUG
          WIFI_DEBUG_WRITE(F("[OK] => ESP-01 restarted."));WIFI_DEBUG_WRITE(NEWLINE);
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
          WIFI_DEBUG_WRITE(F("[OK] => ECHO OFF processed."));WIFI_DEBUG_WRITE(NEWLINE);
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
          WIFI_DEBUG_WRITE(F("[OK] => SoftAP mode is ON."));WIFI_DEBUG_WRITE(NEWLINE);
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
          WIFI_DEBUG_WRITE(F("[OK] => access point created."));WIFI_DEBUG_WRITE(NEWLINE);
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
          WIFI_DEBUG_WRITE(F("[OK] => TCP-server mode now set to 0."));WIFI_DEBUG_WRITE(NEWLINE);
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
          WIFI_DEBUG_WRITE(F("[OK] => Multiple connections allowed."));WIFI_DEBUG_WRITE(NEWLINE);
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
          WIFI_DEBUG_WRITE(F("[OK] => TCP-server started."));WIFI_DEBUG_WRITE(NEWLINE);
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
          WIFI_DEBUG_WRITE(F("[OK] => connected to the router."));WIFI_DEBUG_WRITE(NEWLINE);
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
          WIFI_DEBUG_WRITE(F("[OK] => disconnected from router."));WIFI_DEBUG_WRITE(NEWLINE);
        #endif
       actionsQueue.pop(); // убираем последнюю обработанную команду     
       currentAction = wfaIdle;
      }
     
    }
    break;


    case wfaCIPSEND: // надо отослать данные клиенту
    {
        #ifdef WIFI_DEBUG
          WIFI_DEBUG_WRITE(F("Waiting for wfaCIPSEND >"));WIFI_DEBUG_WRITE(NEWLINE);
        #endif        
            
      if(line == F(">")) // дождались приглашения
      {
        #ifdef WIFI_DEBUG
          WIFI_DEBUG_WRITE(F("SENDING THE DATA!"));WIFI_DEBUG_WRITE(NEWLINE);
        #endif        
        actionsQueue.pop(); // убираем последнюю обработанную команду (wfaCIPSEND)
        actionsQueue.push_back(wfaACTUALSEND); // добавляем команду на актуальный отсыл данных в очередь     
        currentAction = wfaIdle;
      }
      else
      if(line.indexOf(F("FAIL")) != -1 || line.indexOf(F("ERROR")) != -1)
      {
        // передача данных клиенту неудачна, отсоединяем его принудительно
         #ifdef WIFI_DEBUG
          WIFI_DEBUG_WRITE(F("Closing client connection unexpectedly!"));WIFI_DEBUG_WRITE(NEWLINE);
        #endif 
                
        clients[currentClientIDX].SetConnected(false);
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
        WIFI_DEBUG_WRITE(F("DATA SENT!"));WIFI_DEBUG_WRITE(NEWLINE);
        #endif
        actionsQueue.pop(); // убираем последнюю обработанную команду     
        currentAction = wfaIdle;
        if(!clients[currentClientIDX].HasPacket())
        {
           // данные у клиента закончились
        #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("No packets in client, closing connection..."));WIFI_DEBUG_WRITE(NEWLINE);
        #endif

          if(clients[currentClientIDX].IsConnected())
          {
            actionsQueue.push_back(wfaCIPCLOSE); // добавляем команду на закрытие соединения
          } // if
        
        } // if
      
        if(line.indexOf(F("FAIL")) != -1 || line.indexOf(F("ERROR")) != -1)
        {
          // передача данных клиенту неудачна, отсоединяем его принудительно
           #ifdef WIFI_DEBUG
            WIFI_DEBUG_WRITE(F("Closing client connection unexpectedly!"));WIFI_DEBUG_WRITE(NEWLINE);
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
        WIFI_DEBUG_WRITE(F("Client connection closed."));WIFI_DEBUG_WRITE(NEWLINE);
        #endif
        clients[currentClientIDX].SetConnected(false);
        actionsQueue.pop(); // убираем последнюю обработанную команду     
        currentAction = wfaIdle;
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
    WIFI_DEBUG_WRITE(F("[CLIENT CONNECTED] - ")); WIFI_DEBUG_WRITE(s); WIFI_DEBUG_WRITE(NEWLINE);
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
   WIFI_DEBUG_WRITE(F("[CLIENT DISCONNECTED] - ")); WIFI_DEBUG_WRITE(s);WIFI_DEBUG_WRITE(NEWLINE);
   #endif     
      clients[clientID].SetConnected(false);
      waitForQueryCompleted = false;
      
      if(currentClientIDX == clientID && WaitForDataWelcome) // если мы ждём приглашения на отсыл данных этому клиенту,
        WaitForDataWelcome = false; // то снимаем его
        
      currentAction = wfaIdle;
    }
  } // if
  
  
}
void WiFiModule::ProcessQuery()
{
  
  int idx = httpQuery.indexOf(F(",")); // ищем первую запятую после +IPD
  const char* ptr = httpQuery.c_str();
  ptr += idx+1;
  // перешли за запятую, парсим ID клиента
  String connectedClientID = F("");
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
       String requestedURI = str.substring(0,idx);
       if(!requestedURI.length()) // запросили страницу по умолчанию
          requestedURI = DEF_PAGE;
       ProcessURIRequest(connectedClientID.toInt(), requestedURI); // обрабатываем запрос от клиента
    } // if 
  } // if 
   
}
void WiFiModule::ProcessURIRequest(int clientID, const String& requesterURI)
{
 #ifdef WIFI_DEBUG
 WIFI_DEBUG_WRITE(F("Client ID = "));
 WIFI_DEBUG_WRITE(clientID);WIFI_DEBUG_WRITE(NEWLINE);
 WIFI_DEBUG_WRITE(F("Requested URI: "));
 WIFI_DEBUG_WRITE(requesterURI);WIFI_DEBUG_WRITE(NEWLINE);
#endif

  // работаем с клиентом
  if(clientID >=0 && clientID < MAX_WIFI_CLIENTS)
  {
    if(clients[clientID].Prepare(requesterURI)) // говорим клиенту, чтобы он подготовил данные к отправке
    {
      // клиент подготовил данные к отправке, отсылаем их в следующем вызове Update
      #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Client prepared the data..."));WIFI_DEBUG_WRITE(NEWLINE);
      #endif
    }
  } // if

}

void WiFiModule::Setup()
{
  // настройка модуля тут
  ModuleController* c = GetController();
  Settings = c->GetSettings();

  sdCardInited = SD.begin(SDCARD_CS_PIN); // пробуем инициализировать SD-модуль

  nextClientIDX = 0;
  currentClientIDX = 0;
  
  for(uint8_t i=0;i<MAX_WIFI_CLIENTS;i++)
    clients[i].Setup(c,this);

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
    WIFI_DEBUG_WRITE(F("==> Send the \"")); WIFI_DEBUG_WRITE(command); WIFI_DEBUG_WRITE(F("\" command to ESP-01..."));WIFI_DEBUG_WRITE(NEWLINE);
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
        WIFI_DEBUG_WRITE(F("Restart the ESP-01..."));WIFI_DEBUG_WRITE(NEWLINE);
      #endif
      SendCommand(F("AT+RST"));
      }
      break;

      case wfaEchoOff:
      {
        // выключаем эхо
      #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Disable echo..."));WIFI_DEBUG_WRITE(NEWLINE);
      #endif
      SendCommand(F("ATE0"));
      //SendCommand(F("AT+CIOBAUD=57600")); // переводим на другую скорость
      }
      break;

      case wfaCWMODE:
      {
        // переходим в смешанный режим
      #ifdef WIFI_DEBUG
       WIFI_DEBUG_WRITE(F("Go to SoftAP mode..."));WIFI_DEBUG_WRITE(NEWLINE);
      #endif
      SendCommand(F("AT+CWMODE=3"));
      }
      break;

      case wfaCWSAP: // создаём точку доступа
      {

      #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Creating the access point..."));WIFI_DEBUG_WRITE(NEWLINE);
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
        WIFI_DEBUG_WRITE(F("Set the TCP server mode to 0..."));WIFI_DEBUG_WRITE(NEWLINE);
      #endif
      SendCommand(F("AT+CIPMODE=0"));
      
      }
      break;

      case wfaCIPMUX: // разрешаем множественные подключения
      {
      #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Allow the multiple connections..."));WIFI_DEBUG_WRITE(NEWLINE);
      #endif
      SendCommand(F("AT+CIPMUX=1"));
        
      }
      break;

      case wfaCIPSERVER: // запускаем сервер
      {  
      #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Starting TCP-server..."));WIFI_DEBUG_WRITE(NEWLINE);
      #endif
      SendCommand(F("AT+CIPSERVER=1,80"));
      
      }
      break;

      case wfaCWQAP: // отсоединяемся от точки доступа
      {  
      #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Disconnect from router..."));WIFI_DEBUG_WRITE(NEWLINE);
      #endif
      SendCommand(F("AT+CWQAP"));
      
      }
      break;

      case wfaCWJAP: // коннектимся к роутеру
      {
      #ifdef WIFI_DEBUG
        WIFI_DEBUG_WRITE(F("Connecting to the router..."));WIFI_DEBUG_WRITE(NEWLINE);
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
       //  WIFI_DEBUG_WRITE(F("ASSERT: wfaCIPSEND in ProcessQueue!"));WIFI_DEBUG_WRITE(NEWLINE);
        #endif
        
      }
      break;

      case wfaACTUALSEND: // дождались приглашения, отсылаем данные клиенту
      {
            #ifdef WIFI_DEBUG
              WIFI_DEBUG_WRITE(F("Sending data to the client..."));WIFI_DEBUG_WRITE(NEWLINE);
            #endif
      
            if(clients[currentClientIDX].IsConnected()) // не отвалился ли клиент?
            {
              // клиент по-прежнему законнекчен, посылаем данные
              if(!clients[currentClientIDX].SendPacket(&(WIFI_SERIAL)))
              {
              #ifdef WIFI_DEBUG
              WIFI_DEBUG_WRITE(F("All data to the client has sent!"));WIFI_DEBUG_WRITE(NEWLINE);
            #endif
 
              }
              else
              {
                // ещё есть пакеты, продолжаем отправлять в следующих вызовах Update
              } // else
            } // is connected
            else
            {
              // клиент отвалится, чистим...
              actionsQueue.pop(); // убираем wfaACTUALSEND
              clients[currentClientIDX].Clear();
              currentAction = wfaIdle; // разрешаем обработку следующей команды
            }

      }
      break;

      case wfaCIPCLOSE: // закрываем соединение с клиентом
      {
        if(clients[currentClientIDX].IsConnected()) // только если клиент законнекчен 
        {
          #ifdef WIFI_DEBUG
            WIFI_DEBUG_WRITE(F("Closing client connection..."));WIFI_DEBUG_WRITE(NEWLINE);
          #endif
          clients[currentClientIDX].SetConnected(false);
          String command = F("AT+CIPCLOSE=");
          command += currentClientIDX; // закрываем соединение
          SendCommand(command);
        }
        else
        {
          // просто убираем команду из очереди
           actionsQueue.pop();
           currentAction = wfaIdle; // разрешаем обработку следующей команды
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
  if(currentAction != wfaIdle) // чем-то заняты, не можем ничего делать
    return;
    
  // тут ищем, какой клиент сейчас хочет отослать данные

  for(uint8_t idx = nextClientIDX;idx < MAX_WIFI_CLIENTS; idx++,nextClientIDX++)
  { 
    if(clients[idx].IsConnected() && clients[idx].HasPacket())
    {
      currentAction = wfaCIPSEND; // говорим однозначно, что нам надо дождаться >
      actionsQueue.push_back(wfaCIPSEND); // добавляем команду отсылки данных в очередь
      
    #ifdef WIFI_DEBUG
      WIFI_DEBUG_WRITE(F("Sending data command to the ESP..."));WIFI_DEBUG_WRITE(NEWLINE);
    #endif
  
      // клиент подсоединён и ждёт данных от нас - отсылаем ему следующий пакет
      currentClientIDX = idx; // сохраняем номер клиента, которому будем посылать данные
      String command = F("AT+CIPSEND=");
      command += String(idx);
      command += F(",");
      command += String(clients[idx].GetPacketLength());
      WaitForDataWelcome = true; // выставляем флаг, что мы ждём >

      SendCommand(command);
  
      break; // выходим из цикла
    } // if
    
  } // for
  
  if(nextClientIDX >= MAX_WIFI_CLIENTS)
    nextClientIDX = 0;  
}
void WiFiModule::Update(uint16_t dt)
{ 
  UNUSED(dt);
  
  ProcessQueue();
  UpdateClients();

}
bool  WiFiModule::ExecCommand(const Command& command)
{
  ModuleController* c = GetController();
  String answer; answer.reserve(RESERVE_STR_LENGTH);
  answer = NOT_SUPPORTED;
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
         WIFI_DEBUG_WRITE("Request for IP info...");WIFI_DEBUG_WRITE(NEWLINE);
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
                      WIFI_DEBUG_WRITE(F("AP IP found, parse..."));WIFI_DEBUG_WRITE(NEWLINE);
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
                      WIFI_DEBUG_WRITE(F("STA IP found, parse..."));WIFI_DEBUG_WRITE(NEWLINE);
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
          WIFI_DEBUG_WRITE(F("IP info requested."));WIFI_DEBUG_WRITE(NEWLINE);
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

