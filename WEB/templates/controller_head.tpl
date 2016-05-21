{* Smarty *}

<div class='ui-state-highlight ui-corner-all hdn' style='padding:10px;padding-top:0px;margin-bottom:0px;float:right;' id='controller_stats'>
  <div class='controller_stats'>
    
    <div class='freeram hdn' id='freeram_box'>
      Память: <span class='bold' id='controller_freeram'></span> байт
    </div>
    
    <div class='uptime hdn' id='uptime_box'>
      Время работы: <span class='bold' id='controller_uptime'></span>
    </div>   
    
  </div>
</div>

<div id="reset_controller_prompt" title="Перезагрузка" class='hdn'>
  <p>Вы уверены, что хотите перезагрузить контроллер?</p>
</div>

<div id="reset_in_process" title="Перезагрузка" class='hdn'>
  <p>Подождите, пока контроллер перезагрузится...</p>
</div>

<p>
<a href="/" id='back_link'>На главную</a>
<a href="/controller.php?id={$selected_controller.controller_id}" id='view_link'>Показания</a>
<a href="/controller_settings.php?id={$selected_controller.controller_id}" id='settings_link'>Настройки</a>
<a href="javascript:resetController();" id='reset_controller_link'>Перезагрузить</a>
</p>


<h2 class='ui-widget-header ui-corner-all'>Контроллер "{$selected_controller.controller_name}"</h2>

<script type='text/javascript'>
$(document).ready(function(){ldelim}

  $( "#back_link" ).button({ldelim}
      icons: {ldelim}
        primary: "ui-icon-seek-prev"
      {rdelim}
    {rdelim});
 
  $( "#view_link" ).button({ldelim}
      icons: {ldelim}
        primary: "ui-icon-info"
      {rdelim}
    {rdelim});
    
  $( "#settings_link" ).button({ldelim}
      icons: {ldelim}
        primary: "ui-icon-gear"
      {rdelim}
    {rdelim});
 
  $( "#reset_controller_link" ).button({ldelim}
      icons: {ldelim}
        primary: "ui-icon-refresh"
      {rdelim}
    {rdelim}).css('background','#ff794d');            
    
    if(typeof(controller) != 'undefined')
    {ldelim}
      if(controller.IsOnline())
      {ldelim}
        upTime();
        freeRam();
      {rdelim}
      window.setInterval(freeRam,FREERAM_CHECK_INTERVAL);
      
      window.setInterval(upTime,UPTIME_CHECK_INTERVAL);
    {rdelim}
              

{rdelim});  
</script>