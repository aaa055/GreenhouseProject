#ifndef _ALERT_MODULE_H
#define _ALERT_MODULE_H

#include "AbstractModule.h"
#include "Globals.h"

typedef enum
{
  rtUnknown,
  rtTemp, // за температурой следим
  rtLuminosity, // за освещенностью следим
  rtHumidity // за влажностью следим
} RuleTarget; // за чем следит правило

typedef enum
{
  tsPassed, // следим четко за переданной
  tsOpenTemperature, // берём температуру открытия из настроек
  tsCloseTemperature // берём температуру закрытия из настроек
  
} RuleDataSource; // откуда брать настройку для слежения?

typedef enum
{
  roLessThan, // < меньше чем
  roGreaterThan, // > больше чем
  roLessOrEqual, // <=
  roGreaterOrEqual // >=
  
} RuleOperand; // какой операнд использовать

class AlertRule
{
  private:
    RuleTarget target; // за чем следит правило
    int8_t dataAlert; // установка, за которой следим
    uint8_t sensorIdx; // индекс датчика, за которым следим
    RuleOperand operand; // операнд, которым проверяем
    String targetCommand; // команда, которую надо выполнить при срабатывании правила
    AbstractModule* linkedModule; // модуль, показания которого надо отслеживать
    long dataAlertLong; // настройка, за которой следим (4 байта)
    
    String alertRule; // строка с правилом
    bool bEnabled; // включено или нет

    RuleDataSource dataSource; // источник, с которого получаем установку значения для правила

    String ruleName; // имя правила
    uint8_t whichTime; // когда работает?
    unsigned long workTime; // продолжительность работы, мс

    String linkedRuleNames[MAX_ALERT_RULES]; // с какими правилами связано наше?
    uint8_t linkedRulesCnt; // кол-во связанных правил

    bool canWork; // можем ли мы работать?
    
    
  public:
    AlertRule();

    bool GetEnabled() {return bEnabled;}
    void SetEnabled(bool e) {bEnabled = e;}
    String GetName() {return ruleName;}
    bool Construct(AbstractModule* linkedModule, const Command& command);
    String& GetTargetCommand() {return targetCommand;}
    String& GetAlertRule() {return alertRule;}
    AbstractModule* GetModule() {return linkedModule;}

    uint8_t GetLinkedRulesCount() {return linkedRulesCnt;}
    String GetLinkedRuleName(uint8_t idx);

    uint8_t Save(uint16_t writeAddr); // сохраняем себя в EEPROM, возвращаем кол-во записанных байт
    uint8_t Load(uint16_t readAddr, ModuleController* c); // читаем себя из EEPROM, возвращаем кол-во прочитанных байт

    void Update(uint16_t dt
  #ifdef USE_DS3231_REALTIME_CLOCK 
     ,uint8_t currentHour // текущий час
     ,uint8_t currentMinute // текущая минута
  #endif
 );

    bool HasAlert(); // проверяем, есть ли алерт?
};
typedef Vector<AlertRule*> RulesVector;

class AlertModule : public AbstractModule
{
  private:
  
  #if MAX_STORED_ALERTS > 0
    uint8_t curAlertIdx;
    uint8_t cntAlerts;
    String strAlerts[MAX_STORED_ALERTS];
    String& GetAlert(uint8_t idx);
    void AddAlert(const String& strAlert);
#endif

    unsigned long lastUpdateCall;
    ModuleController* controller;
    String controllerID;
    CommandParser* cParser;

    uint8_t rulesCnt;
    AlertRule* alertRules[MAX_ALERT_RULES];
    void InitRules();
    bool AddRule(AbstractModule* m, const Command& c);

    void SolveConflicts(RulesVector& raisedAlerts,RulesVector& workRules);
    bool CanWorkWithRule(RulesVector& checkedRules, AlertRule* rule,RulesVector& raisedAlerts);
    AlertRule* GetLinkedRule(const String& linkedRuleName,RulesVector& raisedAlerts);

    void LoadRules();
    void SaveRules();
    
  public:
    AlertModule() : AbstractModule(F("ALERT")) 
    {
      #if MAX_STORED_ALERTS > 0
        cntAlerts = 0; curAlertIdx = 0;
      #endif 
      InitRules();
    }

    bool ExecCommand(const Command& command);
    void Setup();
    void Update(uint16_t dt);
};
#endif
