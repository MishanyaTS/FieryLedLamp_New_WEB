//
// ======================= ВНИМАНИЕ !!! =============================
// Все настройки производятся на вкладке Constants.h
// Почитайте там то, что на русском языке написано.
// Или ничего не трогайте, если собирали, по схемам из этого архива.
// В любом случае ВНИМАТЕЛЬНО прочтите файл ПРОЧТИ МЕНЯ!!!.txt из этого архива.
// ==================================================================
// Ссылка для менеджера плат:
// ESP8266 :
// https://arduino.esp8266.com/stable/package_esp8266com_index.json
// При установке выбираем версию 2.7.4
//
// ESP32 :
// https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
// Проверялось на ядре 1.0.6 и 2.0.14. На ядре 1.0.6 код занимает на 4% меньше места.
// При не хватки памяти, смотреть файл "Увеличение раздела" в архиве "Hack"
// Для использования ядра 1.0.6 закомментируйте соответствующую строку ниже.
// Выбираем плату ESP32 Dev Module. На платах ESP32S2, ESP32S3, ESP32C2 работа не проверялась
// =================================================================

// ******************************* - ВЫБОР ПЛАТЫ И ЯДРА- *******************************************
#ifdef ESP32           // Определено в IDE если используется ESP32 (не нужно изменять для ESP8266)
 #define ESP32_USED    // Используется контоллер ESP32 (не нужно менять для ESP8266)
 #define CORE_2_0_X    // Если используется ядро ​​ESP32 версии 1.0.Х, закомментируйте эту строку
#endif
//===================================================================================================
// Далее следует код проекта. Не меняйте здесь ничего, если вы не понимаете, к чему это приведет!!!
//===================================================================================================

#include <pgmspace.h>
#ifdef ESP32_USED
 #include "esp_wifi.h"                // Борьба с рестартом esp32 "assertion "Invalid mbox""
 #include "nvs_flash.h"               // Борьба с рестартом esp32 "assertion "Invalid mbox""
 #include <WiFi.h>
 #include <WiFiClient.h>
 #include <WiFiAP.h>
 #include <WebServer.h>
 #include <ESP32SSDP.h>               // https://github.com/luc-github/ESP32SSDP
 #include <time.h>
 #include <HardwareSerial.h>          // Используется аппаратный UART
 #include "esp_system.h"
 #include "esp_int_wdt.h"
 #include "esp_task_wdt.h"
 #include <ElegantOTA.h>
#else
 #include <ESP8266SSDP.h>
 #include <ESP8266WiFi.h>
 #include <ESP8266WebServer.h>
 #include <ElegantOTA.h>
 #define FASTLED_USE_PROGMEM 1        // просим библиотеку FASTLED экономить память контроллера на свои палитры
#endif

#include <FastLED.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <TimeLib.h>
#include "Constants.h"
#ifdef USE_RTC
  #ifdef RTC_3231
  #include <Wire.h>
    #include <RtcDS3231.h>
  #endif
#endif
#ifdef ESP_USE_BUTTON
#include <GyverButton.h>
#endif
#ifdef USE_NTP
#include <NTPClient.h>
#include <Timezone.h>
#endif
#include "Types.h"
#include "timerMinim.h"
#include "fonts.h"
#ifdef OTA
#include "OtaManager.h"
#endif
#if USE_MQTT
#include "MqttManager.h"
#endif
#include "EepromManager.h"
#include "FavoritesManager.h"
#include "TimerManager.h"
#ifdef USE_BLYNK
 #ifdef ESP32_USED
  #include <BlynkSimpleEsp32.h>
 #else
  #include <BlynkSimpleEsp8266.h>
 #endif
#endif
//#ifdef ESP32_USED
// #include "esp_system.h"
// #include "esp_int_wdt.h"
// #include "esp_task_wdt.h"
//#endif
#ifdef TM1637_USE
#include "TM1637Display.h"
#endif
#ifdef MP3_PLAYER_USE
 #ifndef ESP32_USED
  #include <SoftwareSerial.h>     // Подключаем библиотеку для работы с последовательным интерфейсом
 #endif
 #ifdef MP3_DEBUG
  #define FEEDBACK  1
 #else
  #define FEEDBACK  0
 #endif  //MP3_DEBUG
#endif  // MP3_PLAYER_USE
#ifdef IR_RECEIVER_USE
 #include <IRremoteESP8266.h>  // https://github.com/crankyoldgit/IRremoteESP8266
 #include <IRrecv.h>
 #include "IR_Receiver.h"
#endif  //IR_RECEIVER_USE


// --- ИНИЦИАЛИЗАЦИЯ ОБЪЕКТОВ ----------
#ifdef USE_RTC
  #ifdef RTC_3231
    RtcDS3231<TwoWire> Rtc(Wire);
  #endif
RtcDateTime timeToSet;
#endif

CRGB leds[NUM_LEDS];
WiFiUDP Udp;

#ifdef USE_NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, 0, NTP_INTERVAL); // объект, запрашивающий время с ntp сервера; в нём смещение часового пояса не используется (перенесено в объект localTimeZone); здесь всегда должно быть время UTC
   TimeChangeRule summerTime  = { SUMMER_TIMEZONE_NAME, SUMMER_WEEK_NUM, SUMMER_WEEKDAY, SUMMER_MONTH, SUMMER_HOUR, 0 };
   TimeChangeRule winterTime  = { WINTER_TIMEZONE_NAME, WINTER_WEEK_NUM, WINTER_WEEKDAY, WINTER_MONTH, WINTER_HOUR, 0 };
   Timezone localTimeZone(summerTime, winterTime);
  #ifdef PHONE_N_MANUAL_TIME_PRIORITY
    bool stillUseNTP = true;
  #endif    
#endif

timerMinim timeTimer(3000);
bool ntpServerAddressResolved = false;
bool timeSynched = false;
uint32_t lastTimePrinted = 0U;

#if defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE)
time_t manualTimeShift;
#endif

#ifdef GET_TIME_FROM_PHONE
time_t phoneTimeLastSync;
#endif

uint8_t selectedSettings = 0U;

#ifdef ESP_USE_BUTTON
#if (BUTTON_IS_SENSORY == 1)
GButton touch(BTN_PIN, LOW_PULL, NORM_OPEN);  // для сенсорной кнопки LOW_PULL
#else
GButton touch(BTN_PIN, HIGH_PULL, NORM_OPEN); // для физической (не сенсорной) кнопки HIGH_PULL. ну и кнопку нужно ставить без резистора в разрыв между пинами D2 и GND
#endif
#endif

#ifdef OTA
OtaManager otaManager(&showWarning);
OtaPhase OtaManager::OtaFlag = OtaPhase::None;
#endif

#if USE_MQTT
AsyncMqttClient* mqttClient = NULL;
AsyncMqttClient* MqttManager::mqttClient = NULL;
//char* MqttManager::mqttServer = NULL;
//char* MqttManager::mqttUser = NULL;
//char* MqttManager::mqttPassword = NULL;
char* MqttManager::clientId = NULL;
char* MqttManager::lampInputBuffer = NULL;
char* MqttManager::topicInput = NULL;
#ifdef PUBLISH_STATE_IN_OLD_FORMAT
char* MqttManager::topicOutput = NULL;
#endif
char* MqttManager::topicOutputJSON = NULL;
bool MqttManager::needToPublish = false;
char MqttManager::mqttBuffer[] = {};
uint32_t MqttManager::mqttLastConnectingAttempt = 0;
SendCurrentDelegate MqttManager::sendCurrentDelegate = NULL;
#endif

#ifdef ESP32_USED
 WebServer server(80);
 WebServer HTTP (ESP_HTTP_PORT);         // Объект для обнавления с web страницы
#else
 ESP8266WebServer server(80);
 ESP8266WebServer HTTP (ESP_HTTP_PORT);  // Web интерфейс для устройства
#endif
File fsUploadFile;                       // Для файловой системы

// --- ИНИЦИАЛИЗАЦИЯ ПЕРЕМЕННЫХ -------
uint16_t localPort = ESP_UDP_PORT;
char packetBuffer[MAX_UDP_BUFFER_SIZE];  // buffer to hold incoming packet
char inputBuffer[MAX_UDP_BUFFER_SIZE];
static const uint8_t maxDim = max(WIDTH, HEIGHT);


AlarmType alarms[7];

static const uint8_t dawnOffsets[] PROGMEM = {5, 10, 15, 20, 25, 30, 40, 50, 60};   // опции для выпадающего списка параметра "время перед 'рассветом'" (будильник); синхронизировано с android приложением
uint8_t dawnMode;
uint8_t dawnFlag = 0;
uint32_t thisTime;
bool manualOff = false;

int16_t offset = WIDTH;
uint32_t scrollTimer = 0LL;

uint8_t currentMode;
bool loadingFlag = true;
bool ONflag = false;
//uint32_t eepromTimeout;
//bool settChanged = false;
#ifdef ESP_USE_BUTTON
 bool buttonEnabled = true; // Вкл \ откл кнопки
 #if defined(BUTTON_LOCK_ON_START)
  bool buttonBlocing = false;
 #endif
#endif
unsigned char matrixValue[8][16]; //это массив для эффекта Огонь

bool TimerManager::TimerRunning = false;
bool TimerManager::TimerHasFired = false;
uint8_t TimerManager::TimerOption = 1U;
uint32_t TimerManager::TimeToFire = 0ULL;

uint8_t FavoritesManager::FavoritesRunning = 0;
uint16_t FavoritesManager::Interval = DEFAULT_FAVORITES_INTERVAL;
uint16_t FavoritesManager::Dispersion = DEFAULT_FAVORITES_DISPERSION;
uint8_t FavoritesManager::UseSavedFavoritesRunning = 0;
uint8_t FavoritesManager::FavoriteModes[MODE_AMOUNT] = {0};
uint32_t FavoritesManager::nextModeAt = 0UL;
bool FavoritesManager::rndCycle = false;

char TextTicker [86];
int Painting = 0; CRGB DriwingColor = CRGB(255, 255, 255);

//..................... Переменные, добавленные с внедрением web интерфейса .............................................................................................

uint8_t FPSdelay = DYNAMIC;
uint8_t espMode ;
uint8_t random_on;
uint8_t Favorit_only;
//uint32_t my_time;
uint32_t my_timer;
uint8_t time_always;
bool connect = false;
uint32_t lastResolveTryMoment = 0xFFFFFFFFUL;
uint8_t ESP_CONN_TIMEOUT;
uint8_t PRINT_TIME ;
uint8_t day_night = false;     // если день - true, ночь - false
uint8_t save_file_changes =0;
uint32_t timeout_save_file_changes;
uint8_t first_entry = 0;
uint16_t dawnPosition;

#ifdef USE_MULTIPLE_LAMPS_CONTROL
char Host1[16], Host2[16], Host3[16], Host4[16], Host5[16];
uint8_t ml1, ml2, ml3, ml4, ml5;
#endif //USE_MULTIPLE_LAMPS_CONTROL

#ifdef MP3_PLAYER_USE
uint8_t mp3_folder=1;                // Текущая папка для воспроизведения.
uint8_t alarm_sound_on =false;       // Включить/выключить звук будильника
uint8_t alarm_volume;                // Громкость будильника
uint8_t AlarmFolder;                 // Папка будильника
uint8_t Equalizer;                   // Эквалайзер
bool alarm_sound_flag =false;        // Проигрывается ли сейчас будильник
uint8_t dawnflag_sound = false;      // Звук не начал обслуживание рассвета. Если не true - звук обслуживает рассвет
//uint8_t tmp_fold;
bool advert_flag = false;            // Озвучивается время
bool advert_hour;                    // Озвучиваются часы времени
uint8_t day_advert_volume;           // Дневная громкость озвучивания времени
uint8_t night_advert_volume;         // Ночная громкость озвучивания времени
bool day_advert_sound_on;            // Вкл.Выкл озвучивания времени днём
bool night_advert_sound_on;          // Вкл.Выкл озвучивания времени ночью
bool alarm_advert_sound_on;          // Вкл.Выкл озвучивания времени будильником
uint8_t mp3_player_connect = 0;      // Плеер не подключен. true - подключен.
uint8_t mp3_folder_last=255;         // Предыдущая папка для воспроизведения
//uint8_t mp3_folder_change =0;        // Указывает, была ли изменена папка
bool set_mp3_play_now=false;         // Указывает, надо ли играть сейчас мелодии
uint32_t alarm_timer;                // Периодичность проверки и плавного изменения громкости будильника
uint32_t mp3_timer = 0;
bool mp3_stop = true;                        // Озвучка эффектов остановлена
bool pause_on = true;                        // Озвучка эффектов на паузе. false - не на паузе
uint8_t eff_volume = 9;                      // Громкость воспроизведения
uint8_t eff_sound_on = 0;                    // Звук включен - !0 (true), выключен - 0
uint8_t CurrentFolder;                       // Папка, на которую переключились (будет проигрываться)
uint8_t CurrentFolder_last = 0;              // Предыдущая текущая папка
#ifdef ESP32_USED
 HardwareSerial mp3(1);  // Используем UART1
#else
 SoftwareSerial mp3(MP3_RX_PIN, MP3_TX_PIN);  // Создаём объект mySoftwareSerial и указываем выводы, к которым подлючен плеер (RX, TX)
#endif
//#ifndef TM1637_USE
// uint8_t minute_tmp;
//#endif
uint8_t mp3_receive_buf[10];
uint8_t effects_folders[MODE_AMOUNT];    // Номера папок для озвучивания
uint16_t ADVERT_TIMER_H, ADVERT_TIMER_M; // Продолжительность озвучивания часов и минут
uint8_t mp3_delay;                       // Задержка между командами проигрывателя
uint8_t send_sound = 1;                  // Передавать или нет сомнительным параметрам звука (папка,озвучивание_on/off,громкость)
uint8_t send_eff_volume = 1;             // Передавать или нет озвучивания_on/off, громкость
#endif  // MP3_PLAYER_USE
#ifdef TM1637_USE
uint8_t DispBrightness = 1;          // +++ Яркость дисплея от 0 до 255(5 уровней яркости с шагом 51). 0 - дисплей погашен 
bool dotFlag = false;                // +++ Флаг: в часах рисуется двоеточие или нет
uint32_t tmr_clock = 0;              // +++ Таймер мигания разделителя часов на дисплее
uint32_t tmr_blink = 0;              // +++ Таймер плавного изменения яркости дисплея
TM1637Display display(CLK, DIO);     // +++ Подключаем дисплей
bool aDirection = false;             // +++ Направление изменения яркости
uint32_t DisplayTimer;               // Время отображения номера эффекта
uint8_t LastEffect = 255;            // Последний Проигрываемый эффект
uint8_t DisplayFlag=0;               // Флаг, показывающий, что отображается номер эффекта и папки
 #ifdef MP3_PLAYER_USE
 uint8_t LastCurrentFolder = 255;    // Проигрываемая папка
 #endif  // MP3_PLAYER_USE
#endif  //TM1637_USE

#ifdef HEAP_SIZE_PRINT
uint32_t mem_timer;
#endif //HEAP_SIZE_PRINT 

#ifdef IR_RECEIVER_USE
 uint32_t IR_Code = 0x00000000;
 uint32_t IR_Repeat_Timer;
 uint32_t IR_Tick_Timer;
 uint32_t IR_Dgit_Enter_Timer;
 uint8_t Repeat; 
 uint8_t IR_Data_Ready;
 uint8_t Enter_Digit_1;
 uint8_t Enter_Number;

 IRrecv irrecv(IR_RECEIVER_PIN);
 decode_results results;
#endif  //IR_RECEIVER_USE

uint8_t RuninTextOverEffects = 0;
uint32_t Last_Time_RuninText = 0;
bool Fill_String = false;
uint8_t SpeedRunningText;
uint8_t ColorRunningText;
uint8_t ColorTextFon;

uint8_t use_static_ip = 0;
IPAddress Static_IP;//(192,168,0,17);  // Статический IP
IPAddress Gateway;//(192,168,0,1);     // Шлюз
IPAddress Subnet;//(255,255,255,0);    // маска подсети
IPAddress DNS1;//(208,67,222,222);     // Серверы DNS. Можно также DNS1(1,1,1,1) или DNS1(8,8,4,4);
IPAddress DNS2(8,8,8,8);               // Резервный DNS

uint8_t C_flag = 0;                    // Служебное
uint16_t current_limit;                // Лимит настраиваемого тока
uint8_t last_minute;                   // Минуты
uint8_t hours;                         // Часы
//uint8_t last_hours; 
uint8_t AutoBrightness;                // Автояркость on/off
uint8_t last_day_night = 0;
bool hasRtc = true;

#ifdef USE_RTC
bool wasError(const char* errorTopic = "")
{
  #ifdef RTC_3231
    uint8_t error = Rtc.LastError();
    if (error != 0)
    {
        LOG.println(errorTopic);
        LOG.println(error);

        switch (error)
        {
        case Rtc_Wire_Error_None:
            LOG.println(F("(none?!)"));
            break;
        case Rtc_Wire_Error_TxBufferOverflow:
            LOG.println(F("transmit buffer overflow"));
            break;
        case Rtc_Wire_Error_NoAddressableDevice:
            LOG.println(F("no device responded"));
            hasRtc = false;
            break;
        case Rtc_Wire_Error_UnsupportedRequest:
            LOG.println(F("device doesn't support request"));
            break;
        case Rtc_Wire_Error_Unspecific:
            LOG.println(F("unspecified error"));
            break;
        case Rtc_Wire_Error_CommunicationTimeout:
            LOG.println(F("communications timed out"));
            hasRtc = false;
            break;
        }
        return true;
    }
#endif
    return false;
}
#endif

void setup()  //==================================================================  void setup()  =========================================================================
{

  Serial.begin(115200);
  delay(300);
  #ifdef ESP32_USED
  esp_task_wdt_init(8, true);   // Initialize the task watchdog timer
  #else
  ESP.wdtEnable(WDTO_8S);
  #endif

  LOG.print(F("\n\n\nSYSTEM START"));
  #ifdef ESP32
  LOG.print (F("  ESP32\n"));
  #endif
  #ifdef ESP8266
  LOG.print (F("  ESP8266\n"));
  #endif

#ifndef USE_RTC
  hasRtc = false;
#endif

  #if defined(ESP_USE_BUTTON) && defined(BUTTON_LOCK_ON_START)
    #if (BUTTON_IS_SENSORY == 1)
        if (digitalRead(BTN_PIN)) {
            buttonBlocing = true;
        }
    #endif
    #if (BUTTON_IS_SENSORY == 0)
        if (!digitalRead(BTN_PIN)) {
            buttonBlocing = true;
        }
    #endif
#endif

  // ПИНЫ
  #ifdef MOSFET_PIN                                         // инициализация пина, управляющего MOSFET транзистором в состояние "выключен"
  pinMode(MOSFET_PIN, OUTPUT);
  #ifdef MOSFET_LEVEL
  digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
  #endif
  #endif

  #ifdef ALARM_PIN                                          // инициализация пина, управляющего будильником в состояние "выключен"
  pinMode(ALARM_PIN, OUTPUT);
  #ifdef ALARM_LEVEL
  digitalWrite(ALARM_PIN, !ALARM_LEVEL);
  #endif
  #endif
  
  // часы
#ifdef TM1637_USE
  LOG.print(F("\nСтарт дисплея TM1637\n"));
  tmr_clock = millis();                                     // +++ устанавливаем начальное значение счетчика
  display.setBrightness(DispBrightness);                    // +++ яркость дисплея максимальная = 255
  display.displayByte(_empty, _empty, _empty, _empty);      // +++ очистка дисплея
  display.displayByte(_dash, _dash, _dash, _dash);          // +++ отображаем прочерки
#endif

   //File Fystem
  #ifdef GENERAL_DEBUG  
  LOG.print(F("\nСтарт файловой системы\n"));
  #endif
  FS_init();  //Запускаем файловую систему
  #ifdef GENERAL_DEBUG
  LOG.print(F("Чтение файла конфигурации\n"));
  #endif
  configSetup = readFile(F("config.json"), 2048);
  #ifdef GENERAL_DEBUG  
  LOG.println(configSetup);
  #endif
  //Настраиваем и запускаем SSDP интерфейс
  #ifdef GENERAL_DEBUG
  LOG.print(F("Старт SSDP\n"));
  #endif
  SSDP_init();

  
//-----------Инициализируем переменные, хранящиеся в файле config.json--------------
  LAMP_NAME = jsonRead(configSetup, "SSDP");
  AP_NAME = jsonRead(configSetup, "ssidAP");
  AP_PASS = jsonRead(configSetup, "passwordAP");
  Favorit_only = jsonReadtoInt(configSetup, "favorit");
  random_on = jsonReadtoInt(configSetup, "random_on");
  espMode = jsonReadtoInt(configSetup, "ESP_mode");
  PRINT_TIME = jsonReadtoInt(configSetup, "print_time");
  #ifdef ESP_USE_BUTTON
   buttonEnabled = jsonReadtoInt(configSetup, "button_on");
  #endif
  ESP_CONN_TIMEOUT = jsonReadtoInt(configSetup, "TimeOut");
  time_always = jsonReadtoInt(configSetup, "time_always");
  (jsonRead(configSetup, "run_text")).toCharArray (TextTicker, (jsonRead(configSetup, "run_text")).length()+1);
  NIGHT_HOURS_START = 60U * jsonReadtoInt(configSetup, "night_time");
  NIGHT_HOURS_BRIGHTNESS = jsonReadtoInt(configSetup, "night_bright");
  NIGHT_HOURS_STOP = 60U * jsonReadtoInt(configSetup, "day_time");
  DAY_HOURS_BRIGHTNESS = jsonReadtoInt(configSetup, "day_bright");
  DONT_TURN_ON_AFTER_SHUTDOWN = jsonReadtoInt(configSetup, "effect_always"); 
  FavoritesManager::rndCycle = jsonReadtoInt(configSetup, "rnd_cycle");  // Перемешать Цикл
  AUTOMATIC_OFF_TIME = (30UL * 60UL * 1000UL) * ( uint32_t )(jsonReadtoInt(configSetup, "timer5h"));
  RuninTextOverEffects = jsonReadtoInt(configSetup, "toe");  // Выводить бегущую поверх эффектов строку
  SpeedRunningText = jsonReadtoInt(configSetup, "spt");      // Скорость бегущей строки
  ColorRunningText = jsonReadtoInt(configSetup, "sct");      // Цвет бегущей строки
  ColorTextFon = jsonReadtoInt(configSetup, "ctf");          // Выводить бегущую строку на цветном фоне
  jsonWrite(configSetup, "ver", FLL_VERSION);                // Версия ПО
  AutoBrightness = jsonReadtoInt(configSetup, "auto_bri");   // Автоматическое понижение яркости on/off
  #ifdef USE_NTP
  (jsonRead(configSetup, "ntp")).toCharArray (NTP_ADDRESS, (jsonRead(configSetup, "ntp")).length()+1);
  #endif
  #ifdef USE_NTP
  winterTime.offset = jsonReadtoInt(configSetup, "timezone") * 60;
  summerTime.offset = winterTime.offset + jsonReadtoInt(configSetup, "Summer_Time") *60;
  localTimeZone.setRules (summerTime, winterTime);
  #endif
  
  #ifdef USE_RTC
    #ifdef RTC_3231
    Wire.begin(I2C_SDA, I2C_SCL);
    #endif
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    time_t utcCompiledUnix = localTimeZone.toUTC(compiled.Epoch32Time());
    RtcDateTime utcCompiled;
    utcCompiled.InitWithEpoch32Time(utcCompiledUnix);
    printDateTime(utcCompiled);

    if (!Rtc.IsDateTimeValid())
    {
        if (!wasError("setup IsDateTimeValid"))
        {
            // Common Causes:
            //    1) first time you ran and the device wasn't running yet
            //    2) the battery on the device is low or even missing
            LOG.println(F("RTC lost confidence in the DateTime!"));
            // following line sets the RTC to the date & time this sketch was compiled
            // it will also reset the valid flag internally unless the Rtc device is
            // having an issue
            Rtc.SetDateTime(utcCompiled);
        }
    }

    if (!Rtc.GetIsRunning())
    {
        if (!wasError("setup GetIsRunning"))
        {
            LOG.println(F("RTC was not actively running, starting now"));
            Rtc.SetIsRunning(true);
        }
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (!wasError("setup GetDateTime"))
    {
        if (now < utcCompiled)
        {
            LOG.println(F("RTC is older than compile time, updating DateTime"));
            Rtc.SetDateTime(utcCompiled);
        }
        else if (now > utcCompiled)
        {
            LOG.println(F("RTC is newer than compile time, this is expected"));
        }
        else if (now == utcCompiled)
        {
            LOG.println(F("RTC is the same as compile time, while not expected all is still fine"));
        }
    }
    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    #ifdef RTC_3231
    Rtc.Enable32kHzPin(false);
    wasError("setup Enable32kHzPin");
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
    wasError("setup SetSquareWavePin");
    #endif
  #endif //USE_RTC
  
  #ifdef MP3_PLAYER_USE
  eff_volume = jsonReadtoInt(configSetup, "vol");
  eff_sound_on = (jsonReadtoInt(configSetup, "on_sound")==0)? 0 : eff_volume;
  alarm_volume = jsonReadtoInt(configSetup, "alm_vol");
  AlarmFolder = jsonReadtoInt(configSetup, "alm_fold");
  alarm_sound_on = jsonReadtoInt(configSetup, "on_alm_snd");
  day_advert_sound_on = jsonReadtoInt(configSetup,"on_day_adv");
  night_advert_sound_on = jsonReadtoInt(configSetup,"on_night_adv");
  day_advert_volume = jsonReadtoInt(configSetup,"day_vol");
  alarm_advert_sound_on = jsonReadtoInt(configSetup,"on_alm_adv");
  night_advert_volume = jsonReadtoInt(configSetup,"night_vol");
  Equalizer = jsonReadtoInt(configSetup, "eq");
  send_sound = jsonReadtoInt(configSetup, "s_s");
  send_eff_volume = jsonReadtoInt(configSetup, "s_e_v");
  #endif // MP3_PLAYER_USE
  {
  String configHardware = readFile(F("config_hardware.json"), 1024);    
  current_limit = jsonReadtoInt(configHardware, "cur_lim");
  MATRIX_TYPE = jsonReadtoInt(configHardware, "m_t");
  ORIENTATION = jsonReadtoInt(configHardware, "m_o");
  #ifdef MP3_PLAYER_USE
  ADVERT_TIMER_H = 100 * jsonReadtoInt(configHardware, "tim_h");
  ADVERT_TIMER_M = 100 * jsonReadtoInt(configHardware, "tim_m");
  mp3_delay = 10 * jsonReadtoInt(configHardware, "delay");
  #ifdef GENERAL_DEBUG
     LOG.print (F("\nADVERT_TIMER_H = "));
     LOG.println (ADVERT_TIMER_H);
     LOG.print (F("ADVERT_TIMER_M = "));
     LOG.println (ADVERT_TIMER_M);
     LOG.print (F("mp3_delay = "));
     LOG.println (mp3_delay);
  #endif

  #endif
  }
  {
    String Name = F("correct.");
    Name.reserve(17);
    Name += jsonRead (configSetup, "lang");
    Name += F(".json");
    String Correct = readFile(Name, 2048);
    for ( uint8_t n=0; n< MODE_AMOUNT; n++)
    {
        eff_num_correct[n] = jsonReadtoInt (Correct, String(n));
    }
  }

  {
  String configIP = readFile(F("config_ip.json"), 512);
  use_static_ip = jsonReadtoInt(configSetup, "s_IP");
  Static_IP.fromString(jsonRead(configIP, "ip"));
  Gateway.fromString(jsonRead(configIP, "gateway"));
  Subnet.fromString(jsonRead(configIP, "subnet"));
  DNS1.fromString(jsonRead(configIP, "dns"));
  #ifdef GENERAL_DEBUG
     LOG.print (F("\nUse Static IP = "));
     LOG.println (use_static_ip);
     LOG.print (F("Static IP = "));
     LOG.println (Static_IP);
     LOG.print (F("Gateway = "));
     LOG.println (Gateway);
     LOG.print (F("Subnet = "));
     LOG.println (Subnet);
     LOG.print (F("DNS1 = "));
     LOG.println (DNS1);
     LOG.print (F("DNS2 = "));
     LOG.println (DNS2);
  #endif
  }

  // TELNET
  #if defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET
  telnetServer.begin();
  for (uint8_t i = 0; i < 100; i++)                         // пауза 10 секунд в отладочном режиме, чтобы успеть подключиться по протоколу telnet до вывода первых сообщений
  {
    handleTelnetClient();
    delay(100);
    #ifdef ESP32_USED
     esp_task_wdt_reset();
    #else
     ESP.wdtFeed();
    #endif
  }
  #endif


  // ЛЕНТА/МАТРИЦА
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS)/*.setCorrection(TypicalLEDStrip)*/;
  //FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(0xB0FFE0); // Калибровка баланса белого цвета. Последовательность байт RGB (B0-R FF-G E0-B)
  FastLED.setBrightness(BRIGHTNESS);
  if (current_limit > 0)
  {
    FastLED.setMaxPowerInVoltsAndMilliamps(5, current_limit);
  }
  FastLED.clear();
  FastLED.show();


  // КНОПКА
  #if defined(ESP_USE_BUTTON)
  touch.setStepTimeout(BUTTON_STEP_TIMEOUT);
  touch.setClickTimeout(BUTTON_CLICK_TIMEOUT);
  touch.setDebounce(BUTTON_SET_DEBOUNCE);
    #ifdef BUTTON_LOCK_ON_START
    if (buttonBlocing) {
       buttonEnabled = false;
       //jsonWrite(configSetup, "button_on", buttonEnabled);
       //saveConfig();
    }
    #ifdef ESP32_USED
     esp_task_wdt_reset();
    #else
     ESP.wdtFeed();
    #endif
    #endif
  #endif

#ifdef USE_SHUFFLE_FAVORITES // первоначальная очередь избранного до перемешивания
    for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      shuffleFavoriteModes[i] = i;
#endif


  // EEPROM
  EepromManager::InitEepromSettings(modes, &(restoreSettings)); // инициализация EEPROM; запись начального состояния настроек, если их там ещё нет; инициализация настроек лампы значениями из EEPROM
 // не придумал ничего лучше, чем делать восстановление настроек по умолчанию в обработчике инициализации EepromManager
    

  if(DONT_TURN_ON_AFTER_SHUTDOWN){
      ONflag = false;
  jsonWrite(configSetup, "Power", ONflag);
  }
  else
      ONflag = jsonReadtoInt (configSetup, "Power");  // Чтение состояния лампы вкл/выкл,текущий эффект,яркость,скорость,масштаб

  currentMode = eff_num_correct[jsonReadtoInt (configSetup, "eff_sel")];
  modes[currentMode].Brightness = jsonReadtoInt (configSetup, "br");
  modes[currentMode].Speed = jsonReadtoInt (configSetup, "sp");
  modes[currentMode].Scale = jsonReadtoInt (configSetup, "sc");
  first_entry = 1;
  handle_alarm ();
  first_entry = 0;
  FavoritesManager::FavoritesRunning = jsonReadtoInt(configSetup, "cycle_on");  // Чтение состояния настроек режима Цикл 
  FavoritesManager::Interval = jsonReadtoInt(configSetup, "time_eff");          // Вкл/выкл,время переключения, дисперсия, вкл цикла после перезагрузки
  FavoritesManager::Dispersion = jsonReadtoInt(configSetup, "disp");
  FavoritesManager::UseSavedFavoritesRunning = jsonReadtoInt(configSetup, "cycle_allwase");
  jsonWrite(configSetup, "tmr", 0);
  #ifdef ESP_USE_BUTTON
  jsonWrite(configSetup, "button_on", buttonEnabled);
  #endif
  first_entry = 1;
  handle_cycle_set();  // Чтение выбранных эффектов
  first_entry = 0;
#ifdef MP3_PLAYER_USE
  first_entry = 1;
  handle_sound_set();  //Чтение выбранных папок
  first_entry = 0;
#endif  // MP3_PLAYER_USE
#ifdef USE_MULTIPLE_LAMPS_CONTROL  
  multilamp_get ();   // Чтение из файла адресов синхронно управляемых ламп 
#endif //USE_MULTIPLE_LAMPS_CONTROL
  
  // MP3 Player
   
  #ifdef MP3_PLAYER_USE
   #ifdef ESP32_USED
    mp3.begin(9600, SERIAL_8N1, MP3_RX_PIN, MP3_TX_PIN);
   #else
    mp3.begin(9600);
   #endif
   LOG.println (F("\nСтарт MP3 Player"));
   mp3_timer = millis();
   mp3_player_connect = 1;
  #endif 

  // WI-FI
  LOG.printf_P(PSTR("\nРабочий режим лампы: ESP_MODE = %d\n"), espMode);
  //Запускаем WIFI
  LOG.println(F("Старуем WIFI"));
  
  WiFi.persistent(false);   // Побережём EEPROM
 
  if (espMode == 0U)                                        // режим WiFi точки доступа
  {
  // Отключаем WIFI
  WiFi.disconnect();
  // Меняем режим на режим точки доступа
  WiFi.mode(WIFI_AP);
  // Задаем настройки сети
    if (sizeof(AP_STATIC_IP))
    {
      WiFi.softAPConfig(                      
        IPAddress(AP_STATIC_IP[0], AP_STATIC_IP[1], AP_STATIC_IP[2], AP_STATIC_IP[3]),      // IP адрес WiFi точки доступа
        IPAddress(AP_STATIC_IP[0], AP_STATIC_IP[1], AP_STATIC_IP[2], 1),                    // первый доступный IP адрес сети
        IPAddress(255, 255, 255, 0));                                                       // маска подсети
    }
  // Включаем WIFI в режиме точки доступа с именем и паролем
  // хронящихся в переменных _ssidAP _passwordAP в фвйле config.json
    #ifdef ESP32_USED
     WiFi.softAP(AP_NAME.c_str(), AP_PASS.c_str());
    #else
     WiFi.softAP(AP_NAME, AP_PASS);
    #endif
    LOG.print(F("Старт WiFi в режиме точки доступа\n"));
    LOG.print(F("IP адрес: "));
    LOG.println(WiFi.softAPIP());
   #ifdef GENERAL_DEBUG
    LOG.println (F("*******************************************"));
    LOG.print (F("Heap Size after connection AP mode = "));
    LOG.println(ESP.getFreeHeap());
    LOG.println (F("*******************************************"));
    #endif
    connect = true;
    #ifdef DISPLAY_IP_AT_START
        loadingFlag = true;
      #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время вывода текста
        digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
      #endif
        while(!fillString(WiFi.softAPIP().toString().c_str(), CRGB::White, false)) {
           delay(1);
           #ifdef ESP32_USED
            esp_task_wdt_reset();
          #else
           ESP.wdtFeed();
          #endif
           }
        if (ColorTextFon  & (!ONflag || (currentMode == EFF_COLOR && modes[currentMode].Scale < 3))){
          FastLED.clear();
          delay(1);
          FastLED.show();
        }
      #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы или будильника
        digitalWrite(MOSFET_PIN, ONflag || (dawnFlag == 1 && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
      #endif
        loadingFlag = true;
      #endif  // DISPLAY_IP_AT_START
    delay (100);    
  }
  else                                                      // режим WiFi клиента. Подключаемся к роутеру
  {
    LOG.println(F("Старт WiFi в режиме клиента (подключение к роутеру)"));
//  WIFI_start_station_mode (); 
  
   WiFi.persistent(false);

  // Попытка подключения к Роутеру
  WiFi.mode(WIFI_STA);
  String _ssid = jsonRead(configSetup, "ssid");
  String _password = jsonRead(configSetup, "password");
  if (_ssid == "" && _password == "") {
   espMode = 0;
   jsonWrite(configSetup, "ESP_mode", (int)espMode);
   saveConfig(); 
   ESP.restart();
  }
  else {

    if(use_static_ip)
    {  
        WiFi.config(Static_IP, Gateway, Subnet, DNS1, DNS2); // Конфигурация под статический IP Address
    }
  delay(10);  
    WiFi.begin(_ssid.c_str(), _password.c_str());
  }
    
  delay (10);    
    #ifdef USE_BLYNK
    Blynk.config(USE_BLYNK, "blynk.tk", 8080);
    #endif
  }     //if (espMode == 0U) {...} else {...
  
    #ifdef ESP32_USED
     esp_task_wdt_reset();
    #else
     ESP.wdtFeed();
    #endif

   
  // UDP 
  LOG.printf_P(PSTR("\nСтарт UDP сервера. Порт: %u\n"), localPort);
  Udp.begin(localPort);

  //Настраиваем и запускаем HTTP интерфейс
  User_setings ();
  #ifdef GENERAL_DEBUG
  LOG.print (F("Старт WebServer\n"));
  #endif
  HTTP_init();
  WiFiClient client;  //Declare an object of class HTTPClient
 
 // NTP
  #ifdef USE_NTP
  timeClient.begin();
    #ifdef ESP32_USED
     esp_task_wdt_reset();
    #else
     ESP.wdtFeed();
    #endif
  #endif


  // MQTT
  #if (USE_MQTT)
  String configMQTT = readFile(F("config_mqtt.json"), 512);
  String str;
  if(!MqttServer.fromString(jsonRead(configMQTT, "mq_ip"))){
        jsonRead(configMQTT, "mq_ip").toCharArray(MqttHost, jsonRead(configMQTT, "mq_ip").length()+1);
        mqttIPaddr = false;
/*        #ifdef ESP32_USED
         WiFi.hostByName(MqttHost, MqttServer);
        #else
         WiFi.hostByName(MqttHost, MqttServer, RESOLVE_TIMEOUT);
        #endif
*/
  }
  else
      mqttIPaddr = true;
  str = jsonRead(configMQTT, "mq_user");
  str.toCharArray(MqttUser, str.length()+1);
  str = jsonRead(configMQTT, "mq_pass");
  str.toCharArray(MqttPassword, str.length()+1);
  str = jsonRead(configMQTT, "topic");
  str.toCharArray(TopicBase, str.length()+1);
  MqttPort = jsonReadtoInt(configMQTT, "mq_port");
  MqttOn = jsonReadtoInt(configMQTT, "mq_on");
  MqttPeriod = jsonReadtoInt(configMQTT, "mq_prd");
  #ifdef GENERAL_DEBUG
   LOG. println("Start MQTT");
   LOG.print("MQTT server ");
   if(mqttIPaddr)
       LOG.print(MqttServer);
   else
       LOG.print(MqttHost);
   LOG.print(": ");
   LOG.println(MqttPort);
   LOG.print("MQTT User - ");
   LOG.println(MqttUser);
   LOG.print("MQTT Password - ");
   LOG.println(MqttPassword);
  #endif //GENERAL_DEBUG

  if (espMode == 1U)
  {
    mqttClient = new AsyncMqttClient();
    MqttManager::setupMqtt(mqttClient, inputBuffer, &sendCurrent);    // создание экземпляров объектов для работы с MQTT, их инициализация и подключение к MQTT брокеру
  }
    #ifdef ESP32_USED
     esp_task_wdt_reset();
    #else
     ESP.wdtFeed();
    #endif
  #endif


  // ОСТАЛЬНОЕ
  memset(matrixValue, 0, sizeof(matrixValue)); //это массив для эффекта Огонь. странно, что его нужно залить нулями
  randomSeed(micros());
  changePower();
  loadingFlag = true;
  
  //IR receiver
  #ifdef IR_RECEIVER_USE
    irrecv.enableIRIn();  // Start the IR receiver
    IR_Tick_Timer = millis();
    IR_Repeat_Timer = millis();
  #endif  //IR_RECEIVER_USE

  //TM1637
  #ifdef TM1637_USE
    DisplayTimer = millis();
    #ifdef MP3_PLAYER_USE
      CurrentFolder = effects_folders[currentMode];
      mp3_folder = CurrentFolder;
      jsonWrite(configSetup, "fold_sel", CurrentFolder);
    #endif  // MP3_PLAYER_USE
  #endif  //TM1637_USE

  my_timer=millis();
  
  #ifdef HEAP_SIZE_PRINT
   mem_timer = millis();
  #endif //HEAP_SIZE_PRINT 
}


void loop()  //====================================================================  void loop()  ===========================================================================
{
  #ifdef USE_RTC
     if (hasRtc) {
      #ifdef RTC_3231
       if (!Rtc.IsDateTimeValid())
       #endif
       {
           if (!wasError("loop IsDateTimeValid"))
           {
               // Common Causes:
               //    1) the battery on the device is low or even missing and the power line was disconnected
               LOG.println(F("RTC lost confidence in the DateTime!"));
           }
       }
     }
  #endif //USE_RTC

 if (espMode) {
  if (WiFi.status() != WL_CONNECTED) {
    if ((millis()-my_timer) >= 1000UL) {    
      my_timer=millis();
      if (ESP_CONN_TIMEOUT--) {
        LOG.print(F("."));
        #ifdef ESP32_USED
         esp_task_wdt_reset();
        #else
         ESP.wdtFeed();
        #endif
      }
      else {
        // Если не удалось подключиться запускаем в режиме AP
        espMode = 0;
        jsonWrite(configSetup, "ESP_mode", (int)espMode);
        saveConfig(); 
        ESP.restart();
      }
    }
  }
    else {
        // Иначе удалось подключиться отправляем сообщение
        // о подключении и выводим адрес IP
        LOG.print(F("\nПодключение к роутеру установлено\n"));
        LOG.print(F("IP адрес: "));
        LOG.println(WiFi.localIP());
        long rssi = WiFi.RSSI();
        LOG.print(F("Уровень сигнала сети RSSI = "));
        LOG.print(rssi);
        LOG.println(F(" dbm"));
        connect = true;
        lastResolveTryMoment = 0;
      #ifdef GENERAL_DEBUG
        LOG.println (F("***********************************************"));
        LOG.print (F("Heap Size after connection Station mode = "));
        LOG.println(ESP.getFreeHeap());
        LOG.println (F("***********************************************"));
      #endif
      #ifdef DISPLAY_IP_AT_START
        loadingFlag = true;
      #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время вывода текста
        digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
      #endif
        while(!fillString(WiFi.localIP().toString().c_str(), CRGB::White, false)) {
           delay(1);
           #ifdef ESP32_USED
            esp_task_wdt_reset();
          #else
           ESP.wdtFeed();
          #endif
           }
        if (ColorTextFon  & (!ONflag || (currentMode == EFF_COLOR && modes[currentMode].Scale < 3))){
          FastLED.clear();
          delay(1);
          FastLED.show();
        }
      #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы или будильника
        digitalWrite(MOSFET_PIN, ONflag || (dawnFlag == 1 && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
      #endif
        loadingFlag = true;
      #endif  // DISPLAY_IP_AT_START
        delay (0);
    }
 }
 
 if (connect || !espMode)  { my_timer = millis(); }
 #ifdef MAIN_CYCLES_PER_SECOND
   int32_t my_timer2 = millis();
   uint16_t mcps_counter = 0;
 #endif 
do {    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++========= Главный цикл ==========+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Если не устойчивое подключение к WiFi, или не создаётся точка доступа, или лампа не хочет подключаться к вашей сети или вы не можете подключиться к точке доступа, то может быть у вас не качественная плата.
  delay (0);   //Для некоторых плат ( особенно без металлического экрана над ESP и Flash памятью ) эта задержка должна быть увеличена. Подбирается индивидуально в пределах 1-12 мс до устойчивой работы WiFi. Чем меньше, тем лучше. Качественные платы работают с задержкой 0.
  yield();
  
    //if ((connect || !espMode)&&((millis() - my_timer) >= 10UL)) 
    {
    HTTP.handleClient(); // Обработка запросов web страницы. 
    //my_timer = millis();
    }
 
  parseUDP();
  yield();
  #ifdef TM1637_USE
    if (millis() - tmr_clock > 500UL) {         // каждую секунду изменяем
      tmr_clock = millis();                     // обновляем значение счетчика
      dotFlag = !dotFlag;                       // инверсия флага
      boolean points[4] = {0,0,0,0};
      points[1] = dotFlag;
      if (!DisplayFlag) display.setSegmentPoints(points); // выкл/выкл двоеточия 
      Display_Timer ();
    }
    if (dawnFlag == 1) {
    clockTicker_blink();
    }
  #endif  //TM1637_USE
  #ifdef MP3_PLAYER_USE
  switch (mp3_player_connect){
      case 0: break;
      case 1: read_command(1);
              if (millis() > 3000UL || mp3_receive_buf[3] == 0x3F){
                 first_entry = 5;
                 //mp3_timer = millis();
                 mp3_setup ();
                }
              break;
      case 2: read_command(1);
              if ( millis() - mp3_timer > 3000UL || mp3_receive_buf[3] == 0x3F) mp3_player_connect = 3;
              break;
      case 3: mp3_setup(); break;
      case 4: mp3_loop(); break;
      }
                    
  #endif

 if (Painting == 0) {
     
  effectsTick();
  
  #ifdef HEAP_SIZE_PRINT
   if (millis() - mem_timer > 10000UL) {
       mem_timer = millis();
       LOG.print (F("Heap Size = "));
       LOG.println(ESP.getFreeHeap());
   }
  #endif //HEAP_SIZE_PRINT
  
  #ifdef IR_RECEIVER_USE
       IR_Receive_Handle();
    if (millis() - IR_Tick_Timer > 100)
    {
        IR_Tick_Timer = millis();
        if (IR_Data_Ready) 
        {
            IR_Receive_Button_Handle();
            IR_Data_Ready =0;
        }       
    }
  #endif  //IR_RECEIVER_USE

  //EepromManager::HandleEepromTick(&settChanged, &eepromTimeout, modes);
    yield();

  //#ifdef USE_NTP
  #if defined(USE_NTP) || defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE)
    timeTick();
  #endif

  #ifdef ESP_USE_BUTTON
    buttonTick();
  #endif

  #ifdef OTA
  otaManager.HandleOtaUpdate();                             // ожидание и обработка команды на обновление прошивки по воздуху
  #endif
                                                            
  TimerManager::HandleTimer(&ONflag, //&settChanged, //&eepromTimeout, // обработка событий таймера отключения лампы
                            &timeout_save_file_changes,
                            &save_file_changes, &changePower);    
  
  if (FavoritesManager::HandleFavorites(                    // обработка режима избранных эффектов
      &ONflag,
      &currentMode,
      &loadingFlag
      #if defined(USE_NTP) || defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE)
      , &dawnFlag
      #endif
      , &random_on
      , &selectedSettings
      ,espMode
      ))
  {
    #ifdef USE_BLYNK
    updateRemoteBlynkParams();
    #endif
    SetBrightness(modes[currentMode].Brightness);
  }

  #if USE_MQTT
  if (espMode == 1U && mqttClient && WiFi.isConnected() && !mqttClient->connected() && MqttOn)
  {
    MqttManager::mqttConnect();                             // библиотека не умеет восстанавливать соединение в случае потери подключения к MQTT брокеру, нужно управлять этим явно
    MqttManager::needToPublish = true;
  }

  if ((MqttManager::needToPublish || (MqttPeriod && (millis() - mqtt_timer) >= (MqttPeriod * 1000))) && MqttOn)
  //if ((MqttManager::needToPublish ) && MqttOn)
  {
    mqtt_timer = millis();
    if (strlen(inputBuffer) > 0)                            // проверка входящего MQTT сообщения; если оно не пустое - выполнение команды из него и формирование MQTT ответа
    {
      processInputBuffer(inputBuffer, MqttManager::mqttBuffer, true);
    }
  #ifdef PUBLISH_STATE_IN_OLD_FORMAT  
    MqttManager::publishState(0);  //публикация буфера MQTT ответа в топик <TopicBase>/LedLamp_<ChipId>/state
  #endif // PUBLISH_STATE_IN_OLD_FORMAT
    String MqttSnd = "{\"power\":\"ON\"}"; //Строка для ответа "{"power":"ON","cycle":"OFF","effect":"111","bri":"15","spd":"33","sca":"58","sound":"ON","volume":"10","runt":"10","runc":"123","runf":"1","runc":"220","rnde":"0","rndc":"1","rndf":"0","tmr":59900"}"
    jsonWrite(MqttSnd, "power", ONflag ? "ON" : "OFF");   // Создание строки для MQTT ответа в формате JSON в виде выше.
    jsonWrite(MqttSnd, "cycle", FavoritesManager::FavoritesRunning ? "ON" : "OFF"); // Включить/выключить режим "цикл" ("избранное")
     for ( uint8_t n=0; n< MODE_AMOUNT; n++)
     {
         if (eff_num_correct[n] == currentMode){
            jsonWrite(MqttSnd, "effect", (String)n);                    
            break;
         } 
     } 
    jsonWrite(MqttSnd, "bri", (String)modes[currentMode].Brightness);   // Яркость эффектов
    jsonWrite(MqttSnd, "spd", (String)modes[currentMode].Speed);        // Скорость эффектов
    jsonWrite(MqttSnd, "sca", (String)modes[currentMode].Scale);        // Масштаб эффектов
    #ifdef MP3_PLAYER_USE
    jsonWrite(MqttSnd, "sound", eff_sound_on ? "ON" : "OFF");    // Включить/выключить озвучивание эффектов
    jsonWrite(MqttSnd, "vol", (String)eff_volume);                  // Громкость
    #endif //MP3_PLAYER_USE
    jsonWrite(MqttSnd, "runt", (String)RuninTextOverEffects);           // Периодичность ввода бегущей строки
    jsonWrite(MqttSnd, "runc", (String)ColorRunningText);               // Цвет бегущей строки
    jsonWrite(MqttSnd, "runf", (String)ColorTextFon);                   // Фон бегущей строки. 0-черный фон; 1-цветный фон
    jsonWrite(MqttSnd, "runs", (String)SpeedRunningText);               // Скорость бегущей строки
    jsonWrite(MqttSnd, "rnde", (String)FavoritesManager::rndCycle);     // Случайный выбор эффектов в цикле
    jsonWrite(MqttSnd, "rndс", (String)random_on);                      // Случайный выбор настроек эффектов в цикле
    jsonWrite(MqttSnd, "rndf", (String)selectedSettings);               // Случайный выбор настроек текущего эффекта
    uint32_t temp = TimerManager::TimeToFire - millis();
    if (temp && TimerManager::TimerRunning)
        jsonWrite(MqttSnd, "tmr", (String)(temp / 1000));               // Количество секунд до срабатывания таймера
    else
        jsonWrite(MqttSnd, "tmr", (String)0);                           // Количество секунд до срабатывания таймера == 0
    
    MqttSnd.toCharArray(MqttManager::mqttBuffer, MqttSnd.length() +1);  // можно добавить еще какие-то переменные (данные) для вывода в ответ, но длина строки ответа должна быть меньше 255 байт
    MqttManager::publishState(1);  //публикация буфера MQTT ответы (JSON): "{"power":"ON","cycle":"OFF","effect":"111","bri":"15","spd":"33","sca":"58","sound":"ON","volume":"10","runt":"10","runc":"123","runf":"1","runc":"220","rnde":"0","rndc":"1","rndf":"0","tmr":59900"}" в топик <TopicBase>/LedLamp_<ChipId>/snd
  }
  #endif

  #ifdef USE_BLYNK
  if (espMode == 1U && WiFi.isConnected())
    Blynk.run();
  #endif

  #if defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET
  handleTelnetClient();
  #endif
 } //if (Painting == 0)
  yield();
    #ifdef ESP32_USED
     esp_task_wdt_reset();
    #else
     ESP.wdtFeed();
    #endif
  #ifdef MAIN_CYCLES_PER_SECOND
    mcps_counter ++;
    if ((millis() - my_timer2) > 1000)
    {
        my_timer2 = millis();
        LOG.print("MAIN CYCLES PER SECOND = ");
        LOG.println(mcps_counter);
        mcps_counter = 0;
    }
  #endif
} while (connect);
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

#ifdef USE_RTC
void printDateTime(const RtcDateTime& dt)
{
    char datestring[26];

    snprintf_P(datestring,
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Day(),
            dt.Month(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    LOG.println(datestring);
}
#endif
