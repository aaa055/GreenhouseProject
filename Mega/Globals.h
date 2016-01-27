#ifndef _GLOBALS_H
#define _GLOBALS_H

// ПОМЕНЯТЬ НА УНИКАЛЬНОЕ ДЛЯ КАЖДОГО МОДУЛЯ!
#define OUR_ID  "CHILD" // имя модуля (при CONTROLLER_MODE == cdCHILDMODULE), для разрешения конфликтов, кому адресована команда

#define SETT_HEADER1 0xDE // байты, сигнализирующие о наличии сохранённых настроек
#define SETT_HEADER2 0xAD
#define EEPROM_RULES_START_ADDR 1025 // со второго килобайта в EEPROM идут правила

// настройки Serial
#define SERIAL_BAUD_RATE 9600 // скорость работы с портом, бод
#define READY F("READY") // будет напечатано в Serial после загрузки
#define DIODE_READY_PIN 6 // пин, на котором будет диод, мигающий при старте и горящий в режиме работы
#define DIODE_MANUAL_MODE_PIN 7 // пин, на котором будет диод, мигающий, когда мы в ручном режиме управления окнами
#define WORK_MODE_BLINK_INTERVAL 750 // с какой частотой мигать на пине индикации ручного режима работы, мс
#define DIODE_WATERING_MANUAL_MODE_PIN 8 // пин, на котором висит диод индикации ручного режима управления поливом

// директивы условной компиляции 
#define AS_CONTROLLER // закомментировать для дочерних модулей
#define USE_DS3231_REALTIME_CLOCK // закомментировать, если не хотим использовать модуль реального времени
#define USE_PIN_MODULE // закомментировать, если не нужен модуль управления пинами
#define USE_TEMP_SENSORS // закомментировать, если датчики температуры не поддерживаются
#define USE_LOOP_MODULE // закомментировать, если не нужна поддержка модуля LOOP
#define USE_STAT_MODULE // закомментировать, если не нужна поддержка модуля статистики
#define USE_SMS_MODULE // закомментировать, если не нужна поддержка управления по SMS
#define USE_WATERING_MOSULE // закомментировать, если не нужно управление поливом
#define USE_LUMINOSITY_MODULE // закомментировать, если не нужен модуль контроля освещенности

// настройки модуля алертов (событий по срабатыванию каких-либо условий)
#define MAX_STORED_ALERTS 3 // максимальное кол-во сохраняемых последних алертов
#define ALERT F("ALERT") // произошло событие
#define VIEW_ALERT_COMMAND F("VIEW") // команда просмотра события CTGET=ALERT|VIEW|0
#define CNT_COMMAND F("CNT") // сколько зарегистрировано событий CTGET=ALERT|CNT
#define ADD_RULE F("RULE_ADD") // правило алерта CTSET=ALERT|RULE_ADD|STATE|TEMP|0|>|38|CTSET=M|RELAY|0|ON, или, без привязки к модулю, CTSET=ALERT|RULE_ADD|STATE|TEMP|0|<=|38
#define RULE_CNT F("RULES_CNT") // кол-во правил CTGET=ALERT|RULES_CNT
#define RULE_VIEW F("RULE_VIEW") // просмотр правила по индексу CTGET=ALERT|RULE_VIEW|0
#define RULE_STATE F("RULE_STATE") // включить/выключить правило по индексу CTSET=ALERT|RULE_STATE|0|ON, CTSET=ALERT|RULE_STATE|0|OFF, CTSET=ALERT|RULE_STATE|ALL|OFF, CTGET=ALERT|RULE_STATE|0
#define RULE_DELETE F("RULE_DELETE") // удалить правило по индексу CTSET=ALERT|RULE_DELETE|1 - ПРИ УДАЛЕНИИ ВСЕ ПРАВИЛА СДВИГАЮТСЯ К ГОЛОВЕ ОТ УДАЛЁННОГО !!!
#define SAVE_RULES F("SAVE") // команда "сохранить правила"
#define GREATER_THAN F(">") // больше чем
#define GREATER_OR_EQUAL_THAN F(">=") // больше либо равно
#define LESS_THAN F("<") // меньше чем
#define LESS_OR_EQUAL_THAN F("<=") // меньше или равно
#define MAX_ALERT_RULES 20 // максимальное кол-во поддерживаемых правил
#define T_OPEN_MACRO F("%TO%") // макроподстановка температуры открытия из настроек
#define T_CLOSE_MACRO F("%TC%") // макроподстановка температуры закрытия из настроек


// настройки модуля реального времени (шина I2C)
#define RTC_SDA_PIN 20 // на каком пине будет SDA
#define RTC_SCL_PIN 21 // на каком пине будет SCL

// настройки модуля освещенности тут
// команда, на которую модуль выдаёт текущую освещенность - CTGET=LIGHT


#define MAX_PUBLISHERS 3 // максимальное количество паблишеров для модуля
#define MAX_TEMP_SENSORS 4 // максимальное кол-во поддерживаемых датчиков температуры
#define MAX_RELAY_CHANNELS 8 // максимальное кол-во реле (максимум - 8)
#define STATE_ON F("ON") // Включено
#define STATE_ON_ALT F("1") // Включено
#define STATE_OFF F("OFF") // Выключено
#define STATE_OFF_ALT F("0") // Выключено


// состояние канала управления фрамугой
#define DEF_OPEN_INTERVAL 30000 // по умолчанию пять секунд на полное открытие/закрытие
#define DEF_OPEN_TEMP 25 // температура открытия по умолчанию
#define DEF_CLOSE_TEMP 24 // температура закрытия по умолчанию
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
#define SUPPORTED_WINDOWS 4 // кол-во поддерживаемых окон (по два реле на мотор, для 8-ми канального модуля реле - 4 окна)
#define SHORT_CIRQUIT_STATE LOW // статус пинов, на которых висит реле, чтобы закоротить мотор и не дать ему крутиться


// настройки модуля управления поливом
#define WATER_SETTINGS_COMMAND F("T_SETT") // получить/установить настройки управления поливом: CTGET=WATER|T_SETT, CTSET=WATER|T_SETT|WateringOption|WateringDays|WateringTime , где
// WateringOption = 0 (выключено автоматическое управление поливом), 1 - автоматическое управление поливом включено
// WateringDays - битовая маска дней недели (младший бит - понедельник и т.д.)
// WateringTime - продолжительность полива в минутах, максимальное значение - 65535 (два байта)
#define WATER_RELAY_ON LOW // уровень для включения реле
#define WATER_RELAY_OFF HIGH // уровень для выключения реле

#define WATER_RELAYS_COUNT 3 // сколько каналов управления поливом используется
// объявляем пины для управления каналами реле - дописывать в этот массив, через запятую!
#define WATER_RELAYS_PINS 22,23,24 

/*
 * Команды модуля STATE - модуль следит за температурой и управляет открытием/закрытием фрамуг:
 * 
 * CTGET=STATE|TEMP|TEMP_CNT - возвращает кол-во температурных датчиков
 * CTGET=STATE|TEMP|0 - получить данные с первого датчика (и т.д.)
 * 
 * CTGET=STATE|WINDOW|WINDOW_CNT - получить кол-во каналов реле для управления фрамугами
 * CTGET=STATE|WINDOW|2 - получить состояние третьего канала (возможные состояния: OPEN - открыто, OPENING - фрамуга открывается, CLOSING - фрамуга закрываетсяб CLOSED - фрамуга закрыта)
 * 
 * CTSET=STATE|WINDOW|0|OPEN - ОТКРЫТЬ ФРАМУГУ НА ПЕРВОМ КАНАЛЕ (БЕЗ УКАЗАНИЯ ВРЕМЕНИ РАБОТЫ, берётся значение по умолчанию
 * CTSET=STATE|WINDOW|2|OPEN|4000 - открыть фрамугу на третьем канале, держать реле включенным 4000 миллисекунд
 * CTSET=STATE|WINDOW|ALL|CLOSE|5000 - закрыть все фрамуги, держать реле включенными 5 секунд
 * CTSET=STATE|WINDOW|2-7|OPEN|12000 - открыть фрамуги с 3 по восьмой канал, держать реле включенным 12 секунд
 * 
 */

// настройки главного контроллера
#define MAX_RECEIVE_BUFFER_LENGTH 256 // максимальная длина (в байтах) пакета в сети, дла защиты от спама
#define MIN_COMMAND_LENGTH 6 // минимальная длина правильной текстовой команды
#define OK_ANSWER F("OK") // ответ - всё ок
#define ERR_ANSWER F("ER") // ответ - ошибка

#define CMD_PREFIX  F("CT") // запрос к контроллеру
#define CHILD_PREFIX F("CD") // запрос к дочернему модулю
#define CMD_PREFIX_LEN  2  // длина префикса команды

#define CMD_SET F("SET") // установить значение
#define CMD_GET F("GET") // получить значение
#define CMD_TYPE_LEN 3 // длина типа команды

// ОТВЕТЫ ЗА ЗАПРОСЫ
#define UNKNOWN_MODULE F("UNKNOWN_MODULE")
#define PARAMS_MISSED F("PARAMS_MISSED")
#define UNKNOWN_COMMAND F("UNKNOWN_COMMAND")
#define NOT_SUPPORTED F("NOT_SUPPORTED")

// РАЗДЕЛИТЕЛЬ ПАРАМЕТРОВ
#define PARAM_DELIMITER F("|")
// разделитель команды и ответа
#define COMMAND_DELIMITER F("=")

#define MAX_ARGS_IN_LIST 30 // максимальное кол-во аргументов в списке команды


// команды модуля PIN
#define PIN_TOGGLE F("T") // CTGET=PIN|13, CTSET=PIN|13|1, CTSET=PIN|13|ON, CTSET=PIN|13|OFF, CTSET=PIN|13|0, CTSET=PIN|13|T
#define PIN_DETACH F("DETACH") // не устанавливать состояние пина

// Команды модуля статистикм
#define FREERAM_COMMAND F("FREERAM") // показать кол-во свободной памяти CTGET=STAT|FREERAM
#define UPTIME_COMMAND F("UPTIME") // показать время работы (в секундах) CTGET=STAT|UPTIME
#ifdef USE_DS3231_REALTIME_CLOCK
#define CURDATETIME_COMMAND F("DATETIME") // вывести текущую дату и время CTGET=STAT|DATETIME
#endif

// Команды и настройки модуля управления по SMS (модуль NEOWAY M590)
//#define NEOWAY_DEBUG_MODE // закомментировать, если не нужен режим отладки (плюётся в Serial отладочными сообщениями)
#define STAT_COMMAND F("STAT") // получить текущую статистику по SMS, CTGET=SMS|STAT
#define T_INDOOR F("Твн: ") // температура внутри
#define T_OUTDOOR F("Тнар: ") // температура снаружи
#define W_STATE F("Окна: ") // состояние окон
#define W_CLOSED F("закр") // закрыты
#define W_OPEN F("откр") // открыты
#define NEOWAY_NEWLINE F("\r\n") // новая строка для модуля, при отсыле команды
#define NEOWAY_SERIAL Serial1 // какой хардварный Serial будем использовать при работе с NEOWAY?
#define NEOWAY_BAUDRATE 9600 // скорость работы с GSM-модемом NEOWAY
#define SMS_OPEN_COMMAND F("#1") // открыть окна
#define SMS_CLOSE_COMMAND F("#0") // закрыть окна
#define SMS_STAT_COMMAND F("#9") // получить статистику
#define SMS_AUTOMODE_COMMAND F("#8") // установить автоматический режим работы
#define NEOWAY_WAIT_FOR_SMS_SEND_COMPLETE 6000 // интервал, в течение которого мы ждём откравку смс модулем (ждём асинхронно, без блокирования!)
#define NEOWAY_VCCIO_CHECK_PIN 2 // пин, на котором будем проверять сигнал от VCCIO (6 пин) модуля NEOWAY

// команды модуля "0"
#define NEWLINE F("\r\n")
#define ADD_COMMAND F("ADD") // команда регистрации модуля CTSET=0|ADD|MODULE_NAME
#define PING_COMMAND F("PING") // команда пинга контроллера CTGET=0|PING
#define REGISTERED_MODULES_COMMAND F("LIST") // пролистать зарегистрированные модули CTGET=0|LIST
#define PROPERTIES_COMMAND F("PROP") // команда записи/получения настроек CTSET=0|PROP|MODULE_NAME|PROPERTY_NAME|IDX|VALUE, например CTSET=0|PROP|M|TEMP|0|123,45
#define PROP_TEMP_CNT F("TEMP_CNT") // кол-во датчиков температуры CTGET=0|PROP|TEMP|TEMP_CNT, CTSET=0|PROP|TEMP|TEMP_CNT|2
#define PROP_RELAY_CNT F("RELAY_CNT") // кол-во каналов реле CTGET=0|PROP|MODULE_NAME|RELAY_CNT, CTSET=0|PROP|MODULE_NAME|RELAY_CNT|2
#define PROP_TEMP F("TEMP") // нам передали/запросили температуру CTGET=0|PROP|MODULE_NAME|TEMP|0, CTSET=0|PROP|MODULE_NAME|TEMP|0|36,6
#define PROP_RELAY F("RELAY") // нам передали/запросили состояние канала реле CTGET=0|PROP|MODULE_NAME|RELAY|0, CTSET=0|PROP|MODULE_NAME|RELAY|0|ON
#define SMS_NUMBER_COMMAND F("PHONE") // сохранить/вернуть номер телефона для управления контроллером по СМС: CTSET=0|PHONE|+7918..., CTGET=0|PHONE
#define PONG F("PONG") // ответ на запрос пинга
#define REG_SUCC F("ADDED") // модуль зарегистрирован
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


#endif
