#include "HTTPClient.h"
#include "InteropStream.h"
// URL UTILS
String UrlDecode(const String& uri) // декодирует URI-строку
{
  String result;
  int len = uri.length();
  result.reserve(len);
  int s = 0;

 while (s < len) 
 {
    char c = uri[s++];

    if (c == '%' && s + 2 < len) 
    {
        char c2 = uri[s++];
        char c3 = uri[s++];
        if (isxdigit(c2) && isxdigit(c3)) 
        {
            c2 = tolower(c2);
            c3 = tolower(c3);

            if (c2 <= '9')
                c2 = c2 - '0';
            else
                c2 = c2 - 'a' + 10;

            if (c3 <= '9')
                c3 = c3 - '0';
            else
                c3 = c3 - 'a' + 10;

            result  += char(16 * c2 + c3);

        } 
        else 
        { 
            result += c;
            result += c2;
            result += c3;
        }
    } 
    else 
    if (c == '+') 
        result += ' ';
    else 
        result += c;
 } // while 

  return result;
}

// CLIENT IMPLEMENTATION
HTTPClient::HTTPClient()
{
  isConnected = false;
  isFileOpen = false;
  Clear();
}
HTTPClient::~HTTPClient()
{
   Clear();
}
void HTTPClient::SetConnected(bool c) 
{
  isConnected = c;
  
  if(!c) // ничего не будем отсылать, т.к. клиент отсоединился
    Clear(); // очищаем все статусы
}
String HTTPClient::GetContentType(const String& uri)
{
  String s = uri;
  s.toLowerCase();

  // у нас имена файлов формата 8.3, поэтому смысла проверять расширения больше трёх символов - нет
  if(s.endsWith(F(".htm")))
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
  if(s.endsWith(F(".jpg")))
    return F("image/jpeg");
  else
  if(s.endsWith(F(".css")))
    return F("text/css");
  else
  if(s.endsWith(F(".ico")))
    return F("image/vnd.microsoft.icon");//F("image/x-icon");

  return F("text/plain");  
}
void HTTPClient::EnsureCloseFile()
{
  if(isFileOpen)
    workFile.close(); // закрываем файл, с которым работали
    
  isFileOpen = false;  
}
void HTTPClient::Clear()
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

  EnsureCloseFile();
  
}
bool HTTPClient::PrepareFile(bool SDAvailable, const String& fileName,unsigned long& fileSize)
{
  EnsureCloseFile(); // сперва закрываем файл
  
  fileSize = 0;

  if(!SDAvailable) // не можем работать с SD-модулем, поэтому мимо кассы
    return false; 

  if(!SD.exists(fileName)) // файл не найден
    return false;


  workFile = SD.open(fileName);
  if(workFile)
  {
      isFileOpen = true;

      if(workFile.isDirectory()) // запрещаем доступ к папкам
      {
        EnsureCloseFile();
        return false;
      }

    // файл открыли, подготовили, получили размер
    fileSize = workFile.size();
    return true;
  }

  return false;
}
bool HTTPClient::Prepare(bool SDAvailable, const HTTPQuery& query)
{
  Clear(); // закрываем файлы, очищаем переменные и т.п.
  

 COMMAND_TYPE cType = ctUNKNOWN;
 String uriRequested = UrlDecode(query.URI); // декодируем URI
 
 if(uriRequested.startsWith(F("CTGET=")))
  cType = ctGET;
 else
   if(uriRequested.startsWith(F("CTSET=")))
  cType = ctSET;

 if(cType != ctUNKNOWN) // надо получить данные с контроллера
 {
    ajaxQueryFound = true;
     
    //String command = uriRequested.substring(6);
    if(ModuleInterop.QueryCommand(cType,uriRequested.substring(6),false))
    {
      // можем формировать AJAX-ответ
      dataToSend = F("{");
      dataToSend += F("\"query\": \"");
      dataToSend += uriRequested;
      dataToSend += F("\",\"answer\": \"");
      String dt = ModuleInterop.GetData();         
      dt.trim(); // убираем перевод строки в конце
      dataToSend += dt;
      dataToSend += F("\"};");      
    } // if
 } // if

  // проверяем, есть ли файл на диске, например
  bool dataFound = true;
  
 if(!ajaxQueryFound)
  {
   // подготавливаем файл для отправки
   dataFound = PrepareFile(SDAvailable,uriRequested,contentLength);
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

  httpHeaders += H_CONNECTION;
  httpHeaders += NEWLINE;

  // разрешаем кроссдоменные запросы
  httpHeaders += H_CORS_HEADER;
  httpHeaders += NEWLINE;

  // подставляем тип данных
  httpHeaders += H_CONTENT_TYPE;
  if(ajaxQueryFound)
    httpHeaders += F("application/json"); // устанавливаем нужный Content-Type для JSON-ответа
  else
  {
    if(dataFound)
      httpHeaders += GetContentType(uriRequested);
    else
      httpHeaders += F("text/html"); // 404 as HTML
  }
    
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
  // будет переслать по вызовам SendPacket
  contentLength +=  httpHeaders.length();
 
 // вычисляем кол-во пакетов, которые нам надо послать
 if(contentLength < MAX_PACKET_LENGTH)
  packetsCount = 1;
 else
 {
  packetsCount = contentLength/MAX_PACKET_LENGTH;
  if((contentLength > MAX_PACKET_LENGTH) && (contentLength % MAX_PACKET_LENGTH))
    packetsCount++;
 }

 // говорим, что мы не отослали ещё ни одного пакета
 packetsLeft = packetsCount;

 // вычисляем длину пакета для пересылки
 nextPacketLength = MAX_PACKET_LENGTH;
 if(contentLength < MAX_PACKET_LENGTH)
  nextPacketLength = contentLength;

  return HasPacket();
}
bool HTTPClient::SendPacket(Stream* s)
{

 if(!packetsLeft) // нечего больше отсылать
  return false;
  
  // тут отсылаем пакет
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
       // поэтому можем отсылать пакет целиком, предварительно его сформировав.
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
        }
      }
    } // else
   
  } // if
  else
  {
    // у нас идёт работа с данными уже
    if(dataToSend.length())
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
      }
    }
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
  
  return (packetsLeft > 0); // если ещё есть пакеты - продолжаем отсылать
}
uint16_t HTTPClient::GetPacketLength()
{
  return nextPacketLength;
}


