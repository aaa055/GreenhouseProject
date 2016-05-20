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
    
    if(controller.Modules.includes('SMS'))
    {
        controller.queryCommand(true,'0|PHONE',function(obj,answer){
           
          $('#phone_number').toggle(answer.IsOK);
          
          if(answer.IsOK)
          {
            $('#edit_phone_number').val(answer.Params[1]);
          }
        
        });
    }
    
    
    if(controller.Modules.includes('WIFI'))
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
    
    
  
};
//-----------------------------------------------------------------------------------------------------
$(document).ready(function(){

  lastVisibleContent = $('#welcome');

  controller.querySensorNames(); // получаем имена датчиков
  
  
  $( "#get_delta_button" ).button({
      icons: {
        primary: "ui-icon-arrowthickstop-1-s"
      }
    });
    
  $( "#save_delta_button" ).button({
      icons: {
        primary: "ui-icon-arrowthickstop-1-n"
      }
    }); 
    
  $( "#new_delta_button" ).button({
      icons: {
        primary: "ui-icon-document"
      }
    });
    
      $( "#phone_number" ).button({
      icons: {
        primary: "ui-icon-note"
      }
    }).hide().css('width','100%');
    
 $( "#wifi_menu" ).button({
      icons: {
        primary: "ui-icon-signal"
      }
    }).hide().css('width','100%');       
    
    $('#delta_index1').forceNumericOnly();     
    $('#delta_index2').forceNumericOnly();
    
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