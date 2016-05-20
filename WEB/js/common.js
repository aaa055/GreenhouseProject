//-----------------------------------------------------------------------------------------------------
var lastSelectedMenuItem = null; // последний выбранный пункт меню
var lastVisibleContent = null; // последний видимый контент
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
