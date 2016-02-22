#ifndef _WIFI_MODULE_H
#define _WIFI_MODULE_H

#include "AbstractModule.h"
#include "TinyVector.h"
#include "Settings.h"

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
    void ProcessURIRequest();
    
    WIFIActions currentAction; // текущая операция, завершения которой мы ждём
    ActionsVector actionsQueue; // что надо сделать, шаг за шагом 
    
    String httpQuery;
    bool waitForQueryCompleted;
    String  connectedClientID;
    String requestedURI;
    unsigned long dataToSendLength;

    String TEMP_DATA_TO_SEND;
    
  
  public:
    WiFiModule() : AbstractModule(F("WIFI")) {}

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);

    void ProcessAnswerLine(const String& line);
    volatile bool WaitForDataWelcome; // флаг, что мы ждём приглашения на отсыл данных - > (плохое ООП, негодное :) )

};


#endif
