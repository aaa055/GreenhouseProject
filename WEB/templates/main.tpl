{* Smarty *}

<script type="text/javascript" src="js/edit_controller.js"></script>


<div id="controller_dialog" title="Данные контроллера" class='hdn'>
  <form>
  ID контроллера:<br/>
  <input type='text' id='edit_controller_id' maxlength='5' value='' style='width:100%;'/><br/>
  Имя контроллера:<br/>
  <input type='text' id='controller_name' maxlength='50' value='' style='width:100%;'/><br/>
  Адрес контроллера:<br/>
  <input type='text' id='controller_address' maxlength='100' value='' style='width:100%;'/><br/>
  </form>
</div>

<div id='controller_online' class='hdn'><span class='auto_mode'>да</span></div>
<div id='controller_offline' class='hdn'><span class='manual_mode'>нет</span></div>

<div id="controller_in_process_dialog" title="Идёт обработка данных..." class='hdn'>
  <p>Пожалуйста, подождите, пока данные обрабатываются...</p>
</div>

<script type='text/javascript'>
var controllers = new Array();
{foreach key=k item=controller from=$controllers}
controllers.push(new Controller({$controller.controller_id},'{$controller.controller_name}','{$controller.controller_address}'));
{/foreach}
{literal}
  for(var i=0;i<controllers.length;i++)
  {
    var c = controllers[i];
    c.OnStatus = function(controller)
    {
      var stat = controller.IsOnline() ? $('#controller_online').html() :  $('#controller_offline').html();
      $("#controller_status" + controller.getId()).html(stat);
    };
  }
  
{/literal}
</script>

<h1 class='ui-widget-header ui-corner-all'>Список контроллеров</h1>
{foreach key=k item=controller from=$controllers}
{include file='controller_info.tpl'}
{/foreach}
