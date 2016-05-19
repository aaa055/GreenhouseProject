//-----------------------------------------------------------------------------------------------------
// класс представления списка дельт
//-----------------------------------------------------------------------------------------------------
var DeltaView = function(controller, list)
{
  this.OnDeleteDelta = null; // событие "запрошено удаление дельты"
  
  this.Controller = controller;
  this.List = list;
  return this;
}
//-----------------------------------------------------------------------------------------------------
DeltaView.prototype.buildRow = function(delta, row_id,typeName,mname1,index1,mname2,index2, addActions)
{
        var view = this;

        var row = $('<div/>',{'class': 'row', id: row_id});
        
        var add_class = addActions ?  'type ' : 'header ';
        
        var type = $('<div/>',{'class': add_class + 'row_item', id: 'type'}).appendTo(row);
        type.html(typeName);
        
        add_class = addActions ?  'module_name ' : 'header ';
        
        var module1 = $('<div/>',{'class': add_class + 'row_item', id: 'module1'}).appendTo(row);
        module1.html(mname1);
        
        add_class = addActions ?  'sensor_index ' : 'header ';
        
        var idx1 = $('<div/>',{'class': add_class + 'row_item', id : 'index1'}).appendTo(row);
        idx1.html(index1);
        
        add_class = addActions ?  'module_name ' : 'header ';

        var module2 = $('<div/>',{'class': add_class + 'row_item', id: 'module2'}).appendTo(row);
        module2.html(mname2);
        
         add_class = addActions ?  'sensor_index ' : 'header ';
        
        var idx2 = $('<div/>',{'class': add_class + 'row_item', id : 'index2'}).appendTo(row);
        idx2.html(index2);

                
        if(addActions)
        {
              var actions = $('<div/>',{'class': 'row_item actions', id: 'actions'}).appendTo(row);
              var editAction = $('<span/>',{'class': 'action pointer link'}).appendTo(actions).html("удалить").click({row: row, delta : delta, view: view}, function(ev){
              
                if(ev.data.view.OnDeleteDelta != null)
                {
                  var cont = ev.data.view.Controller;
                  ev.data.view.OnDeleteDelta(cont, ev.data.delta, ev.data.row);
                }
              
              
              });
        }
        
   return row;

}
//-----------------------------------------------------------------------------------------------------
DeltaView.prototype.fillList = function(parentElement)
{
  $(parentElement).html('');
    
  var view = this;
  
   var header = this.buildRow(
        null
      , 'header'
      , "Тип"
      , "Модуль 1"
      , "Датчик 1"
      , "Модуль 2"
      , "Датчик 2"
      , false
      );
      
   header.appendTo(parentElement);
  
  for(var i=0;i<this.List.List.length;i++)
  {
    var delta = this.List.List[i];

    var child_id = $(parentElement).attr('id') + i;
    
    var row = this.buildRow(
        delta
      , child_id,delta.getDisplayName()
      , ModuleNamesBindings[delta.ModuleName1]
      , this.Controller.SensorsNames.getMnemonicName(new Sensor(delta.Index1,delta.ModuleName1))
      , ModuleNamesBindings[delta.ModuleName2]
      , this.Controller.SensorsNames.getMnemonicName(new Sensor(delta.Index2,delta.ModuleName2))
      , true
      );
    
    /*
   
      // надо добавить в таблицу
        var row = $('<div/>',{'class': 'row', id: child_id});
        
        var type = $('<div/>',{'class': 'row_item module_name', id: 'type'}).appendTo(row);
        type.html(delta.getDisplayName());
        
        var module1 = $('<div/>',{'class': 'row_item module_name', id: 'module1'}).appendTo(row);
        module1.html(ModuleNamesBindings[delta.ModuleName1]);
        
        var idx1 = $('<div/>',{'class': 'row_item sensor_index', id : 'index1'}).appendTo(row);
        idx1.html(this.Controller.SensorsNames.getMnemonicName(new Sensor(delta.Index1,delta.ModuleName1)));

        var module2 = $('<div/>',{'class': 'row_item module_name', id: 'module2'}).appendTo(row);
        module2.html(ModuleNamesBindings[delta.ModuleName2]);
        
        var idx2 = $('<div/>',{'class': 'row_item sensor_index', id : 'index2'}).appendTo(row);
        idx2.html(this.Controller.SensorsNames.getMnemonicName(new Sensor(delta.Index2,delta.ModuleName2)));

                
        var actions = $('<div/>',{'class': 'row_item actions', id: 'actions'}).appendTo(row);
        var editAction = $('<span/>',{'class': 'action pointer link'}).appendTo(actions).html("удалить").click({row: row, delta : delta, view: view}, function(ev){
        
          if(ev.data.view.OnDeleteDelta != null)
          {
            var cont = ev.data.view.Controller;
            ev.data.view.OnDeleteDelta(cont, ev.data.delta, ev.data.row);
          }
        
        
        });
        
     */   
        
     
     row.appendTo(parentElement);
  
      
  } // for
}
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
