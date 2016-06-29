{* Smarty *}

<div class='ui-widget-header ui-corner-all'>Таймер №{$num}</div>
<div class='ui-widget-content' style='text-align:left;padding:8px;'> 

  <table border='0' width='100%'>
  <tr><td width='50%' valign='top'>
  <div class='button_menu_spacer'>
  Пин: <br/>
  <input type='text' id='timerPin{$num}' maxlength='2' style='width:50px;'/>
  </div>
  <div class='button_menu_spacer'>
  Включён (мин, сек):<br/>
  <input type='text' id='timerOnMin{$num}' maxlength='4' style='width:50px;'/> : <input type='text' id='timerOnSec{$num}' maxlength='2' style='width:50px;'/>
  </div>
  Выключён (мин, сек):<br/>
  <input type='text' id='timerOffMin{$num}' maxlength='4' style='width:50px;'/> : <input type='text' id='timerOffSec{$num}' maxlength='2' style='width:50px;'/>

  </td>
  <td valign='top'>
  
  <div id='timerDayMask{$num}'>
    <input type='checkbox' value='0'>Понедельник<br/>
    <input type='checkbox' value='1'>Вторник<br/>
    <input type='checkbox' value='2'>Среда<br/>
    <input type='checkbox' value='3'>Четверг<br/>
    <input type='checkbox' value='4'>Пятница<br/>
    <input type='checkbox' value='5'>Суббота<br/>
    <input type='checkbox' value='6'>Воскресенье<br/>
  </div>
  </td>
  </tr>
  </table>
  
  <input type='checkbox' id='timerEnabled{$num}'/><label for='timerEnabled{$num}'>Таймер активен?</label>
  
</div>