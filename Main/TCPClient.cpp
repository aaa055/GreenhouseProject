#include "TCPClient.h"
#include "InteropStream.h"

// CLIENT IMPLEMENTATION
#define TCP_WRITE_TO_FILE(dt) workFile.write((const uint8_t*) dt.c_str(), dt.length())

TCPClient::TCPClient()
{
  isConnected = false;
  commandHolder = F("");
  hasFullCommand = false; 
  Clear();
}
TCPClient::~TCPClient()
{
}
void TCPClient::SetConnected(bool c) 
{
  isConnected = c;
  
}
void TCPClient::Clear()
{
  nextPacketLength = 0;
  packetsCount = 0;
  contentLength = 0;
  packetsLeft = 0;
  packetsSent = 0;
  sentContentLength = 0;
  cachedData = F("");
}
void TCPClient::Update()
{
  if(hasFullCommand)
  {
    // есть полная команда, надо её обработать
    Prepare(commandHolder.c_str()); 
    commandHolder = F(""); // подготавливаем команду

    hasFullCommand = false; // сбрасываем флаг наличия полной команды
  }
}
void TCPClient::CommandRequested(int dataLen, const char* command)
{
  if(hasFullCommand) // игнорируем новую команду, т.к. предыдущая ещё не обработана
    return;
    
  int ln = 0;
  while(ln < dataLen)
  {
    if(*command == '\r') // если прямо в пакете нашли \r - значит, команда получена полностью, иначе - будем ждать следующего пакета
    {
      hasFullCommand = true; // выставляем флаг, что мы получили полную команду, и выходим
      break;
    }
    commandHolder += *command; // складываем байтики во внутренний буфер
    ln++;
    command++;
  }
}

bool TCPClient::Prepare(const char* command)
{
 // проверяем, не пустая ли строка и не имеем ли мы пакеты для отсылки
 if(!command || *command =='\r' || *command == '\n' || *command == '\0' || HasPacket())
  return false;
  
  Clear(); // очищаем переменные и т.п.

  CloseSDFile(); // закрываем файл

  CommandParser* cParser = MainController->GetCommandParser();

 COMMAND_TYPE cType = ctUNKNOWN;
 
 const char* strGET = strstr_P(command,(const char*) F("CTGET="));
 const char* strSET = strstr_P(command,(const char*) F("CTSET="));
 
 if(strGET == command)
  cType = ctGET;
 else
   if(strSET == command)
      cType = ctSET;

 if(cType != ctUNKNOWN) // надо получить данные с контроллера
 {   
  // пишем в промежуточный файл, поскольку не знаем - какой длины данные выплюнет модуль в ответе.
  // у нас асинхронная посылка данных, поэтому надо быть уверенным, что данные всегда доступны.
  
   Command cmd;
   if(cParser->ParseCommand(command, cmd))
   {
     
     // команду разобрали, надо назначить поток вывода в неё
     cmd.SetIncomingStream(this);
     // и просим контроллер выполнить эту команду
     MainController->ProcessModuleCommand(cmd);

     // в файл всё записано на этом этапе
   }
   else // не удалось распарсить, пишем в файл ошибку
    WriteErrorToFile();

 } // if
 else
 {
   // неизвестная команда
   // пишем в файл ошибку
   WriteErrorToFile();
 }

   // теперь считаем длину данных
  contentLength = cachedData.length();

  if(workFile)
  {
    // прибавляем длину данных, которые надо передать
    contentLength += workFile.size();
    // переходим на начало файла
    workFile.seek(0);
  }
  
 // вычисляем кол-во пакетов, которые нам надо послать
 if(contentLength < MAX_PACKET_LENGTH)
  packetsCount = 1;
 else
 {
  packetsCount = contentLength/MAX_PACKET_LENGTH;
  if((contentLength > MAX_PACKET_LENGTH) && (contentLength % MAX_PACKET_LENGTH))
    packetsCount++;
 }

 if(!cachedData.length()) // что-то не срослось - нет кешированных данных
 {
  contentLength = 0;
  packetsCount = 0;
 }

 // говорим, что мы не отослали ещё ни одного пакета
 packetsLeft = packetsCount;

 // вычисляем длину пакета для пересылки
 nextPacketLength = MAX_PACKET_LENGTH;
 if(contentLength < MAX_PACKET_LENGTH)
  nextPacketLength = contentLength;

  return HasPacket();
 
}
void TCPClient::OpenSDFile()
{
  if(workFile)
    return;

  char file_name[13];
  sprintf_P(file_name,(const char*) F("%u.TCP"),tcpClientID);
  // открываем файл на запись
  workFile = SD.open(file_name, FILE_WRITE | O_TRUNC); // открываем файл и усекаем его до нуля   
    
}
void TCPClient::WriteErrorToFile()
{
 if(cachedData.length() < CACHE_LENGTH)
 {
   // можем записать в кеш
   cachedData = ERR_ANSWER;
   cachedData += COMMAND_DELIMITER;
   cachedData += UNKNOWN_COMMAND;
   cachedData += NEWLINE;
 }
 else
 {
  OpenSDFile();
  
   if(workFile)
   {     
     String b = ERR_ANSWER; TCP_WRITE_TO_FILE(b);
     b = COMMAND_DELIMITER; TCP_WRITE_TO_FILE(b);
     b = UNKNOWN_COMMAND; TCP_WRITE_TO_FILE(b);
     b = NEWLINE; TCP_WRITE_TO_FILE(b);
   }
 } // else  
}
void TCPClient::CloseSDFile()
{
  if(workFile)
    workFile.close(); // закрываем файл
}
size_t TCPClient::write(uint8_t toWr)
{
 // чтение ответов от модулей с кешированием первых N байт

 if(cachedData.length() < CACHE_LENGTH) // ещё можно писать в кеш
 {
  cachedData += (char) toWr;
  return 1;
 }

 OpenSDFile();
 
 if(workFile)
  workFile.write(toWr);

  return 1;
   
}
bool TCPClient::SendPacket(Stream* s)
{

 if(!packetsLeft) // нечего больше отсылать
 {
  CloseSDFile();
  return false;
 }
  
  // тут отсылаем пакет
  // возвращаем false, если больше нечего посылать

  uint16_t cachedDataLen = cachedData.length();
  if(cachedDataLen)
  {
    // ещё читаем из кешированных данных, надо послать либо все даннные, либо их часть, при этом дочитать остаток из файла
    if(cachedDataLen >= nextPacketLength)
    {
       // длина оставшихся к отсылу данных больше, чем размер одного пакета.
       // поэтому можем отсылать пакет целиком, предварительно его сформировав.
     //  String str = cachedData.substring(0,nextPacketLength);
       s->write(cachedData.c_str(),nextPacketLength); // пишем данные в поток
       cachedData = cachedData.substring(nextPacketLength);
       
    }
    else
    {
      // длина оставшихся данных меньше, чем длина следующего пакета, поэтому нам надо дочитать
      // необходимое кол-во байт из данных.
      uint16_t dataLeft = nextPacketLength - cachedData.length();

      s->write(cachedData.c_str(),cachedData.length()); // пишем данные в поток
      cachedData = F("");

     // тут вычитываем данные из файла, длиной dataLeft
        if(workFile) // если файл открыт
        {
        // Блочное чтение из файла в нашем случае показало себя медленней, чем побайтовое (WTF???)            
          for(uint16_t i=0;i<dataLeft;i++)
            s->write(workFile.read()); // пишем данные в поток
        }

    } // else
     
    
  } // if(cachedDataLen)
  else
  {
    // уже только работа с файлом
    if(workFile)
    {
      // у нас идёт работа только с данными, полученными с контроллера
      for(uint16_t i=0;i<nextPacketLength;i++) // читаем данные из файла и выдаём их в поток
        s->write(workFile.read());
    }
  } // else
  
  
 // вычисляем, сколько осталось пакетов
 packetsLeft--;
 packetsSent++;
 sentContentLength += nextPacketLength;

 // вычисляем длину следующего пакета
 if((contentLength - sentContentLength) >= MAX_PACKET_LENGTH)
   nextPacketLength = MAX_PACKET_LENGTH;
 else
  nextPacketLength = (contentLength - sentContentLength);

  if(!packetsLeft) // пакеты закончились
    CloseSDFile(); // закрываем и удаляем файл
  
  return (packetsLeft > 0); // если ещё есть пакеты - продолжаем отсылать
}
uint16_t TCPClient::GetPacketLength()
{
  return nextPacketLength;
}


