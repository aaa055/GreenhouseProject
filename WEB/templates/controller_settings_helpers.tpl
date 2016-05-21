{* Smarty *}


<script type="text/javascript" src="js/deltas.js"></script>
<script type="text/javascript" src="js/deltas_view.js"></script>

<script type="text/javascript" src="js/cc.js"></script>


<script type='text/javascript'>
//-----------------------------------------------------------------------------------------------------
// наш контроллер
var controller = new Controller({$selected_controller.controller_id},'{$selected_controller.controller_name}','{$selected_controller.controller_address}');

var deltaList = new DeltaList();
var deltaView = new DeltaView(controller, deltaList);

var compositeCommands = new CompositeCommands();

{literal}
//-----------------------------------------------------------------------------------------------------
// запрошено редактирование дельты
deltaView.OnEditDelta = function(controllerObject, delta, row)
{

}
//-----------------------------------------------------------------------------------------------------
// запрошено удаление дельты
deltaView.OnDeleteDelta = function(controllerObject, delta, row)
{
  deltaList.List.remove(delta); // удаляем дельту из списка
  row.remove(); // удаляем строку из таблицы  
}
//-----------------------------------------------------------------------------------------------------
// добавляем дельту
function newDelta()
{
  
 $("#new_delta_dialog").dialog({modal:true, buttons: [{text: "Добавить", click: function(){
  
      var delta_type = $('#delta_type').val();
      var delta_module1 = $('#delta_module1').val();
      var delta_module2 = $('#delta_module2').val();
      var delta_index1 = parseInt($('#delta_index1').val());
      var delta_index2 = parseInt($('#delta_index2').val());
      
      if(isNaN(delta_index1)) delta_index1 = 0;
      if(isNaN(delta_index2)) delta_index2 = 0;
      
      var delta = new Delta(delta_type,delta_module1,delta_index1,delta_module2,delta_index2);
      if(!deltaList.Add(delta))
      {
        alert('Такая дельта уже существует!');
        return;
      }
      
      deltaView.fillList('#DELTA_LIST');            
      $(this).dialog("close");
  
  } }
  
  , {text: "Отмена", click: function(){$(this).dialog("close");} }
  ] });     
  
}
//-----------------------------------------------------------------------------------------------------
// сохраняем список дельт в контроллер
function saveDeltasList()
{

  $("#data_requested_dialog" ).dialog({
                dialogClass: "no-close",
                modal: true,
                closeOnEscape: false,
                draggable: false,
                resizable: false,
                buttons: []
              });
              
              
    controller.queryCommand(false,'DELTA|DEL',function(obj,answer){
    
        if(!answer.IsOK)
        {
           $("#data_requested_dialog" ).dialog('close');
           return;
        } 
        
          
          for(var i=0;i<deltaList.List.length;i++)
          {
            var delta = deltaList.List[i];
            var cmd = 'DELTA|ADD|' + delta.Type + '|' + delta.ModuleName1 + '|' + delta.Index1 + '|' + delta.ModuleName2 + '|' + delta.Index2;

            
              controller.queryCommand(false,cmd,function(obj,addResult){
              
                                   
              
              });
            
              
          } // for
          
                    controller.queryCommand(false,'DELTA|SAVE',function(obj,saveResult){
                    
                      $("#data_requested_dialog" ).dialog('close');
                      
                    });
          
      
    
    });

}
//-----------------------------------------------------------------------------------------------------
// редактируем настройки Wi-Fi
function editWiFiSettings()
{
 $("#wifi_dialog").dialog({modal:true, buttons: [{text: "Изменить", click: function(){

      
      var shouldConnect = $('#connect_to_router').is(':checked') ? 1 : 0;
      
      var data = {
      
        connect_to_router: shouldConnect,
        router_id: $('#router_id').val(),
        router_pass: $('#router_pass').val(),
        station_id: $('#station_id').val(),
        station_pass: $('#station_pass').val()
        
        };
        
        if(data.router_id == '' || data.router_pass == '' || data.station_id == '' || data.station_pass == '')
          return;
      
        $(this).dialog("close");

      
      $("#data_requested_dialog" ).dialog({
                dialogClass: "no-close",
                modal: true,
                closeOnEscape: false,
                draggable: false,
                resizable: false,
                buttons: []
              });
                    
        controller.queryServerScript("/x_save_wifi_settings.php",data, function(obj,answer){
        
            var cmd = 'WIFI|T_SETT|' + data.connect_to_router + '|' + data.router_id + '|' 
            + data.router_pass + '|' + data.station_id + '|' + data.station_pass;
        
            controller.queryCommand(false,cmd,function(obj,answer){
           
                $("#data_requested_dialog" ).dialog('close');
        
            });
        
           
            
        });      
      
  
  
  } }
  
  , {text: "Отмена", click: function(){$(this).dialog("close");} }
  ] });  

}
//-----------------------------------------------------------------------------------------------------
// редактируем номер телефона
function editPhoneNumber()
{
 $("#phone_number_dialog").dialog({modal:true, buttons: [{text: "Изменить", click: function(){

      $(this).dialog("close");

      var num = $('#edit_phone_number').val();
      
      $("#data_requested_dialog" ).dialog({
                dialogClass: "no-close",
                modal: true,
                closeOnEscape: false,
                draggable: false,
                resizable: false,
                buttons: []
              });
                    
      controller.queryCommand(false,'0|PHONE|' + num,function(obj,answer){
      
      $("#data_requested_dialog" ).dialog('close');
      
      });
      
  
  
  } }
  
  , {text: "Отмена", click: function(){$(this).dialog("close");} }
  ] });  
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
     
            if(!answer.IsOK)
            {
              $("#data_requested_dialog" ).dialog('close');
              return;
            }
     
            var cnt = parseInt(answer.Params[2]);
            deltaList.clear();
            
            if(!cnt)
            {
              $("#data_requested_dialog" ).dialog('close');
              return;
            }
            
            for(var i=0;i<cnt;i++)
            {
            
                  controller.queryCommand(true,'DELTA|VIEW|' + i.toString(), function(obj,deltaInfo) {
                  
                    if(!deltaInfo.IsOK)
                    {
                      $("#data_requested_dialog" ).dialog('close');
                      return;
                    }                  
                          
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
// обработчик онлайн-статуса контроллера
controller.OnStatus = function(obj)
{
  var is_online = controller.IsOnline();  
  
  if(is_online)
  {  
    $('#offline_block').hide();
    $('#online_block').show();
  }
  else
  {
    $('#offline_block').show();
    $('#online_block').hide();
  }
};
//-----------------------------------------------------------------------------------------------------
// событие "Получен список модулей в прошивке"
controller.OnGetModulesList = function(obj)
{
    $('#DELTA_MENU').toggle(controller.Modules.includes('DELTA')); // работаем с дельтами только если в прошивке есть модуль дельт
    
    if(controller.Modules.includes('SMS')) // если в прошивке есть модуль Neoway M590
    {
        controller.queryCommand(true,'0|PHONE',function(obj,answer){
           
          $('#phone_number').toggle(answer.IsOK);
          
          if(answer.IsOK)
          {
            $('#edit_phone_number').val(answer.Params[1]);
          }
        
        });
    }
    
    
    if(controller.Modules.includes('WIFI')) // если в прошивке есть модуль wi-fi
    {
        controller.queryServerScript("/x_get_wifi_settings.php",{}, function(obj,result){
           
          $('#wifi_menu').toggle(true);
          
          var answer = result.wifi;
          
            var checked = false;
            if(answer.connect_to_router && answer.connect_to_router == 1)
              checked = true;
              
            $('#router_id').val(answer.router_id);
            $('#router_pass').val(answer.router_pass);
            $('#station_id').val(answer.station_id);
            $('#station_pass').val(answer.station_pass);
            if(checked)
              $('#connect_to_router').attr('checked', 'checked');
        
        });
    }
    
    if(controller.Modules.includes('CC')) // если в прошивке есть модуль составных команд
    {
        controller.queryServerScript("/x_get_composite_commands.php",{}, function(obj,result){
           
          
          var answer = result.composite_commands;
          compositeCommands.Clear();
          
          for(var i=0;i<answer.length;i++)
          {            
            var cc = answer[i];
           
            compositeCommands.Add(cc.list_index, cc.list_name, cc.command_action, cc.command_param);
            
          } // for
          
          // тут заполнение списка составных команд
            for(var i=0;i<compositeCommands.List.length;i++)
            {
              var ccList = compositeCommands.List[i];
              $('#cc_lists').append($('<option/>',{value: ccList.Index }).text(ccList.Name));
            } // for
          
          $('#cc_lists').change(function(){
          
            var list_index = $(this).val();
            compositeCommands.fillList('#CC_LIST',list_index);
          
          });
          
          $('#CC_MENU').toggle(true);
          $('#cc_lists_box').toggle(true);
          
           $('#cc_lists').trigger('change');
          
          
        });
    } // composite commands end
    
    
  
};
//-----------------------------------------------------------------------------------------------------
// создаём новый список составных команд
function newCCList()
{

$("#new_cc_list_dialog").dialog({modal:true, buttons: [{text: "Добавить", click: function(){


      var list_name = $('#cc_list_name').val();
      
      if(list_name == '')
        return;

      $(this).dialog("close");
      
      var newList = compositeCommands.addNewList(list_name);
      $('#cc_lists').append($('<option/>',{value: newList.Index }).text(newList.Name));
    
      $(this ).dialog('close');
      
 
  
  } }
  
  , {text: "Отмена", click: function(){$(this).dialog("close");} }
  ] });  
}
//-----------------------------------------------------------------------------------------------------
// создаём новую команду в списке составных команд
function newCCCommand()
{
  var idx = $('#cc_lists').val();
  if(idx === null)
  {
    alert('Создайте список составных команд!');
    return;
  }


$("#new_cc_command_dialog").dialog({modal:true, buttons: [{text: "Добавить", click: function(){


      var cc_action = $('#cc_type').val();
      var cc_param = parseInt($('#cc_param').val());
      if(isNaN(cc_param))
        cc_param = 0;
                
        compositeCommands.Add(idx,'',cc_action,cc_param);
        $('#cc_lists').trigger('change');
              
      $(this ).dialog('close');
 
  
  } }
  
  , {text: "Отмена", click: function(){$(this).dialog("close");} }
  ] });  


}
//-----------------------------------------------------------------------------------------------------
// сохраняем все списки составных команд в БД
function saveCCLists()
{
  $("#data_requested_dialog" ).dialog({
                dialogClass: "no-close",
                modal: true,
                closeOnEscape: false,
                draggable: false,
                resizable: false,
                buttons: []
              });
      
   var totalCommands = compositeCommands.List.length;
   var commandsProcessed = 0;
   var currentListIndex = 0;  
            
   for(var i=0;i<compositeCommands.List.length;i++)
   {
    var ccList = compositeCommands.List[i];
    totalCommands += ccList.List.length;
   } // for
   
   controller.queryServerScript("/x_clear_cc_lists.php",{}, function(obj,result){
 
 
      if(commandsProcessed == totalCommands)
      {
        $("#data_requested_dialog" ).dialog('close');
      }
 
       for(var i=0;i<compositeCommands.List.length;i++)
       {
          var ccList = compositeCommands.List[i];
                    
          controller.queryServerScript("/x_add_cc_list.php",{list_name: ccList.Name, list_index: ccList.Index }, function(obj,result){
          
            commandsProcessed++;
            
                  if(commandsProcessed == totalCommands)
                  {
                    $("#data_requested_dialog" ).dialog('close');
                  }
            
                 var thisCCList = compositeCommands.List[currentListIndex++];
            
                for(var j=0;j<thisCCList.List.length;j++)
                { 
                  var cc = thisCCList.List[j];
                  
                  controller.queryServerScript("/x_add_cc_command.php",{list_index: cc.ParentList.Index, command_action: cc.Action, command_param: cc.Param }, function(obj,result){
                  commandsProcessed++;
                  
                  if(commandsProcessed == totalCommands)
                  {
                    $("#data_requested_dialog" ).dialog('close');
                  }
                  
                  });
                } // for
          });
       } // for   
   
   });
   

}
//-----------------------------------------------------------------------------------------------------
// загружаем составные команды в контроллер
function uploadCCCommands()
{

  // cc_list_selector
  $('#cc_list_selector').empty();
  var selectedLists = new Array();
  var listCounter = 0;
 
  for(var i=0;i<compositeCommands.List.length;i++)
  {
    var ccList = compositeCommands.List[i];
    var row = $('<div/>');
    $('<input/>',{type: 'checkbox', id: 'cc_list_checkbox', value: ccList.Index}).appendTo(row);
    $('<label/>').appendTo(row).html(ccList.Name);
    
    row.appendTo('#cc_list_selector');
  } // for
  

$("#select_cc_lists_dialog").dialog({modal:true, width:500, buttons: [{text: "Загрузить", click: function(){


      var lst = $('#cc_list_selector').find('div #cc_list_checkbox');
      for(var i=0;i<lst.length;i++)
      {
        var elem = lst.get(i);
        if(elem.checked)
          selectedLists.push(parseInt(elem.value));
      }

              
      $('#select_cc_lists_dialog').dialog('close');
 

             $("#data_requested_dialog" ).dialog({
                            dialogClass: "no-close",
                            modal: true,
                            closeOnEscape: false,
                            draggable: false,
                            resizable: false,
                            buttons: []
                          });
                          
                          
                controller.queryCommand(false,'CC|DEL',function(obj,answer){
                
                    if(!answer.IsOK)
                    {
                       $("#data_requested_dialog" ).dialog('close');
                       return;
                    } 
                    
                      
                      for(var i=0;i<compositeCommands.List.length;i++)
                      {
                        var ccList = compositeCommands.List[i];
                        
                        
                        if(!selectedLists.includes(ccList.Index))
                          continue;
                        
                          for(var j=0;j<ccList.List.length;j++)
                          {
                            var cc = ccList.List[j];
                              
                            var cmd = 'CC|ADD|' + listCounter + '|' + cc.Action + '|' + cc.Param;

                        
                            controller.queryCommand(false,cmd,function(obj,addResult){
                                                
                          
                              });                
                          } // for
                        
                          listCounter++;
                        
                        
                          
                      } // for
                      
                                controller.queryCommand(false,'CC|SAVE',function(obj,saveResult){
                                
                                  $("#data_requested_dialog" ).dialog('close');
                                  
                                });
                      
                  
                
                });
                
  
  } }
  
  , {text: "Отмена", click: function(){$(this).dialog("close");} }
  ] });  
                

}
//-----------------------------------------------------------------------------------------------------
// удаляем список составных команд
function deleteCCList()
{
  var idx = $('#cc_lists').val();

  
  compositeCommands.remove(idx);
  
  var index = $('#cc_lists').get(0).selectedIndex;
  $('#cc_lists option:eq(' + index + ')').remove();
  
  $('#cc_lists').get(0).selectedIndex = 0;  
  $('#cc_lists').trigger('change');
}
//-----------------------------------------------------------------------------------------------------
$(document).ready(function(){

  lastVisibleContent = $('#welcome');

  controller.querySensorNames(); // получаем имена датчиков
  
  
  $( "#get_delta_button" ).button({
      icons: {
        primary: "ui-icon-arrowthickstop-1-s"
      }
    });
    
  $( "#save_delta_button, #save_cc_button" ).button({
      icons: {
        primary: "ui-icon-arrowthickstop-1-n"
      }
    }); 
    
  $( "#new_delta_button, #new_cc_button" ).button({
      icons: {
        primary: "ui-icon-document"
      }
    });
    
      $( "#phone_number" ).button({
      icons: {
        primary: "ui-icon-note"
      }
    }).hide().css('width','100%');
    
    $('#new_cc_list').button({
      icons: {
        primary: "ui-icon-note"
      }
    });
    
    $('#delete_cc_list').button({
      icons: {
        primary: "ui-icon-close"
      }
    });  
    
    $('#save_cc_lists').button({
      icons: {
        primary: "ui-icon-disk"
      }
    });  
      
    
 $( "#wifi_menu" ).button({
      icons: {
        primary: "ui-icon-signal"
      }
    }).hide().css('width','100%');       
    
    $('#delta_index1').forceNumericOnly();     
    $('#delta_index2').forceNumericOnly();
    $('#cc_param').forceNumericOnly();
    
    for(var i=0;i<CompositeActionsNames.length;i++)
    {
      $('#cc_type').append($('<option/>',{value: i}).text(CompositeActionsNames[i]));
    }
    
    $('#delta_type').change(function() {
    
      var delta_type = $(this).val();
      
       $('#delta_module1').empty().text('');
       $('#delta_module2').empty().text('');
       
        if(delta_type == 'TEMP')
        {
          $('#delta_module1').append($('<option/>',{value: 'STATE'}).text('Модуль температур'));
          $('#delta_module2').append($('<option/>',{value: 'STATE'}).text('Модуль температур'));
          
          $('#delta_module1').append($('<option/>',{value: 'HUMIDITY'}).text('Модуль влажности'));
          $('#delta_module2').append($('<option/>',{value: 'HUMIDITY'}).text('Модуль влажности'));
                   
        } // if TEMP
        else if(delta_type == 'HUMIDITY')
        {
          $('#delta_module1').append($('<option/>',{value: 'HUMIDITY'}).text('Модуль влажности'));
          $('#delta_module2').append($('<option/>',{value: 'HUMIDITY'}).text('Модуль влажности'));
        } // HUMIDITY
        else if(delta_type == 'LIGHT')
        {
          $('#delta_module1').append($('<option/>',{value: 'LIGHT'}).text('Модуль освещенности'));
          $('#delta_module2').append($('<option/>',{value: 'LIGHT'}).text('Модуль освещенности'));
        } // LIGHT
       
    
    });   
    
    $('#delta_type').trigger('change');  

});
//-----------------------------------------------------------------------------------------------------
</script>
{/literal}