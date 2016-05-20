{* Smarty *}

<div id="data_requested_dialog" title="Обработка данных..." class='hdn'>
  <p>Пожалуйста, подождите, пока данные обрабатываются...</p>
</div>

<div id='phone_number_dialog' title='Номер телефона' class='hdn'>
  <form>
    Номер телефона:<br/>
    <input type='text' id='edit_phone_number' maxlength='20' value='' style='width:100%;'/><br/>
  </form>
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



{include file='controller_head.tpl'}

<div id='offline_block' class='hdn'>
{include file='controller_offline.tpl'}
</div>

<div id='online_block' class='hdn'>

    <div class='left_menu'>
    
      <div class='menuitem ui-corner-all hdn' id='DELTA_MENU' onclick="content(this);">Список дельт</div>
      <div class='ui-corner-all hdn' id='phone_number' onclick="editPhoneNumber();">Номер телефона для SMS</div>
      <div class='ui-corner-all hdn' id='wifi_menu' onclick="editWiFiSettings();">Настройки Wi-Fi</div>

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
    
    
                  <div class='content' id='welcome'>

                    <h3 class='ui-widget-header ui-corner-all'>Настройки контроллера</h3>
                    
                      Для редактирования настроек контроллера выберите пункт меню слева.
 
                  </div>    
    
    </div>


</div>

{include file="controller_settings_helpers.tpl"}