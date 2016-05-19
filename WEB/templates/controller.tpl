{* Smarty *}
{* Данные с контроллера *}

<div id="temp_settings_dialog" title="Настройки температур и моторов" class='hdn'>
  <form>
  T открытия:<br/>
  <input type='text' id='edit_t_open' maxlength='5' value='' style='width:100%;'/><br/>
  Т закрытия:<br/>
  <input type='text' id='edit_t_close' maxlength='50' value='' style='width:100%;'/><br/>
  Время работы моторов, с:<br/>
  <input type='text' id='edit_motor_time' maxlength='100' value='' style='width:100%;'/><br/>
  </form>
</div>

<div id="sensor_name_dialog" title="Новое имя датчика" class='hdn'>
  <form>
  Имя датчика:<br/>
  <input type='text' id='edit_sensor_name' maxlength='100' value='' style='width:100%;'/><br/>
  </form>
</div>


<a href="/">Назад</a>


<h1>Контроллер "{$selected_controller.controller_name}"</h1>

<div id='offline_block' class='hdn'>
<div class='error'>Контроллер в оффлайн-режиме!</div>
</div>
<div id='online_block' class='hdn'>
<div class='controller_menu'>
<a href="javascript:editTempSettings();" id='temp_motors_settings' class='hdn'>Настройки температур и моторов</a>
</div>

<table width='100%' border='0'>
<tr>
<td width='200' valign='top'>

<div class='menuitem hdn' id='STATUS_MENU' onclick="content(this);">Статус</div>
<div class='menuitem hdn' id='TEMPERATURE_MENU' onclick="content(this);">Температура</div>
<div class='menuitem hdn' id='HUMIDITY_MENU' onclick="content(this);">Влажность</div>
<div class='menuitem hdn' id='LIGHT_MENU' onclick="content(this);">Освещенность</div>
<div class='menuitem hdn' id='SOIL_MENU' onclick="content(this);">Влажность почвы</div>
<div class='menuitem hdn' id='FLOW_MENU' onclick="content(this);">Расход воды</div>

</td>
<td width='*' valign='top'>
<div class='page_content'>

<div class='content hdn' id='TEMPERATURE_MENU_CONTENT'>

  <h3>Показания датчиков температуры</h3>
  <div class='row' id='TEMPERATURE_HEADER'>
  <div class='row_item header'>Название модуля</div>
  <div class='row_item header'>Индекс датчика</div>
  <div class='row_item header'>Температура</div>
  </div>
  <div id='TEMPERATURE_LIST'></div>


</div>


<div class='content hdn' id='HUMIDITY_MENU_CONTENT'>

  <h3>Показания датчиков влажности</h3>
  <div class='row' id='HUMIDITY_HEADER'>
  <div class='row_item header'>Название модуля</div>
  <div class='row_item header'>Индекс датчика</div>
  <div class='row_item header'>Влажность</div>
  </div>
  <div id='HUMIDITY_LIST'></div>



</div>
<div class='content hdn' id='LIGHT_MENU_CONTENT'>

  <h3>Показания датчиков освещенности</h3>
  <div class='row' id='LUMINOSITY_HEADER'>
  <div class='row_item header'>Название модуля</div>
  <div class='row_item header'>Индекс датчика</div>
  <div class='row_item header'>Освещенность</div>
  </div>
  <div id='LUMINOSITY_LIST'></div>


</div>
<div class='content hdn' id='SOIL_MENU_CONTENT'>

  <h3>Показания датчиков влажности почвы</h3>
  <div class='row' id='SOIL_HEADER'>
  <div class='row_item header'>Индекс датчика</div>
  <div class='row_item header'>Влажность почвы</div>
  </div>
  <div id='SOIL_LIST'></div>

</div>


<div class='content hdn' id='FLOW_MENU_CONTENT'>

  <h3>Расход воды</h3>
  Мгновенный: <span id='flow_instant' class='bold'>0</span> литров<br/>
  Накопительный: <span id='flow_incremental' class='bold'>0</span> литров<br/>
  
</div>




<div class='content hdn' id='STATUS_MENU_CONTENT'>

  <h3>Состояние контроллера</h3>

  <span id='mode_auto' class='hdn'><span class='auto_mode'>автоматический</span></span>
  <span id='mode_manual' class='hdn'><span class='manual_mode'>ручной</span></span>

  <span id='mode_auto_switch' class='hdn'>Переключиться в автоматический режим</span>
  <span id='mode_manual_switch' class='hdn'>Переключиться в ручной режим</span>

  <span id='toggle_open' class='hdn'>Открыть</span>
  <span id='toggle_close' class='hdn'>Закрыть</span>

  <span id='toggle_on' class='hdn'>Включить</span>
  <span id='toggle_off' class='hdn'>Выключить</span>
  
  <span id='window_state_on' class='hdn'>открыты</span>
  <span id='window_state_off' class='hdn'>закрыты</span>

  <span id='water_state_on' class='hdn'>включён</span>
  <span id='water_state_off' class='hdn'>выключен</span>

  <span id='light_state_on' class='hdn'>включёна</span>
  <span id='light_state_off' class='hdn'>выключена</span>
  
  

  Окна: <span class='bold' id='window_state'></span><br/>
  Режим: <span class='bold' id='window_mode'></span><br/>
  <input type='button' value='' id='toggler_windows' onclick='controller.toggleWindows();updateWindowsState();'/>
  <input type='button' value='' id='toggler_windows_mode' onclick='controller.toggleWindowsMode();updateWindowsState();'/>
  <br/><br/>
  
  Полив: <span class='bold' id='water_state'></span><br/>
  Режим: <span class='bold' id='water_mode'></span><br/>
  <input type='button' value='' id='toggler_water' onclick='controller.toggleWater();updateWaterState();'/>
  <input type='button' value='' id='toggler_water_mode' onclick='controller.toggleWaterMode();updateWaterState();'/>
  <br/><br/>

  Досветка: <span class='bold' id='light_state'></span><br/>
  Режим: <span class='bold' id='light_mode'></span><br/>
  <input type='button' value='' id='toggler_light' onclick='controller.toggleLight();updateLightState();'/>
  <input type='button' value='' id='toggler_light_mode' onclick='controller.toggleLightMode();updateLightState();'/>
  <br/><br/>
  
</div>


<div class='content' id='welcome'>

  <h3>Состояние контроллера</h3>
  
  Для просмотра состояния выберите пункт меню слева.

  
</div>

</div>
</td>
</tr>
</table>

</div>


{include file="controller_helpers.tpl"}
