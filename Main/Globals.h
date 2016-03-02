#ifndef _GLOBALS_H
#define _GLOBALS_H

// директивы условной компиляции 
#define AS_CONTROLLER // закомментировать для дочерних модулей
#define USE_DS3231_REALTIME_CLOCK // закомментировать, если не хотим использовать модуль реального времени
#define USE_PIN_MODULE // закомментировать, если не нужен модуль управления пинами
#define USE_TEMP_SENSORS // закомментировать, если не нужно управление окнами по температуре
#define USE_LOOP_MODULE // закомментировать, если не нужна поддержка модуля LOOP
#define USE_STAT_MODULE // закомментировать, если не нужна поддержка модуля статистики (FREERAM, UPTIME, DATETIME)
#define USE_SMS_MODULE // закомментировать, если не нужна поддержка управления по SMS
#define USE_WATERING_MODULE // закомментировать, если не нужно управление поливом
#define USE_LUMINOSITY_MODULE // закомментировать, если не нужен модуль контроля освещенности
#define USE_HUMIDITY_MODULE // закомментировать, если не нужен модуль работы с датчиками влажности DHT
#define USE_WIFI_MODULE // закомментировать, если не нужна поддержка управления через Wi-Fi (ESP8266)
#define USE_LOG_MODULE // закомментировать, если не нужен модуль логгирования информации. Внимание: модуль работает только с модулем реального времени (USE_DS3231_REALTIME_CLOCK должна быть определена!)
//#define USE_PUBLISHERS // раскомментировать, если используется парадигма паблишеров (пока я не понял, нужна ли она в реале)
//#define SAVE_RELAY_STATES // раскомментировать, если для модуля надо хранить актуальное состояние каналов реле (ещё не понял - надо ли показывать это в реале)

// ПОМЕНЯТЬ НА УНИКАЛЬНОЕ ДЛЯ КАЖДОГО МОДУЛЯ, СДЕЛАННОГО В ЖЕЛЕЗЕ!
#define OUR_ID  "CHILD" // имя модуля (при CONTROLLER_MODE == cdCHILDMODULE), для разрешения конфликтов, кому адресована команда

// Настройки EEPROM
#define SETT_HEADER1 0xDE // байты, сигнализирующие о наличии сохранённых настроек
#define SETT_HEADER2 0xAD
#define EEPROM_RULES_START_ADDR 1025 // со второго килобайта в EEPROM идут правила

// настройки Serial
#define SERIAL_BAUD_RATE 9600 // скорость работы с портом, бод
#define READY F("READY") // будет напечатано в Serial после загрузки

// настройки информационных диодов
#define DIODE_READY_PIN 6 // пин, на котором будет диод, мигающий при старте и горящий в режиме работы
#define DIODE_MANUAL_MODE_PIN 7 // пин, на котором будет диод, мигающий, когда мы в ручном режиме управления окнами
#define DIODE_WATERING_MANUAL_MODE_PIN 8 // пин, на котором висит диод индикации ручного режима управления поливом
#define DIODE_LIGHT_MANUAL_MODE_PIN 9 // пин, на котором висит диод индикации ручного режима управления досветкой
#define WORK_MODE_BLINK_INTERVAL 500 // с какой частотой мигать на пинах индикации ручного режима работы, мс

// настройки железных модулей реле 
#define RELAY_ON LOW // уровень для включения реле
#define RELAY_OFF HIGH // уровень для выключения реле
#define SHORT_CIRQUIT_STATE HIGH // статус пинов, на которых висит реле, чтобы закоротить мотор и не дать ему крутиться

// настройки максимумов
#define MAX_STORED_ALERTS 0 // максимальное кол-во сохраняемых последних текстовых алертов (0 - нет поддержки сохраняемых событий)
#define MAX_ALERT_RULES 20 // максимальное кол-во поддерживаемых правил
#define MAX_RECEIVE_BUFFER_LENGTH 256 // максимальная длина (в байтах) пакета в сети, дла защиты от спама
#define MAX_ARGS_IN_LIST 30 // максимальное кол-во аргументов у команды, передаваемой контроллеру по UART


// настройки модуля алертов (событий по срабатыванию каких-либо условий)
#define ALERT_UPD_INTERVAL 500 // интервал обновления состояния модуля ALERT, мс. Нужен, чтобы часто не разрешать зависимости - это ресурсоёмкая операция.
#define ALERT F("ALERT") // произошло событие
#define VIEW_ALERT_COMMAND F("VIEW") // команда просмотра события CTGET=ALERT|VIEW|0
#define CNT_COMMAND F("CNT") // сколько зарегистрировано событий CTGET=ALERT|CNT
// правило алерта CTSET=ALERT|RULE_ADD|RuleName|STATE|TEMP|1|>|23|Час начала работы|Продолжительность работы, мин|Список связанных правил|Команды для стороннего модуля
// пример №1: CTSET=ALERT|RULE_ADD|N1|STATE|TEMP|1|>|23|0|30|N3,N4|CTSET=STATE|WINDOW|ALL|OPEN
// пример №2: CTSET=ALERT|RULE_ADD|N1|STATE|TEMP|1|>|23|0|0|_|CTSET=STATE|WINDOW|ALL|OPEN
#define ADD_RULE F("RULE_ADD") // добавить правило
#define RULE_CNT F("RULES_CNT") // кол-во правил CTGET=ALERT|RULES_CNT
#define RULE_VIEW F("RULE_VIEW") // просмотр правила по индексу CTGET=ALERT|RULE_VIEW|0
#define RULE_STATE F("RULE_STATE") // включить/выключить правило по имени CTSET=ALERT|RULE_STATE|RuleName|ON, CTSET=ALERT|RULE_STATE|RuleName|OFF, CTSET=ALERT|RULE_STATE|ALL|OFF
// получить состояние правила по индексу -  CTGET=ALERT|RULE_STATE|0
#define RULE_DELETE F("RULE_DELETE") // удалить правило по имени CTSET=ALERT|RULE_DELETE|RuleName - ПРИ УДАЛЕНИИ ВСЕ ПРАВИЛА СДВИГАЮТСЯ К ГОЛОВЕ ОТ УДАЛЁННОГО !!!
#define SAVE_RULES F("SAVE") // команда "сохранить правила", CTSET=ALERT|SAVE
#define GREATER_THAN F(">") // больше чем
#define GREATER_OR_EQUAL_THAN F(">=") // больше либо равно
#define LESS_THAN F("<") // меньше чем
#define LESS_OR_EQUAL_THAN F("<=") // меньше или равно
#define T_OPEN_MACRO F("%TO%") // макроподстановка температуры открытия из настроек
#define T_CLOSE_MACRO F("%TC%") // макроподстановка температуры закрытия из настроек


// настройки модуля логгирования информации
//#define LOGGING_DEBUG_MODE // раскомментировать для отладочного режима (не работает с конфигуратором, плюётся в Serial отладочной информацией)
#define LOGGING_INTERVAL 300000 // интервал логгирования, мс (300000 - каждые 5 минут и т.п.)
//#define ADD_LOG_HEADER // закомментировать, если не надо добавлять первые строки с информацией, с каких модулей есть данные).
// Первая трока информации имеет вид:
// MODULE_NAME1=MODULE_IDX1,MODULE_NAME2=MODULE_IDX2,MODULE_NAMEn=MODULE_IDXn
// где MODULE_NAMEn - имя модуля, MODULE_IDXn = индекс модуля в системе.
// Вторая строка информации сообщает о типах датчиков, и имеет вид:
// TYPE1=IDX1,TYPEn=IDXn
// где TYPEn - название типа (например, TEMP для температуры), IDXn - числовое представление типа.
//#define LOG_CNANGE_NAME_TO_IDX // раскомментировать, если нужен лог меньшего формата.
// в этом случае в каждой строке вместо имени модуля подставляется его индекс в системе. 
//#define LOG_CHANGE_TYPE_TO_IDX // раскомментировать, если нужен лог сжатого формата.
// в этом случае в каждой строке вместо названия типа датчика подставляется его индекс в системе.
#define WRITE_ABSENT_SENSORS_DATA // раскомментировать, если надо писать показания датчика, даже если показаний с него нет
#define LOG_TEMP_TYPE F("RT") // тип для температуры, который запишется в файл
#define LOG_HUMIDITY_TYPE F("RH") // тип для влажности, который запишется в файл
#define LOG_LUMINOSITY_TYPE F("RL") // тип для освещенности, который запишется в файл
#define COMMA_DELIMITER F(",") // разделитель полей в CSV
#define LOGS_DIRECTORY F("logs") // название папки с логами на карточке

// настройки модуля освещенности
// команда, на которую модуль выдаёт текущую освещенность - CTGET=LIGHT
#define LUMINOSITY_UPDATE_INTERVAL 3000 // через сколько мс обновлять показания с датчиков освещенности 
#define LIGHT_SENSORS_COUNT 2 // кол-во датчиков освещенности, 0, 1 или 2, 2 - максимум
#define LAMP_RELAYS_COUNT 1 // кол-во реле для управления досветкой
#define LAMP_RELAYS_PINS 34 // пины, на которых сидят реле управления досветкой (через запятую, кол-во равно LAMP_RELAYS_COUNT!)
#define LIGHT_STATE_COMMAND F("STATE") // CTGET=LIGHT|STATE
#define NO_LUMINOSITY_DATA -1 // нет показаний с датчика освещенности

// настройки модуля влажности
// GTGET=HUMIDITY|CNT - получить кол-во датчиков влажности
// GTGET=HUMIDITY|0 - получить показания первого датчика, возвращается OK=HUMIDITY|0|RH|RT, где RH - влажность, RT - температура
// CTGET=HUMIDITY|ALL - получить показания всех датчиков, возвращается OK=HUMIDITY|CNT|RH0|RT0|RH1|RT1|RHn|RTn, где CNT - кол-во записей
#define HUMIDITY_UPDATE_INTERVAL 5000 // через сколько мс обновлять показания с датчиков влажности
#define ADD_HUMIDITY_SENSOR(pin,type) { pin , type } // для удобства добавления сенсора в массив
// типы поддерживаемых сенсоров: DHT11, DHT2x
#define SUPPORTED_HUMIDITY_SENSORS 1 // кол-во поддерживаемых датчиков влажности
// описание поддерживаемых датчиков влажности, через запятую, кол-вом SUPPORTED_DHT_SENSORS.
// формат: ADD_HUMIDITY_SENSOR(пин, тип)
// следующий после первого датчик добавляется через запятую.
// Примеры:
// для одного датчика:
// #define HUMIDITY_SENSORS ADD_HUMIDITY_SENSOR(12,DHT2x)
// для двух и более датчиков:
// #define HUMIDITY_SENSORS ADD_HUMIDITY_SENSOR(12,DHT2x), ADD_HUMIDITY_SENSOR(14,DHT11), ADD_HUMIDITY_SENSOR(15,DHT2x)
#define HUMIDITY_SENSORS ADD_HUMIDITY_SENSOR(12,DHT2x)


// состояния вкл/выкл, для команд
#define STATE_ON F("ON") // Включено
#define STATE_ON_ALT F("1") // Включено
#define STATE_OFF F("OFF") // Выключено
#define STATE_OFF_ALT F("0") // Выключено


// настройки модуля управления фрамугами
#define DEF_OPEN_INTERVAL 30000 // по умолчанию 30 секунд на полное открытие/закрытие
#define DEF_OPEN_TEMP 25 // температура открытия по умолчанию
#define DEF_CLOSE_TEMP 24 // температура закрытия по умолчанию
#define TEMP_UPDATE_INTERVAL 5000 // через сколько мс обновлять показания с датчиков температуры
#define STATE_OPENING F("OPENING") // Открывается
#define STATE_CLOSING F("CLOSING") // Закрывается
#define STATE_CLOSED F("CLOSED") // Закрыто
#define WM_AUTOMATIC F("AUTO") // автоматический режим управления фрамугами
#define WM_MANUAL F("MANUAL") // ручной режим управления фрамугами
#define WORK_MODE F("MODE") // получить/установить режим работы CTGET=STATE|MODE, CTSET=STATE|MODE|AUTO, CTSET=STATE|MODE|MANUAL
#define WM_INTERVAL F("INTERVAL") // получить/установить интервал на открытие/закрытие окон CTGET=STATE|INTERVAL, CTSET=STATE|INTERVAL|3000
#define STATE_OPEN F("OPEN") // Открыть CTSET=STATE|WINDOW|0|OPEN, CTSET=STATE|WINDOW|ALL|OPEN, CTSET=STATE|WINDOW|0-2|OPEN|2000
#define ALL F("ALL") // отработать все каналы
#define PROP_WINDOW F("WINDOW") // название канала, чтобы было понятно
#define PROP_WINDOW_CNT F("WINDOW_CNT") // кол-во фрамуг CTGET=STATE|WINDOW_CNT
#define TEMP_SETTINGS F("T_SETT") // получить/установить температуры срабатывания, CTGET=STATE|T_SETT, CTSET=STATE|T_SETT|t open|t close
#define NO_TEMPERATURE_DATA -128 // нет данных с датчика температуры
#define SUPPORTED_SENSORS 2 // кол-во поддерживаемых датчиков температуры "из коробки"
#define TEMP_SENSORS_PINS 31,32 // пины, на которых висят наши датчики температуры (указываются через запятую, общее кол-во равно SUPPORTED_SENSORS)
#define SUPPORTED_WINDOWS 4 // кол-во поддерживаемых окон (по два реле на мотор, для 8-ми канального модуля реле - 4 окна)
// пины реле управления фрамугами (попарно, через запятую!) На каждом пине висит одно реле, пара реле (например,
// 40 и 41) образуют одну пару управления DC-мотором. Кол-во реле равно SUPPORTED_WINDOWS*2, соответственно, кол-во используемых
// пинов - всегда чётно! Поэтому будьте внимательны при редактировании этой настройки!
// Как подключается мотор: контакты двигателя подключаются к общим (COM) контактам пары реле.
// Плюс питания - к NO (нормально разомкнутым контактам пары реле).
// Минус питания - к NC (нормально замкнутым контактам реле).
#define WINDOWS_RELAYS_PINS 40,41,42,43,44,45,46,47 

// настройки модуля управления поливом
#define WATER_SETTINGS_COMMAND F("T_SETT") // получить/установить настройки управления поливом: CTGET=WATER|T_SETT, CTSET=WATER|T_SETT|WateringOption|WateringDays|WateringTime|StartTime|TurnOnPump , где
// WateringOption = 0 (выключено автоматическое управление поливом), 1 - автоматическое управление поливом включено (все каналы), 2 - автоуправление отдельно по каналам
// WateringDays - битовая маска дней недели (младший бит - понедельник и т.д.)
// WateringTime - продолжительность полива в минутах, максимальное значение - 65535 (два байта)
// StartTime - час начала полива (1 байт) - от 1 до 23
// TurnOnPump - включать (1) или нет (0) насос при активном поливе на любом из каналов
#define WATER_CHANNEL_SETTINGS F("CH_SETT") // получить/установить настройки отдельного канала управления поливом: CTGET=WATER|CH_SETT|0, CTSET=WATER|CH_SETT|0|WateringDays|WateringTime|StartTime
#define WATER_CHANNELS_COUNT_COMMAND F("CHANNELS") // получить кол-во поддерживаемых каналов полива: CTGET=WATER|CHANNELS
#define PUMP_RELAY_PIN 22 // пин, на котором сидит реле управления насосом
#define USE_PUMP_RELAY // закомментировать, если не нужен отдельный канал управления насосом при поливе
#define WATER_RELAYS_COUNT 2 // сколько каналов управления поливом используется
// объявляем пины для управления каналами реле - дописывать в этот массив, через запятую,
// кол-во равно WATER_RELAYS_COUNT!
#define WATER_RELAYS_PINS 23,24 


// настройки главного контроллера
#define MIN_COMMAND_LENGTH 6 // минимальная длина правильной текстовой команды
#define CMD_PREFIX  F("CT") // запрос к контроллеру
#define CHILD_PREFIX F("CD") // запрос к дочернему модулю
#define CMD_PREFIX_LEN  2  // длина префикса команды

#define CMD_SET F("SET") // установить значение
#define CMD_GET F("GET") // получить значение
#define CMD_TYPE_LEN 3 // длина типа команды

// ОТВЕТЫ ЗА ЗАПРОСЫ
#define OK_ANSWER F("OK") // ответ - всё ок
#define ERR_ANSWER F("ER") // ответ - ошибка
#define UNKNOWN_MODULE F("UNKNOWN_MODULE") // запрос к неизвестному модулю
#define PARAMS_MISSED F("PARAMS_MISSED") // пропущены параметры команды
#define UNKNOWN_COMMAND F("UNKNOWN_COMMAND") // неизвестная команда
#define NOT_SUPPORTED F("NOT_SUPPORTED") // не поддерживается

// РАЗДЕЛИТЕЛЬ ПАРАМЕТРОВ
#define PARAM_DELIMITER F("|")
// разделитель команды и ответа
#define COMMAND_DELIMITER F("=")


// настройки модуля PIN
#define PIN_TOGGLE F("T") // CTGET=PIN|13, CTSET=PIN|13|1, CTSET=PIN|13|ON, CTSET=PIN|13|OFF, CTSET=PIN|13|0, CTSET=PIN|13|T
#define PIN_DETACH F("DETACH") // не устанавливать состояние пина

// настройки модуля статистикм
#define FREERAM_COMMAND F("FREERAM") // показать кол-во свободной памяти CTGET=STAT|FREERAM
#define UPTIME_COMMAND F("UPTIME") // показать время работы (в секундах) CTGET=STAT|UPTIME
#ifdef USE_DS3231_REALTIME_CLOCK
#define CURDATETIME_COMMAND F("DATETIME") // вывести текущую дату и время CTGET=STAT|DATETIME
#endif

// настройки модуля управления по SMS (модуль NEOWAY M590)
//#define NEOWAY_DEBUG_MODE // закомментировать, если не нужен режим отладки (плюётся в Serial отладочными сообщениями, не работает с конфигуратором!)
#define STAT_COMMAND F("STAT") // получить текущую статистику по SMS, CTGET=SMS|STAT
#define T_INDOOR F("Твн: ") // температура внутри
#define T_OUTDOOR F("Тнар: ") // температура снаружи
#define W_STATE F("Окна: ") // состояние окон
#define W_CLOSED F("закр") // закрыты
#define W_OPEN F("откр") // открыты
#define WTR_STATE F("Полив: ") // состояние полива
#define WTR_OFF F("выкл") // полив выкл
#define WTR_ON F("вкл") // полив вкл
#define NEOWAY_SERIAL Serial1 // какой хардварный Serial будем использовать при работе с NEOWAY?
#define NEOWAY_EVENT_FUNC serialEvent1 // функция для обработки событий входящего трафика для модуля
#define NEOWAY_BAUDRATE 57600 // скорость работы с GSM-модемом NEOWAY
#define SMS_OPEN_COMMAND F("#1") // открыть окна
#define SMS_CLOSE_COMMAND F("#0") // закрыть окна
#define SMS_STAT_COMMAND F("#9") // получить статистику
#define SMS_AUTOMODE_COMMAND F("#8") // установить автоматический режим работы
#define SMS_WATER_ON_COMMAND F("#4") // включить полив
#define SMS_WATER_OFF_COMMAND F("#6") // выключить полив
#define SMS_NO_DATA F("<нет данных>") // нет данных с датчика
#define NEOWAY_VCCIO_CHECK_PIN 2 // пин, на котором будем проверять сигнал от VCCIO (6 пин) модуля NEOWAY

// настройки модуля WI-FI
//#define WIFI_DEBUG // закомментировать, если не нужен режим отладки (режим отладки не работаем совместно с конфигуратором!) 
#define SDCARD_CS_PIN 53 // номер пина Chip Select для SD-модуля 
#define WIFI_SERIAL Serial2 // какой хардварный сериал использовать для WI-FI?
#define WIFI_EVENT_FUNC serialEvent2 // функция для обработки событий входящего трафика от модуля
#define WIFI_BAUDRATE 115200 // скорость работы с UART для WI-FI
#define STATION_ID F("TEPLICA") // ID точки доступа, которую создаёт модуль WI-FI
#define STATION_PASSWORD F("12345678") // пароль к точке доступа, которую создаёт вай-фай (МИНИМУМ 8 СИМВОЛОВ, ИНАЧЕН НЕ БУДЕТ РАБОТАТЬ!)
#define ROUTER_ID F("")  // SSID домашнего роутера, к которому коннектится модуль WI-FI
#define ROUTER_PASSWORD F("") // пароль к домашнему роутеру, к которому коннектится модуль WI-FI
#define WIFI_SETTINGS_COMMAND F("T_SETT") // установить настройки модуля: CTSET=WIFI|T_SETT|SHOULD_CONNECT_TO_ROUTER(0 or 1)|ROUTER_ID|ROUTER_PASS|STATION_ID|STATION_PASS
#define IP_COMMAND F("IP") // получить текущие IP-адреса, как самой точки доступа, так и назначенный роутером, CTGET=WIFI|IP
#define BUSY F("BUSY") // если мы не можем ответить на запрос - тогда возвращаем ER=WIFI|BUSY

// в дебаг-режиме переводим отладочный порт на такую же скорость, как и скорость
// порта, через который мы работаем с ESP
#ifdef WIFI_DEBUG
#undef SERIAL_BAUD_RATE
#define SERIAL_BAUD_RATE WIFI_BAUDRATE
#warning Serial BAUD RATE IS CHANGED TO WIFI_SERIAL BAUD RATE DUE TO WIFI_DEBUG MODE!
#endif

// настройки модуля LOOP
#define MIN_LOOP_PARAMS 5 // минимальное количество параметров, которые надо передать
#define MAX_LOOP_PARAMS 15 // максимальное кол-во параметров, которые можно передать
#define LOOP_NAME_IDX 0 // имя циклически выполняемой команды
#define COMMAND_TYPE_IDX 1 // индекс типа команды в параметрах
#define INTERVAL_IDX 2 // индекс параметра "интервал"
#define COUNT_PASSES_IDX 3 // индекс параметра "кол-во проходов"
#define MODULE_ID_IDX 4 // индекс ID модуля в параметрах
/*
 * Структура команды LOOP:
 * 
 * LOOP|NAME|SET_OR_GET|INTERVAL|COUNT_PASSES|LINKED_MODULE_ID|PARAMS_FOR_MODULE
 * где
 *  NAME - имя, к которому привязывается циклическая команда
 *  SET_OR_GET = SET или GET команда для связанного модуля
 *  INTERVAL - интервал в мс между вызовами (0 - выключить модуль из циклического опроса)
 *  COUNT_PASSES - кол-во проходов (0 - бесконечно)
 *  LINKED_MODULE_ID - идентификатор модуля для передачи команд
 *  PARAMS_FOR_MODULE - параметры для связанного модуля (разделенные '|')
 *  
 *  Для примера, команда
 *  CTSET=LOOP|MyLoop|SET|500|10|PIN|13|T
 *  передаст параметры 13 и T модулю с идентификатором "PIN" 10 раз через каждые 500 мс
 *  
 *  Выключить модуль из обработки просто:
 *  CTSET=LOOP|MyLoop|SET|0|0|PIN|13
 *  
 *    Если для модуля поступила новая команда - старая перезаписывается, т.е. цепочка команд не поддерживается!
 */

 // свойства модулей, которые мы можем проверять/устанавливать с помощью модуля 0.
 // 
#define PROP_TEMP_CNT F("TEMP_CNT") // кол-во датчиков температуры CTGET=0|PROP|TEMP|TEMP_CNT, CTSET=0|PROP|TEMP|TEMP_CNT|2
#define PROP_RELAY_CNT F("RELAY_CNT") // кол-во каналов реле CTGET=0|PROP|MODULE_NAME|RELAY_CNT, CTSET=0|PROP|MODULE_NAME|RELAY_CNT|2
#define PROP_CNT F("CNT") // свойство - кол-во любых датчиков
#define PROP_TEMP F("TEMP") // нам передали/запросили температуру CTGET=0|PROP|MODULE_NAME|TEMP|0, CTSET=0|PROP|MODULE_NAME|TEMP|0|36,6
#define PROP_RELAY F("RELAY") // нам передали/запросили состояние канала реле CTGET=0|PROP|MODULE_NAME|RELAY|0, CTSET=0|PROP|MODULE_NAME|RELAY|0|ON
#define PROP_LIGHT F("LIGHT") // свойство "освещенность"
#define PROP_HUMIDITY F("HUMIDITY") // свойство "влажность"

// команды модуля "0"
//#define USE_REMOTE_MODULES // раскомментировать, если нужна регистрация модулей на лету (при использовании сторонних железок, общающихся с контроллером)
#define NEWLINE F("\r\n")
#define SETTIME_COMMAND F("DATETIME") // установка даты/времени CTSET=0|DATETIME|DD.MM.YYYY hh:mm:ss
#define ADD_COMMAND F("ADD") // команда регистрации модуля CTSET=0|ADD|MODULE_NAME
#define PING_COMMAND F("PING") // команда пинга контроллера CTGET=0|PING
#define REGISTERED_MODULES_COMMAND F("LIST") // пролистать зарегистрированные модули CTGET=0|LIST
#define PROPERTIES_COMMAND F("PROP") // команда записи/получения настроек CTSET=0|PROP|MODULE_NAME|PROPERTY_NAME|IDX|VALUE, например CTSET=0|PROP|M|TEMP|0|123,45
#define SMS_NUMBER_COMMAND F("PHONE") // сохранить/вернуть номер телефона для управления контроллером по СМС: CTSET=0|PHONE|+7918..., CTGET=0|PHONE
#define PONG F("PONG") // ответ на запрос пинга
#define REG_SUCC F("ADDED") // модуль зарегистрирован, или команда обработана
#define REG_DEL F("DELETED") // удалено
#define REG_ERR F("EXIST") // модуль уже зарегистрирован
#define UNKNOWN_PROPERTY F("UNKNOWN_PROPERTY") // неизвестное свойство
#define HAS_CHANGES_COMMAND F("HAS_CHANGES") // есть ли изменения в состоянии модулей? CTGET=0|HAS_CHANGES
#define LIST_CHANGES_COMMAND F("LIST_CHANGES") // пролистать, что и в каких модулях изменилось CTGET=0|LIST_CHANGES , РЕЗУЛЬТАТЫ ОТВЕТА:
/*
 * MODULE_NAME|PROP_NAME|IDX|FROM|TO\r\n
 * MODULE_NAME|PROP_NAME|IDX|FROM|TO\r\n
 * MODULE_NAME|PROP_NAME|IDX|FROM|TO\r\n
 * и т.д., список заканчивается двойным переводом строки (\r\n\r\n)
 * 
 * 
 */

#define UNUSED(expr) do { (void)(expr); } while (0)
#define MAX_PUBLISHERS 2 // максимальное количество паблишеров для модуля
#define RESERVE_STR_LENGTH 32 // сколько байт резервировать для строки ответа при выполнении ExecCommand

#endif
