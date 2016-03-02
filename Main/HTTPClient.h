#ifndef _HTTP_CLIENT_H
#define _HTTP_CLIENT_H
#include <SD.h>
#include "Globals.h"
#include "ModuleController.h"

// список заголовков, с которыми мы работаем
#define H_HTTP_STATUS F("HTTP/1.1 ") // статус ответа
#define H_CONTENT_TYPE F("Content-Type: ") // тип данных
#define H_CONNECTION F("Connection: close") // закрывать соединение
#define H_CONTENT_LENGTH F("Content-Length: ") // длина данных
#define H_CORS_HEADER F("Access-Control-Allow-Origin: *") // разрешаем кроссдоменные запросы

#define STATUS_200 F("200 OK") // всё ок, дальше идут данные
#define STATUS_404 F("404 Not Found") // не найдено

#define DEF_PAGE F("index.htm") // имя страницы по умолчанию
#define STATUS_404_TEXT F("<html><head><title>Not Found</title></head><body><h1>Not found</h1></body></html>")

// структура запроса HTTP, на будущее
typedef struct
{
  String URI; // URI, которое запросили
  // тут будут появляться другие заголовки
} HTTPQuery; 

// класс обработки HTTP-запроса от сервера. Подготавливает данные, формирует заголовки ответа,
// настраивает кол-во пакетов для отсылки, отсылает очередной пакет по приглашению.
// ничего не знает о статусах соединения, надо дёргать SetConnected вручную.
// был вынесен в отдельный файл на будущее, когда придётся работать с Ethernet Shield,
// чтобы не переписывать по два раза одно и то же.
class HTTPClient 
{
  private:

    ModuleController* controller; // контроллер, который всем рулит и предоставляет нам различные интерфейсы
    uint16_t MAX_PACKET_LENGTH; // максимальная длина пакета

    File workFile; // файл, с которым работаем
    bool isFileOpen; // флаг открытия файла
    void EnsureCloseFile(); // закрываем файл
  
    bool isConnected; // флаг, что клиент подсоединён

    uint16_t nextPacketLength; // длина следующего пакета данных
    uint16_t packetsCount; // сколько всего пакетов отослать
    uint16_t packetsLeft; // сколько пакетов осталось отослать
    uint16_t packetsSent; // сколько пакетов отослали
    unsigned long contentLength; // длина контента, которую надо отослать
    unsigned long sentContentLength; // какую общую длину уже отослали
    

    String httpHeaders; // заголовки, которые будем отсылать
    
    String GetContentType(const String& uri); // возвращаем тип данных
    bool PrepareFile(bool SDAvailable, const String& fileName,unsigned long& fileSize); // подготавливаем файл с карточки
 

    String dataToSend; // данные, которые надо переслать по запросу со скрипта
    bool ajaxQueryFound; // флаг, что мы нашли запрос AJAX

    
  public:

    // настраиваем клиента, передаём ему ссылку на контроллер и максимальную длину пакета, которую можно отослать за раз
    void Setup(ModuleController* c, uint16_t maxPacketLength) {controller = c; MAX_PACKET_LENGTH = maxPacketLength;}

    void Clear(); // очищаем все данные, закрываем открытые файлы и т.д.

    bool IsConnected(){return isConnected;} // клиент законнекчен?
    void SetConnected(bool c); // устанавливаем флаг соединения клиента

    bool HasPacket() {return (packetsLeft > 0);} // есть ли ещё пакеты для отправки?
    bool Prepare(bool SDAvailable, const HTTPQuery& query); // подготавливаем данные для отправки (первый параметр - флаг успешной инициализации SD-модуля)
    

    bool SendPacket(Stream* s); // отсылает пакет

    uint16_t GetTotalPacketsCount() {return packetsCount; } // возвращает кол-во пакетов, которые надо отослать
    uint16_t GetPacketLength(); // возвращает длину пакета, который надо переслать в следующей итерации
    unsigned long GetContentLength() {return contentLength;} // возвращает длину всех данных, которые надо отослать
    uint16_t GetPacketsLeft() {return packetsLeft;} // возвращает кол-во пакетов, которые осталось отправить

    HTTPClient();
    ~HTTPClient();
  
}; 

// URL Utils
String UrlDecode(const String& uri);  // декодирует URI-строку

#endif
