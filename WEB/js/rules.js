//-----------------------------------------------------------------------------------------------------
// Классы для работы с правилами
//-----------------------------------------------------------------------------------------------------
RULE_TARGETS = {'TEMP': 'температурой', 'LIGHT' : 'освещенностью', 'HUMIDITY': 'влажностью', 'PIN': 'пином', 'SOIL': 'влажностью почвы'};
RULE_COMMANDS = {
  '.+STATE\\|WINDOW\\|.*\\|OPEN' : 'открываем окна'
, '.+STATE\\|WINDOW\\|.*\\|CLOSE': 'закрываем окна'
, '.+LIGHT\\|ON' : 'включаем досветку'
, '.+LIGHT\\|OFF' : 'выключаем досветку'
, '.+PIN\\|.*\\|OFF' : 'выключаем пин'
, '.+PIN\\|.*\\|ON' : 'включаем пин'
, '.+CC\\|EXEC\\|.*' : 'выполняем составную команду'
};
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
AlertRule.prototype.getTargetCommandDescription = function()
{
  for(var propName in RULE_COMMANDS)
  {
    
    var propVal = RULE_COMMANDS[propName];

    var reg = new RegExp(propName,'i');

    if(reg.test(this.TargetCommand))
    {
      return propVal;
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
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------
