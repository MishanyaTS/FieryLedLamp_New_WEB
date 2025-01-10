#ifdef USE_MQTT
/*
 * Библиотека асинхронных MQTT запросов https://github.com/marvinroger/async-mqtt-client
 * Не умеет автоматически восстанавливать разорванное соединение с MQTT брокером, поэтому требует периодической проверки подключения
 * Зависит от библиотек:
 *   ESPAsyncTCP https://github.com/me-no-dev/ESPAsyncTCP
 *   AsyncTCP https://github.com/me-no-dev/AsyncTCP
 *
 * Лампа подписана на топик: <TopicBase>/LedLamp_<ChipId>/cmnd, где <TopicBase> – базовая (корневая) часть топиков. Например "Test/LedLamp" (изменяется в веб-интерфейсе);
 *   <ChipId> - getChipID(); payload - строка, содержащая те же команды, отправляемые приложением (реестр важен):
 *   P_ON / P_OFF - включить/выключить матрицу
 *   FAV_ON/FAV_OFF - включить/выключить режим "цикл" ("избранное")
 *   SO_ON / SO_OFF - включить/выключить озвучивание эффектов
 *   VOL15 - установить громкость озвучивания эффектов 15 [1...30]
 *   EFF10 - сделать активным эффект №10 (нумерация с нуля)
 *   BRI44 - установить яркость 44; диапазон [1..255]
 *   SPD3 - установить скорость 3; диапазон [1..255]
 *   SCA1 - установить масштаб 1; диапазон [1..100]
 *   ALM_SET1 ON - завести будильник 1 (понедельник); ON - вкл, OFF - выкл.
 *   ALM_SET1 390 - установить время будильника 1 (понедельник) на 06:30 (количество минут от начала суток).
 *   DAWN1 - установить "рассвет" за 5 минут до будильника (1 = 5 минут - номер опции в выпадающем списке в приложении, нумерация с единицы).
 *   TMR_SET 1 3 300 - установить таймер; описание параметров - см. команду TMR ниже.
 *   FAV_SET 1 60 120 0 0 1 0 0 0 0 0 1 1 0 0 1 0 0 0 0 0 0 1 0 0 0 1 0 0 0 - установить режим "избранное", параметры - см. команду FAV ниже.
 *   BTN_ON / BTN_OFF - разблокировать/заблокировать кнопку на лампе.
 *   STATE - получить текущее состояние лампы.
 *   RND_0 / RND_1 - выбрать случайные настройки текущего эффекта/вернуть параметры по умолчанию текущего эффекта.
 *   RND_Z - вернуть параметры по умолчанию всем эффектам.
 *   RND_C1 / RND_C0 - Включаем/Выключаем случайный выбор эффектов в режиме Цикл
 *   RND_ON / RND_OFF - Включить/Выключить выбор случайных настроек в режиме Цикл
 *   TXT_  - ваш текст для вывода бегущей строкой.
 *   RUN_T15 - Периодичность вывода (минуты) бегущей строки. [0...99] 0-не выводить; более 60-выводить постоянно.
 *   RUN_C80 - Бегущая строка строки. [0...255]
 *   RUN_F1 -  Фон бегущей строки. 0-черный фон; 1-цветный фон.
 *   RUN_S240- Скорость бегущей строки [0...255]
 *   SLEEP2 - Таймер сна. SLEEP2 – 10 минут. SLEEP1 – 5 минут
 *   RESET_ALL - Сброс всех настроек в исходные (по умолчанию)
 *
 * Лампа отправляет свое состояние сразу после включения и после каждой смены в топик <TopicBase>/LedLamp_<ChipId>/state;
 *  "CURR 7 14 4 50 1 1 1 0 1 21:25:50", где:
 *     CURR - идентификатор команды, CURR - текущее состояние лампы
 *     7 - номер текущего эффекта
 *     14 - яркость
 *     4 - скорость
 *     50 - масштаб
 *     1 - признак "матрица включена"
 *     1 - режим ESP_MODE
 *     1 - признак USE_NTP (пытаться синхронизировать время по серверам времени в интернете)
 *     0 - признак "работает таймер"
 *     1 - признак "кнопка разблокирована"
 *     21:25:50 - текущее время (если не синхронизировано, показывает время от старта модуля)
 *  "ALMS 1 0 0 0 0 0 0 0 390 0 0 0 0 0 0 1"
 *     ALMS - идентификатор команды, ALMS - настройки будильников
 *     первые 7 цифр - признак "будильник заведён" по дням недели, начиная с понедельника
 *     последующие 7 цифр - время в минутах от начала суток, на которое заведён будильник (по дням недели); 390 = 06:30
 *     последняя цифра - опция "рассвет за ... минут", цифра указывает на номер значения в выпадающем списке: 1 - 5 минут, 2 - 10 минут... (см. в приложении)
 *  "TMR 1 3 300"
 *     TMR - идентификатор команды, TMR - таймер
 *     1 - признак "таймер взведён"
 *     3 - опция "выключить лампу через ...", цифра указывает на номер значения в выпадающем списке: 1 - не выключать, 2 - 1 минута... (см. в приложении)
 *     300 - количество секунд, через которое выключится лампа (0 - не выключать)
 *  "FAV 1 60 120 0 0 1 0 0 0 0 0 1 1 0 0 1 0 0 0 0 0 0 1 0 0 0 1 0 0 0............МНОГО НОЛИКОВ И ЕДИНИЧЕК... 1 0"
 *     FAV - идентификатор команды, FAV - избранное
 *     1 - режим "избранное" включен
 *     60 - интервал смены эффектов в секундах
 *     120 - случайный разброс смены эффектов (применяется дополнительно к интервалу) в секундах
 *     0 - признак "запомнить состояние" вкл/выкл режима "избранное" в энергонезависимую память
 *     оставшиеся цифры - признак (0/1) "эффект №... добавлен в избранные", где номер цифры соотвтетсвует номеру эффекта в списке (см. приложение)
 *     КОЛИЧЕСТВО ЦИФЕРОК ДОЛЖНО СООТВЕТСТВОВАТЬ КОЛИЧЕСТВУ ЭФФЕКТОВ!!!!!!
 *
 * Также лампа отправляет свое состояние в топик <TopicBase>/LedLamp_<ChipId>/snd в виде JSON;
 *    {"power":"ON","cycle":"OFF","effect":"111","bri":"15","spd":"33","sca":"58","sound":"ON","volume":"10","runt":"10","runc":"123","runf":"1","runc":"220","rnde":"0","rndc":"1","rndf":"0","tmr":59900"}
 *    Можно добавить (или заменить) еще какие-то переменные (данные) для вывода в ответ, но длина строки ответа должна быть меньше 255 байт. Формирование ответа-смотри файл FieryLedLamp_New_WEB.ino
*/

#include <AsyncMqttClient.h>
#include "pgmspace.h"
#include "Constants.h"
#include "Types.h"

// #define PUBLISH_STATE_IN_OLD_FORMAT                               // Нужно ли публиковать состояние лампы в старом формате

static const char TopicCmnd[]          PROGMEM = "cmnd";             // часть командных топиков (входящие команды лампе)
static const char TopicState[]         PROGMEM = "state";            // часть топиков состояния (ответ от лампы)
static const char TopicSnd[]           PROGMEM = "snd";              // часть топиков состояния в JSON формате (ответ от лампы)

static bool mqttIPaddr              = true;                          // true - IP адрес, false - URL адрес
static IPAddress MqttServer         ;                                // Строка с IP адресом MQTT брокера
static char MqttHost[33]            = "";                            // Строка с URL адресом MQTT брокера
static uint16_t MqttPort            = 1883U;                         // порт MQTT брокера
static char TopicBase[33]           = "";                            // базовая (корневая) часть топиков. Например "LedLamp"
static char MqttUser[33]            = "";                            // пользователь MQTT брокера
static char MqttPassword[33]        = "";                            // пароль пользователя MQTT брокера
static uint8_t MqttOn               = 1U;                            // MQTT on / off
static uint8_t MqttPeriod           = 0U;                            // Период публикации ответа лампы (0 – 60 секунд). Если 0-публиковать ответ только при наступлении события (изменение параметров).
static uint32_t mqtt_timer          = 0UL;                           // используется при периодической публикации раз в 1 секунду, когда запущен таймер
static const char MqttClientIdPrefix[] PROGMEM = "LedLamp_";         // id клиента MQTT брокера (к нему будет добавлен ESP.getChipId)


class MqttManager
{
  public:
    static uint32_t mqttLastConnectingAttempt;
    static void setupMqtt(AsyncMqttClient* mqttClient, char* lampInputBuffer, SendCurrentDelegate sendCurrentDelegate);
    static void mqttConnect();
    static void onMqttConnect(bool sessionPresent);
    static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    static void onMqttMessage(char *topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    static bool publish(const char *topic, const char *value);
    static void publishState(uint8_t flag = 0);
    static bool needToPublish;
    static char mqttBuffer[255U];
    static char* clientId;

  private:
    static char* topicInput;                                                    // TopicBase + '/' + MqttClientIdPrefix + ESP.getChipId + '/' + TopicCmnd
    #ifdef PUBLISH_STATE_IN_OLD_FORMAT
    static char* topicOutput;                                                   // TopicBase + '/' + MqttClientIdPrefix + ESP.getChipId + '/' + TopicState
    #endif
    static char* topicOutputJSON;                                               // TopicBase + '/' + MqttClientIdPrefix + ESP.getChipId + '/' + TopicSnd
    static char* lampInputBuffer;                                               // ссылка на inputBuffer для записи в него пришедшей MQTT команды и последующей её обработки лампой
    static AsyncMqttClient* mqttClient;
    static const uint8_t qos = 0U;                                              // MQTT quality of service
    static const uint32_t connectionTimeout = MQTT_RECONNECT_TIME * 1000U;      // период времени для проверки (пере)подключения к MQTT брокеру, 10 секунд
    static char* byteToHex(char *out, uint8_t value);
    static bool allocStr(char **str, const char *src);
    //static bool allocStr_P(char **str, PGM_P src);
    static SendCurrentDelegate sendCurrentDelegate;
};


void MqttManager::setupMqtt(AsyncMqttClient* mqttClient, char* lampInputBuffer, SendCurrentDelegate sendCurrentDelegate)
{
  MqttManager::mqttClient = mqttClient;
  MqttManager::lampInputBuffer = lampInputBuffer;
  MqttManager::sendCurrentDelegate = sendCurrentDelegate;
  if(mqttIPaddr)
      MqttManager::mqttClient->setServer(MqttServer, MqttPort); // Если введен IP адрес
  else
      MqttManager::mqttClient->setServer(MqttHost, MqttPort);  // Если введена URL

  char clientIdBuf[sizeof(MqttClientIdPrefix) + 8];
  strcpy_P(clientIdBuf, MqttClientIdPrefix);
  #ifdef ESP32_USED
  uint32_t chipId = get_Chip_ID();
  #else
  uint32_t chipId = ESP.getChipId();
  #endif
  for (uint8_t i = 0; i < 4; ++i)
  {
    byteToHex(&clientIdBuf[i * 2 + sizeof(MqttClientIdPrefix) - 1], chipId >> ((3 - i) * 8));
  }
  allocStr(&clientId, clientIdBuf);
  MqttManager::mqttClient->setClientId(clientId);

  if (MqttUser != NULL) // if (MqttManager::mqttUser != NULL)
  {
    MqttManager::mqttClient->setCredentials(MqttUser, MqttPassword);
  }

  uint8_t topicLength = strlen(TopicBase) + 1 + strlen(clientId) + 1 + sizeof(TopicCmnd) + 1;
  topicInput = (char*)malloc(topicLength);
  sprintf_P(topicInput, PSTR("%s/%s/%s"), TopicBase, clientId, TopicCmnd);      // topicInput = TopicBase + '/' + MqttClientIdPrefix + ESP.getChipId + '/' + TopicCmnd

  #ifdef PUBLISH_STATE_IN_OLD_FORMAT
  topicLength = strlen(TopicBase) + 1 + strlen(clientId) + 1 + sizeof(TopicState) + 1;
  topicOutput = (char*)malloc(topicLength);
  sprintf_P(topicOutput, PSTR("%s/%s/%s"), TopicBase, clientId, TopicState);    // topicOutput = TopicBase + '/' + MqttClientIdPrefix + ESP.getChipId + '/' + TopicState
  #endif

  topicLength = strlen(TopicBase) + 1 + strlen(clientId) + 1 + sizeof(TopicSnd) + 1;
  topicOutputJSON = (char*)malloc(topicLength);
  sprintf_P(topicOutputJSON, PSTR("%s/%s/%s"), TopicBase, clientId, TopicSnd);    // topicOutputJSON = TopicBase + '/' + MqttClientIdPrefix + ESP.getChipId + '/' + TopicSnd

  #ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR("MQTT топик для входящих команд: %s\n"), topicInput);
  #ifdef PUBLISH_STATE_IN_OLD_FORMAT
  LOG.printf_P(PSTR("MQTT топик для выходных ответов лампы: %s\n"), topicOutput);
  #endif
  LOG.printf_P(PSTR("MQTT топик для выходных ответов лампы в формате JSON: %s\n"), topicOutputJSON);
  #endif

  mqttClient->onConnect(onMqttConnect);
  mqttClient->onDisconnect(onMqttDisconnect);
  mqttClient->onMessage(onMqttMessage);
}

void MqttManager::mqttConnect()
{
  if ((!mqttLastConnectingAttempt) || (millis() - mqttLastConnectingAttempt >= connectionTimeout))
  {
    #ifdef GENERAL_DEBUG
    LOG.print(F("Подключение к MQTT брокеру \""));
    if(mqttIPaddr)
        LOG.print(MqttServer);
    else
        LOG.print(MqttHost);
    LOG.print(':');
    LOG.print(MqttPort);
    LOG.println(F("\"..."));
    #endif
    mqttClient->disconnect();
    mqttClient->connect();
    mqttLastConnectingAttempt = millis();
  }
}

bool MqttManager::publish(const char *topic, const char *value)
{
  if (mqttClient->connected())
  {
    #ifdef GENERAL_DEBUG
    LOG.print(F("Отправлено MQTT: топик \""));
    LOG.print(topic);
    LOG.print(F("\", значение \""));
    LOG.print(value);
    LOG.println('"');
    LOG.println();
    #endif

    return mqttClient->publish(topic, qos, true, value, 0) != 0;
  }

  return false;
}

void MqttManager::onMqttConnect(bool sessionPresent)
{
  #ifdef GENERAL_DEBUG
  LOG.println(F("Подключен к MQTT брокеру"));
  #endif
  mqttLastConnectingAttempt = 0;

  mqttClient->subscribe(topicInput, 1);
  //publishState(0);
  needToPublish = true;
}

void MqttManager::onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  #ifdef GENERAL_DEBUG
  LOG.println(F("Отключен от брокера MQTT"));
  #endif
}

void MqttManager::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  if (payload != NULL)                                      // сохраняем пришедшее MQTT сообщение для дальнейшей обработки
  {
    strncpy(lampInputBuffer, payload, len);
    lampInputBuffer[len] = '\0';
    needToPublish = true;
  }

  #ifdef GENERAL_DEBUG
  LOG.print(F("Получен MQTT:"));
  LOG.print(F(" топик \""));
  LOG.print(topic);
  LOG.print("\"");
  /*
  LOG.print(F(" qos: "));
  LOG.println(properties.qos);
  LOG.print(F(" dup: "));
  LOG.println(properties.dup);
  LOG.print(F(" retain: "));
  LOG.println(properties.retain);
  LOG.print(F(" len: "));
  LOG.println(len);
  LOG.print(F(" index: "));
  LOG.println(index);
  LOG.print(F(" total: "));
  LOG.println(total);
  */
  LOG.print(F(", значение \""));
  LOG.print(lampInputBuffer);
  LOG.println("\"");
  LOG.println();
  #endif
}

void MqttManager::publishState(uint8_t flag)
{
  if(!flag){                                                // если flag == 0 публикуем буфер в топик <TopicBase>/LedLamp_<ChipId>/state
  #ifdef PUBLISH_STATE_IN_OLD_FORMAT
  if (mqttBuffer == NULL || strlen(mqttBuffer) <= 0)
  {
    sendCurrentDelegate(mqttBuffer);                        // если буфер MQTT ответа не задан, но метод MQTT публикации вызван, закполняем его текущим состоянием лампы
  }

  if (mqttBuffer != NULL && strlen(mqttBuffer) > 0)
  {
    publish(topicOutput, mqttBuffer);                       // публикация буфера MQTT ответа
    mqttBuffer[0] = '\0';                                   // очистка буфера
    needToPublish = false;                                  // сброс флага для предотвращения повторной публикации
  }
  #endif // PUBLISH_STATE_IN_OLD_FORMAT
  ;
  }
  else{                                                     // иначе публикуем буфер в топик <TopicBase>/LedLamp_<ChipId>/snd
    publish(topicOutputJSON, mqttBuffer);                   // публикация буфера MQTT ответа (JSON)
    //Serial.print("topicOutputJSON = ");
    //Serial.println(topicOutputJSON);
    mqttBuffer[0] = '\0';                                   // очистка буфера
    needToPublish = false;                                  // сброс флага для предотвращения повторной публикации
  } 
}

char* MqttManager::byteToHex(char *out, uint8_t value)
{
  uint8_t b;

  b = value >> 4;
  if (b < 10)
  {
    out[0] = '0' + b;
  }
  else
  {
    out[0] = 'A' + (b - 10);
  }
  b = value & 0x0F;
  if (b < 10)
  {
    out[1] = '0' + b;
  }
  else
  {
    out[1] = 'A' + (b - 10);
  }
  out[2] = '\0';

  return out;
}

bool MqttManager::allocStr(char **str, const char *src)
{
  if (src && *src)
  {
    if (*str)
    {
      void *ptr = realloc(*str, strlen(src) + 1);

      if (!ptr)
      {
        return false;
      }
      *str = (char*)ptr;
    }
    else
    {
      *str = (char*)malloc(strlen(src) + 1);
      if (!*str)
      {
        return false;
      }
    }
    strcpy(*str, src);
  }
  else
  {
    if (*str)
    {
      free(*str);
      *str = NULL;
    }
  }

  return true;
}
/*
bool MqttManager::allocStr_P(char **str, PGM_P src)
{
  if (src && pgm_read_byte(src))
  {
    if (*str)
    {
      void *ptr = realloc(*str, strlen_P(src) + 1);

      if (!ptr)
      {
        return false;
      }
      *str = (char*)ptr;
    }
    else
    {
      *str = (char*)malloc(strlen_P(src) + 1);
      if (!*str)
      {
        return false;
      }
    }
    strcpy_P(*str, src);
  }
  else
  {
    if (*str)
    {
      free(*str);
      *str = NULL;
    }
  }

  return true;
}
*/
#endif
