{* Smarty *}


<div id="message_dialog" title="Сообщение" class='hdn'>
  <p id='message_dialog_message'></p>
</div>

<div id="prompt_dialog" title="Подтверждение" class='hdn'>
  <p id='prompt_dialog_message'></p>
</div>


<div id="data_requested_dialog" title="Обработка данных..." class='hdn'>
  <p>Пожалуйста, подождите, пока данные обрабатываются...</p>
</div>

<div id='phone_number_dialog' title='Номер телефона' class='hdn'>
  <form>
    Номер телефона:<br/>
    <input type='text' id='edit_phone_number' maxlength='20' value='' style='width:100%;'/><br/>
  </form>
</div>

<div id='new_cc_list_dialog' title='Новый список' class='hdn'>
  <form>
    Имя списка:<br/>
    <input type='text' id='cc_list_name' maxlength='100' value='' style='width:100%;'/><br/>
  </form>
</div>

<div id='new_cc_command_dialog' title='Новая команда' class='hdn'>
  <form>
    Тип команды:<br/>
    <select id='cc_type' style='width:100%;'/></select><br/>
    Параметр:<br/>
    <input type='text' id='cc_param' maxlength='100' value='' style='width:100%;'/><br/>
  </form>
</div>

<div id='select_cc_lists_dialog' title='Выбор списков' class='hdn'>
  Выберите списки команд, которые надо загрузить в контроллер:
  <div id='cc_list_selector' class='padding_around8px'>
  </div>
</div>



<div id='wifi_dialog' title='Настройки Wi-Fi' class='hdn'>
  <form>
    SSID роутера:<br/>
    <input type='text' id='router_id' maxlength='50' value='' style='width:100%;'/><br/>
    Пароль роутера:<br/>
    <input type='text' id='router_pass' maxlength='50' value='' style='width:100%;'/><br/>
    <input type='checkbox' id='connect_to_router' value='1'/>
    <label for='connect_to_router'>Коннектиться к роутеру</label><br/><br/>
    SSID модуля ESP:<br/>
    <input type='text' id='station_id' maxlength='50' value='' style='width:100%;'/><br/>
    Пароль модуля ESP:<br/>
    <input type='text' id='station_pass' maxlength='50' value='' style='width:100%;'/><br/>
    
  </form>
</div>

<div id="new_delta_dialog" title="Новая дельта" class='hdn'>

  <form>
  Тип дельты:<br/>
  <select id='delta_type' style='width:100%;'>
  <option value='TEMP'>Температура</option>
  <option value='HUMIDITY'>Влажность</option>
  <option value='LIGHT'>Освещенность</option>
  </select><br/>
  Модуль 1:<br/>
  <select id='delta_module1' style='width:100%;'></select><br/>
  Датчик 1:<br/>
  <input type='text' id='delta_index1' maxlength='3' value='' style='width:100%;'/><br/>
  Модуль 2:<br/>
  <select id='delta_module2' style='width:100%;'></select><br/>
  Датчик 2:<br/>
  <input type='text' id='delta_index2' maxlength='3' value='' style='width:100%;'/><br/>
  </form>

</div>

<div id="water_channel_settings_dialog" title="Настройки канала" class='hdn'>

   Дни работы канала:
   <div id='water_channels_days' class='padding_around8px'>
    <div><input type='checkbox' id='watering_channel_day' value='1'/>Понедельник</div>
    <div><input type='checkbox' id='watering_channel_day' value='2'/>Вторник</div>
    <div><input type='checkbox' id='watering_channel_day' value='4'/>Среда</div>
    <div><input type='checkbox' id='watering_channel_day' value='8'/>Четверг</div>
    <div><input type='checkbox' id='watering_channel_day' value='16'/>Пятница</div>
    <div><input type='checkbox' id='watering_channel_day' value='32'/>Суббота</div>
    <div><input type='checkbox' id='watering_channel_day' value='64'/>Воскресенье</div>
  </div>

    <div>
      Час начала работы:<br/>
      <input type='text' maxlength='2' id='watering_start_hour' style='width:100%;'/>
    </div>

    <div>
      Продолжительность полива, минут:<br/>
      <input type='text' maxlength='5' id='watering_time' style='width:100%;'/>
    </div>    

  
</div>



{include file='controller_head.tpl'}

<div id='offline_block' class='hdn'>
{include file='controller_offline.tpl'}
</div>

<div id='online_block' class='hdn'>

    <div class='left_menu'>
    
      <div class='menuitem ui-corner-all hdn' id='DELTA_MENU' onclick="content(this);">Список дельт</div>
      <div class='menuitem ui-corner-all hdn' id='CC_MENU' onclick="content(this);">Составные команды</div>
      <div class='menuitem ui-corner-all hdn' id='RULES_MENU' onclick="content(this);">Правила</div>
      <div class='menuitem ui-corner-all hdn' id='WATER_MENU' onclick="content(this);">Настройки полива</div>

      <div class='ui-corner-all button_menu_spacer hdn' id='controller_time_button' onclick="setControllerTime();">Дата/время</div>

      <div class='ui-corner-all button_menu_spacer hdn' id='phone_number' onclick="editPhoneNumber();">Номер телефона для SMS</div>
      <div class='ui-corner-all button_menu_spacer hdn' id='wifi_menu' onclick="editWiFiSettings();">Настройки Wi-Fi</div>

    </div>
    
    
    <div class="page_content">
      
                  <div class='content hdn' id='DELTA_MENU_CONTENT'>
                  
                    <h3 class='ui-widget-header ui-corner-all'>Список виртуальных датчиков дельт</h3>
                    
                    <div id='DELTA_LIST'></div>
                    <br clear='left'/>
                    <div><br/><br/>
                    
                        <button id='get_delta_button' onclick='queryDeltasList();'>Получить список дельт</button>
                        <button id='save_delta_button' onclick='saveDeltasList();'>Сохранить</button>
                        <button id='new_delta_button' onclick='newDelta();'>Новая дельта</button>
                        
                    </div>
                    
                  </div>

                  <div class='content hdn' id='RULES_MENU_CONTENT'>
                  
                    <h3 class='ui-widget-header ui-corner-all'>Список правил</h3>
                    
                    <div id='RULES_LIST'></div>
                    <br clear='left'/>
                    <div><br/><br/>
                    
                        <button id='get_rules_button' onclick='doRequestRulesList();'>Получить список правил</button>
                        <button id='save_rules_button' onclick='saveRulesList();'>Сохранить</button>
                        <button id='new_rule_button' onclick='newRule();'>Новое правило</button>
                        
                    </div>
                    
                  </div>

                  <div class='content hdn' id='CC_MENU_CONTENT'>
                  
                    <h3 class='ui-widget-header ui-corner-all'>Список составных команд</h3>
                    <div id='cc_lists_box' class='hdn'>
                        <select id='cc_lists' style='width:100%'></select>
                        <div class='padding_top8px'>
                          <button id='new_cc_list' onclick='newCCList();'>Новый список</button>
                          <button id='delete_cc_list' onclick='deleteCCList();'>Удалить текущий список</button>
                          <button id='save_cc_lists' onclick='saveCCLists();'>Сохранить данные в БД</button>
                        </div>

                      <br/>
                    </div>
                    <div id='CC_LIST'></div>
                    <br clear='left'/>
                    <div><br/><br/>
                    
                        <button id='save_cc_button' onclick="uploadCCCommands();">Загрузить команды в контроллер</button>
                        <button id='new_cc_button' onclick="newCCCommand();">Новая составная команда</button>
                        
                    </div>
                    
                  </div>

                  <div class='content hdn' id='WATER_MENU_CONTENT'>

                    <h3 class='ui-widget-header ui-corner-all'>Настройки полива</h3>
                    
                    
                    
                         <div>
                          Автоматическое управление поливом: <br/>
                          <select id='watering_option' style='width:100%'>
                            <option value='0'>Отключено</option>
                            <option value='1'>По дням недели (все каналы одновременно)</option>
                            <option value='2'>По дням недели (раздельное управление каналами)</option>
                          </select>
                         </div>
                         
                         <div id='watering_all_channels' class='hdn'>
                          <p>
                           
                            <div class='half'>
                                <div class='half_box half_left'>
                                  <div class='ui-widget-header ui-corner-all'>Дни недели</div>
                                  <div class='ui-widget-content padding_around8px left_align'id='all_watering_channels_days'>
                                      <div><input type='checkbox' id='all_watering_channels_day' value='1'/>Понедельник</div>
                                      <div><input type='checkbox' id='all_watering_channels_day' value='2'/>Вторник</div>
                                      <div><input type='checkbox' id='all_watering_channels_day' value='4'/>Среда</div>
                                      <div><input type='checkbox' id='all_watering_channels_day' value='8'/>Четверг</div>
                                      <div><input type='checkbox' id='all_watering_channels_day' value='16'/>Пятница</div>
                                      <div><input type='checkbox' id='all_watering_channels_day' value='32'/>Суббота</div>
                                      <div><input type='checkbox' id='all_watering_channels_day' value='64'/>Воскресенье</div>
                                  </div>
                                </div>
                            </div>    

                            <div class='half'>
                                <div class='half_box half_right'>

                                  <div class='ui-widget-header ui-corner-all'>Настройки</div>
                                  <div class='ui-widget-content padding_around8px left_align'>
                                    <div>
                                      Час начала работы:<br/>
                                      <input type='text' maxlength='2' id='all_watering_start_hour' style='width:100%;'/>
                                    </div>
                                    
                                    <div>
                                      Продолжительность полива, минут:<br/>
                                      <input type='text' maxlength='5' id='all_watering_time' style='width:100%;'/>
                                    </div>
                                  </div>

                                </div>
                            </div>    
                           
                          </p>
                         </div>
                         
                         <div id='watering_separate_channels' class='hdn'>
                          <p>
                            <div id='WATER_CHANNELS_LIST'></div>
                          </p>
                         </div>
                         
                         <div id='turn_pump_box' class='hdn'><br clear='left'/><br/>
                          <input type='checkbox' value='1' id='turn_on_pump'><label for='turn_on_pump'>Включать насос при поливе на любом из каналов</label>
                         </div>
                         
                         <div><br clear='left'/><br/>
                         
                              <button id='save_watering_button' onclick="saveWateringSettings();">Сохранить в контроллер</button>

                         </div>
                     
                  </div>        
    
                  <div class='content' id='welcome'>

                    <h3 class='ui-widget-header ui-corner-all'>Настройки контроллера</h3>
                    
                      Для редактирования настроек контроллера выберите пункт меню слева.
 
                  </div>    
    
    </div>


</div>

{include file="controller_settings_helpers.tpl"}