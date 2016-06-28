//-----------------------------------------------------------------------------------------------------
// Классы для работы с резервированием датчиков
//-----------------------------------------------------------------------------------------------------
RESERVATION_TYPES = {'TEMP' : 'Температура', 'LIGHT': 'Освещённость', 'HUMIDITY': 'Влажность', 'SOIL': 'Влажность почвы', 'PH': 'Показания pH'}
RESERVATION_MODULES = {'STATE' : 'модуля температур','LIGHT': 'модуля освещённости', 'HUMIDITY': 'модуля влажности', 'SOIL': 'модуля влажности почвы', 'PH': 'модуля показаний pH'}
//-----------------------------------------------------------------------------------------------------
var Reservation = function(moduleName,sensorIndex)
{
  this.ModuleName = moduleName;
  this.SensorIndex = sensorIndex;
  
  return this;
}
//-----------------------------------------------------------------------------------------------------
Reservation.prototype.getDisplayString = function()
{
  var result = 'датчик #' + this.SensorIndex + ' ' + RESERVATION_MODULES[this.ModuleName];
  return result;
}
//-----------------------------------------------------------------------------------------------------
var ReservationList = function(t)
{
  this.Type = t;
  this.Items = new Array();
  return this;
}
//-----------------------------------------------------------------------------------------------------
ReservationList.prototype.Add = function(mn,idx)
{
  var res = new Reservation(mn,idx);
  this.Items.push(res);
  
  return res;
}
//-----------------------------------------------------------------------------------------------------
ReservationList.prototype.getTypeString = function()
{
    return RESERVATION_TYPES[this.Type];
}
//-----------------------------------------------------------------------------------------------------
ReservationList.prototype.getSensorsString = function()
{
  var result = '';
  for(var i=0;i<this.Items.length;i++)
  {
    var resInfo = this.Items[i];
    var mn = controller.SensorsNames.getMnemonicName(new Sensor(resInfo.SensorIndex,resInfo.ModuleName));
    if(mn == resInfo.SensorIndex)
    {
      // ничего не нашли, сами формируем данные
      mn = resInfo.getDisplayString();
    }
    
    result += mn;
    result += '<br/>';
    
  } // for
  
  return result;
}
//-----------------------------------------------------------------------------------------------------
var Reservations = function()
{
  this.Items = new Array();
}
//-----------------------------------------------------------------------------------------------------
Reservations.prototype.Add = function(t)
{
  var lst = new ReservationList(t);
  this.Items.push(lst);
  
  return lst;
}
//-----------------------------------------------------------------------------------------------------
Reservations.prototype.Clear = function()
{
  this.Items = new Array();
}
//-----------------------------------------------------------------------------------------------------
