//-----------------------------------------------------------------------------------------------------
// Классы для работы с правилами
//-----------------------------------------------------------------------------------------------------
MAX_RULES = 20; // максимальное кол-во правил
RULE_TARGETS = {'_': 'нет слежения', 'TEMP': 'температурой', 'LIGHT' : 'освещенностью', 'HUMIDITY': 'влажностью', 'PIN': 'пином', 'SOIL': 'влажностью почвы'};
RULE_COMMANDS = {
  '.+STATE\\|WINDOW\\|.*\\|OPEN' : [0, 'открываем окна']
, '.+STATE\\|WINDOW\\|.*\\|CLOSE': [1, 'закрываем окна']
, '.+LIGHT\\|ON' : [2, 'включаем досветку']
, '.+LIGHT\\|OFF' : [3, 'выключаем досветку']
, '.+PIN\\|(.*)\\|ON' : [4, 'включаем пины']
, '.+PIN\\|(.*)\\|OFF' : [5, 'выключаем пины']
, '.+CC\\|EXEC\\|(.*)' : [6, 'выполняем составную команду']
};

RULE_BUILD_COMMANDS = [
  'CTSET=STATE|WINDOW|ALL|OPEN'
, 'CTSET=STATE|WINDOW|ALL|CLOSE'
, 'CTSET=LIGHT|ON'
, 'CTSET=LIGHT|OFF'
, 'CTSET=PIN|{0}|ON'
, 'CTSET=PIN|{0}|OFF'
, 'CTSET=CC|EXEC|{0}'

];
//-----------------------------------------------------------------------------------------------------
// конструктор нового правила
var AlertRule = function()
{
  return this;
}
//-----------------------------------------------------------------------------------------------------
// конструирует правило из параметров, полученных с контроллера
AlertRule.prototype.Construct = function(params)
{
  var startIdx = 3;
  this.Name = params[startIdx++];
  this.ModuleName = params[startIdx++];
  this.Target = params[startIdx++];
  this.SensorIndex = params[startIdx++];
  this.Operand = params[startIdx++];
  this.AlertCondition = params[startIdx++];
  this.StartTime = params[startIdx++];
  this.WorkTime = params[startIdx++];
  this.LinkedRules = params[startIdx++].split(',');
  
  this.TargetCommand = '';
  
  for(var i=startIdx;i<params.length;i++)
  {
    if(this.TargetCommand != '')
      this.TargetCommand += '|';
      
    this.TargetCommand += params[i];
  }
  
  
}
//-----------------------------------------------------------------------------------------------------
// возвращает строку с правилом
AlertRule.prototype.getAlertRule = function()
{
  var result = '' + this.Name + '|' + this.ModuleName + '|' + this.Target + '|' + this.SensorIndex +
  '|' + this.Operand + '|' + this.AlertCondition + '|' + this.StartTime + '|' + this.WorkTime + '|' +
  this.LinkedRules.join(',') + '|' + this.TargetCommand;
  
  return result;
}
//-----------------------------------------------------------------------------------------------------
AlertRule.prototype.getTargetDescription = function()
{
  return RULE_TARGETS[this.Target];
}
//-----------------------------------------------------------------------------------------------------
AlertRule.prototype.getTargetCommandIndex = function()
{
  for(var propName in RULE_COMMANDS)
  {
    
    var propVal = RULE_COMMANDS[propName];

    var reg = new RegExp(propName,'i');

    if(reg.test(this.TargetCommand))
    {
      return propVal[0];
    }
  }
  
  return this.TargetCommand;
}
//-----------------------------------------------------------------------------------------------------
AlertRule.prototype.getAdditionalParam = function()
{
  for(var propName in RULE_COMMANDS)
  {
    
    var propVal = RULE_COMMANDS[propName];

    var reg = new RegExp(propName,'i');

    if(reg.test(this.TargetCommand))
    {
      var res = reg.exec(this.TargetCommand);
      if(res.length > 1)
        return res[1];
    }
  }
  
  return '';
}
//-----------------------------------------------------------------------------------------------------
AlertRule.prototype.getTargetCommandDescription = function()
{
  for(var propName in RULE_COMMANDS)
  {
    
    var propVal = RULE_COMMANDS[propName];

    var reg = new RegExp(propName,'i');

    if(reg.test(this.TargetCommand))
    {
      return propVal[1];
    }
  }
  
  return this.TargetCommand;
}
//-----------------------------------------------------------------------------------------------------
// список правил
//-----------------------------------------------------------------------------------------------------
var RulesList = function()
{
  this.Rules = new Array();
  return this;
}
//-----------------------------------------------------------------------------------------------------
// добавляет правило в список
RulesList.prototype.Add = function()
{
  var rule = new AlertRule();
  this.Rules.push(rule);
  
  return rule;
}
//-----------------------------------------------------------------------------------------------------
// удаляет все правила
RulesList.prototype.Clear = function()
{
  this.Rules = new Array();
}
//-----------------------------------------------------------------------------------------------------
RulesList.prototype.buildTargetCommand = function(idx,param)
{
  var result = '';
  var pattern = RULE_BUILD_COMMANDS[idx];
  result = pattern.replace("{0}",param);
  return result;
}
