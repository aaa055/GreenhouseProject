{* Smarty *}


<script type="text/javascript" src="js/deltas.js"></script>
<script type="text/javascript" src="js/deltas_view.js"></script>

<script type='text/javascript'>
//-----------------------------------------------------------------------------------------------------
// наш контроллер
var controller = new Controller({$selected_controller.controller_id},'{$selected_controller.controller_name}','{$selected_controller.controller_address}');
var deltaList = new DeltaList();
var deltaView = new DeltaView(controller, deltaList);
{literal}
//-----------------------------------------------------------------------------------------------------
// запрошено редактирование дельты
deltaView.OnEditDelta = function(controllerObject, delta, row)
{
  alert('Пока не реализовано :(');
}
//-----------------------------------------------------------------------------------------------------
// запрошено удаление дельты
deltaView.OnDeleteDelta = function(controllerObject, delta, row)
{
  deltaList.List.remove(delta); // удаляем дельту из списка
  row.remove(); // удаляем строку из таблицы  
}
//-----------------------------------------------------------------------------------------------------
// получаем список дельт с контроллера
function queryDeltasList()
{

  $("#data_requested_dialog" ).dialog({
                dialogClass: "no-close",
                modal: true,
                closeOnEscape: false,
                draggable: false,
                resizable: false,
                buttons: []
              });
              
              
     controller.queryCommand(true,'DELTA|CNT',function(obj,answer){
     
     
            var cnt = answer.Params[2];
            
            for(var i=0;i<cnt;i++)
            {
            
                  controller.queryCommand(true,'DELTA|VIEW|' + i.toString(), function(obj,deltaInfo) {
                  
                    var delta = new Delta(deltaInfo.Params[3],deltaInfo.Params[4],deltaInfo.Params[5],deltaInfo.Params[6],deltaInfo.Params[7]);
                    deltaList.Add(delta);
                    if(deltaList.List.length == cnt)
                    {
                        $("#data_requested_dialog" ).dialog('close');
                        
                        // заполняем список дельт
                        deltaView.fillList('#DELTA_LIST');
                    }
                  
                  });
            
            } // for
     
     });

}
//-----------------------------------------------------------------------------------------------------
var lastIsOnline = controller.IsOnline();
// обработчик онлайн-статуса контроллера
controller.OnStatus = function(obj)
{
  var is_online = controller.IsOnline();
  
  if(!lastIsOnline && is_online)
  {
    controller.queryModules(); // запрашиваем модули у контроллера
  }
  
  lastIsOnline = is_online; 
  
  
  
  if(is_online)
  {  
    $('#offline_block').hide();
    $('#online_block').show();
    $('#get_delta_button').removeAttr('disabled');
  }
  else
  {
    $('#offline_block').show();
    $('#online_block').hide();
    $('#get_delta_button').attr('disabled','disabled');
  }
};
//-----------------------------------------------------------------------------------------------------
// событие "Получен список модулей в прошивке"
controller.OnGetModulesList = function(obj)
{
    $('#DELTA_MENU').toggle(controller.Modules.includes('DELTA'));
  
};
//-----------------------------------------------------------------------------------------------------
$(document).ready(function(){

  lastVisibleContent = $('#welcome');

  controller.querySensorNames(); // получаем имена датчиков
  controller.queryModules(); // запрашиваем модули у контроллера

});
//-----------------------------------------------------------------------------------------------------
</script>
{/literal}