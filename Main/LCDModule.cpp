#include "LCDModule.h"
#include "ModuleController.h"
#include "LCDMenu.h"

// наше меню
LCDMenu lcdMenu(SCREEN_SCK_PIN, SCREEN_MOSI_PIN, SCREEN_CS_PIN);
// наш энкодер
RotaryEncoder rotaryEncoder(ENCODER_A_PIN,ENCODER_B_PIN,ENCODER_PULSES_PER_CLICK);

void LCDModule::Setup()
{
  rotaryEncoder.begin(); // инициализируем энкодер

  // инициализируем меню
  lcdMenu.init(mainController);
  
 }

void LCDModule::Update(uint16_t dt)
{ 
  rotaryEncoder.update(); // обновляем энкодер
  lcdMenu.update(dt);

  int ch = rotaryEncoder.getChange();
   if(ch)
   {
    // было движение энкодера
    // выбираем следующее меню LCD
    lcdMenu.selectNextMenu(ch);
   }

    lcdMenu.draw(); // рисуем меню

}

bool  LCDModule::ExecCommand(const Command& command, bool wantAnswer)
{
  UNUSED(wantAnswer);
  UNUSED(command);


  return true;
}

