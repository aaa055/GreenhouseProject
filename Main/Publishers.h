#ifndef _PUBLISHERS_H
#define _PUBLISHERS_H
#include <WString.h>
#include "CommandParser.h"
struct PublishStruct; // forward declaration
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
