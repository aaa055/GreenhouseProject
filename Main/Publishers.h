#ifndef _PUBLISHERS_H
#define _PUBLISHERS_H
#include <WString.h>
#include "CommandParser.h"
class AbstractModule;

// структура для публикации
struct PublishStruct
{
  const Command* SourceCommand; // исходная команда, ответ на которую надо публиковать
  AbstractModule* Module; // модуль, который опубликовал ответ или событие
  bool Status; // Статус ответа на запрос: false - ошибка, true - нет ошибки
  String Text; // текстовое сообщение о публикации
  bool AddModuleIDToAnswer; // добавлять ли имя модуля в ответ?
  void* Data; // любая информация, в зависимости от типа модуля
  
  

};

// абстрактный класс для публикации изменений (вывода их на экран, в сериал и т.п.)
class AbstractPublisher
{
  public:
    AbstractPublisher() {}
    virtual bool Publish(const PublishStruct* toPublish, Stream* commandStream) = 0;
};


class SerialPublisher : public AbstractPublisher
{
  public:
    SerialPublisher() {};
    virtual bool Publish(const PublishStruct* toPublish, Stream* commandStream);
};

class DisplayPublisher : public AbstractPublisher
{
  public:
    DisplayPublisher() {};
    virtual bool Publish(const PublishStruct* toPublish, Stream* commandStream);
};

#endif
