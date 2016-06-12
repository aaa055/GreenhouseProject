#ifndef _WIFI_MODULE_H
#define _WIFI_MODULE_H

#include "AbstractModule.h"
#include "TinyVector.h"
#include "Settings.h"
#include "TCPClient.h"

#define MAX_WIFI_CLIENTS 4 // максимальное кол-во клиентов
#define WIFI_PACKET_LENGTH 2048 // по скольку байт в пакете отсылать данные


typedef enum
{
  /*0*/  wfaIdle, // пустое состояние
  /*1*/  wfaWantReady, // надо получить ready от модуля
  /*2*/  wfaEchoOff, // выключаем эхо
  /*3*/  wfaCWMODE, // переводим в смешанный режим
  /*4*/  wfaCWSAP, // создаём точку доступа
  /*5*/  wfaCWJAP, // коннектимся к роутеру
  /*6*/  wfaCWQAP, // отсоединяемся от роутера
  /*7*/  wfaCIPMODE, // устанавливаем режим работы
  /*8*/  wfaCIPMUX, // разрешаем множественные подключения
  /*9*/  wfaCIPSERVER, // запускаем сервер
  /*10*/ wfaCIPSEND, // отсылаем команду на передачу данных
  /*11*/ wfaACTUALSEND, // отсылаем данные
  /*12*/ wfaCIPCLOSE // закрываем соединение
  
} WIFIActions;

typedef Vector<WIFIActions> ActionsVector;

class WiFiModule : public AbstractModule // модуль поддержки WI-FI
{
  private:

    GlobalSettings* Settings;

    volatile bool inSendData;

    bool IsKnownAnswer(const String& line); // если ответ нам известный, то возвращает true
    void SendCommand(const String& command, bool addNewLine=true); // посылает команды модулю вай-фай
    void ProcessQueue(); // разбираем очередь команд
    void ProcessQuery(const String& command); // обрабатываем запрос
    void ProcessCommand(int clientID, int dataLen,const char* command);
    void UpdateClients();
    
    uint8_t currentAction; // текущая операция, завершения которой мы ждём
    ActionsVector actionsQueue; // что надо сделать, шаг за шагом 
    
    uint8_t currentClientIDX; // индекс клиента, с которым мы работаем сейчас
    uint8_t nextClientIDX; // индекс клиента, статус которого надо проверить в следующий раз

    // список клиентов
    TCPClient clients[MAX_WIFI_CLIENTS];
    
  
  public:
    WiFiModule() : AbstractModule("WIFI") {}

    bool ExecCommand(const Command& command, bool wantAnswer);
    void Setup();
    void Update(uint16_t dt);

    void ProcessAnswerLine(const String& line);
    volatile bool WaitForDataWelcome; // флаг, что мы ждём приглашения на отсыл данных - > (плохое ООП, негодное :) )

};


#endif
