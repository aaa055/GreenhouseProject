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
//-----------------------------------------------------------------------------------------------------
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
