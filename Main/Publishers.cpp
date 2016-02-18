#include "Publishers.h"
#include "AbstractModule.h"
#include <Arduino.h>

bool SerialPublisher::Publish(const PublishStruct* toPublish, Stream* commandStream)
{

    // публикуем ответ в сериал
     Serial.print(toPublish->Status ? OK_ANSWER : ERR_ANSWER);
     Serial.print(COMMAND_DELIMITER);
     
     if(toPublish->AddModuleIDToAnswer)
     {
      Serial.print(toPublish->Module->GetID());
      Serial.print(PARAM_DELIMITER);
     }
     Serial.println(toPublish->Text);

    // проверяем, чего пришло в void* Data, для теста
     //int ls = (int)  toPublish->Data;
     //Serial.println("CURRENT LED STATE=" + String(ls));

    // ЕСЛИ МЫ ХОТИМ, ЧТОБЫ СООБЩЕНИЕ НЕ ВЫВОДИЛОСЬ В ПОТОК ПО УМОЛЧАНИЮ,
    // НАДО ВЕРНУТЬ TRUE.
     return commandStream == &Serial; // проверяем, вывели ли мы в тот же поток?
  
}

bool DisplayPublisher::Publish(const PublishStruct* toPublish, Stream* commandStream)
{
  // ЗДЕСЬ В ЗАВИСИМОСТИ ОТ ТИПА МОДУЛЯ МОЖНО ВЫВОДИТЬ ИНФОРМАЦИЮ В КАКУЮ-ЛИБО ЧАСТЬ ЭКРАНА!!!
  
    Serial.print("SCREEN OUTPUT=");
    Serial.println(toPublish->Text);
  
  return commandStream == &Serial; // проверяем, вывели ли мы в тот же поток?
}
