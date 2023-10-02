#ifdef ESP_USE_BUTTON

bool brightDirection;
static bool startButtonHolding = false;                     // флаг: кнопка удерживается для изменения яркости/скорости/масштаба лампы кнопкой
static bool Button_Holding = false;


void buttonTick()
{
  if (!buttonEnabled) // события кнопки не обрабатываются, если она заблокирована , но обрабатывается сброс в default
  {
    touch.tick();
    if (touch.isStep() && touch.getHoldClicks() == 14U) {
        LOG.println("\n*** Reset to Default ***");
        showWarning(CRGB::Red, 500, 250U);
        ESP.wdtFeed();
        setModeSettings();
        updateSets();    
        if(FileCopy (F("/default/config.json"), F("/config.json"))) {
            ESP.wdtFeed();
            showWarning(CRGB::Green, 500, 250U);
            ESP.restart();
        }
        else {
            ESP.wdtFeed();
            showWarning(CRGB::Red, 500, 250U);
        }
    }  
    return;
  }

  touch.tick();
  uint8_t clickCount = touch.hasClicks() ? touch.getClicks() : 0U;


  // однократное нажатие
  if (clickCount == 1U)
  {
    if (dawnFlag) {
        #ifdef MP3_TX_PIN
        if (alarm_sound_flag) {
           send_command(0x0E,0,0,0); //Пауза
           mp3_stop = true;
           alarm_sound_flag = false;
        }
        else
        #endif  //MP3_TX_PIN
        {
            manualOff = true;
            dawnFlag = false;
            #ifdef TM1637_USE
            clockTicker_blink();
            #endif
            FastLED.setBrightness(modes[currentMode].Brightness);
            changePower();
       }
       return;
    }
    else
    {
      ONflag = !ONflag;
	  jsonWrite(configSetup, "Power", ONflag);
    }
    if (!ONflag)  {
        //eepromTimeout = millis() - EEPROM_WRITE_DELAY; // eepromTimeout = millis(); // 
        timeout_save_file_changes = millis() - SAVE_FILE_DELAY_TIMEOUT;
        if (!FavoritesManager::FavoritesRunning) EepromManager::EepromPut(modes);
        save_file_changes = 7;
        timeTick();
    }
    else EepromManager::EepromGet(modes);
    changePower();
    loadingFlag = true;

    #if (USE_MQTT)
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
    #ifdef USE_BLYNK
    updateRemoteBlynkParams();
    #endif
    #ifdef USE_MULTIPLE_LAMPS_CONTROL
    multiple_lamp_control ();
    #endif  //USE_MULTIPLE_LAMPS_CONTROL
  }


  // двухкратное нажатие
  if (clickCount == 2U)
     #ifdef MP3_TX_PIN
     if (dawnFlag) {            //if (dawnFlag && alarm_sound_flag) {
        //myDFPlayer.pause();
        send_command(0x0E,0,0,0);  //Пауза
        mp3_stop = true;
        alarm_sound_flag = false;
        manualOff = true;
        dawnFlag = false;
        #ifdef TM1637_USE
        clockTicker_blink();
        #endif
        FastLED.setBrightness(modes[currentMode].Brightness);
        changePower();
       }
       else
       #endif  //MP3_TX_PIN
      
  if (ONflag)    
  {
    uint8_t temp = jsonReadtoInt(configSetup, "eff_sel");
    if (Favorit_only)
	{
      uint8_t lastMode = currentMode;
      do 
      {
        if (++temp >= MODE_AMOUNT) temp = 0;
        currentMode = eff_num_correct[temp];
      } while (FavoritesManager::FavoriteModes[currentMode] == 0 && currentMode != lastMode);
      if (currentMode == lastMode) // если ни один режим не добавлен в избранное, всё равно куда-нибудь переключимся
        if (++temp >= MODE_AMOUNT) temp = 0;
        currentMode = eff_num_correct[temp];
	}
    else
      if (++temp >= MODE_AMOUNT) temp = 0;
    currentMode = eff_num_correct[temp];
	jsonWrite(configSetup, "eff_sel", temp);
	jsonWrite(configSetup, "br", modes[currentMode].Brightness);
    jsonWrite(configSetup, "sp", modes[currentMode].Speed);
    jsonWrite(configSetup, "sc", modes[currentMode].Scale);
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;

      if (random_on && FavoritesManager::FavoritesRunning)
        selectedSettings = 1U;

    #if (USE_MQTT)
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
    #ifdef USE_BLYNK
    updateRemoteBlynkParams();
    #endif
    #ifdef USE_MULTIPLE_LAMPS_CONTROL
    repeat_multiple_lamp_control = true;
    #endif  //USE_MULTIPLE_LAMPS_CONTROL
  }


  // трёхкратное нажатие
  if (ONflag && clickCount == 3U)
  {
    uint8_t temp = jsonReadtoInt(configSetup, "eff_sel");
	if (Favorit_only) 
	{
      uint8_t lastMode = currentMode;
      do
      {
        if (--temp >= MODE_AMOUNT) temp = MODE_AMOUNT - 1;
        currentMode = eff_num_correct[temp];
      } while (FavoritesManager::FavoriteModes[currentMode] == 0 && currentMode != lastMode);
      if (currentMode == lastMode) // если ни один режим не добавлен в избранное, всё равно куда-нибудь переключимся
        if (--temp >= MODE_AMOUNT) temp = MODE_AMOUNT - 1;
        currentMode = eff_num_correct[temp];
	}
	else 
	  if (--temp >= MODE_AMOUNT) temp = MODE_AMOUNT - 1;
    currentMode = eff_num_correct[temp];
	jsonWrite(configSetup, "eff_sel", temp);
	jsonWrite(configSetup, "br", modes[currentMode].Brightness);
    jsonWrite(configSetup, "sp", modes[currentMode].Speed);
    jsonWrite(configSetup, "sc", modes[currentMode].Scale);
    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;

      if (random_on && FavoritesManager::FavoritesRunning)
        selectedSettings = 1U;

    #if (USE_MQTT)
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
    #ifdef USE_BLYNK
    updateRemoteBlynkParams();
    #endif
    #ifdef USE_MULTIPLE_LAMPS_CONTROL
    repeat_multiple_lamp_control = true;
    #endif  //USE_MULTIPLE_LAMPS_CONTROL
  }


  // четырёхкратное нажатие
  if (clickCount == 4U)
  {
    #ifdef OTA
    if (otaManager.RequestOtaUpdate())
    {
      ONflag = true;
	  jsonWrite(configSetup, "Power", ONflag);
      currentMode = EFF_MATRIX;                             // принудительное включение режима "Матрица" для индикации перехода в режим обновления по воздуху
	  jsonWrite(configSetup, "eff_sel", currentMode);
	  jsonWrite(configSetup, "br", modes[currentMode].Brightness);
      jsonWrite(configSetup, "sp", modes[currentMode].Speed);
      jsonWrite(configSetup, "sc", modes[currentMode].Scale);
      changePower();
    }
    else
    #endif
	
	#ifdef BUTTON_CAN_SET_SLEEP_TIMER
	//if (!ota)
	{
    // мигать об успехе операции лучше до вызова changePower(), иначе сперва мелькнут кадры текущего эффекта
    showWarning(CRGB::Blue, 1000, 250U);                    // мигание синим цветом 1 секунду
    if (!ONflag) EepromManager::EepromGet(modes);
    ONflag = true;
    changePower();
	jsonWrite(configSetup, "Power", ONflag);
    jsonWrite(configSetup, "tmr", 1);
    #ifdef USE_BLYNK
    updateRemoteBlynkParams();
    #endif
    TimerManager::TimeToFire = millis() + BUTTON_SET_SLEEP_TIMER1 * 60UL * 1000UL;
    TimerManager::TimerRunning = true;
	}
    #endif //BUTTON_CAN_SET_SLEEP_TIMER	
    ;
  }


  // пятикратное нажатие
  if (clickCount == 5U)                                     // вывод IP на лампу
  {
    if (espMode == 1U)
    {
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
    }
  }


  // шестикратное нажатие
  if (clickCount == 6U)                                     // вывод текущего времени бегущей строкой
  {
    printTime(thisTime, true, ONflag);
  }


  // семикратное нажатие
  if (clickCount == 7U)  // if (ONflag &&                   // смена рабочего режима лампы: с WiFi точки доступа на WiFi клиент или наоборот
  {
    #ifdef RESET_WIFI_ON_ESP_MODE_CHANGE
      if (espMode) wifiManager.resetSettings();                             // сброс сохранённых SSID и пароля (сброс настроек подключения к роутеру)
    #endif
    espMode = (espMode == 0U) ? 1U : 0U;
	jsonWrite(configSetup, "ESP_mode", (int)espMode);
	saveConfig();  

    #ifdef GENERAL_DEBUG
    LOG.printf_P(PSTR("Рабочий режим лампы изменён и сохранён в энергонезависимую память\nНовый рабочий режим: ESP_MODE = %d, %s\nРестарт...\n"),
      espMode, espMode == 0U ? F("WiFi точка доступа") : F("WiFi клиент (подключение к роутеру)"));
    delay(1000);
    #endif

    showWarning(CRGB::Red, 3000U, 500U);                    // мигание красным цветом 3 секунды - смена рабочего режима лампы, перезагрузка
    ESP.restart();
  }

  #ifdef MP3_TX_PIN
  // Восьмикратное нажатие
  if (clickCount == 8U)  {                // Вкл / Откл звука
    if (mp3_player_connect == 4) {
      if (eff_sound_on) {
        eff_sound_on = 0;
        showWarning(CRGB::Yellow, 1000, 250U);                    // мигание желтым цветом 1 секунду
        #ifdef GENERAL_DEBUG
        LOG.println (F("Звук выключен"));
        #endif
      }
      else {
        eff_sound_on = eff_volume;
        showWarning(CRGB::Blue, 1000, 250U);                    // мигание синим цветом 1 секунду
        #ifdef GENERAL_DEBUG
        LOG.println (F("Звук включен"));
        #endif
      }
    }
    else  {
        showWarning(CRGB::Red, 1000, 250U);                    // мигание красным цветом 1 секунду
        #ifdef GENERAL_DEBUG
        LOG.println (F("mp3 player не подключен"));
        #endif
    }
    jsonWrite(configSetup, "on_sound", constrain (eff_sound_on,0,1));
  }
  #endif  //MP3_TX_PIN

  // кнопка только начала удерживаться
  if (touch.isHolded()) // пускай для выключенной лампы удержание кнопки включает белую лампу
  {
    brightDirection = !brightDirection;
    startButtonHolding = true;
  }


  // кнопка нажата и удерживается
if (touch.isStep())
  if (ONflag && !Button_Holding)
  {

    int8_t but = touch.getHoldClicks();
        //Serial.println (but);

    switch (but )
    {
      case 0U:                                              // просто удержание (до удержания кнопки кликов не было) - изменение яркости
      {
        uint8_t delta = modes[currentMode].Brightness < 10U // определение шага изменения яркости: при яркости [1..10] шаг = 1, при [11..16] шаг = 3, при [17..255] шаг = 15
          ? 1U
          : 5U;
        modes[currentMode].Brightness =
          constrain(brightDirection
            ? modes[currentMode].Brightness + delta
            : modes[currentMode].Brightness - delta,
          1, 255);
		jsonWrite(configSetup, "br", modes[currentMode].Brightness);
        FastLED.setBrightness(modes[currentMode].Brightness);
        #ifdef TM1637_USE
        DisplayFlag = 3;
        Display_Timer(modes[currentMode].Brightness);
        #endif    

        #ifdef GENERAL_DEBUG
        LOG.printf_P(PSTR("Новое значение яркости: %d\n"), modes[currentMode].Brightness);
        #endif
        #ifdef USE_MULTIPLE_LAMPS_CONTROL
        repeat_multiple_lamp_control = true;
        #endif  //USE_MULTIPLE_LAMPS_CONTROL

        break;
      }

      case 1U:                                              // удержание после одного клика - изменение скорости
      {
        modes[currentMode].Speed = constrain(brightDirection ? modes[currentMode].Speed + 1 : modes[currentMode].Speed - 1, 1, 255);
		jsonWrite(configSetup, "sp", modes[currentMode].Speed);
        loadingFlag = true; // без перезапуска эффекта ничего и не увидишь

        #ifdef GENERAL_DEBUG
        LOG.printf_P(PSTR("Новое значение скорости: %d\n"), modes[currentMode].Speed);
        #endif
        #ifdef USE_MULTIPLE_LAMPS_CONTROL
        repeat_multiple_lamp_control = true;
        #endif  //USE_MULTIPLE_LAMPS_CONTROL
        #ifdef TM1637_USE
        DisplayFlag = 3;
        Display_Timer(modes[currentMode].Speed);
        #endif    

        break;
      }

      case 2U:                                              // удержание после двух кликов - изменение масштаба
      {
        modes[currentMode].Scale = constrain(brightDirection ? modes[currentMode].Scale + 1 : modes[currentMode].Scale - 1, 1, 100);
		jsonWrite(configSetup, "sc", modes[currentMode].Scale);
        loadingFlag = true; // без перезапуска эффекта ничего и не увидишь

        #ifdef GENERAL_DEBUG
        LOG.printf_P(PSTR("Новое значение масштаба: %d\n"), modes[currentMode].Scale);
        #endif
        #ifdef USE_MULTIPLE_LAMPS_CONTROL
        repeat_multiple_lamp_control = true;
        #endif  //USE_MULTIPLE_LAMPS_CONTROL
        #ifdef TM1637_USE
        DisplayFlag = 3;
        Display_Timer(modes[currentMode].Scale);
        #endif

        break;
      }
	  
	    #ifdef BUTTON_CAN_SET_SLEEP_TIMER
	  case 3U:
	  {
		Button_Holding = true;
		// мигать об успехе операции лучше до вызова changePower(), иначе сперва мелькнут кадры текущего эффекта
		showWarning(CRGB::Blue, 1500U, 250U);                    // мигание синим цветом 1 секунду
		ONflag = true;
		changePower();
		jsonWrite(configSetup, "Power", ONflag);
        jsonWrite(configSetup, "tmr", 1);
		#ifdef USE_BLYNK
		updateRemoteBlynkParams();
		#endif
		TimerManager::TimeToFire = millis() + BUTTON_SET_SLEEP_TIMER2 * 60UL * 1000UL;
		TimerManager::TimerRunning = true;
		break;
	  }
		#endif //BUTTON_CAN_SET_SLEEP_TIMER
	  case 14U:
	  {
          showWarning(CRGB::Red, 500, 250U);
          ESP.wdtFeed();
          setModeSettings();
          updateSets();    
          if(FileCopy (F("/default/config.json"), F("/config.json"))){
              ESP.wdtFeed();
              showWarning(CRGB::Green, 2500, 250U);
              ESP.restart();
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 2500, 250U);
          }
          break;
      }
      case 19U:
	  {
          showWarning(CRGB::Red, 500, 250U);
          ESP.wdtFeed();
          setModeSettings();
          updateSets();    
          if(FileCopy (F("/default/config.json"), F("/config.json"))) {
              ESP.wdtFeed();
              showWarning(CRGB::Green, 500, 250U);
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 500, 250U);
          }
          if(FileCopy (F("/default/cycle_config.json"), F("/cycle_config.json"))) {
              ESP.wdtFeed();
              showWarning(CRGB::Green, 500, 250U);
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 500, 250U);
          }
          if(FileCopy (F("/default/sound_config.json"), F("/sound_config.json"))) {
              ESP.wdtFeed();
              showWarning(CRGB::Green, 500, 250U);
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 500, 250U);
          }
          if(FileCopy (F("/default/alarm_config.json"), F("/alarm_config.json"))) {
              ESP.wdtFeed();
              showWarning(CRGB::Green, 500, 250U);
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 500, 250U);
          }
          if(FileCopy (F("/default/hardware_config.json"), F("/hardware_config.json"))) {
              ESP.wdtFeed();
              showWarning(CRGB::Green, 500, 250U);
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 500, 250U);
          }
          if(FileCopy (F("/default/multilamp_config.json"), F("/multilamp_config.json"))) {
              ESP.wdtFeed();
              showWarning(CRGB::Green, 500, 250U);
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 500, 250U);
          }
          if(FileCopy (F("/default/index.json.gz"), F("/index.json.gz"))) {
             ESP.wdtFeed();
             showWarning(CRGB::Green, 500, 250U);
          }
          else {
             ESP.wdtFeed();
             showWarning(CRGB::Red, 500, 250U);
          }
          ESP.restart();
          break;
      }

      default:
        break;
    }		
  }
  else
  {
  if (!Button_Holding ) {
    int8_t but = touch.getHoldClicks();
        //Serial.println (but);

    switch (but )
    {
      case 0U:                                              // просто удержание (до удержания кнопки кликов не было) - белый свет
	  {
		Button_Holding = true;
		currentMode = EFF_WHITE_COLOR;
    //String Name = "correct." + jsonRead (configSetup, "lang") + ".json";
    //String Correct = readFile(Name, 2048);
    for ( uint8_t n=0; n< MODE_AMOUNT; n++)
    {
        if (eff_num_correct[n] == currentMode)
        {
            jsonWrite(configSetup, "eff_sel", n);
            break;
        }
    }
		jsonWrite(configSetup, "br", modes[currentMode].Brightness);
		jsonWrite(configSetup, "sp", modes[currentMode].Speed);
		jsonWrite(configSetup, "sc", modes[currentMode].Scale);
		ONflag = true;
		jsonWrite(configSetup, "Power", ONflag);
		changePower();
		#ifdef USE_BLYNK
		updateRemoteBlynkParams();
		#endif
		break;
	  }
	    #ifdef BUTTON_CAN_SET_SLEEP_TIMER	  
	  case 3U:
	  {
		Button_Holding = true;
		// мигать об успехе операции лучше до вызова changePower(), иначе сперва мелькнут кадры текущего эффекта
		showWarning(CRGB::Blue, 1500U, 250U);                    // мигание синим цветом 1 секунду
        EepromManager::EepromGet(modes);
		ONflag = true;
		changePower();
		jsonWrite(configSetup, "Power", ONflag);
        jsonWrite(configSetup, "tmr", 1);
		#ifdef USE_BLYNK
		updateRemoteBlynkParams();
		#endif
		TimerManager::TimeToFire = millis() + BUTTON_SET_SLEEP_TIMER2 * 60UL * 1000UL;
		TimerManager::TimerRunning = true;
		break;		
	  }
		#endif //BUTTON_CAN_SET_SLEEP_TIMER
/*        
      case 14U:
	  {
          showWarning(CRGB::Red, 500, 250U);
          ESP.wdtFeed();
          if(FileCopy (F("/default/config.json"), F("/config.json"))) {
              ESP.wdtFeed();
              showWarning(CRGB::Green, 2500, 250U);
              ESP.restart();
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 2500, 250U);
          }
          break;
      }
      case 19U:
	  {
          showWarning(CRGB::Red, 500, 250U);
          ESP.wdtFeed();
          if(FileCopy (F("/default/config.json"), F("/config.json"))) {
              ESP.wdtFeed();
              showWarning(CRGB::Green, 500, 250U);
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 500, 250U);
          }
          if(FileCopy (F("/default/cycle_config.json"), F("/cycle_config.json"))) {
              ESP.wdtFeed();
              showWarning(CRGB::Green, 500, 250U);
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 500, 250U);
          }
          if(FileCopy (F("/default/sound_config.json"), F("/sound_config.json"))) {
              ESP.wdtFeed();
              showWarning(CRGB::Green, 500, 250U);
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 500, 250U);
          }
          if(FileCopy (F("/default/alarm_config.json"), F("/alarm_config.json"))) {
              ESP.wdtFeed();
              showWarning(CRGB::Green, 500, 250U);
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 500, 250U);
          }
          if(FileCopy (F("/default/hardware_config.json"), F("/hardware_config.json"))) {
              ESP.wdtFeed();
              showWarning(CRGB::Green, 500, 250U);
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 500, 250U);
          }
          if(FileCopy (F("/default/multilamp_config.json"), F("/multilamp_config.json"))) {
              ESP.wdtFeed();
              showWarning(CRGB::Green, 500, 250U);
          }
          else {
              ESP.wdtFeed();
              showWarning(CRGB::Red, 500, 250U);
          }
          ESP.restart();
          break;
      }
*/      
	}
   }
  }

  // кнопка отпущена после удерживания
  if (ONflag && !touch.isHold() && startButtonHolding)      // кнопка отпущена после удерживания, нужно отправить MQTT сообщение об изменении яркости лампы
  {
    startButtonHolding = false;
    Button_Holding = false;
    loadingFlag = true;

    #if (USE_MQTT)
    if (espMode == 1U)
    {
      MqttManager::needToPublish = true;
    }
    #endif
    
    #ifdef USE_BLYNK
    updateRemoteBlynkParams();
    #endif
    
  }
}
#endif
