//-----------------------------------------------------------------------------------------------------
// класс представления для просмотра состояния контроллера
//-----------------------------------------------------------------------------------------------------
var View = function(controller)
{
  this.OnEditSensorName = null; // обработчик события "Запрошено редактирование мнемонического имени для датчика"
  this.Controller = controller;
  return this;
}
//-----------------------------------------------------------------------------------------------------
View.prototype.fillSensorsList = function(parentElement, list, add, pattern = {index : true, module: true, data: true})
{
  var arr = list.List;
  var view = this;
  for(var i=0;i<arr.length;i++)
  {
    var sensor = arr[i];

    var child_id = $(parentElement).attr('id') + sensor.ModuleName + sensor.Index;
    var chld = $(parentElement).find("#" + child_id);
    
    if(chld.length < 1)
    {
      // надо добавить в таблицу
        var row = $('<div/>',{'class': 'row', id: child_id});
        
        if(pattern.module)
        {
          var module = $('<div/>',{'class': 'row_item module_name', id: 'module'}).appendTo(row);
          module.html(ModuleNamesBindings[sensor.ModuleName]);
        }
        
        if(pattern.index)
        {
          var idx = $('<div/>',{'class': 'row_item sensor_index', id : 'index'}).appendTo(row);
          idx.html(this.Controller.SensorsNames.getMnemonicName(sensor));
        }

        if(pattern.data)
        {
            var dt = $('<div/>',{'class': 'row_item sensor_data', id: 'data_col'}).appendTo(row);
            
            var dataDiv = $('<div/>',{'class': 'sensor_data_float', id : 'data'}).appendTo(dt);
            
            if(sensor.HasData)
              dataDiv.html(sensor.Data + add);
            else
              dataDiv.html(NO_DATA);
              
              var updatingChart = $('<div/>',{'class': 'peity', id: 'inline_chart'}).appendTo(dt);
              updatingChart.peity("line", { width: 64, height: 20 });
              
              if(sensor.HasData)
              {
                var values = updatingChart.text().split(",");
                var normData = sensor.normalizedData();
                
                for(var nd=0;nd<20;nd++)
                  values.push(normData);
                  
                updatingChart.text(values.join(",")).change();
              }
              
        }
        
        var actions = $('<div/>',{'class': 'row_item actions', id: 'actions'}).appendTo(row);
        var editAction = $('<div/>',{'class': 'action', title: 'Редактировать имя датчика'}).appendTo(actions).button({
      icons: {
        primary: "ui-icon-pencil"
      }, text: false
    }).click({row: row, sensor : sensor, view: view}, function(ev){
        
          if(ev.data.view.OnEditSensorName != null)
          {
            var cont = ev.data.view.Controller;
            ev.data.view.OnEditSensorName(cont.SensorsNames.getMnemonicObject(ev.data.sensor), ev.data.sensor, ev.data.row);
          }
        
        
        });
        
     
         row.appendTo(parentElement);
     }
     else
     {
      // надо обновить таблицу
        var childElem = $(parentElement).find("#" + child_id);
        

        childElem.children('#module').html(ModuleNamesBindings[sensor.ModuleName]);
        childElem.children('#index').html(this.Controller.SensorsNames.getMnemonicName(sensor));
        
        var dataCol = childElem.children('#data_col');
        
        if(sensor.HasData)
          dataCol.children('#data').html(sensor.Data + add);
        else
          dataCol.children('#data').html(NO_DATA);
          
              var updatingChart = dataCol.children('#inline_chart');
              
              if(sensor.HasData)
              {
                var values = updatingChart.text().split(",");
                
                if(values.length > 20)
                  values.shift();
                                  
                values.push(sensor.normalizedData());
                updatingChart.text(values.join(",")).change();
              }          
     }
    
  }
}
//-----------------------------------------------------------------------------------------------------
View.prototype.fillTemperatureList = function(parentElement)
{
  this.fillSensorsList(parentElement,this.Controller.TemperatureList, ' &#0176;С');
}
//-----------------------------------------------------------------------------------------------------
View.prototype.fillHumidityList = function(parentElement)
{
  this.fillSensorsList(parentElement,this.Controller.HumidityList, '%');
}
//-----------------------------------------------------------------------------------------------------
View.prototype.fillLuminosityList = function(parentElement)
{
  this.fillSensorsList(parentElement,this.Controller.LuminosityList, ' люкс');
}
//-----------------------------------------------------------------------------------------------------
View.prototype.fillSoilMoistureList = function(parentElement)
{
  this.fillSensorsList(parentElement,this.Controller.SoilMoistureList, '%', {index : true, module: false, data: true});
}
//-----------------------------------------------------------------------------------------------------
