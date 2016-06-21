//-----------------------------------------------------------------------------------------------------
// Классы для работы с СМС
//-----------------------------------------------------------------------------------------------------
SMS_BUILD_COMMANDS = [
  ['CTSET=STATE|WINDOW|ALL|OPEN',false,'Открыть окна','']
, ['CTSET=STATE|WINDOW|ALL|CLOSE',false,'Закрыть окна','']
, ['CTSET=LIGHT|ON',false,'Включить досветку','']
, ['CTSET=LIGHT|OFF',false,'Выключить досветку','']
, ['CTSET=WATER|ON',false,'Включить полив','']
, ['CTSET=WATER|OFF',false,'Выключить полив','']
, ['CTSET=PIN|{0}|ON',true,'Выставить на пинах высокий уровень','Номера пинов, через запятую:']
, ['CTSET=PIN|{0}|OFF',true,'Выставить на пинах низкий уровень','Номера пинов, через запятую:']
, ['CTSET=CC|EXEC|{0}',true,'Выполнить составную команду','Индекс составной команды:']
, ['CTSET=ALERT|RULE_STATE|ALL|ON',false,'Включить все правила','']
, ['CTSET=ALERT|RULE_STATE|ALL|OFF',false,'Выключить все правила','']
, ['CTSET=0|AUTO',false,'Перевести контроллер в автоматический режим работы','']

];
//-----------------------------------------------------------------------------------------------------
// конструктор нового СМС
var CustomSMS = function(smsText,answerText,command)
{
  this.SMSText = smsText;
  this.AnswerText = answerText;
  this.Command = command;
  
  return this;
}
//-----------------------------------------------------------------------------------------------------
CustomSMS.prototype.toUCS2 = function(str)
{
 var tmp = unescape( encodeURIComponent( str ) );
 var result = '';
 for(var i=0;i<tmp.length;i++)
 {
   var code = tmp.charCodeAt(i).toString(16).toUpperCase();
   
   if(code.length < 2)
    code = "0" + code;
    
    result += code;
 }
 
 return result;
}
//-----------------------------------------------------------------------------------------------------
// возвращает строку для добавления SMS в контроллер
CustomSMS.prototype.getSMSCommand = function()
{
  var result = this.toUCS2(this.SMSText) + '|' + this.toUCS2(this.AnswerText) + '|' + this.Command;
  return result;
}
//-----------------------------------------------------------------------------------------------------
// список SMS
//-----------------------------------------------------------------------------------------------------
var SMSList = function()
{
  this.List = new Array();
  return this;
}
//-----------------------------------------------------------------------------------------------------
// добавляет SMS в список
SMSList.prototype.Add = function(smsText,answerText,command)
{

  for(var i=0;i<this.List.length;i++)
  {
    var s = this.List[i];
    if(s.SMSText == smsText)
    {
      s.AnswerText = answerText;
      s.Command = command;
      return s;
    }
  } // for


  var sms = new CustomSMS(smsText,answerText,command);
  this.List.push(sms);
  
  return sms;
}
//-----------------------------------------------------------------------------------------------------
// удаляет все SMS
SMSList.prototype.Clear = function()
{
  this.List = new Array();
}
//-----------------------------------------------------------------------------------------------------