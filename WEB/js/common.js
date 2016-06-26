//-----------------------------------------------------------------------------------------------------
function resetController()
{

 $("#reset_controller_prompt").dialog({modal:true, buttons: [{text: "ДА!", click: function(){
  
      $(this).dialog("close");
      
      $("#reset_in_process" ).dialog({
                dialogClass: "no-close",
                modal: true,
                closeOnEscape: false,
                draggable: false,
                resizable: false,
                buttons: []
              });
  
    controller.queryCommand(false,'0|RST',function(obj,answer){
    
        $("#reset_in_process" ).dialog('close');
    
      });           
      
  
  } }
  
  , {text: "Отмена", click: function(){$(this).dialog("close");} }
  ] });     
  
    
}
//-----------------------------------------------------------------------------------------------------
var lastSelectedMenuItem = null; // последний выбранный пункт меню
var lastVisibleContent = null; // последний видимый контент
var freeRamCounter = 15000;
var upTimeCounter = 60000;

FREERAM_CHECK_INTERVAL = 500;
UPTIME_CHECK_INTERVAL = 500;

var controllerUptime = 0;

var waitControllerTimer = 0;
var controllerTimeTicksTimer = 0;
var controllerInternalDate = new Date();
//-----------------------------------------------------------------------------------------------------
function formatDateTime(dt)
{
  var result = '';
  
  var day = dt.getDate();
  var month = dt.getMonth() + 1;
  var year = dt.getFullYear();
  
  var hours = dt.getHours();
  var mins = dt.getMinutes();
  var secs = dt.getSeconds();
  
  if(day < 10)
    day = '0' + day;
    
  if(month < 10)
    month = '0' + month;
    
  if(hours < 10)
    hours = '0' + hours;
    
  if(mins < 10)
    mins = '0' + mins;
    
  if(secs < 10)
    secs = '0' + secs;
  
  
  result = day + '.' + month + '.' + year + ' ' + hours + ':' + mins + ':' + secs;
  
  
  return result;
}
//-----------------------------------------------------------------------------------------------------
function controllerTicks()
{
  controllerInternalDate.setSeconds(controllerInternalDate.getSeconds()+1);
  $('#controller_date_time').html(formatDateTime(controllerInternalDate));
}
//-----------------------------------------------------------------------------------------------------
function waitControllerTime()
{
  if(!controller.IsOnline())
    return;
    
  if(!controller.Modules.length)
    return;    
    
   window.clearInterval(waitControllerTimer);
   waitControllerTimer = 0;
   
   if(controllerTimeTicksTimer != 0)
    window.clearInterval(controllerTimeTicksTimer);
    controllerTimeTicksTimer = 0;
    
    if(controller.Modules.includes('STAT'))
    {
   
         controller.queryCommand(true,'STAT|DATETIME',function(obj,answer){
        
              if(answer.IsOK)
              {
                var unparsedDate = answer.Params.toString();
                
                var idx = unparsedDate.indexOf(' ');
                unparsedDate = unparsedDate.substring(idx+1);
                
                idx = unparsedDate.indexOf(' ');
                var dt = unparsedDate.substring(0,idx);
                var tm = unparsedDate.substring(idx+1);
                
                var dtParts = dt.split('.');
                var tmParts = tm.split(':');
                
                controllerInternalDate = new Date(dtParts[2],parseInt(dtParts[1])-1,dtParts[0],tmParts[0],tmParts[1],tmParts[2]);            
                
                $('#controller_date_time').html(formatDateTime(controllerInternalDate));
                $('#controller_date_time').show();
                
                controllerTimeTicksTimer = window.setInterval(controllerTicks,1000);

                
              }
          }); 
      
      } // if(controller.Modules.includes('STAT'))
     
}
//-----------------------------------------------------------------------------------------------------
function freeRam()
{

  if(!controller.IsOnline())
    return;
    
  if(!controller.Modules.length)
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
//-----------------------------------------------------------------------------------------------------
function showUptime()
{
  var mins = parseInt(controllerUptime/60);
  var c_hours = parseInt(mins/60);
  var c_minutes = mins%60;
  
  
  if(c_minutes < 10)
    c_minutes = '0' + c_minutes;
  
  $('#controller_uptime').html(c_hours + ' ч ' + c_minutes + ' мин');
}
//-----------------------------------------------------------------------------------------------------
function upTime()
{

  if(!controller.IsOnline())
    return;
    
  if(!controller.Modules.length)
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
//-----------------------------------------------------------------------------------------------------
// показываем тот или иной контент по клику на пункт меню
function content(elem)
{
  var jElem = $(elem);
  var eId = jElem.attr("id");
  
  if(lastSelectedMenuItem != null)
  {
    lastSelectedMenuItem.removeClass("menuitem_selected");
  }
  
  if(lastVisibleContent != null)
      lastVisibleContent.hide();
    
  var contentItem = $("#" + eId + "_CONTENT");
  contentItem.show();
  
  lastSelectedMenuItem = jElem;
  lastSelectedMenuItem.addClass("menuitem_selected");
  
  lastVisibleContent = contentItem;
  
}
//-----------------------------------------------------------------------------------------------------
function numericExtension()
{
    $.fn.forceNumericOnly =
    function()
    {
        return this.each(function()
        {
            $(this).keydown(function(e)
            {
                var key = e.charCode || e.keyCode || 0;
                // allow backspace, tab, delete, enter, arrows, numbers and keypad numbers ONLY
                // home, end, period, and numpad decimal
                return (
                    key == 189 || // minus sign 
                    key == 8 || 
                    key == 9 ||
                    key == 13 ||
                    key == 46 ||
                    key == 110 ||
                    key == 190 ||
                    (key >= 35 && key <= 40) ||
                    (key >= 48 && key <= 57) ||
                    (key >= 96 && key <= 105));
            });
        });
    };
    
}
//-----------------------------------------------------------------------------------------------------
