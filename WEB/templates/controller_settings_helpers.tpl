{* Smarty *}


<script type="text/javascript" src="js/deltas.js"></script>
<script type="text/javascript" src="js/deltas_view.js"></script>
<script type="text/javascript" src="js/cc.js"></script>
<script type="text/javascript" src="js/water_settings.js"></script>
<script type="text/javascript" src="js/rules.js"></script>


<script type='text/javascript'>
//-----------------------------------------------------------------------------------------------------
// наш контроллер
var controller = new Controller({$selected_controller.controller_id},'{$selected_controller.controller_name}','{$selected_controller.controller_address}');

var deltaList = new DeltaList();
var deltaView = new DeltaView(controller, deltaList);

var compositeCommands = new CompositeCommands(); // список составных команд
var wateringSettings = new WateringSettings(); // настройки полива
var rulesList = new RulesList(); // список правил из контроллера

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
function showMessage(message)
{
  $('#message_dialog_message').html(message);

  $("#message_dialog").dialog({modal:true, buttons: [
    {text: "ОК", click: function(){$(this).dialog("close");} }
  ] });     
}
//-----------------------------------------------------------------------------------------------------
function promptMessage(message, yesFunc, cancelFunc)
{
  $('#prompt_dialog_message').html(message);

  $("#prompt_dialog").dialog({modal:true, buttons: [
    {text: "ОК", click: function(){$(this).dialog("close"); if(yesFunc) yesFunc(); } },
    {text: "Отмена", click: function(){$(this).dialog("close");  if(cancelFunc) cancelFunc(); } }
  ] });     
}
//-----------------------------------------------------------------------------------------------------
// устанавливаем дату/время для контроллера
function setControllerTime()
{
  promptMessage('Установить дату/время контроллера на текущее время компьютера?',
  function() {
    
    controllerInternalDate = new Date();
    
    var cmd = '0|DATETIME|' + formatDateTime(controllerInternalDate);
    controller.queryCommand(false,cmd);
    
  }

  );
}
//-----------------------------------------------------------------------------------------------------
// добавляем дельту
function newDelta()
{

  if(deltaList.List.length >= MAX_DELTAS)
  {
    showMessage("Достигнуто максимальное количество дельт: " + MAX_DELTAS);
    return;
  }
  
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
        showMessage('Такая дельта уже существует!');
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
    
  $('#reset_controller_link, #controller_time_button').toggle(is_online);  
  
  
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
// редактируем настройки канала
function editWateringChannel(channel, row)
{

      var lst = $('#water_channels_days').find('div #watering_channel_day');

      for(var i=0;i<lst.length;i++)
      {
         var elem = lst.get(i);
          var dayMask = parseInt(elem.value);
          elem.checked = (channel.WateringDays & dayMask) == dayMask;
      } // for
      
      $('#watering_start_hour').val(channel.StartTime); 
      $('#watering_time').val(channel.WateringTime); 



 $("#water_channel_settings_dialog").dialog({modal:true, buttons: [{text: "Изменить", click: function(){

      $(this).dialog("close");
      
      var watering_start_hour = parseInt($('#watering_start_hour').val());
      if(isNaN(watering_start_hour))
        watering_start_hour = channel.StartTime;
      
      watering_start_hour = Math.abs(watering_start_hour);
      if(watering_start_hour > 23)
        watering_start_hour = 23;
        
      channel.StartTime = watering_start_hour;
      
      
      var watering_time = parseInt($('#watering_time').val());
      if(isNaN(watering_time))
        watering_time = channel.WateringTime;
        
       channel.WateringTime = Math.abs(watering_time);
        
       channel.WateringDays = 0;
        
       var lst = $('#water_channels_days').find('div #watering_channel_day');

        for(var i=0;i<lst.length;i++)
        {
           var elem = lst.get(i);
           if(elem.checked)
           {
            var dayMask = parseInt(elem.value);
             channel.WateringDays |= dayMask;
           }
           
        } // for 

      row.find('#water_channel_days').html(channel.getWateringDaysString(true));
      row.find('#water_channel_start').html(channel.StartTime + ' ч');
      row.find('#water_channel_time').html(channel.WateringTime + ' мин');
  
  } }
  
  , {text: "Отмена", click: function(){$(this).dialog("close");} }
  ] });
}
//-----------------------------------------------------------------------------------------------------
// добавляет строку в таблицу настроек каналов
function addWaterChannelRow(parentElement, channel, num)
{
  if(!channel)
  {
    // запросили заголовок
    var row = $('<div/>',{'class': 'row', id: 'water_channels_header'});
    $('<div/>',{'class': 'row_item ui-widget-header'}).html("#").appendTo(row);
    $('<div/>',{'class': 'row_item ui-widget-header'}).html("Дни недели").appendTo(row);
    $('<div/>',{'class': 'row_item ui-widget-header'}).html("Начало").appendTo(row);
    $('<div/>',{'class': 'row_item ui-widget-header'}).html("Время").appendTo(row);
    
    row.appendTo(parentElement);
    return;
  }
  
    var row = $('<div/>',{'class': 'row', id: 'water_channel_' + num});
    $('<div/>',{'class': 'row_item', id: 'water_channel_index'}).html(num + 1).appendTo(row);
    $('<div/>',{'class': 'row_item', id: 'water_channel_days'}).html(channel.getWateringDaysString(true)).appendTo(row);
    $('<div/>',{'class': 'row_item', id: 'water_channel_start'}).html(channel.StartTime + ' ч').appendTo(row);
    $('<div/>',{'class': 'row_item', id: 'water_channel_time'}).html(channel.WateringTime + ' мин').appendTo(row);
    
    var actions = $('<div/>',{'class': 'row_item actions', id: 'actions'}).appendTo(row);
    
    $('<div/>',{'class': 'action', title: 'Редактировать настройки канала'}).appendTo(actions).button({
      icons: {
        primary: "ui-icon-pencil"
      }, text: false
    }).click({row: row, channel : channel, channel_index: num}, function(ev){
              
              editWateringChannel(ev.data.channel,ev.data.row);
              
      
              });    
    
    row.appendTo(parentElement);
  
}
//-----------------------------------------------------------------------------------------------------
// заполняем таблицу настроек каналов полива
function fillWaterChannelsList()
{
  addWaterChannelRow('#WATER_CHANNELS_LIST', null);
  
  for(var i=0;i<wateringSettings.Channels.length;i++)
  {
    var channel = wateringSettings.Channels[i];
    addWaterChannelRow('#WATER_CHANNELS_LIST', channel, i);
  } // for
}
//-----------------------------------------------------------------------------------------------------
// добавляет строку в таблицу правил
function addRuleRow(parentElement, rule, num)
{
  if(!rule)
  {
    // запросили заголовок
    var row = $('<div/>',{'class': 'row', id: 'rules_list_header'});
    $('<div/>',{'class': 'row_item ui-widget-header'}).html("#").appendTo(row);
    $('<div/>',{'class': 'row_item ui-widget-header'}).html("Имя").appendTo(row);
    $('<div/>',{'class': 'row_item ui-widget-header'}).html("Начало").appendTo(row);
    $('<div/>',{'class': 'row_item ui-widget-header'}).html("Активно").appendTo(row);
    $('<div/>',{'class': 'row_item ui-widget-header'}).html("Следим за").appendTo(row);
    $('<div/>',{'class': 'row_item ui-widget-header'}).html("Действие").appendTo(row);
    
    row.appendTo(parentElement);
    return;
  }
  
    var row = $('<div/>',{'class': 'row', id: 'rule_' + num});
    $('<div/>',{'class': 'row_item', id: 'rule_index'}).html(num + 1).appendTo(row);
    $('<div/>',{'class': 'row_item', id: 'rule_name'}).html(rule.Name).appendTo(row);
    $('<div/>',{'class': 'row_item', id: 'rule_start'}).html(rule.StartTime + ' ч').appendTo(row);
    $('<div/>',{'class': 'row_item', id: 'rule_time'}).html(rule.WorkTime + ' мин').appendTo(row);
    $('<div/>',{'class': 'row_item', id: 'rule_target'}).html(rule.getTargetDescription()).appendTo(row);
    $('<div/>',{'class': 'row_item', id: 'rule_command'}).html(rule.getTargetCommandDescription()).appendTo(row);
    
    var actions = $('<div/>',{'class': 'row_item actions', id: 'actions'}).appendTo(row);
    
    $('<div/>',{'class': 'action', title: 'Редактировать правило'}).appendTo(actions).button({
      icons: {
        primary: "ui-icon-pencil"
      }, text: false
    }).click({row: row, rule : rule, rule_index: num}, function(ev){
              
             newRule(ev.data.rule,ev.data.row);
              
      
              });
              
    $('<div/>',{'class': 'action', title: 'Удалить правило'}).appendTo(actions).button({
      icons: {
        primary: "ui-icon-close"
      }, text: false
    }).click({row: row, rule : rule}, function(ev){
              
                ev.data.row.remove();
                rulesList.Rules.remove(ev.data.rule);
                
              });                  
    
    row.appendTo(parentElement);
    
  
}
//-----------------------------------------------------------------------------------------------------
// создаёт новое правило
function newRule(editedRule, editedRow)
{

  if(rulesList.Rules.length >= MAX_RULES)
  {
    showMessage('Достигнуто максимальное количество правил!');
    return;
  }

  $('#rule_target_input').val('_').trigger('change');
  $('#rule_name_input').val('');
  $('#rule_name_input').removeAttr('disabled');
  $('#rule_start_time_input').val('0');
  $('#rule_work_time_input').val('0');
  $('#rule_sensor_index_input').val('0');
  $('#rule_sensor_value_box').val('0');
  $('#linked_rules_box').empty();
  $('#rule_sensor_value_input').val('');
  $('#rule_action_input').val('0').trigger('change');
  $('#rule_additional_param_input').val('');

  
  var newRuleRequested = !editedRule;
  
  
      
  for(var i=0;i<rulesList.Rules.length;i++)
  {
    var rule = rulesList.Rules[i];
    
    if(editedRule && editedRule.Name == rule.Name)
      continue;
      
     
    
    var row = $('<div/>');
    var chb = $('<input/>',{type: 'checkbox', id: 'linked_rule_index', value: i});
    chb.appendTo(row);
    
    if(editedRule && editedRule.LinkedRules.includes(rule.Name))
      chb.get(0).checked = true;
    
    $('<label/>').appendTo(row).html(rule.Name + ': ' + rule.getTargetCommandDescription());
    
    row.appendTo('#linked_rules_box');
  }
  
  if(editedRule)
  {
    // запросили редактирование правила, заполняем поля в форме
    $('#rule_name_input').attr('disabled','disabled');
    $('#rule_name_input').val(editedRule.Name);
    
    var targ = editedRule.Target.toString();
    if(targ == '0')
      targ = '_';
    
    $('#rule_target_input').val(targ).trigger('change');
    $('#rule_module_select').val(editedRule.ModuleName).trigger('change');
    $('#rule_start_time_input').val(editedRule.StartTime);
    $('#rule_work_time_input').val(editedRule.WorkTime);
    $('#rule_sensor_index_input').val(editedRule.SensorIndex);
    $('#rule_sensor_operand').val(editedRule.Operand);
    $('#rule_sensor_value_input').val(editedRule.AlertCondition);
    $('#rule_pin_state_input').val(editedRule.AlertCondition);
    $('#rule_action_input').val(editedRule.getTargetCommandIndex()).trigger('change');
    $('#rule_additional_param_input').val(editedRule.getAdditionalParam());
    
  }  
      


 $("#rule_edit_dialog").dialog({modal:true, width:600, buttons: [{text: "OK", click: function(){
  
      var ruleName = $('#rule_name_input').val();
      if(ruleName == '')
      {
        showMessage('Укажите имя правила!');
        $('#rule_name_input').focus();
        return;
      }
      
      ruleName = ruleName.toUpperCase();
      
      var ruleTarget = $('#rule_target_input').val();
      var ruleStartTime = parseInt($('#rule_start_time_input').val());
      if(isNaN(ruleStartTime))
        ruleStartTime = 0;
        
      var ruleWorkTime = parseInt($('#rule_work_time_input').val());
      if(isNaN(ruleWorkTime))
        ruleWorkTime = 0;
        
      var sensorIndex = parseInt($('#rule_sensor_index_input').val());
      if(isNaN(sensorIndex))
        sensorIndex = 0;
        
      var targetPinState = $('#rule_pin_state_input').val();
      
      var moduleName = $('#rule_module_select').val();
      
      var operand = $('#rule_sensor_operand').val();
      var alertCondition = $('#rule_sensor_value_input').val();
      
      var rule_action_input = parseInt($('#rule_action_input').val());
      var rule_additional_param_input = $('#rule_additional_param_input').val();
      
      var linked_rules = '';
      
      var lst = $('#linked_rules_box').find('div #linked_rule_index');

      for(var i=0;i<lst.length;i++)
      {
         var elem = lst.get(i);
         if(elem.checked)
         {
          if(linked_rules != '')
            linked_rules += ',';
            
            linked_rules += rulesList.Rules[elem.value].Name;
         }
     }
     
     if(linked_rules == '')
      linked_rules = '_';
        
      
      if(ruleTarget != '_')
      {
        // если следим за чем-то, то проверяем параметры
        if(ruleTarget == 'PIN')
        {
            alertCondition = '1';
            moduleName = '0';
            operand = targetPinState;
        }
        else
        {
          if(alertCondition == '')
          {
            showMessage('Введите показания датчика!');
            return;
          }
        }
      }
      else
      {
        // когда ни за чем не следим, ссылаемся на модуль "0"
          moduleName = '0';
          alertCondition = '0';
          operand = '>';
          
      }
      
       if(rule_action_input > 3 && rule_additional_param_input == '')
       {
        showMessage('Укажите дополнительные параметры!');
        $('#rule_additional_param_input').focus();
        return;
       }
       
       var targetCommand = rulesList.buildTargetCommand(rule_action_input,rule_additional_param_input);
       
       
       // вроде всё проверили, пытаемся посмотреть
       var fullRuleString = 'dummy|dummy|dummy|' + ruleName + '|' + moduleName + '|' + ruleTarget + '|' + sensorIndex + '|' + operand + '|' + alertCondition + '|' + 
       ruleStartTime + '|' + ruleWorkTime + '|' + linked_rules + '|' + targetCommand;
       
       
       if(newRuleRequested)
       {
        // новое правило
        editedRule = rulesList.Add();
        editedRule.Construct(fullRuleString.split('|'));
        addRuleRow('#RULES_LIST', editedRule, rulesList.Rules.length - 1);
       }
       else
       {
        // редактируем правило
        editedRule.Construct(fullRuleString.split('|'));
        editedRow.find('#rule_name').html(editedRule.Name);
        editedRow.find('#rule_start').html(editedRule.StartTime + ' ч');
        editedRow.find('#rule_time').html(editedRule.WorkTime + ' мин');
        editedRow.find('#rule_target').html(editedRule.getTargetDescription());
        editedRow.find('#rule_command').html(editedRule.getTargetCommandDescription());
       }
      

      $(this).dialog("close");
  
  } }
  
  , {text: "Отмена", click: function(){$(this).dialog("close");} }
  ] });     
  
}
//-----------------------------------------------------------------------------------------------------
// сохраняем список правил
function saveRulesList()
{

    $("#data_requested_dialog" ).dialog({
                dialogClass: "no-close",
                modal: true,
                closeOnEscape: false,
                draggable: false,
                resizable: false,
                buttons: []
              });
              
     var rulesToProcess = rulesList.Rules.length;
     var processedRules = 0;
              
     controller.queryCommand(false,"ALERT|RULE_DELETE|ALL",function(obj,answer){
     
          for(var i=0;i<rulesList.Rules.length;i++)
          {
            var rule = rulesList.Rules[i];
            var cmd = 'ALERT|RULE_ADD|' + rule.getAlertRule();
            controller.queryCommand(false,cmd, function(obj, processResult){
            
              processedRules++;
              if(processedRules >= rulesToProcess)
              {
                  controller.queryCommand(false,"ALERT|SAVE", function() {
                
                    $("#data_requested_dialog" ).dialog('close');
                
                });
              }
            
            });
          } // for
          
          
          if(!rulesToProcess)
          {
             controller.queryCommand(false,"ALERT|SAVE", function() {
                
                    $("#data_requested_dialog" ).dialog('close');
                
                });
          }
     
     });

}
//-----------------------------------------------------------------------------------------------------
// заполняем список правил
function fillRulesList()
{
  $('#RULES_LIST').html('');
  addRuleRow('#RULES_LIST', null);
  
  for(var i=0;i<rulesList.Rules.length;i++)
  {
    var rule = rulesList.Rules[i];
    addRuleRow('#RULES_LIST', rule, i);
  } // for  
}
//-----------------------------------------------------------------------------------------------------
// сохраняем настройки для всех каналов полива одновременно
var wateringTotalCommands = 1;
var wateringProcessedCommands = 0;

function saveAllChannelsWateringOptions(watering_option)
{
  wateringSettings.WateringOption = watering_option;
  wateringTotalCommands = 1;
  wateringProcessedCommands = 0;
    
  if(watering_option == 1) // все каналы сразу
  {
      var wTime = parseInt($('#all_watering_time').val());
      if(!isNaN(wTime))
        wateringSettings.WateringTime = Math.abs(wTime);
      else
        $('#all_watering_time').val(wateringSettings.WateringTime);
        
      var wStart = parseInt($('#all_watering_start_hour').val());
      if(!isNaN(wStart))
      {
        wStart = Math.abs(wStart);
        if(wStart > 23)
          wStart = 23;
          
          $('#all_watering_start_hour').val(wStart);
          wateringSettings.StartTime = wStart;
          
      } // if(!isNaN(wStart))
      else
        $('#all_watering_start_hour').val(wateringSettings.StartTime);
      

      // теперь сохраняем дни недели
      wateringSettings.WateringDays = 0;
      
      var lst = $('#all_watering_channels_days').find('div #all_watering_channels_day');

      for(var i=0;i<lst.length;i++)
      {
         var elem = lst.get(i);
         if(elem.checked)
         {
          var dayMask = parseInt(elem.value);
           wateringSettings.WateringDays |= dayMask;
         }
      } // for 

   } // watering_option == 1 
   
    $("#data_requested_dialog" ).dialog({
                dialogClass: "no-close",
                modal: true,
                closeOnEscape: false,
                draggable: false,
                resizable: false,
                buttons: []
              });   
   
   if(watering_option == 2)
   {
      wateringTotalCommands += wateringSettings.Channels.length;
    
     
     for(var i=0;i<wateringSettings.Channels.length;i++)
     {
        var channel = wateringSettings.Channels[i];
        var cmd = "WATER|CH_SETT|" + i + '|' + channel.WateringDays + '|' + channel.WateringTime + '|' + channel.StartTime;
        
        controller.queryCommand(false,cmd,function(obj,answer){
                
                      wateringProcessedCommands++;
                      
                      if(wateringProcessedCommands >= wateringTotalCommands)
                        $("#data_requested_dialog" ).dialog('close');
                                      
                });         
        
     } // for

    
   } // if(watering_option == 2)
   
   wateringSettings.TurnOnPump = $('#turn_on_pump').get(0).checked ? 1 : 0;
              
    // теперь можем загружать всё это добро в контроллер
    var cmd = "WATER|T_SETT|" + wateringSettings.WateringOption + '|' +   wateringSettings.WateringDays + '|' +
               wateringSettings.WateringTime + '|' + wateringSettings.StartTime + '|' + wateringSettings.TurnOnPump;
                                  
                controller.queryCommand(false,cmd,function(obj,answer){
                
                      wateringProcessedCommands++;
                      
                      if(wateringProcessedCommands >= wateringTotalCommands)
                        $("#data_requested_dialog" ).dialog('close');
                                      
                }); 
    
    
    
}
//-----------------------------------------------------------------------------------------------------
// сохраняем настройки полива
function saveWateringSettings()
{
  var watering_option = parseInt($('#watering_option').val());  
   saveAllChannelsWateringOptions(watering_option);
}
//-----------------------------------------------------------------------------------------------------
function doRequestRulesList()
{
  $("#data_requested_dialog" ).dialog({
                dialogClass: "no-close",
                modal: true,
                closeOnEscape: false,
                draggable: false,
                resizable: false,
                buttons: []
              });
              
    requestRulesList(function(){
    
      $("#data_requested_dialog" ).dialog('close');
    
    });
              
}
//-----------------------------------------------------------------------------------------------------
// запрашивает список правил
function requestRulesList(doneFunc)
{
    rulesList.Clear();
    $('#RULES_LIST').html('');
    
    var rulesCnt = 0;
    
     controller.queryCommand(true,'ALERT|RULES_CNT',function(obj,answer){
           
          
          if(answer.IsOK)
          {
            rulesCnt = parseInt(answer.Params[2]);
            for(var i=0;i<rulesCnt;i++)
            {
                var cmd = 'ALERT|RULE_VIEW|' + i;
                
                controller.queryCommand(true,cmd,function(obj,ruleSettings){
                
                  if(ruleSettings.IsOK)
                  {
                    var rule = rulesList.Add();
                    rule.Construct(ruleSettings.Params);
                    
                    if(rulesCnt == rulesList.Rules.length)
                    {
                      fillRulesList();
                      
                      if(doneFunc)
                        doneFunc();
                    }
                  } // is ok
                
                });
            } // for
            
            if(!rulesCnt)
            {
              if(doneFunc)
                doneFunc();
            } // !rulesCnt
            
          } // is ok
        
        });
    

}
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
    
    // запрашиваем список правил
    requestRulesList(function(){ 
      
      $('#RULES_MENU').toggle(true);
    
    });
    
    
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
              $('#cc_lists').append($('<option/>',{value: ccList.Index }).text('#' + i + ' - ' + ccList.Name));
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
    
    if(controller.Modules.includes('WATER')) // если есть модуль полива в прошивке
    {
       
       //$('#WATER_MENU').toggle(true);
       
       var channelsToRetrieve = 0;
       var retrievedChannels = 0;
       
        controller.queryCommand(true,'WATER|T_SETT',function(obj,answer){
        
              //$('#WATER_MENU').toggle(answer.IsOK);
              
              if(answer.IsOK)
              {              
              
                  wateringSettings = new WateringSettings(answer.Params[2],answer.Params[3],answer.Params[4],answer.Params[5],answer.Params[6]);
                  
                  $('#all_watering_start_hour').val(wateringSettings.StartTime);
                  $('#all_watering_time').val(wateringSettings.WateringTime);
                  
                  var lst = $('#all_watering_channels_days').find('div #all_watering_channels_day');

                  for(var i=0;i<lst.length;i++)
                  {
                     var elem = lst.get(i);
                     var dayMask = parseInt(elem.value);
                     elem.checked = (wateringSettings.WateringDays & dayMask) == dayMask;
                      
                  } // for
                  
                  controller.queryCommand(true,'WATER|CHANNELS',function(obj,cntChannels){
                  
                      if(cntChannels.IsOK)
                      {
                        channelsToRetrieve = parseInt(cntChannels.Params[2]);
                        
                        if(!channelsToRetrieve)
                          return;
                          
                        for(var i=0;i<channelsToRetrieve;i++)
                        {
                            controller.queryCommand(true,'WATER|CH_SETT|' + i,function(obj,channelData){
                            retrievedChannels++;
                            
                                if(channelData.IsOK)
                                {
                                  var channel = new WaterChannelSettings(channelData.Params[3], channelData.Params[4], channelData.Params[5]);
                                  wateringSettings.Add(channel);
                                } // if
                            
                                if(retrievedChannels >= channelsToRetrieve)
                                {
                                  $('#WATER_MENU').toggle(true);
                                  $('#watering_option').val(wateringSettings.WateringOption);
                                  $('#watering_option').trigger('change');
                                  
                                  $('#turn_pump_box').toggle(wateringSettings.WateringOption > 0);
                                  
                                  $('#turn_on_pump').get(0).checked = wateringSettings.TurnOnPump == 1;
                                  
                                  fillWaterChannelsList(); // заполняем таблицу настроек каналов полива 
                                } // if
                            
                            });
                        } // for
                      } // if(cntChannels.IsOK)
                  
                  });
              } // answer.IsOK
        
        });
    } // water
    
    
  
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
      $('#cc_lists').append($('<option/>',{value: newList.Index }).text('#' + (newList.Index - 1) + ' - ' + newList.Name));
      
      $('#cc_lists').val(newList.Index);
      $('#cc_lists').trigger('change');
    
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
    showMessage('Создайте список составных команд!');
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
function addRuleModuleToList(moduleName)
{
  $('#rule_module_select').append($('<option/>',{value: moduleName}).text(ModuleNamesBindings[moduleName]));
}
//-----------------------------------------------------------------------------------------------------
// выполняем команду, введённую пользователем
function execCommandPrompt()
{
  var cmd = $('#command_prompt_text').val();
  cmd = cmd.toUpperCase();
 
  if(cmd == "")
  {
    showMessage("Введите текст команды!");
    return;
  }
  
  var getIdx = cmd.indexOf("CTGET=");
  var setIdx = cmd.indexOf("CTSET=");
  
  if((getIdx == -1 && setIdx == -1))
  {
    showMessage("Команда должна начинаться с CTGET= или CTSET=<p>Пожалуйста, напишите команду правильно.");
    return;
  }
  
  var isGet = getIdx != -1;
  cmd = cmd.substring(6);
  
  if(cmd.length < 1)
  {
    showMessage("Команда не должна быть пустой!");
    return;
  }
  
  $("#exec_command_button").button('disable');
  
    controller.queryCommand(isGet,cmd,function(obj,answer){
                                
                                  $("#exec_command_button").button('enable');
                                  $('#controller_answer_text').append(answer.RawData);
                                  
                                });  
  
  
}
//-----------------------------------------------------------------------------------------------------
$(document).ready(function(){

  lastVisibleContent = $('#welcome');

  controller.querySensorNames(); // получаем имена датчиков
  
  
  $('#rule_action_input').change(function(){
  
      var val = parseInt($(this).val());
      var ed = $('#rule_additional_param_input');
      ed.attr('placeholder','');
      $('#rule_additional_param').toggle(false);
      
      switch(val)
      {
        case 4:
        case 5:
          ed.attr('placeholder','номера пинов, через запятую');
          $('#rule_additional_param').toggle(true);
        break;
        
        case 6:
          ed.attr('placeholder','индекс составной команды');
          $('#rule_additional_param').toggle(true);
        break;
      } // switch
  });
  
  $('#rule_target_input').change(function(){
  
    var val = $(this).val();
    $('#rule_module_box').toggle(val != '_');
    $('#rule_index_box').toggle(val != '_');
    $('#rule_sensor_value_box').toggle(val != '_');
    $('#rule_pin_state_box').toggle(false);
    
    $('#rule_module_select').empty().val('');
    $('#rule_sensor_index_description').text('Индекс датчика:');
    
    switch(val)
    {
      case '_': // ни за чем не следим
      {
      
      }
      break;
      
      case 'TEMP': // следим за температурой
      {
        addRuleModuleToList('STATE');
        addRuleModuleToList('HUMIDITY');
        addRuleModuleToList('DELTA');        
      }
      break;
      
      case 'HUMIDITY': // следим за влажностью
      {
        addRuleModuleToList('HUMIDITY');
        addRuleModuleToList('DELTA');        
      }
      break;
      
      case 'LIGHT': // следим за освещенностью
      {
        addRuleModuleToList('LIGHT');
        addRuleModuleToList('DELTA');        
      }
      break;
      
      case 'SOIL': // следим за влажностью почвы
      {
        addRuleModuleToList('SOIL');
        addRuleModuleToList('DELTA');        
      }
      break;
      
      case 'PIN': // следим за уровнем пина
      {
        $('#rule_module_box').toggle(false);
        $('#rule_sensor_index_description').text('Номер пина:');
        $('#rule_sensor_value_box').toggle(false);
        $('#rule_pin_state_box').toggle(true);
      }
      break;
    } // switch
  
  });
  
  $('#command_prompt_text').keypress(function(e){
  
    if(e.which == 13)
    {
      execCommandPrompt();
    }
  
  });
  
  
  $( "#get_delta_button, #get_rules_button" ).button({
      icons: {
        primary: "ui-icon-arrowthickstop-1-s"
      }
    });
    
  $( "#save_delta_button, #save_cc_button, #save_watering_button, #save_rules_button" ).button({
      icons: {
        primary: "ui-icon-arrowthickstop-1-n"
      }
    }); 
    
  $( "#new_delta_button, #new_cc_button, #new_rule_button" ).button({
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
    
    $('#exec_command_button').button({
      icons: {
        primary: "ui-icon-play"
      }
    });
    exec_command_button
      
    
    $( "#controller_time_button" ).button({
      icons: {
        primary: "ui-icon-clock"
      }
    }).hide().css('width','100%');       
      
    
 $( "#wifi_menu" ).button({
      icons: {
        primary: "ui-icon-signal"
      }
    }).hide().css('width','100%');       
    
    $('#delta_index1, #delta_index2, #cc_param').forceNumericOnly();     

    $('#all_watering_start_hour, #all_watering_time').forceNumericOnly();
    $('#watering_start_hour, #watering_time').forceNumericOnly(); 

    $('#rule_work_time_input, #rule_sensor_index_input, #rule_start_time_input, #rule_sensor_value_input').forceNumericOnly();
    
    for(var i=0;i<CompositeActionsNames.length;i++)
    {
      $('#cc_type').append($('<option/>',{value: i}).text(CompositeActionsNames[i]));
    }
    
    $('#watering_option').change(function() {
    
      var watering_option = parseInt($(this).val());
      $('#watering_all_channels').toggle(false);
      $('#watering_separate_channels').toggle(false);
      $('#turn_pump_box').toggle(false);
      
      switch(watering_option)
      {
        case 1:
          $('#watering_all_channels').toggle(true);
          $('#turn_pump_box').toggle(true);
        break;
        
        case 2:
          $('#watering_separate_channels').toggle(true);
          $('#turn_pump_box').toggle(true);
        break;
      } // switch
    
    });
    
    
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