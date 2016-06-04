{* Smarty *}
<br clear='left'/><br/>
  
  <h3 class='ui-widget-header ui-corner-all'>
  
  <div style='float:right;'>
    <button onclick="toggleHelp();" id='help_button'>Показать</button>
  </div>  
  Помощь
  
  </h3>
  
 <div class='hdn' style='font-size:80%;' id='help_text_box'>
  Тут текст помощи по командам и т.п.
 </div>
  
 {literal}
 <script type='text/javascript'>
var __helpVisible = false;
function toggleHelp()
{
  __helpVisible = !__helpVisible;
  $('#help_text_box').toggle(__helpVisible);
  
  $("#help_button").text(__helpVisible ? "Скрыть" : "Показать");
}


 </script>
 {/literal}
