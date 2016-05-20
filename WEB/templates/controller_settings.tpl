{* Smarty *}

<div id="data_requested_dialog" title="Обработка данных..." class='hdn'>
  <p>Пожалуйста, подождите, пока данные обрабатываются...</p>
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