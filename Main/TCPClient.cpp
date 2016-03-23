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

  RemoveSDFile(); // удаляем файл с диска

   // создаём файл на карточке для записи туда данных
   String fname = String(tcpClientID); fname += F(".TCP");

   // открываем файл на запись
   workFile = SD.open(fname.c_str(), FILE_WRITE | O_TRUNC); // усекаем файл до нуля

  CommandParser* cParser = controller->GetCommandParser();

 COMMAND_TYPE cType = ctUNKNOWN;

 static String _ctget = F("CTGET=");
 static String _ctset = F("CTSET=");
 
 const char* strGET = strstr(command,_ctget.c_str());
 const char* strSET = strstr(command,_ctset.c_str());
 
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
   if(cParser->ParseCommand(command, OUR_ID, cmd))
   {
     
     // команду разобрали, надо назначить поток вывода в неё
     cmd.SetIncomingStream(this);
     // и просим контроллер выполнить эту команду
     controller->ProcessModuleCommand(cmd);

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

  if(workFile)
  {
   // теперь считаем длину данных
    // выставляем длину данных, которые надо передать
    contentLength = workFile.size();
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

 if(!workFile) // что-то с файлом не срослось
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
void TCPClient::WriteErrorToFile()
{
   if(workFile)
   {     
     String b = ERR_ANSWER; TCP_WRITE_TO_FILE(b);
     b = COMMAND_DELIMITER; TCP_WRITE_TO_FILE(b);
     b = UNKNOWN_COMMAND; TCP_WRITE_TO_FILE(b);
     b = NEWLINE; TCP_WRITE_TO_FILE(b);
   }  
}
void TCPClient::RemoveSDFile()
{
  if(workFile)
  {
    workFile.close(); // закрываем файл
   // SD.remove(workFile.name()); // удаляем файл с карточки
  } 
}
size_t TCPClient::write(uint8_t toWr)
{
 // тут пишем в промежуточный файл
 if(workFile)
  workFile.write(toWr);

  return 1;
   
}
bool TCPClient::SendPacket(Stream* s)
{

 if(!packetsLeft) // нечего больше отсылать
 {
  RemoveSDFile();
  return false;
 }
  
  // тут отсылаем пакет
  // возвращаем false, если больше нечего посылать
  if(workFile)
  {
    // у нас идёт работа только с данными, полученными с контроллера
    for(uint16_t i=0;i<nextPacketLength;i++) // читаем данные из файла и выдаём их в поток
      s->write(workFile.read());
  }
  
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
    RemoveSDFile(); // закрываем и удаляем файл
  
  return (packetsLeft > 0); // если ещё есть пакеты - продолжаем отсылать
}
uint16_t TCPClient::GetPacketLength()
{
  return nextPacketLength;
}


