#include "TempSensors.h"
#include "ModuleController.h"

void WindowState::Setup(ModuleState* state, uint8_t relay1, uint8_t relay2)
{
  RelayStateHolder = state;
  RelayChannel1 = relay1;
  RelayChannel2 = relay2;
}

bool WindowState::ChangePosition(DIRECTION dir, unsigned int newPos)
{
  bool bRet = false;
  
  if(IsBusy()) // заняты сменой позиции
    return bRet;

  if(CurrentPosition == newPos) // та же самая позиция запрошена, ничего не делаем
    return bRet;

  
  if(dir == dirOPEN)
  {
      if(newPos < CurrentPosition) // запросили открыть назад - невозможно.
      {
       //  do Nothing();
      }
      else
      {  
       // открываем тут
       TimerInterval = newPos - CurrentPosition;
       TimerTicks = 0;
       RequestedPosition = newPos;
       Direction = dir;
       OnMyWay = true; // поогнали!
       bRet = true;

      // Serial.println("OPEN FROM POSITION " + String(CurrentPosition) + " to " + String(newPos));
      }
  }
  else
  if(dir == dirCLOSE)
  {
      if(newPos > CurrentPosition) // запросили закрыть вперёд - невозможно
      {
       // do Nothing();
      }
      else
       {
        TimerInterval = CurrentPosition - newPos;
        TimerTicks = 0;
        RequestedPosition = newPos;
        Direction = dir;
        OnMyWay = true; // поогнали!
        bRet = true;

       // Serial.println("CLOSE FROM POSITION " + String(CurrentPosition) + " to " + String(newPos));
       }
  }
    return bRet;
}
void WindowState::SwitchRelays(uint16_t rel1State, uint16_t rel2State)
{
  //TODO: Здесь включаем реле, устанавливая на нужный пин переданное состояние!!
  
  if(RelayStateHolder) // сообщаем, что реле мы выключили или включили
  {
    RelayStateHolder->SetRelayState(RelayChannel1,rel1State == HIGH);
    RelayStateHolder->SetRelayState(RelayChannel2,rel2State == HIGH);
  } // if
  
}
void WindowState::UpdateState(uint16_t dt)
{
  
    if(!OnMyWay) // ничего не делаем
      return;

   uint16_t bRelay1State, bRelay2State; // состояние выходов реле
   
   switch(Direction)
   {
      case dirOPEN:
        bRelay1State = HIGH; // крутимся в одну сторону
        bRelay2State = LOW;
        
      break;

      case dirCLOSE:
        bRelay1State = LOW; // или в другую
        bRelay2State = HIGH;
        
      break;

      case dirNOTHING:
      default:

        bRelay1State = SHORT_CIRQUIT_STATE; // накоротко, мотор не крутится
        bRelay2State = SHORT_CIRQUIT_STATE;
        
      break;
   } // switch

    TimerTicks += dt;
    if(TimerTicks >= TimerInterval) // отработали, выключаем
    {
        CurrentPosition = RequestedPosition; // сохранили текущую позицию
        TimerInterval = 0; // обнуляем интервал
        TimerTicks = 0; // и таймер
        Direction = dirNOTHING; // уже никуда не движемся

        //TODO: ВЫКЛЮЧАЕМ РЕЛЕ
        SwitchRelays();
        
        OnMyWay = false;

       // Serial.println("POSITION " + String(CurrentPosition) + " REACHED!");

        return;
        
    } // if

    // продолжаем работу, включаем реле в нужное состояние
    SwitchRelays(bRelay1State,bRelay2State);
  
}
void TempSensors::SetupWindows()
{
  // настраиваем фрамуги
  for(uint8_t i=0, j=0;i<SUPPORTED_WINDOWS;i++, j+=2)
  {
      //Serial.println("first relay=" + String(j) + "; second relay=" + String(j+1));
      Windows[i].Setup(&State,j,j+1); // раздаём каналы реле: первому окну - 0,1, второму - 2,3 и т.д.
  } // for
}

void TempSensors::Setup()
{
  // настройка модуля тут

  //TODO: ВЫЧИТЫВАТЬ ИНТЕРВАЛ РАБОТЫ С НАСТРОЕК
  // CurrentOpenInterval = Settings.GetOpenInterval();
  
  /*
   * Пишем в State настройки - кол-во поддерживаемых датчиков температуры
   * 
   */
   State.SetTempSensors(supportedSensorsCnt); // сколько датчиков поддерживаем?
   State.SetRelayChannels(supportedWindowsCnt*2); // сколько каналов реле? Каждым мотором фрамуги управляют два реле
  
   SetupWindows(); // настраиваем фрамуги

 }
void TempSensors::Update(uint16_t dt)
{ 


  for(uint8_t i=0;i<SUPPORTED_WINDOWS;i++) // обновляем каналы управления фрамугами
  {
      Windows[i].UpdateState(dt);
  } // for 


  // TEST CODE BEGIN //
  lastUpdateCall += dt;
  if(lastUpdateCall < 500) // нечего обновлять раньше, чем раз в полсекунды
    return;

  lastUpdateCall = 0;
  
  // обновляем значения температуры
  for(uint8_t i=0;i<supportedSensorsCnt;i++)
  {
    String s = String(random(0,40)) + F(",") + String(random(50,100));
    State.SetTemp(i,s);
  }
  // TEST CODE END //

}

bool  TempSensors::ExecCommand(const Command& command)
{
  ModuleController* c = GetController();
  String answer = PARAMS_MISSED;  
  bool answerStatus = false;

 // Serial.println("RECEIVED=" + command.GetRawArguments());

  if(command.GetType() == ctSET) // напрямую запись в датчики запрещена, пишем только в состояние каналов
  {
    uint8_t argsCnt = command.GetArgsCount();
    if(argsCnt > 2)
    {
      String s = command.GetArg(0);
      s.toUpperCase();
      if(s == PROP_WINDOW) // надо записать состояние окна, от нас просят что-то сделать
      {
        if(command.IsInternal() // если команда пришла от другого модуля
        && workMode == wmManual) // и мы в ручном режиме, то
        {
          // просто игнорируем команду, потому что нами управляют в ручном режиме
        }
        else
        {
          if(!command.IsInternal()) // пришла команда от пользователя,
            workMode = wmManual; // переходим на ручной режим работы
            
          String token = command.GetArg(1);
          token.toUpperCase();

          String whichCommand = command.GetArg(2); // какую команду запросили?
          whichCommand.toUpperCase();
          bool bOpen = (whichCommand == STATE_OPEN); // запросили открытие фрамуг?
          
          bool bAll = (token == ALL); // на все реле распространяется запрос?
          bool bIntervalAsked = token.indexOf("-") != -1; // запросили интервал каналов?
          uint8_t channelIdx = token.toInt();
          unsigned long interval = this->CurrentOpenInterval;
          
          if(command.GetArgsCount() > 3)
            interval = command.GetArg(3).toInt(); // получили интервал для работы реле

 
          answerStatus = true;
          // откуда до куда шаримся
          uint8_t from = 0;
          uint8_t to = supportedWindowsCnt;


          if(bIntervalAsked)
          {
             // парсим интервал
             int delim = token.indexOf("-");
             from = token.substring(0,delim).toInt();
             to = token.substring(delim+1,token.length()).toInt();
             
          }

          // правильно расставляем шаги - от меньшего к большему
          uint8_t tmp = min(from,to);
          to = max(from,to);
          from = tmp;

             to++; // включаем to в интервал, это надо, если пришла команда интервала, например, 2-3, тогда в этом случае опросятся третий и четвертый каналы
             if(to >= supportedWindowsCnt)
              to = supportedWindowsCnt;
          
          if(bAll || bIntervalAsked)
          {
            // по всем каналам шаримся
            bool bAnyPosChanged = false;
            for(uint8_t i=from;i<to;i++)
            {
              if(Windows[i].ChangePosition(bOpen? dirOPEN : dirCLOSE,bOpen ? interval : 0))
              {
                answer = bOpen ? STATE_OPENING : STATE_CLOSING;
                bAnyPosChanged = true;
              } 
            } // for
            if(!bAnyPosChanged) // ничего не сменили
            {
              answer =  bOpen ? STATE_OPEN : STATE_CLOSED;
            }

          }
          else
          { 
            
              if(Windows[channelIdx].ChangePosition( bOpen ? dirOPEN : dirCLOSE, interval) )
                  answer = bOpen ? STATE_OPENING : STATE_CLOSING;
               else
               {
                answer =  bOpen ? STATE_OPEN : STATE_CLOSED;
               }
          }

        } // else can process
        
      } // if PROP_WINDOW
      
    } // if(argsCnt > 2)
    else if(argsCnt > 1)
    {
      String s = command.GetArg(0);
      s.toUpperCase();

      if (s == WORK_MODE)
      {
        // запросили установить режим работы
        s = command.GetArg(1);
        s.toUpperCase();


        if(s == WM_AUTOMATIC)
        {
          answerStatus = true;
          answer = String(WORK_MODE) + PARAM_DELIMITER + s;
          workMode = wmAutomatic;
        }
        else if(s == WM_MANUAL)
        {
          answerStatus = true;
          answer = String(WORK_MODE) + PARAM_DELIMITER + s;
          workMode = wmManual;
        }
        
      } // WORK_MODE
      else if(s == WM_INTERVAL) // запросили установку интервала
      {
              int newInt = command.GetArg(1).toInt();
              if(newInt > 0)
              {
                CurrentOpenInterval = newInt;
                //TODO: ТУТ СОХРАНЕНИЕ ИНТЕРВАЛА В НАСТРОЙКАХ !!!
                // Settings.WriteInterval(CurrentOpenInterval);
                answerStatus = true;
                answer = String(WM_INTERVAL) + PARAM_DELIMITER + REG_SUCC;
              } // if
      } // WM_INTERVAL
    } // argsCnt > 1
  }
  else
  if(command.GetType() == ctGET) // запросили показание датчика
  {
      uint8_t argsCnt = command.GetArgsCount();
       
      if(argsCnt > 1)
      {
        // параметров хватает
        // проверяем, есть ли там запрос на кол-во, или просто индекс?
        String s = command.GetArg(0);
        s.toUpperCase();

          if(s == PROP_TEMP) // обращение по температуре
          {
              s = command.GetArg(1);
              s.toUpperCase();

              if(s == PROP_TEMP_CNT) // кол-во датчиков
              {
                 answerStatus = true;
                 answer = String(PROP_TEMP_CNT) + PARAM_DELIMITER + State.GetTempSensors();
              } // if
              else // запросили по индексу
              {
                uint8_t sensorIdx = s.toInt();
                if(sensorIdx >= supportedSensorsCnt)
                   answer = NOT_SUPPORTED; // неверный индекс
                 else
                  {
                    // получаем текущее значение датчика
                    answerStatus = true;
                    answer = String(PROP_TEMP) + PARAM_DELIMITER + String(sensorIdx) + PARAM_DELIMITER + State.GetTemp(sensorIdx);
                  }
              } // else
              
          } // if
          else if(s == PROP_WINDOW) // статус окна
          {
             s = command.GetArg(1);
              s.toUpperCase();

             if(s == PROP_WINDOW_CNT)
             {
                    answerStatus = true;
                    answer = String(PROP_WINDOW_CNT) + PARAM_DELIMITER + State.GetRelayChannels();

             }
            else // запросили по индексу
            {
             uint8_t windowIdx = s.toInt();
             if(windowIdx >= supportedWindowsCnt)
              answer = NOT_SUPPORTED; // неверный индекс
              else
              {
                WindowState* ws = &(Windows[windowIdx]);
                String sAdd;
                if(ws->IsBusy())
                {
                  //куда-то едем
                  sAdd = ws->GetDirection() == dirOPEN ? STATE_OPENING : STATE_CLOSING;
                  
                } // if
                else
                {
                    // никуда не едем
                    if(ws->GetCurrentPosition() > 0)
                      sAdd = STATE_OPEN;
                    else
                      sAdd = STATE_CLOSED;
                } // else
                
                
                answerStatus = true;
                answer = String(PROP_WINDOW) + PARAM_DELIMITER + s + PARAM_DELIMITER + sAdd;
              }
            } // else
          } // else
         
        
      } // if
      else if(argsCnt > 0)
      {
        String s = command.GetArg(0);
        s.toUpperCase();

        if(s == WORK_MODE) // запросили режим работы
        {
          String wm = workMode == wmAutomatic ? WM_AUTOMATIC : WM_MANUAL;
          answerStatus = true;
          answer = String(WORK_MODE) + PARAM_DELIMITER + wm;
          
        } // if
        else
        if(s == WM_INTERVAL) // запросили интервал срабатывания форточек
        {
          answerStatus = true;
          answer = String(WM_INTERVAL) + PARAM_DELIMITER + String(CurrentOpenInterval);
        } // WM_INTERVAL
        
      } // else if(argsCnt > 0)
  } // if
  
 // отвечаем на команду
    SetPublishData(&command,answerStatus,answer); // готовим данные для публикации
    c->Publish(this);
  
  
  return answerStatus;
}


