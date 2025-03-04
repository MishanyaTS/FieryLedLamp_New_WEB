#ifdef USE_NTP

#define RESOLVE_INTERVAL      (INTERNET_CHECK_PERIOD * 1000UL)            // интервал проверки подключения к интеренету в миллисекундах (INTERNET_CHECK_PERIOD секунд)
                                                                          // при старте ESP пытается получить точное время от сервера времени в интрнете
                                                                          // эта попытка длится RESOLVE_TIMEOUT
                                                                          // если при этом отсутствует подключение к интернету (но есть WiFi подключение),
                                                                          // модуль будет подвисать на RESOLVE_TIMEOUT каждое срабатывание таймера, т.е., 1.5 секунды
                                                                          // чтобы избежать этого, будем пытаться узнать состояние подключения 1 раз в RESOLVE_INTERVAL 
                                                                          // попытки будут продолжаться до первой успешной синхронизации времени
                                                                          // до этого момента функции будильника работать не будут (или их можно ввести через USE_MANUAL_TIME_SETTING)
                                                                          // интервал последующих синхронизаций времени определяён в NTP_INTERVAL (30-60 минут)
                                                                          // при ошибках повторной синхронизации времени функции будильника отключаться не будут
#define RESOLVE_TIMEOUT       (1500UL)                                    // таймаут ожидания подключения к интернету в миллисекундах (1,5 секунды)
IPAddress ntpServerIp = {0, 0, 0, 0};

#endif

#if defined(USE_NTP) || defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE)

/* оптимизируем структуру данных и их обработчик
static CHSV dawnColor = CHSV(0, 0, 0);                                    // цвет "рассвета"
static CHSV dawnColorMinus1 = CHSV(0, 0, 0);                              // для большей плавности назначаем каждый новый цвет только 1/10 всех диодов; каждая следующая 1/10 часть будет "оставать" на 1 шаг
static CHSV dawnColorMinus2 = CHSV(0, 0, 0);
static CHSV dawnColorMinus3 = CHSV(0, 0, 0);
static CHSV dawnColorMinus4 = CHSV(0, 0, 0);
static CHSV dawnColorMinus5 = CHSV(0, 0, 0);
static CHSV dawnColor = CHSV(0, 0, 0);*/
static CRGB dawnColor[6];

static uint8_t dawnCounter = 0;                                           // счётчик первых 10 шагов будильника

void timeTick()
{
Save_File_Changes();
  //{
  
  #ifdef USE_RTC
  if (hasRtc && !timeSynched) {
    RtcDateTime now = Rtc.GetDateTime();
    if (!wasError("setup IsDateTimeValid")) {
      LOG.print(F("Time synced from RTC: "));
      printDateTime(now);
      timeSynched = true;
    }
  }
#endif

if (timeTimer.isReady())
    {
      #ifdef USE_NTP
  if (espMode == 1U) {      
      if (!timeSynched)
      {
        if ((millis() - lastResolveTryMoment >= RESOLVE_INTERVAL || lastResolveTryMoment == 0) && connect)
        {
          resolveNtpServerAddress(ntpServerAddressResolved);              // пытаемся получить IP адрес сервера времени (тест интернет подключения) до тех пор, пока время не будет успешно синхронизировано
          lastResolveTryMoment = millis();
        }
        if (!ntpServerAddressResolved)
        {
         #ifndef USE_RTC
            return;                                                         // если нет интернет подключения, отключаем будильник до тех пор, пока оно не будет восстановлено
          #endif
        }
      }

#ifdef PHONE_N_MANUAL_TIME_PRIORITY
if (stillUseNTP)
#endif      
// если прошло более NTP_INTERVAL, значит, можно попытаться получить время с сервера точного времени один разок
      if (timeClient.update()){
         #ifdef WARNING_IF_NO_TIME
           noTimeClear();
         #endif
         timeSynched = true;
         #if defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE) // если ручное время тоже поддерживается, сохраняем туда реальное на случай отвалившегося NTP
           manualTimeShift = localTimeZone.toLocal(timeClient.getEpochTime()) - millis() / 1000UL;
         #endif
          #ifdef USE_RTC
          if (hasRtc) {
           timeToSet.InitWithEpoch32Time(timeClient.getEpochTime());
           Rtc.SetDateTime(timeToSet);
           LOG.println(F("Время синхронизировано с NTP"));
           }
         #endif
         #if defined(PHONE_N_MANUAL_TIME_PRIORITY) && !defined(USE_RTC)
           stillUseNTP = false;
         #endif
         getBrightnessForPrintTime();
      }
} //if (espMode == 1U)
      #endif //USE_NTP
      
      if (!timeSynched)                                                   // если время не было синхронизиировано ни разу, отключаем будильник до тех пор, пока оно не будет синхронизировано
      {
#ifdef TM1637_USE
        if (!DisplayFlag) display.displayByte(_dash, _dash, _dash, _dash);                  // отображаем прочерки
#endif
        return;
      }

      time_t currentLocalTime = getCurrentLocalTime();
      uint8_t thisDay = dayOfWeek(currentLocalTime);
      if (thisDay == 1) thisDay = 8;                                      // в библиотеке Time воскресенье - это 1; приводим к диапазону [0..6], где воскресенье - это 6
      thisDay -= 2;
      thisTime = hour(currentLocalTime) * 60 + minute(currentLocalTime);
      uint32_t thisFullTime = hour(currentLocalTime) * 3600 + minute(currentLocalTime) * 60 + second(currentLocalTime);
      printTime(thisTime, false, ONflag);   // проверка текущего времени и его вывод (если заказан и если текущее время соответстует заказанному расписанию вывода)
      if (last_minute != minute(currentLocalTime)
          #ifdef TM1637_USE
            && !DisplayFlag
          #endif
          ) {
        hours = hour(currentLocalTime);                   // получаем значение часов
        last_minute = minute(currentLocalTime);           // получаем значение минут
        #ifdef TM1637_USE
        clockTicker_blink();
        #endif
        if (last_minute == 1) getBrightnessForPrintTime();
        
    #ifdef MP3_PLAYER_USE
      if (alarm_advert_sound_on && mp3_player_connect == 4 && dawnFlag == 1 && dawnPosition >= 245) {
        //Serial.println ("Alarm");
        first_entry = 1;
        advert_hour = true;
        delay(mp3_delay);
        play_time_ADVERT();
        while (advert_flag) {
           play_time_ADVERT();
           #ifdef ESP32_USED
            esp_task_wdt_reset();
           #else
            ESP.wdtFeed();
           #endif
        }
      }
    #endif  // MP3_PLAYER_USE
      }

      // проверка рассвета
      if (alarms[thisDay].State &&                                                                                          // день будильника
          thisTime >= (uint16_t)constrain(alarms[thisDay].Time - pgm_read_byte(&dawnOffsets[dawnMode]), 0, (24 * 60)) &&    // позже начала
          thisTime < (alarms[thisDay].Time + DAWN_TIMEOUT))                                                                 // раньше конца + минута
      {
        if (!manualOff)                                                   // будильник не был выключен вручную (из приложения или кнопкой)
        {
          // величина рассвета 0-255
          dawnPosition = (uint16_t) (255 * ((float)(thisFullTime - (alarms[thisDay].Time - pgm_read_byte(&dawnOffsets[dawnMode])) * 60) / (pgm_read_byte(&dawnOffsets[dawnMode]) * 60)));
          dawnPosition = dawnPosition < 255U ? dawnPosition : 255U;  //constrain(dawnPosition, 0, 255);
          for (uint8_t j = 5U; j > 0U; j--)
            if (dawnCounter >= j)
              dawnColor[j] = dawnColor[j - 1U];
          dawnColor[0] = CHSV(map(dawnPosition, 0, 255, 10, 35),
                           map(dawnPosition, 0, 255, 255, 170),
                           map(dawnPosition, 0, 255, 2, DAWN_BRIGHT));

          if (dawnCounter < 5U) dawnCounter++;
          
          // fill_solid(leds, NUM_LEDS, dawnColor);
          
          for (uint16_t i = 0U; i < NUM_LEDS; i++)
          leds[i] = dawnColor[i % 6U];
          FastLED.setBrightness(255);
          delay(1);
          FastLED.show();
          dawnFlag = 1;
#ifdef TM1637_USE
          //blink_clock = true;
#endif
        }
#ifdef TM1637_USE
        //else blink_clock = false;
#endif

        #if defined(ALARM_PIN) && defined(ALARM_LEVEL)                    // установка сигнала в пин, управляющий будильником
        if (thisTime == alarms[thisDay].Time)                             // установка, только в минуту, на которую заведён будильник
        {
          digitalWrite(ALARM_PIN, manualOff ? !ALARM_LEVEL : ALARM_LEVEL);// установка сигнала в зависимости от того, был ли отключен будильник вручную
        }
        #endif

        #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)                  // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время работы будильника
        digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
        #endif
      }
      else
      {
        // не время будильника (ещё не начался или закончился по времени)
        if (dawnFlag == 1)
        {
          dawnFlag = 2;
          #ifdef TM1637_USE
          clockTicker_blink();
          #endif
          //FastLED.clear();
          //delay(2);
          //FastLED.show();
          changePower();                                                  // выключение матрицы или установка яркости текущего эффекта в засисимости от того, была ли включена лампа до срабатывания будильника
        }
#ifdef TM1637_USE
        //blink_clock = false;
#endif
        manualOff = false;
        for (uint8_t j = 0U; j < 6U; j++)
          dawnColor[j] = 0;
          
        dawnCounter = 0;
        

        #if defined(ALARM_PIN) && defined(ALARM_LEVEL)                    // установка сигнала в пин, управляющий будильником
        digitalWrite(ALARM_PIN, !ALARM_LEVEL);
        #endif

        #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)                  // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы
        digitalWrite(MOSFET_PIN, ONflag ? MOSFET_LEVEL : !MOSFET_LEVEL);
        #endif
      }
    jsonWrite(configSetup, "time", Get_Time(currentLocalTime));
    }
  //}
}

#ifdef USE_NTP
void resolveNtpServerAddress(bool &ntpServerAddressResolved)              // функция проверки подключения к интернету
{
  if (ntpServerAddressResolved)
  {
    return;
  }
  #ifdef ESP32_USED
    int err = WiFi.hostByName(NTP_ADDRESS, ntpServerIp);
  #else
    int err = WiFi.hostByName(NTP_ADDRESS, ntpServerIp, RESOLVE_TIMEOUT);
  #endif
  if (err!=1 || ntpServerIp[0] == 0 || ntpServerIp == IPAddress(255U, 255U, 255U, 255U)) 
  {
    #ifdef GENERAL_DEBUG
    LOG.print(F("IP адрес NTP: "));
    LOG.println(ntpServerIp);
    #endif    
    LOG.println(F("\nПодключение к Интернету отсутствует\n"));
    ntpServerAddressResolved = false;
  }
  else
  {
    #ifdef GENERAL_DEBUG
    LOG.print(F("IP адрес NTP: "));
    LOG.println(ntpServerIp);
    #endif

    LOG.println(F("\nПодключение к Интернету установлено\n"));
    ntpServerAddressResolved = true;
  }
}
#endif

void getFormattedTime(char *buf)
{
  //time_t currentLocalTime = localTimeZone.toLocal(timeClient.getEpochTime());
  time_t currentLocalTime = getCurrentLocalTime();
  sprintf_P(buf, PSTR("%02u:%02u:%02u"), hour(currentLocalTime), minute(currentLocalTime), second(currentLocalTime));
}

time_t getCurrentLocalTime()
{
  #if defined(USE_NTP) || defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE)
    #if defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE)
      static uint32_t milliscorrector;
    #endif

    if (timeSynched)
    {
      #if defined(USE_NTP) && defined(USE_MANUAL_TIME_SETTING) || defined(USE_NTP) && defined(GET_TIME_FROM_PHONE)
        if (milliscorrector > millis()
          #ifdef GET_TIME_FROM_PHONE
            && manualTimeShift + millis() / 1000UL < phoneTimeLastSync
          #endif
        ){
          manualTimeShift += 4294967; // защищаем время от переполнения millis()
          #ifdef GET_TIME_FROM_PHONE
            phoneTimeLastSync += 4294967; // а это, чтобы через 49 дней всё не заглючило
          #endif
        }
        milliscorrector = millis();
   
       if (ntpServerAddressResolved || hasRtc)
          return localTimeZone.toLocal(getCurrentEpochTime());
        else    
          return millis() / 1000UL + manualTimeShift;
      #endif

      #if !defined(USE_NTP) && defined(USE_MANUAL_TIME_SETTING) || !defined(USE_NTP) && defined(GET_TIME_FROM_PHONE)
        if (milliscorrector > millis()
          #ifdef GET_TIME_FROM_PHONE
            && manualTimeShift + millis() / 1000UL < phoneTimeLastSync
          #endif
        ){
          manualTimeShift += 4294967; // защищаем время от переполнения millis()
          #ifdef GET_TIME_FROM_PHONE
            phoneTimeLastSync += 4294967; // а это, чтобы через 49 дней всё не заглючило
          #endif
        }
        milliscorrector = millis();
        return millis() / 1000UL + manualTimeShift;
      #endif

      #if defined(USE_NTP) && !defined(USE_MANUAL_TIME_SETTING) || defined(USE_NTP) && !defined(GET_TIME_FROM_PHONE)
        return localTimeZone.toLocal(getCurrentEpochTime());
      #endif
    }
      else
  #endif
        return millis() / 1000UL;
}

time_t getCurrentEpochTime() {
  #ifdef USE_RTC
    if (hasRtc) {
    RtcDateTime now = Rtc.GetDateTime();
    return now.Epoch32Time();
    }
  #endif
  return timeClient.getEpochTime();
}

// Получение текущего времени
/*
String Get_Time(time_t LocalTime) {
 String Time = ""; // Строка для результатов времени
 Time += ctime(&LocalTime); // Преобразуем время в строку формата Thu Jan 19 00:55:35 2017
 int i = Time.indexOf(":"); //Ишем позицию первого символа :
 Time = Time.substring(i - 2, i + 6); // Выделяем из строки 2 символа перед символом : и 6 символов после
 return Time; // Возврашаем полученное время
}
*/

//+++++ Функция ctime() отсутствует в библиотеке time.h для ESP32 Поэтому замена от V.Matchenko +++++
String Get_Time(time_t LocalTime) {
  // Преобразование секунд во время (часы, минуты, секунды)
  int hours = (LocalTime % 86400L) / 3600;
  int minutes = (LocalTime % 3600) / 60;
  int seconds = LocalTime % 60;

  // Форматирование времени в строку "HH:MM:SS"
  char buffer[9]; // Достаточно места для строки "HH:MM:SS"
  snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, seconds);
  return String(buffer);
}

#ifdef TM1637_USE
void clockTicker_blink()
{
  if (timeSynched && !DisplayFlag) {  
  
  //tm1637_brightness ();
  if (dawnFlag == 1)  //если рассвет - мигаем  часами
  {
    display.displayClock(hours, last_minute);                         // выводим время функцией часов
    if (millis() - tmr_blink > 100) {
      tmr_blink = millis();
      display.setBrightness((DispBrightness/51U)>4 ? 7 : DispBrightness/51U , DispBrightness);
      if (DispBrightness >= 204) {
        aDirection = false;
      }
      if (DispBrightness < 51U ) {
        if (!DispBrightness)  DispBrightness=1;
        aDirection = true;
      }
      if (aDirection) DispBrightness+=51U; else DispBrightness-=51U;
    }
  }
  else {
        tm1637_brightness ();
        display.setBrightness((DispBrightness/51U)>4 ? 7 : DispBrightness/51U , DispBrightness);
        display.displayClock(hours, last_minute);
      } 
  }    
}

void tm1637_brightness ()   {  // установка яркости в зависимости от  день/ночь

  if (NIGHT_HOURS_START >= NIGHT_HOURS_STOP)                          // ночное время включает переход через полночь
  {
    if (thisTime >= NIGHT_HOURS_START || thisTime <= NIGHT_HOURS_STOP)   {  // период действия ночного времени
       if (!NIGHT_HOURS_BRIGHTNESS)  DispBrightness = 0;
       else  DispBrightness = NIGHT_HOURS_BRIGHTNESS;
    }
    else   {
      if (!DAY_HOURS_BRIGHTNESS) DispBrightness = 0;
      else DispBrightness = DAY_HOURS_BRIGHTNESS;
    }
  }
  else                                                                // ночное время не включает переход через полночь
  {
    if (thisTime >= NIGHT_HOURS_START && thisTime <= NIGHT_HOURS_STOP)   {// период действия ночного времени
       if (!NIGHT_HOURS_BRIGHTNESS)  DispBrightness = 0;
       else  DispBrightness = NIGHT_HOURS_BRIGHTNESS;
    }
    else   {
      if (!DAY_HOURS_BRIGHTNESS) DispBrightness = 0;
      else DispBrightness = DAY_HOURS_BRIGHTNESS;
    }
  }
}

 #endif

#endif

void Save_File_Changes() {
    if (save_file_changes && millis() - timeout_save_file_changes >= SAVE_FILE_DELAY_TIMEOUT) {
        //writeFile("config.json", configSetup );
        //save_file_changes = 0;
        switch (save_file_changes) {
        case 1:
            writeFile(F("config.json"), configSetup );
            save_file_changes = 0;
            break;
        case 2:
            save_alarms();
            save_file_changes = 0;
            break;
        case 3:
            save_alarms();
            writeFile(F("config.json"), configSetup );
            save_file_changes = 0;
            break;
        case 4:
            cycle_get();
            save_file_changes = 0;
            break;
        case 5:
            cycle_get();
            writeFile(F("config.json"), configSetup );
            save_file_changes = 0;
            break;
        case 6:
            cycle_get();
            save_alarms();
            save_file_changes = 0;
            break;
        case 7:
            save_alarms();
            cycle_get();
            writeFile(F("config.json"), configSetup );
            save_file_changes = 0;
            break;
        }
    }
}
