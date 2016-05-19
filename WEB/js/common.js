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
