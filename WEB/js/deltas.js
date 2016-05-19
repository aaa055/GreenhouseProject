//-----------------------------------------------------------------------------------------------------
// классы для работы с дельтами
//-----------------------------------------------------------------------------------------------------
DeltaTypes = {'TEMP' : 'Температура', 'HUMIDITY' : 'Влажность', 'LIGHT' : 'Освещенность'}
//-----------------------------------------------------------------------------------------------------
var Delta = function(type,module1,idx1,module2,idx2)
{
  this.Type = type;

  this.ModuleName1 = module1;
  this.Index1 = idx1;

  this.ModuleName2 = module2;
  this.Index2 = idx2;
  
  return this;
}
//-----------------------------------------------------------------------------------------------------
Delta.prototype.equalsTo = function(delta)
{
  return this.Type == delta.Type &&
  ( 
  (this.ModuleName1 == delta.ModuleName1 && this.Index1 == delta.Index1
  && this.ModuleName2 == delta.ModuleName2 && this.Index2 == delta.Index2)
  || 
  (this.ModuleName1 == delta.ModuleName2 && this.Index1 == delta.Index2
  && this.ModuleName2 == delta.ModuleName1 && this.Index2 == delta.Index1)
  
  );
}
//-----------------------------------------------------------------------------------------------------
Delta.prototype.getDisplayName = function()
{
  return DeltaTypes[this.Type];
}
//-----------------------------------------------------------------------------------------------------
var DeltaList = function()
{
  this.List = new Array();
  return this;
}
//-----------------------------------------------------------------------------------------------------
DeltaList.prototype.Add = function(delta)
{
  var canAdd = true;
  
  for(var i=0;i<this.List.length;i++)
  {
    var dt = this.List[i];
    if(dt.equalsTo(delta))
    {
      canAdd = false;
      break;
    }
  }
  
  if(canAdd)
    this.List.push(delta);

  return canAdd;
}
//-----------------------------------------------------------------------------------------------------
