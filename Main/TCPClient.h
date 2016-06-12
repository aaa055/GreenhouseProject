#ifndef _TCP_CLIENT_H
#define _TCP_CLIENT_H
#include "Globals.h"
#include "ModuleController.h"
#include <SD.h>
#include <Arduino.h>

// класс обработки запроса, посланного по TCP/IP на ESP8266. Подготавливает данные,
// настраивает кол-во пакетов для отсылки, отсылает очередной пакет по приглашению.
// ничего не знает о статусах соединения, надо дёргать SetConnected вручную.
// Предназначен для взаимодействия с ESP8266, поскольку из-за особенностей
// общения с этой железкой имеем ограничение на отсыл 2048 байт за раз,
// не более. Соответственно, есть необходимость разбивать ответ контроллера
// на пакеты - в случае с отсылом лог-файла это актуально.
// Именно поэтому пришлось делать отдельный класс, который, помимо всего прочего,
// складывает ответ с контроллера в промежуточный файл на SD, если ответ
// не помещается в CACHE_LENGTH. Такие вот пляски.

#define CACHE_LENGTH 256 // сколько байт кешировать в ответе

class TCPClient : public Stream
{
  private:

    uint16_t MAX_PACKET_LENGTH; // максимальная длина пакета
    
    bool isConnected; // флаг, что клиент подсоединён

    uint16_t nextPacketLength; // длина следующего пакета данных
    uint16_t packetsCount; // сколько всего пакетов отослать
    uint16_t packetsLeft; // сколько пакетов осталось отослать
    uint16_t packetsSent; // сколько пакетов отослали
    
    unsigned long contentLength; // длина контента, которую надо отослать
    unsigned long sentContentLength; // какую общую длину уже отослали

    File workFile; // файл, в который мы будем складывать ответы от модулей
    uint8_t tcpClientID; // ID клиента
    String cachedData; // данные, которые будем кешировать для отсыла

    String commandHolder; // сюда складываем команду
    bool hasFullCommand; // флаг, что приняли всю команду
    bool Prepare(const char* command); // подготавливаем данные для отправки

    void OpenSDFile();
    void CloseSDFile();
    void WriteErrorToFile();
    void Clear(); // очищаем все данные

    
    
  public:

    // настраиваем клиента, передаём ему его ID, ссылку на контроллер и максимальную длину пакета, которую можно отослать за раз
    void Setup(uint8_t clientID, uint16_t maxPacketLength) {tcpClientID = clientID; MAX_PACKET_LENGTH = maxPacketLength;}

    // обновляем внутреннее состояние клиента в вызове Update модуля, в котором работает клиент. Как только клиент получит полный пакет данных - он
    // обработает команду, сложит весь ответ в промежуточный файл и будет готов к передаче (HasPacket будет возвращать true).
    void Update(); 


    bool IsConnected(){return isConnected;} // клиент законнекчен?
    void SetConnected(bool c); // устанавливаем флаг соединения клиента

    bool HasPacket() {return (packetsLeft > 0);} // есть ли ещё пакеты для отправки?

    // вызываем по приходу данных для клиента, концом команды считается \r\n. Клиент ничего не делает в этом методе,
    // поскольку посылка ответа может быть асинхронной. Данные подготавливаются в методе Update, который вызывается тогда,
    // когда входящие из порта данные уже обработаны. Если клиенту придёт новая команда до тех пор, пока не подготовлены
    // данные предыдущей - этот метод ничего не сделает.
    void CommandRequested(int dataLen, const char* command); // складываем данные в буфер команды до тех пор, пока не придёт \r\n в пакете данных

    bool SendPacket(Stream* s); // отсылает очередной пакет, если остались пакеты - возвращает true, иначе - false

    uint16_t GetTotalPacketsCount() {return packetsCount; } // возвращает кол-во пакетов, которые надо отослать
    uint16_t GetPacketLength(); // возвращает длину пакета, который надо переслать в следующей итерации
    unsigned long GetContentLength() {return contentLength;} // возвращает длину всех данных, которые надо отослать
    uint16_t GetPacketsLeft() {return packetsLeft;} // возвращает кол-во пакетов, которые осталось отправить


    virtual int available(){ return false; };
    virtual int read(){ return -1;};
    virtual int peek(){return -1;};
    virtual void flush(){};

 
    virtual size_t write(uint8_t toWr);  

    TCPClient();
    ~TCPClient();
  
}; 

#endif
