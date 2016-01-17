#ifndef _MENU_H
#define _MENU_H

#include <WString.h>


// Классы для работы с меню

enum MenuSettingsType// тип меню
{
  mstNop, // никакой настройки не поддерживается
  mstInterval, // перебирает числовой интервал с указанным шагом в указанных пределах
  mstCheckbox // чекбокс
  
};

typedef struct _MenuItem
{
  MenuSettingsType Type; // тип настроек в этом меню
  String DisplayName; // что отображается на экране

  _MenuItem* Parent; // указатель на родительское меню
  _MenuItem* Child; // указатель на дочернее меню
  _MenuItem* Previous; // указатель на предыдушее меню
  _MenuItem* Next; // указатель на следующее меню
  
 
} MenuItem;




#endif
