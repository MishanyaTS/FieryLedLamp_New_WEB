// 
// 
//
// Далее следует код проекта. Не меняйте здесь ничего, если вы не понимаете, к чему это приведет!
// ======================= ВНИМАНИЕ !!! =============================
// Все настройки производятся на вкладке Constants.h
// Почитайте там то, что на русском языке написано.
// Или ничего не трогайте, если собирали, по схемам из этого архива.
// В любом случае ВНИМАТЕЛЬНО прочтите файл ПРОЧТИ МЕНЯ!!!.txt из этого архива.
//
// ================================================ ==================
//
// Ссылка для менеджера плат:
// https://arduino.esp8266.com/stable/package_esp8266com_index.json
// При установке выбираем версию 2.7.4
//
// ================================================ ==================
//
//
//+++++++++++++++++==========================================+++++++++++++++++++++++++++++++
//
#define FASTLED_USE_PROGMEM 1 // просим библиотеку FASTLED экономить память контроллера на свои палитры
#include "pgmspace.h"
#include <ESP8266WebServer.h>
#include "Constants.h"
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include "Types.h"
#include "timerMinim.h"
#ifdef ESP_USE_BUTTON
#include <GyverButton.h>
#endif
#include "fonts.h"
#ifdef USE_NTP
#include <NTPClient.h>
#include <Timezone.h>
#endif
#include <TimeLib.h>
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
#include <BlynkSimpleEsp8266.h>
#endif
#include <ESP8266SSDP.h>        
#include <ESP8266HTTPUpdateServer.h>    // Обновление с web страницы
/*
#include <ArduinoJson.h>        // Перенесен в файл constants.h в разделе "для Разработчиков"
#ifdef USE_LittleFS
#include <LittleFS.h>           // Перенесен в файл constants.h в разделе "для Разработчиков"
#define SPIFFS LittleFS         // Перенесен в файл constants.h в разделе "для Разработчиков"
#endif
*/
#ifdef TM1637_USE
#include "TM1637Display.h"
#endif
#ifdef MP3_TX_PIN
#include <SoftwareSerial.h>     // Подключаем библиотеку для работы с последовательным интерфейсом
 #ifdef MP3_DEBUG
  #define FEEDBACK  1
 #else
  #define FEEDBACK  0
 #endif  //MP3_DEBUG
#endif  //MP3_TX_PIN
#ifdef IR_RECEIVER_USE
 #include <IRremoteESP8266.h>  // https://github.com/crankyoldgit/IRremoteESP8266
 #include <IRrecv.h>
 #include "IR_Receiver.h"
#endif  //IR_RECEIVER_USE


// --- ИНИЦИАЛИЗАЦИЯ ОБЪЕКТОВ ----------
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
char* MqttManager::mqttServer = NULL;
char* MqttManager::mqttUser = NULL;
char* MqttManager::mqttPassword = NULL;
char* MqttManager::clientId = NULL;
char* MqttManager::lampInputBuffer = NULL;
char* MqttManager::topicInput = NULL;
char* MqttManager::topicOutput = NULL;
bool MqttManager::needToPublish = false;
char MqttManager::mqttBuffer[] = {};
uint32_t MqttManager::mqttLastConnectingAttempt = 0;
SendCurrentDelegate MqttManager::sendCurrentDelegate = NULL;
#endif

ESP8266HTTPUpdateServer httpUpdater;  // Объект для обнавления с web страницы
ESP8266WebServer HTTP (ESP_HTTP_PORT);//ESP8266WebServer HTTP;  // Web интерфейс для устройства
File fsUploadFile;  // Для файловой системы



// --- ИНИЦИАЛИЗАЦИЯ ПЕРЕМЕННЫХ -------
uint16_t localPort = ESP_UDP_PORT;
char packetBuffer[MAX_UDP_BUFFER_SIZE];                     // buffer to hold incoming packet
char inputBuffer[MAX_UDP_BUFFER_SIZE];
static const uint8_t maxDim = max(WIDTH, HEIGHT);


AlarmType alarms[7];

static const uint8_t dawnOffsets[] PROGMEM = {5, 10, 15, 20, 25, 30, 40, 50, 60};   // опции для выпадающего списка параметра "время перед 'рассветом'" (будильник); синхронизировано с android приложением
uint8_t dawnMode;
bool dawnFlag = false;
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

#ifdef MP3_TX_PIN
uint8_t mp3_folder=1;                // Текущая папка для воспроизведения.
uint8_t alarm_sound_on =false;       // Включить/выключить звук будильника
uint8_t alarm_volume;                // Громкость будильника
uint8_t AlarmFolder;                 // Папка будильника
uint8_t Equalizer;                   // Эквалайзер
bool alarm_sound_flag =false;        // проигрывается ли сейчас будильник
uint8_t dawnflag_sound = false;      // Звук не начал обслуживание рассвета. Если не true - звук обслуживает рассвет
//uint8_t tmp_fold;
bool advert_flag = false;            // Озвучивается время
bool advert_hour;                    // Озвучиваются часы времени
uint8_t day_advert_volume;           // Дневная Громкость озвучивания времени
uint8_t night_advert_volume;         // Ночная Громкость озвучивания времени
bool day_advert_sound_on;            // Вкл.Выкл озвучивания времени днём
bool night_advert_sound_on;          // Вкл.Выкл озвучивания времени ночью
bool alarm_advert_sound_on;          // Вкл.Выкл озвучивания времени будильником
uint8_t mp3_player_connect = 0;      // Плеер не подключен. true - подключен.
uint8_t mp3_folder_last=255;         // Предыдущая папка для воспроизведения
//uint8_t mp3_folder_change =0;      // Указывает, была ли изменена папка
bool set_mp3_play_now=false;         // Указывает, надо ли играть сейчас мелодии
uint32_t alarm_timer;                // Периодичность проверки и плавного изменения громкости будильника
uint32_t mp3_timer = 0;
bool mp3_stop = true;                        // Озвучка эффектов остановлена
bool pause_on = true;                        // Озвучка эффектов на паузе. false - не на паузе
uint8_t eff_volume = 9;                      // громкость воспроизведения
uint8_t eff_sound_on = 0;                    // звук включен - !0 (true), выключен - 0
uint8_t CurrentFolder;                       // Папка, на которую переключились (будет проигрываться)
uint8_t CurrentFolder_last = 0;              // Предыдущая текущая папка
SoftwareSerial mp3(MP3_RX_PIN, MP3_TX_PIN);  // создаём объект mySoftwareSerial и указываем выводы, к которым подлючен плеер (RX, TX)
//#ifndef TM1637_USE
// uint8_t minute_tmp;
//#endif
uint8_t mp3_receive_buf[10];
uint8_t effects_folders[MODE_AMOUNT];    // Номера папок для озвучивания
uint16_t ADVERT_TIMER_H, ADVERT_TIMER_M; // продолжительность озвучивания часов и минут
uint8_t mp3_delay;                       // Задержка между командами проигрывателя
uint8_t send_sound = 1;                  // Передавать или нет сомнительным параметрам звука (папка,озвучивание_on/off,громкость)
uint8_t send_eff_volume = 1;             // Передавать или нет озвучивания_on/off, громкость
#endif  //MP3_TX_PIN
#ifdef TM1637_USE
uint8_t DispBrightness = 1;          // +++ Яркость дисплея от 0 до 255(5 уровней яркости с шагом 51). 0 - дисплей погашен 
bool dotFlag = false;                // +++ флаг: в часах рисуется двоеточие или нет
uint32_t tmr_clock = 0;              // +++ таймер мигания разделителя часов на дисплее
uint32_t tmr_blink = 0;              // +++ таймер плавного изменения яркости дисплея
TM1637Display display(CLK, DIO);     // +++ подключаем дисплей
bool aDirection = false;             // +++ Направление изменения яркости
uint32_t DisplayTimer;               // Время отображения номера эффекта
uint8_t LastEffect = 255;            // последний Проигрываемый эффект
uint8_t DisplayFlag=0;               // Флаг, показывающий, что отображается номер эффекта и папки
 #ifdef MP3_TX_PIN
 uint8_t LastCurrentFolder = 255;    // Проигрываемая папка
 #endif  //MP3_TX_PIN
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
IPAddress Static_IP;//(192,168,0,17);  // Статичний IP
IPAddress Gateway;//(192,168,0,1);     // Шлюз
IPAddress Subnet;//(255,255,255,0);    // Маска подсети
IPAddress DNS1;//(208,67,222,222);     // Серверы DNS. Можно также DNS1(1,1,1,1) или DNS1(8,8,4,4);
IPAddress DNS2(8,8,8,8);               // Резервный DNS

uint8_t C_flag = 0;
uint16_t current_limit;              // Лимит настраиваемого тока
uint8_t last_minute;                 // минуты
uint8_t hours;                       // часы
//uint8_t last_hours; 
uint8_t m_date,d_date;               // дата
uint8_t AutoBrightness;              // Автояркость on/off
uint8_t last_day_night = 0;

void setup()  //==================================================================  void setup()  =========================================================================
{
	
  Serial.begin(115200);
  delay(300);
  ESP.wdtEnable(WDTO_8S);

  LOG.print(F("\n\n\nSYSTEM START\n"));

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

   //HTTP
  User_setings ();
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
  //Настраиваем и запускаем HTTP интерфейс
  #ifdef GENERAL_DEBUG
  LOG.print (F("Старт WebServer\n"));
  #endif
  HTTP_init();

  
//-----------Инициализируем переменные, хранящиеся в файле config.json--------------
  LAMP_NAME = jsonRead(configSetup, "SSDP");
  AP_NAME = jsonRead(configSetup, "ssidAP");
  AP_PASS = jsonRead(configSetup, "passwordAP");
  Favorit_only = jsonReadtoInt(configSetup, "favorit");
  random_on = jsonReadtoInt(configSetup, "random_on");
  espMode = jsonReadtoInt(configSetup, "ESP_mode");
  PRINT_TIME = jsonReadtoInt(configSetup, "print_time");
  buttonEnabled = jsonReadtoInt(configSetup, "button_on");
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
  RuninTextOverEffects = jsonReadtoInt(configSetup, "toe");  // Выводить бегущую строку поверх эффектов
  SpeedRunningText = jsonReadtoInt(configSetup, "spt");  // Скорость бегущей строки
  ColorRunningText = jsonReadtoInt(configSetup, "sct");  // Цвет бегущей строки
  ColorTextFon = jsonReadtoInt(configSetup, "ctf");      // Выводить бегущую строку на цветном фоне 
  AutoBrightness = jsonReadtoInt(configSetup, "auto_bri");   // Автоматическое понижение яркости on/off
  #ifdef USE_NTP
  (jsonRead(configSetup, "ntp")).toCharArray (NTP_ADDRESS, (jsonRead(configSetup, "ntp")).length()+1);
  #endif
  #ifdef USE_NTP
  winterTime.offset = jsonReadtoInt(configSetup, "timezone") * 60;
  summerTime.offset = winterTime.offset + jsonReadtoInt(configSetup, "Summer_Time") *60;
  localTimeZone.setRules (summerTime, winterTime);
  #endif
  #ifdef MP3_TX_PIN
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
  #endif //MP3_TX_PIN
  {
  String configHardware = readFile(F("hardware_config.json"), 1024);    
  current_limit = jsonReadtoInt(configHardware, "cur_lim");
  MATRIX_TYPE = jsonReadtoInt(configHardware, "m_t");
  ORIENTATION = jsonReadtoInt(configHardware, "m_o");
  #ifdef MP3_TX_PIN
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
  String configIP = readFile(F("ip_config.json"), 512);
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
    ESP.wdtFeed();
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
    ESP.wdtFeed();
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
/*
  {
    File file = SPIFFS.open(F("/index.json.gz"),"r");
    if ((EEPROM.read(EEPROM_FIRST_RUN_ADDRESS+1)!= MODE_AMOUNT) && (file.size() > 700UL))
    {
        for (uint8_t i = 0; i < 85; i++) TextTicker[i] = pgm_read_byte(&Default_Settings[i]);
    SPIFFS.format();
    buttonEnabled = 0;
    currentMode = EFF_TEXT;
    ONflag = 1;
    changePower();
    }
    file.close();
  }
*/
  first_entry = 1;
  handle_alarm ();
  first_entry = 0;
  FavoritesManager::FavoritesRunning = jsonReadtoInt(configSetup, "cycle_on");  // чтение состояния настроек режима Цикл 
  FavoritesManager::Interval = jsonReadtoInt(configSetup, "time_eff");          // вкл/выкл,время переключения,дисперсия,вкл цикла после перезагрузки
  FavoritesManager::Dispersion = jsonReadtoInt(configSetup, "disp");
  FavoritesManager::UseSavedFavoritesRunning = jsonReadtoInt(configSetup, "cycle_allwase");
  jsonWrite(configSetup, "tmr", 0);
  jsonWrite(configSetup, "button_on", buttonEnabled);
  first_entry = 1;
  handle_cycle_set();  // чтение выбранных эффектов
  first_entry = 0;
#ifdef MP3_TX_PIN
  first_entry = 1;
  handle_sound_set();  //чтение выбранных папок
  first_entry = 0;
#endif  //MP3_TX_PIN
#ifdef USE_MULTIPLE_LAMPS_CONTROL  
  multilamp_get ();   // Чтение из файла адресов синхронно управляемых ламп 
#endif //USE_MULTIPLE_LAMPS_CONTROL
  
  // MP3 Player
   
  #ifdef MP3_TX_PIN
   mp3.begin(9600);
   LOG.println (F("\nСтарт MP3 Player"));
   mp3_timer = millis();
   mp3_player_connect = 1;
  #endif 
  
  // UDP
  
  LOG.printf_P(PSTR("\nСтарт UDP сервера. Порт: %u\n"), localPort);
  Udp.begin(localPort);
  
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
  WiFi.softAP(AP_NAME, AP_PASS);
    LOG.print(F("Старт WiFi в режиме точки доступа\n"));
    LOG.print(F("IP адрес: "));
    LOG.println(WiFi.softAPIP());
   #ifdef GENERAL_DEBUG
    LOG.println (F("*******************************************"));
    LOG.print (F("Heap Size after connection AP mode = "));
    LOG.println(system_get_free_heap_size());
    LOG.println (F("*******************************************"));
    #endif    
  connect = true;
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
  
  ESP.wdtFeed();

  // NTP
  #ifdef USE_NTP
  timeClient.begin();
  ESP.wdtFeed();
  #endif


  // MQTT
  #if (USE_MQTT)
  if (espMode == 1U)
  {
    mqttClient = new AsyncMqttClient();
    MqttManager::setupMqtt(mqttClient, inputBuffer, &sendCurrent);    // создание экземпляров объектов для работы с MQTT, их инициализация и подключение к MQTT брокеру
  }
  ESP.wdtFeed();
  #endif


  // ОСТАЛЬНОЕ
  memset(matrixValue, 0, sizeof(matrixValue)); //это массив для эффекта Огонь. странно, что его нужно залить нулями
  randomSeed(micros());
  changePower();
  loadingFlag = true;
  #ifdef IR_RECEIVER_USE
    irrecv.enableIRIn();  // Start the IR receiver
    IR_Tick_Timer = millis();
    IR_Repeat_Timer = millis();
  #endif  //IR_RECEIVER_USE

  //delay (100);
  
#ifdef TM1637_USE
  DisplayTimer = millis();
 #ifdef MP3_TX_PIN
    CurrentFolder = effects_folders[currentMode];
    mp3_folder = CurrentFolder;
    jsonWrite(configSetup, "fold_sel", CurrentFolder);
 #endif  //MP3_TX_PIN
#endif  //TM1637_USE

  my_timer=millis();
  
  #ifdef HEAP_SIZE_PRINT
   mem_timer = millis();
  #endif //HEAP_SIZE_PRINT 
  WiFiClient client;  //Declare an object of class HTTPClient
}


void loop()  //====================================================================  void loop()  ===========================================================================
{
  
 if (espMode) {
  if (WiFi.status() != WL_CONNECTED) {
	if ((millis()-my_timer) >= 1000UL) {	
	  my_timer=millis();
	  if (ESP_CONN_TIMEOUT--) {
		LOG.print(F("."));
		ESP.wdtFeed();
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
        LOG.println(system_get_free_heap_size());
        LOG.println (F("***********************************************"));
      #endif
      #ifdef DISPLAY_IP_AT_START
        loadingFlag = true;
      #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время вывода текста
        digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
      #endif
        while(!fillString(WiFi.localIP().toString().c_str(), CRGB::White, false)) { delay(1); ESP.wdtFeed(); }
        if (ColorTextFon  & (!ONflag || (currentMode == EFF_COLOR && modes[currentMode].Scale < 3))){
          FastLED.clear();
          delay(1);
          FastLED.show();
        }
      #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы или будильника
        digitalWrite(MOSFET_PIN, ONflag || (dawnFlag && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
      #endif
        loadingFlag = true;
      #endif  // DISPLAY_IP_AT_START
		delay (0);
	}
 }
 
 if (connect || !espMode)  { my_timer = millis(); }
  
do {	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++========= Главный цикл ==========+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
    if (dawnFlag) {
    clockTicker_blink();
    }
  #endif  //TM1637_USE
  #ifdef MP3_TX_PIN
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
       LOG.println(system_get_free_heap_size());
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
      ))
  {
    #ifdef USE_BLYNK
    updateRemoteBlynkParams();
    #endif
    SetBrightness(modes[currentMode].Brightness);
  }

  #if USE_MQTT
  if (espMode == 1U && mqttClient && WiFi.isConnected() && !mqttClient->connected())
  {
    MqttManager::mqttConnect();                             // библиотека не умеет восстанавливать соединение в случае потери подключения к MQTT брокеру, нужно управлять этим явно
    MqttManager::needToPublish = true;
  }

  if (MqttManager::needToPublish)
  {
    if (strlen(inputBuffer) > 0)                            // проверка входящего MQTT сообщения; если оно не пустое - выполнение команды из него и формирование MQTT ответа
    {
      processInputBuffer(inputBuffer, MqttManager::mqttBuffer, true);
    }
    
    MqttManager::publishState();
  }
  #endif

  #ifdef USE_BLYNK
  if (espMode == 1U && WiFi.isConnected())
    Blynk.run();
  #endif

  #if defined(GENERAL_DEBUG) && GENERAL_DEBUG_TELNET
  handleTelnetClient();
  #endif
 }//if (Painting == 0)
  yield();
  ESP.wdtFeed();
} while (connect);
}
