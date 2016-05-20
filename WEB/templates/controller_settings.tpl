{* Smarty *}

<div id="data_requested_dialog" title="Идёт запрос данных..." class='hdn'>
  <p>Пожалуйста, подождите, пока данные запрашиваются...</p>
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
                    <div><br/><br/><button id='get_delta_button' onclick='queryDeltasList();'>Получить список дельт</button></div>
                    
                  </div>
    
    
                  <div class='content' id='welcome'>

                    <h3 class='ui-widget-header ui-corner-all'>Настройки контроллера</h3>
                    
                      Для редактирования настроек контроллера выберите пункт меню слева.
 
                  </div>    
    
    </div>


</div>

{include file="controller_settings_helpers.tpl"}