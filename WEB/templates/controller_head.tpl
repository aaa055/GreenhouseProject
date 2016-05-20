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

<p>
<a href="/" id='back_link'>На главную</a>
<a href="/controller.php?id={$selected_controller.controller_id}" id='view_link'>Показания</a>
<a href="/controller_settings.php?id={$selected_controller.controller_id}" id='settings_link'>Настройки</a>
</p>


<h2 class='ui-widget-header ui-corner-all'>Контроллер "{$selected_controller.controller_name}"</h2>

<script type='text/javascript'>
{literal}

var freeRamCounter = 15000;
var upTimeCounter = 60000;

FREERAM_CHECK_INTERVAL = 500;
UPTIME_CHECK_INTERVAL = 500;

var controllerUptime = 0;

function freeRam()
{

  if(!controller.IsOnline())
    return;
    
  freeRamCounter += FREERAM_CHECK_INTERVAL;  

  if(freeRamCounter < 15000)
    return;
    
  freeRamCounter = 0;
    
  if(controller.Modules.includes('STAT'))
  {
  
    controller.queryCommand(true,'STAT|FREERAM',function(obj,answer){
    
          if(answer.IsOK)
          {
            $('#controller_freeram').html(answer.Params[1]);
            $('#freeram_box').show();
            $('#controller_stats').show();
            
          }
      });
  }
}

function showUptime()
{
  var mins = parseInt(controllerUptime/60);
  var c_hours = parseInt(mins/60);
  var c_minutes = mins%60;
  
  
  if(c_minutes < 10)
    c_minutes = '0' + c_minutes;
  
  $('#controller_uptime').html(c_hours + ' ч ' + c_minutes + ' мин');
}

function upTime()
{

  if(!controller.IsOnline())
    return;
    
  upTimeCounter += UPTIME_CHECK_INTERVAL;  

  if(upTimeCounter < 60000)
    return;
    
  upTimeCounter = 0;

  if(controller.Modules.includes('STAT'))
  {
  
    controller.queryCommand(true,'STAT|UPTIME',function(obj,answer){
    
          if(answer.IsOK)
          {
            
            controllerUptime = parseInt(answer.Params[1]);
            showUptime();
  
            $('#uptime_box').show();
            $('#controller_stats').show();
            
          }
            
      });
  }
}
{/literal}

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