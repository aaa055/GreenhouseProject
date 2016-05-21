//-----------------------------------------------------------------------------------------------------
// классы для работы с составными командами
//-----------------------------------------------------------------------------------------------------
var CompositeActionsNames = new Array('закрыть форточки','открыть форточки','выключить досветку','включить досветку','выставить на пине низкий уровень','выставить на пине высокий уровень');
//-----------------------------------------------------------------------------------------------------
var CompositeCommand = function(action, param)
{
  this.ParentList = null;
  this.Action = action;
  this.Param = param;
  
  return this;
}
//-----------------------------------------------------------------------------------------------------
CompositeCommand.prototype.equalsTo = function(cc)
{
  return cc.Action == this.Action && cc.Param == this.Param;
}
//-----------------------------------------------------------------------------------------------------
CompositeCommand.prototype.getDescription = function()
{
  if(!this.ParentList)
    return this.Action;
    
  return this.ParentList.getCommandDescription(this);
}
//-----------------------------------------------------------------------------------------------------
CompositeCommand.prototype.remove = function()
{
  if(!this.ParentList)
    return;
    
  this.ParentList.List.remove(this);
}
//-----------------------------------------------------------------------------------------------------
var CompositeCommandsList = function(name, idx)
{
  this.Name = name;
  this.Index = idx;
  
  this.List = new Array();
  
  return this;
}
//-----------------------------------------------------------------------------------------------------
CompositeCommandsList.prototype.Add = function(cc)
{
  var canAdd = true;
  
  for(var i=0;i<this.List.length;i++)
  {
    var dt = this.List[i];
    if(dt.equalsTo(cc))
    {
      canAdd = false;
      break;
    }
  }
  
  if(canAdd)
  {
    cc.ParentList = this;
    this.List.push(cc);
  }

  return canAdd;
}
//-----------------------------------------------------------------------------------------------------
CompositeCommandsList.prototype.getCommandDescription = function(cc)
{
  var idx = this.List.indexOf(cc);
  if(idx != -1)
    return CompositeActionsNames[cc.Action];
    
  return cc.Action;
}
//-----------------------------------------------------------------------------------------------------
CompositeCommandsList.prototype.Clear = function()
{
  this.List = new Array();
}
//-----------------------------------------------------------------------------------------------------
var CompositeCommands = function()
{
  this.List = new Array();
  
  return this;
}
//-----------------------------------------------------------------------------------------------------
CompositeCommands.prototype.getMaxListIndex = function()
{
  var result = 0;
  for(var i=0;i<this.List.length;i++)
  {
    var ccList = this.List[i];
    result = Math.max(result,ccList.Index);
  }
  
  return result;
}
//-----------------------------------------------------------------------------------------------------
CompositeCommands.prototype.addNewList = function(list_name)
{
  var idx = this.getMaxListIndex() + 1;
  
  var cc = new CompositeCommandsList(list_name,idx);
  this.List.push(cc);
  
  return cc;
}
//-----------------------------------------------------------------------------------------------------
CompositeCommands.prototype.remove = function(list_idx)
{
 for(var i=0;i<this.List.length;i++)
    {
      var ccList = this.List[i];
      if(ccList.Index == list_idx)
      {
        this.List.remove(ccList);
        break;
      }
    }
}
//-----------------------------------------------------------------------------------------------------
CompositeCommands.prototype.Add = function(list_idx,list_name,cc_action,cc_param)
{
    for(var i=0;i<this.List.length;i++)
    {
      var ccList = this.List[i];
      if(ccList.Index == list_idx)
      {
        if(list_name != '')
          ccList.Name = list_name;
                    
        ccList.Add(new CompositeCommand(cc_action,cc_param));
        return;
      }
    } // for
    
    var newList = new CompositeCommandsList(list_name, list_idx);
    newList.Add(new CompositeCommand(cc_action,cc_param));
    
    this.List.push(newList);
}
//-----------------------------------------------------------------------------------------------------
CompositeCommands.prototype.Clear = function()
{
  this.List = new Array();
}
//-----------------------------------------------------------------------------------------------------
CompositeCommands.prototype.buildRow = function(ccCommand, row_id, action, param, addActions)
{
        var view = this;

        var row = $('<div/>',{'class': 'row', id: row_id});
        
        var add_class = addActions ?  'action ' : 'ui-widget-header ';
        
        var actionCol = $('<div/>',{'class': add_class + 'row_item', id: 'action'}).appendTo(row);
        actionCol.html(action);
        
        add_class = addActions ?  'module_name ' : 'ui-widget-header ';
        
        var paramCol = $('<div/>',{'class': add_class + 'row_item', id: 'param'}).appendTo(row);
        paramCol.html(param);
        

                
        if(addActions)
        {
              var actions = $('<div/>',{'class': 'row_item actions', id: 'actions'}).appendTo(row);
              $('<div/>',{'class': 'action', title: 'Удалить'}).appendTo(actions).button({
      icons: {
        primary: "ui-icon-close"
      }, text: false
    }).click({row: row, command : ccCommand, view: view}, function(ev){              
                  ev.data.row.remove();
                  ev.data.command.remove();
              });                         
        }
        
   return row;

}
//-----------------------------------------------------------------------------------------------------
CompositeCommands.prototype.getList = function(list_index)
{
    for(var i=0;i<this.List.length;i++)
    {
      var ccList = this.List[i];
      if(ccList.Index == list_index)
        return ccList;
    }
    
    return null;
}
//-----------------------------------------------------------------------------------------------------
CompositeCommands.prototype.fillList = function(parentElement,list_index)
{
  var list = this.getList(list_index);
  
  $(parentElement).empty();
  
      
  if(!list || !list.List.length)
    return;
  
    var header = this.buildRow(
        null
      , 'header'
      , "Действие"
      , "Параметр"
      , false
      );
      
   header.appendTo(parentElement);
   
   for(var i=0;i<list.List.length;i++)
   {
      var cc = list.List[i];
    
      var child_id = $(parentElement).attr('id') + i;
      
      var row = this.buildRow(
          cc
        , child_id
        , cc.getDescription()
        , cc.Param
        , true
        );
        
        row.appendTo(parentElement);
        
     
   } // for
  
  
}
//-----------------------------------------------------------------------------------------------------


