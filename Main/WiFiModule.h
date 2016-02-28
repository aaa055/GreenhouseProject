#ifndef _WIFI_MODULE_H
#define _WIFI_MODULE_H

#include "AbstractModule.h"
#include "TinyVector.h"
#include "Settings.h"
#include <SD.h>

#define MAX_WIFI_CLIENTS 4 // максимальное кол-во клиентов
#define WIFI_PACKET_LENGTH 512 // по скольку байт в пакете отсылать данные

// список заголовков, с которыми мы работаем
#define H_HTTP_STATUS F("HTTP/1.1 ") // статус ответа
#define H_CONTENT_TYPE F("Content-Type: ") // тип данных
#define H_CONNECTION F("Connection: close") // закрывать соединение
#define H_CONTENT_LENGTH F("Content-Length: ") // длина данных

#define STATUS_200 F("200 OK") // всё ок, дальше идут данные
#define STATUS_404 F("404 Not Found") // не найдено

#define DEF_PAGE F("index.htm") // имя страницы по умолчанию
#define STATUS_404_TEXT F("<html><head><title>Not Found</title></head><body><h1>Not found</h1></body></html>")

class WiFiModule; // forward declaration
class WIFIClient // класс обработки клиента
{
  private:

    ModuleController* controller;
    WiFiModule* parent;

    File workFile;
    bool isFileOpen;
    void EnsureCloseFile();
  
    bool isConnected; // флаг, что клиент подсоединён

    uint16_t nextPacketLength; // длина следующего пакета данных
    uint16_t packetsCount; // сколько всего пакетов отослать
    uint16_t packetsLeft; // сколько пакетов осталось отослать
    uint16_t packetsSent; // сколько пакетов отослали
    unsigned long contentLength; // длина контента, которую надо отослать
    unsigned long sentContentLength; // какую общую длину уже отослали
    

    String httpHeaders; // заголовки, которые будем отсылать
    String GetContentType(const String& uri);
    bool PrepareFile(const String& fileName,unsigned long& fileSize);
 

    String dataToSend; // данные, которые надо переслать по запросу со скрипта
    bool ajaxQueryFound; // флаг, что мы нашли запрос AJAX

    
  public:

    void Setup(ModuleController* c, WiFiModule* p) {controller = c; parent = p;}

    void Clear(); // очищаем все данные, закрываем открытые файлы и т.д.

    bool IsConnected(){return isConnected;} // клиент законнекчен?
    void SetConnected(bool c); // устанавливаем флаг соединения клиента

    bool HasPacket() {return (packetsLeft > 0);} // есть ли ещё пакеты для отправки?
    bool Prepare(const String& uriRequested); // подготавливаем данные для отправки
    

    bool SendPacket(Stream* s); // отсылает пакет по Wi-Fi

    uint16_t GetTotalPacketsCount() {return packetsCount; } // возвращает кол-во пакетов, которые надо отослать
    uint16_t GetPacketLength(); // возвращает длину пакета, который надо переслать в следующей итерации
    unsigned long GetContentLength() {return contentLength;} // возвращает длину всех данных, которые надо отослать

    WIFIClient();
  
}; 

typedef enum
{
  wfaIdle, // пустое состояние
  wfaWantReady, // надо получить ready от модуля
  wfaEchoOff, // выключаем эхо
  wfaCWMODE, // переводим в смешанный режим
  wfaCWSAP, // создаём точку доступа
  wfaCWJAP, // коннектимся к роутеру
  wfaCWQAP, // отсоединяемся от роутера
  wfaCIPMODE, // устанавливаем режим работы
  wfaCIPMUX, // разрешаем множественные подключения
  wfaCIPSERVER, // запускаем сервер
  wfaCIPSEND, // отсылаем команду на передачу данных
  wfaACTUALSEND, // отсылаем данные
  wfaCIPCLOSE // закрываем соединение
  
} WIFIActions;

typedef Vector<WIFIActions> ActionsVector;

class WiFiModule : public AbstractModule // модуль поддержки WI-FI
{
  private:

    GlobalSettings* Settings;

    bool IsKnownAnswer(const String& line); // если ответ нам известный, то возвращает true
    void SendCommand(const String& command, bool addNewLine=true); // посылает команды модулю вай-фай
    void ProcessQueue(); // разбираем очередь команд
    void ProcessQuery(); // обрабатываем запрос
    void ProcessURIRequest(int clientID, const String& requesterURI);

    void UpdateClients();
    
    WIFIActions currentAction; // текущая операция, завершения которой мы ждём
    ActionsVector actionsQueue; // что надо сделать, шаг за шагом 
    
    String httpQuery;
    bool waitForQueryCompleted;
    uint8_t currentClientIDX; // индекс клиента, с которым мы работаем сейчас
    uint8_t nextClientIDX; // индекс клиента, статус которого надо проверить в следующий раз

    // список клиентов
    WIFIClient clients[MAX_WIFI_CLIENTS];

    bool sdCardInited; // флаг инициализации SD-модуля
    
    
  
  public:
    WiFiModule() : AbstractModule(F("WIFI")) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);

    bool CanWorkWithSD() {return sdCardInited;}

    void ProcessAnswerLine(const String& line);
    volatile bool WaitForDataWelcome; // флаг, что мы ждём приглашения на отсыл данных - > (плохое ООП, негодное :) )

};


#endif
