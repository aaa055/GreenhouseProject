#ifndef _ALERT_MODULE_H
#define _ALERT_MODULE_H

#include "AbstractModule.h"
#include "Globals.h"

typedef enum
{
  rtUnknown,
  rtTemp, // за температурой следим
  rtLuminosity, // за освещенностью следим
  rtHumidity, // за влажностью следим
  rtPinState, // следим за статусом пина
  rtSoilMoisture // следим за влажностью почвы
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
class AlertModule; // forward declaration

typedef Vector<size_t> LinkedRulesToIdxVector;


// флаги правила:
// бит 0 - включено или выключено
// бит 1 - может работать или нет
// бит 2 - флаг первого вызова
#define RULE_ENABLED_BIT 0
#define RULE_CAN_WORK_BIT 1
#define RULE_FIRST_CALL_BIT 2

class AlertRule
{
  private:
  
    AlertModule* parent; // родитель, который нас создал
    
    uint8_t target; // за чем следит правило
    int8_t dataAlert; // установка, за которой следим
    uint8_t sensorIdx; // индекс датчика, за которым следим
    uint8_t operand; // операнд, которым проверяем
    String targetCommand; // команда, которую надо выполнить при срабатывании правила
    AbstractModule* linkedModule; // модуль, показания которого надо отслеживать
    long dataAlertLong; // настройка, за которой следим (4 байта)

    /*
    bool bEnabled; // включено или нет
    bool bFirstCall; // первый ли вызов правила?
    bool canWork; // можем ли мы работать?
    */

    uint8_t flags; // флаги состояний
    
    uint8_t dataSource; // источник, с которого получаем установку значения для правила

    size_t ruleNameIdx; // индекс имени правила у родителя
    uint8_t whichTime; // когда работает?
    unsigned long workTime; // продолжительность работы, мс

   LinkedRulesToIdxVector linkedRulesIndices; // привязка имён связанных правил к их индексу у родителя

    
    
  public:
    AlertRule(AlertModule* am);

    bool GetEnabled() {return /*bEnabled*/ bitRead(flags,RULE_ENABLED_BIT);}
    void SetEnabled(bool e) {/*bEnabled = e;*/ bitWrite(flags,RULE_ENABLED_BIT, (e ? 1 : 0));}
    const char* GetName();
    bool Construct(AbstractModule* linkedModule, const Command& command);
    String GetTargetCommand() {return targetCommand;}
    String GetAlertRule();
    AbstractModule* GetModule() {return linkedModule;}

    size_t GetLinkedRulesCount();
    const char* GetLinkedRuleName(uint8_t idx);

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
typedef Vector<char*> NamesVector;

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

    NamesVector paramsArray; // всякие общие имена храним здесь

    unsigned long lastUpdateCall;
    CommandParser* cParser;

    uint8_t rulesCnt;
    AlertRule* alertRules[MAX_ALERT_RULES];
    void InitRules();
    bool AddRule(AbstractModule* m, const Command& c);

    void SolveConflicts(RulesVector& raisedAlerts,RulesVector& workRules);
    bool CanWorkWithRule(RulesVector& checkedRules, AlertRule* rule,RulesVector& raisedAlerts);
    AlertRule* GetLinkedRule(const char* linkedRuleName,RulesVector& raisedAlerts);

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

    size_t AddParam(char* nm, bool& added); // добавляем строку в общий массив
    char* GetParam(size_t idx); // возвращает строку из массива по индексу

    bool ExecCommand(const Command& command, bool wantAnswer);
    void Setup();
    void Update(uint16_t dt);
};
#endif
