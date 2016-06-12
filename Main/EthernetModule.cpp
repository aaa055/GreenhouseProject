#include "EthernetModule.h"
#include "ModuleController.h"
#include <Ethernet.h>

// наш локальный мак-адрес
byte local_mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// IP-адрес по умолчанию
IPAddress default_ip(192, 168, 0, 177);

// наш сервер, который будет обработывать клиентов
EthernetServer lanServer(1975);

void EthernetModule::Setup()
{
  // настраиваем всё необходимое добро тут
  bInited = false;
  
}

void EthernetModule::Update(uint16_t dt)
{ 
  UNUSED(dt);
  // обновление модуля тут

  if(!bInited) // не было инициализации, инициализируемся
  {

    // пытаемся по DHCP получить конфигурацию
    if(!Ethernet.begin(local_mac))
    {
      // стартуем пока с настройками по умолчанию
      Ethernet.begin(local_mac, default_ip);
    }
    
    lanServer.begin();    

  #ifdef ETHERNET_DEBUG
    Serial.print(F("[LAN] server started at "));
    Serial.println(Ethernet.localIP());
  #endif

    bInited = true;
    return;
    
  } // if(!bInited)

  EthernetClient client = lanServer.available();
  if(client)
  {
    // есть активный клиент
    uint8_t sockNumber = client.getSocketNumber(); // получили номер сокета клиента

    while(client.available()) // пока есть данные с клиента
    {
      char c = client.read(); // читаем символ
      
      if(c == '\r') // этот символ нам не нужен, мы ждём '\n'
        continue;

      if(c == '\n') // дождались перевода строки
      {
        // пытаемся распарсить команду
        Command cmd;
        CommandParser* cParser = MainController->GetCommandParser();

        if(cParser->ParseCommand(clientCommands[sockNumber], cmd))
        {
          // команду разобрали, выполняем
          
          cmd.SetIncomingStream(&client); // назначаем команде поток, куда выводить данные

          // запустили команду в обработку
          MainController->ProcessModuleCommand(cmd);
        }

        // останавливаем клиента, т.к. все данные ему уже посланы.
        // даже если команда неправильная - считаем, что раз мы
        // получили строку, значит, имеем полное право с ней работать,
        // и каждый ССЗБ, если пришло что-то не то.
        client.stop();

        // очищаем внутренний буфер, подготавливая его к приёму следующей команды
        clientCommands[sockNumber] = F(""); 
        
        break; // выходим из цикла
        
      } // if(c == '\n')

      // если символ не '\r' и не '\n' -
      // запоминаем его во внутренний буфер, 
      // привязанный к номеру клиента
      clientCommands[sockNumber] += c; 
      
    } // while

    Ethernet.maintain(); // обновляем состояние Ethernet
    
  } // if(client)

}

bool EthernetModule::ExecCommand(const Command& command, bool wantAnswer)
{
  UNUSED(wantAnswer);
  UNUSED(command);

  return true;
}

