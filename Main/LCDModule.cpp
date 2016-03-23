#include "LCDModule.h"
#include "ModuleController.h"
#include "LCDMenu.h"

#ifdef USE_LCD_MODULE
// наше меню
LCDMenu lcdMenu(SCREEN_SCK_PIN, SCREEN_MOSI_PIN, SCREEN_CS_PIN);
// наш энкодер
RotaryEncoder rotaryEncoder(ENCODER_A_PIN,ENCODER_B_PIN,ENCODER_PULSES_PER_CLICK);
#endif

void LCDModule::Setup()
{
#ifdef USE_LCD_MODULE  
  rotaryEncoder.begin(); // инициализируем энкодер

  // инициализируем меню
  lcdMenu.init(mainController);
#endif  
 }

void LCDModule::Update(uint16_t dt)
{ 
#ifdef USE_LCD_MODULE  
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
#else
  UNUSED(dt);    
#endif
}

bool  LCDModule::ExecCommand(const Command& command, bool wantAnswer)
{
  UNUSED(wantAnswer);
  UNUSED(command);


  return true;
}

