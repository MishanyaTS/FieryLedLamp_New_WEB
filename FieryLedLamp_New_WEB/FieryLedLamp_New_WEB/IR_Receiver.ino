// 
#ifdef IR_RECEIVER_USE

#define IR_REPEAT_TIMER      500   // Время ожидания повтора
#define IR_TICK_TIMER        100    // Время между автоповтором
#define IR_DIGIT_ENTER_TIMER 2000   // Время для ввода второй цифры номера эффекта

void IR_Receive_Handle ()   {       // Обработка принятого сигнала
    if (irrecv.decode(&results)) {
        if (results.repeat) { 
            if (millis() - IR_Repeat_Timer > IR_REPEAT_TIMER) {
                //Serial.print("Repeat  ");
                IR_Data_Ready = 2;
            }
        }
        else {
            IR_Code = (uint32_t)results.value;
            IR_Repeat_Timer = millis();
            IR_Data_Ready = 1;
        }
    irrecv.resume();  // Receive the next value
    }

    if (Enter_Digit_1 && millis() - IR_Dgit_Enter_Timer > IR_DIGIT_ENTER_TIMER){  // Если одна цифра нажата и время ожидания нажатия второй цифры вышло
        Enter_Digit_1 = 0;
        currentMode = eff_num_correct[Enter_Number];
	    jsonWrite(configSetup, "eff_sel", Enter_Number);
	    jsonWrite(configSetup, "br", modes[currentMode].Brightness);
        jsonWrite(configSetup, "sp", modes[currentMode].Speed);
        jsonWrite(configSetup, "sc", modes[currentMode].Scale);
        SetBrightness(modes[currentMode].Brightness);
        loadingFlag = true;
        //settChanged = true;
        //eepromTimeout = millis();
        //timeout_save_file_changes = millis();
        //bitSet (save_file_changes, 0);
        if (random_on && FavoritesManager::FavoritesRunning)
            selectedSettings = 1U;
        #if (USE_MQTT)
           if (espMode == 1U) MqttManager::needToPublish = true;
        #endif
        #ifdef USE_BLYNK
          updateRemoteBlynkParams();
        #endif
        #ifdef USE_MULTIPLE_LAMPS_CONTROL
          repeat_multiple_lamp_control = true;
        #endif  //USE_MULTIPLE_LAMPS_CONTROL
        //Serial.println(Enter_Number);
        //Serial.println("  1 цифра");
  }
}

void IR_Receive_Button_Handle()   {     //Обработка принятых команд (нажатых кнопок пульта ДУ)
    switch(IR_Code) {
        case IR_ON_OFF:
        if (IR_Data_Ready != 2)  // No repeat
        IR_Power();
        break;
        #ifdef USE_2_PULTS
        case IR2_ON_OFF:
        if (IR_Data_Ready != 2)  // No repeat
        IR_Power();
        break;
        #endif //USE_2_PULTS
        
        case IR_MUTE:
        if (IR_Data_Ready != 2)  // No repeat
        Mute();
        break;
        #ifdef USE_2_PULTS
        case IR2_MUTE:
        if (IR_Data_Ready != 2)  // No repeat
        Mute();
        break;
        #endif //USE_2_PULTS
        
        case IR_PREV:
        Prev_Next_eff(false);
        break;
        #ifdef USE_2_PULTS
        case IR2_PREV:
        Prev_Next_eff(false);
        break;
        #endif //USE_2_PULTS
        case IR_NEXT:
        Prev_Next_eff(true);
        break;
        #ifdef USE_2_PULTS
        case IR2_NEXT:
        Prev_Next_eff(true);
        break;
        #endif //USE_2_PULTS
        
        case IR_CYCLE:
        if (IR_Data_Ready != 2)  // No repeat
        Cycle_on_off();
        break;
        #ifdef USE_2_PULTS
        case IR2_CYCLE:
        if (IR_Data_Ready != 2)  // No repeat
        Cycle_on_off();
        break;
        #endif //USE_2_PULTS
       
        case IR_BR_UP:
        Bright_Up_Down(true);
        break;
        #ifdef USE_2_PULTS
        case IR2_BR_UP:
        Bright_Up_Down(true);
        break;
        #endif //USE_2_PULTS
        case IR_BR_DOWN:
        Bright_Up_Down(false);
        break;
        #ifdef USE_2_PULTS
        case IR2_BR_DOWN:
        Bright_Up_Down(false);
        break;
        #endif //USE_2_PULTS
       
        case IR_SP_UP:
        Speed_Up_Down(true);
        break;
        #ifdef USE_2_PULTS
        case IR2_SP_UP:
        Speed_Up_Down(true);
        break;
         #endif //USE_2_PULTS
       case IR_SP_DOWN:
        Speed_Up_Down(false);
        break;
        #ifdef USE_2_PULTS
        case IR2_SP_DOWN:
        Speed_Up_Down(false);
        break;
        #endif //USE_2_PULTS
       
        case IR_SC_UP:
        Scale_Up_Down(true);
        break;
        #ifdef USE_2_PULTS
        case IR2_SC_UP:
        Scale_Up_Down(true);
        break;
        #endif //USE_2_PULTS
       case IR_SC_DOWN:
        Scale_Up_Down(false);
        break;
        #ifdef USE_2_PULTS
        case IR2_SC_DOWN:
        Scale_Up_Down(false);
        break;
        #endif //USE_2_PULTS
       
        case IR_VOL_UP:
        Volum_Up_Down(true);
        break;
        #ifdef USE_2_PULTS
        case IR2_VOL_UP:
        Volum_Up_Down(true);
        break;
        #endif //USE_2_PULTS
        case IR_VOL_DOWN:
        Volum_Up_Down(false);
        break;
        #ifdef USE_2_PULTS
        case IR2_VOL_DOWN:
        Volum_Up_Down(false);
        break;
        #endif //USE_2_PULTS
        
        case IR_TIME:
        if (IR_Data_Ready != 2)  // No repeat
        printTime(thisTime, true, ONflag);
        break;
        #ifdef USE_2_PULTS
        case IR2_TIME:
        if (IR_Data_Ready != 2)  // No repeat
        printTime(thisTime, true, ONflag);
        break;
        #endif //USE_2_PULTS
        
        case IR_IP:
        if (IR_Data_Ready != 2)  // No repeat
        Print_IP();
        break;
        #ifdef USE_2_PULTS
        case IR2_IP:
        if (IR_Data_Ready != 2)  // No repeat
        Print_IP();
        break;
        #endif //USE_2_PULTS
        
        case IR_FOLD_PREV:
        Folder_Next_Prev(false);
        break;
        #ifdef USE_2_PULTS
        case IR2_FOLD_PREV:
        Folder_Next_Prev(false);
        break;
        #endif //USE_2_PULTS
       case IR_FOLD_NEXT:
        Folder_Next_Prev(true);
        break;
        #ifdef USE_2_PULTS
        case IR2_FOLD_NEXT:
        Folder_Next_Prev(true);
        break;
        #endif //USE_2_PULTS
        
        case IR_RND:
        if (IR_Data_Ready != 2)  // No repeat
        Current_Eff_Rnd_Def(true);
        break;
        #ifdef USE_2_PULTS
        case IR2_RND:
        if (IR_Data_Ready != 2)  // No repeat
        Current_Eff_Rnd_Def(true);
        break;
        #endif //USE_2_PULTS
        case IR_DEF:
        if (IR_Data_Ready != 2)  // No repeat
        Current_Eff_Rnd_Def(false);
        break;
        #ifdef USE_2_PULTS
        case IR2_DEF :
        if (IR_Data_Ready != 2)  // No repeat
        Current_Eff_Rnd_Def(false);
        break;
        #endif //USE_2_PULTS
        
        case IR_EQ:
        if (IR_Data_Ready != 2)  // No repeat
        IR_Equalizer();
        break;
        #ifdef USE_2_PULTS
        case IR2_EQ:
        if (IR_Data_Ready != 2)  // No repeat
        IR_Equalizer();
        break;
        #endif //USE_2_PULTS
        
        case IR_FAV_ADD:
        if (IR_Data_Ready != 2)  // No repeat
        Favorit_Add_Del(true);
        break;
        #ifdef USE_2_PULTS
        case IR2_FAV_ADD:
        if (IR_Data_Ready != 2)  // No repeat
        Favorit_Add_Del(true);
        break;
        #endif //USE_2_PULTS
        case IR_FAV_DEL:
        if (IR_Data_Ready != 2)  // No repeat
        Favorit_Add_Del(false);
        break;
        #ifdef USE_2_PULTS
        case IR2_FAV_DEL :
        if (IR_Data_Ready != 2)  // No repeat
        Favorit_Add_Del(false);
        break;
        #endif //USE_2_PULTS

        case IR_1:
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(1);
        break;
        #ifdef USE_2_PULTS
        case IR2_1 :
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(1);
        break;
        #endif //USE_2_PULTS
        case IR_2:
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(2);
        break;
        #ifdef USE_2_PULTS
        case IR2_2 :
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(2);
        break;
        #endif //USE_2_PULTS
        case IR_3:
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(3);
        break;
        #ifdef USE_2_PULTS
        case IR2_3 :
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(3);
        break;
        #endif //USE_2_PULTS
        case IR_4:
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(4);
        break;
        #ifdef USE_2_PULTS
        case IR2_4 :
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(4);
        break;
        #endif //USE_2_PULTS
        case IR_5:
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(5);
        break;
        #ifdef USE_2_PULTS
        case IR2_5 :
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(5);
        break;
        #endif //USE_2_PULTS
        case IR_6:
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(6);
        break;
        #ifdef USE_2_PULTS
       case IR2_6 :
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(6);
        break;
        #endif //USE_2_PULTS
        case IR_7:
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(7);
        break;
        #ifdef USE_2_PULTS
       case IR2_7 :
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(7);
        break;
        #endif //USE_2_PULTS
        case IR_8:
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(8);
        break;
        #ifdef USE_2_PULTS
        case IR2_8 :
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(8);
        break;
        #endif //USE_2_PULTS
        case IR_9:
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(9);
        break;
        #ifdef USE_2_PULTS
        case IR2_9 :
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(9);
        break;
        #endif //USE_2_PULTS
        case IR_0:
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(0);
        break;
        #ifdef USE_2_PULTS
        case IR2_0 :
        if (IR_Data_Ready != 2)  // No repeat
        Digit_Handle(0);
        break;
        #endif //USE_2_PULTS

        default: break;
    }
    #ifdef GENERAL_DEBUG
    LOG.print("IR_CODE = ");
    LOG.println(IR_Code, HEX);
    #endif  //GENERAL_DEBUG
}

void IR_Power()   {
    if (dawnFlag) {
        #ifdef MP3_TX_PIN
        if (alarm_sound_flag) {
           //myDFPlayer.pause();
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
            SetBrightness(modes[currentMode].Brightness);
            changePower();
       }
       return;
    }
    else
    {
        ONflag = !ONflag;
	    jsonWrite(configSetup, "Power", ONflag);
        if (!ONflag)  {
            //eepromTimeout = millis() - EEPROM_WRITE_DELAY;
            timeout_save_file_changes = millis() - SAVE_FILE_DELAY_TIMEOUT;
            if (!FavoritesManager::FavoritesRunning) EepromManager::EepromPut(modes);
            save_file_changes = 7;
            timeTick();
        }
        else {
            EepromManager::EepromGet(modes);
            timeout_save_file_changes = millis();
            bitSet (save_file_changes, 0);
        }
        changePower();
    }
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
    if (ONflag) {
        repeat_multiple_lamp_control=true;
    }
    else {
        multiple_lamp_control ();
    }
    #endif  //USE_MULTIPLE_LAMPS_CONTROL
}

void Mute()   {                // Вкл / Откл звука
    #ifdef MP3_TX_PIN
    if (mp3_player_connect == 4) {
      if (eff_sound_on) {
        eff_sound_on = 0;
        #ifdef GENERAL_DEBUG
        LOG.println (F("Звук выключен"));
        #endif
      }
      else {
        eff_sound_on = eff_volume;
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
    timeout_save_file_changes = millis();
    bitSet (save_file_changes, 0);
    #ifdef USE_MULTIPLE_LAMPS_CONTROL
    repeat_multiple_lamp_control = true;
    #endif  //USE_MULTIPLE_LAMPS_CONTROL
  #endif  //MP3_TX_PIN
}

void Prev_Next_eff(bool direction)   {
    if (ONflag)    
    {
      uint8_t temp = jsonReadtoInt(configSetup, "eff_sel");
      if (direction) {
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
      }
      else {
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
      }
    currentMode = eff_num_correct[temp];
	jsonWrite(configSetup, "eff_sel", temp);
	jsonWrite(configSetup, "br", modes[currentMode].Brightness);
    jsonWrite(configSetup, "sp", modes[currentMode].Speed);
    jsonWrite(configSetup, "sc", modes[currentMode].Scale);
    SetBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
    //settChanged = true;
    //eepromTimeout = millis();
    #ifdef TM1637_USE
    DisplayFlag = 0;
    Display_Timer();
    #endif
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
}

void Cycle_on_off()   {
    if (ONflag)   {
        uint8_t tmp;
        jsonReadtoInt(configSetup, "cycle_on") == 0? tmp = 1 : tmp = 0;
	    jsonWrite(configSetup, "cycle_on", tmp);
	    FavoritesManager::FavoritesRunning = tmp;
        if (tmp){
            showWarning(CRGB::Blue, 500, 250U);        // мигание синим цветом 0.5 секунды
            EepromManager::EepromPut(modes);
            //eepromTimeout = millis() - EEPROM_WRITE_DELAY;
        }
        else {
            showWarning(CRGB::Red, 500, 250U);        // мигание красным цветом 0.5 секунды
            EepromManager::EepromGet(modes);
        }
        #if (USE_MQTT)
        if (espMode == 1U)
        {
          MqttManager::needToPublish = true;
        }
        #endif
    }
}

void Bright_Up_Down(bool direction)   {
    uint8_t delta = IR_Data_Ready == 1 ? 1U : 4U;
    modes[currentMode].Brightness = constrain(direction ? modes[currentMode].Brightness + delta : modes[currentMode].Brightness - delta, 1, 255);
	jsonWrite(configSetup, "br", modes[currentMode].Brightness);
    SetBrightness(modes[currentMode].Brightness);
    #ifdef TM1637_USE
    DisplayFlag = 3;
    Display_Timer(modes[currentMode].Brightness);
    #endif    
    #ifdef GENERAL_DEBUG
        LOG.printf_P(PSTR("Новое значение яркости: %d\n"), modes[currentMode].Brightness);
    #endif
    //timeout_save_file_changes = millis();
    //bitSet (save_file_changes, 0);
    //settChanged = true;
    //eepromTimeout = millis();
    #ifdef USE_MULTIPLE_LAMPS_CONTROL
        repeat_multiple_lamp_control = true;
    #endif  //USE_MULTIPLE_LAMPS_CONTROL
    #if (USE_MQTT)
    if (espMode == 1U)
    {
        MqttManager::needToPublish = true;
    }
    #endif
}

void Speed_Up_Down(bool direction)   {
    uint8_t delta = IR_Data_Ready == 1 ? 1U : 4U;
    modes[currentMode].Speed = constrain(direction ? modes[currentMode].Speed + delta : modes[currentMode].Speed - delta, 1, 255);
	jsonWrite(configSetup, "sp", modes[currentMode].Speed);
    loadingFlag = true; // без перезапуска эффекта ничего и не увидишь
    #ifdef TM1637_USE
    DisplayFlag = 3;
    Display_Timer(modes[currentMode].Speed);
    #endif    
    #ifdef GENERAL_DEBUG
        LOG.printf_P(PSTR("Новое значение скорости: %d\n"), modes[currentMode].Speed);
    #endif
    //timeout_save_file_changes = millis();
    //bitSet (save_file_changes, 0);
    //settChanged = true;
    //eepromTimeout = millis();
    #ifdef USE_MULTIPLE_LAMPS_CONTROL
        repeat_multiple_lamp_control = true;
    #endif  //USE_MULTIPLE_LAMPS_CONTROL
    #if (USE_MQTT)
    if (espMode == 1U)
    {
        MqttManager::needToPublish = true;
    }
    #endif
}

void Scale_Up_Down(bool direction)   {
    uint8_t delta = IR_Data_Ready == 1 ? 1U : 2U;
    modes[currentMode].Scale = constrain(direction ? modes[currentMode].Scale + delta : modes[currentMode].Scale - delta, 1, 100);
	jsonWrite(configSetup, "sc", modes[currentMode].Scale);
    loadingFlag = true; // без перезапуска эффекта ничего и не увидишь
    #ifdef TM1637_USE
    DisplayFlag = 3;
    Display_Timer(modes[currentMode].Scale);
    #endif
    
    #ifdef GENERAL_DEBUG
        LOG.printf_P(PSTR("Новое значение масштаба: %d\n"), modes[currentMode].Scale);
    #endif
    //timeout_save_file_changes = millis();
    //bitSet (save_file_changes, 0);
    //settChanged = true;
    //eepromTimeout = millis();
    #ifdef USE_MULTIPLE_LAMPS_CONTROL
        repeat_multiple_lamp_control = true;
    #endif  //USE_MULTIPLE_LAMPS_CONTROL
    #if (USE_MQTT)
    if (espMode == 1U)
    {
        MqttManager::needToPublish = true;
    }
    #endif
}

void Volum_Up_Down (bool direction)   {
    #ifdef MP3_TX_PIN
    eff_volume = constrain(direction ? eff_volume + 1 : eff_volume - 1, 1, 30);
    jsonWrite(configSetup, "vol", eff_volume);
    if (!dawnflag_sound) send_command(6,FEEDBACK,0,eff_volume); //Громкость
    #ifdef TM1637_USE
    DisplayFlag = 3;
    Display_Timer(eff_volume);
    #endif
    #ifdef USE_MULTIPLE_LAMPS_CONTROL
    repeat_multiple_lamp_control = true;
    #endif  //USE_MULTIPLE_LAMPS_CONTROL
    #endif  //MP3_TX_PIN
}

void Print_IP()   {
    #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время вывода текста
      digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
    #endif
    if (espMode == 1U)
    {
      loadingFlag = true;
      while(!fillString(WiFi.localIP().toString().c_str(), CRGB::White, false)) { delay(1); ESP.wdtFeed();}
      if (ColorTextFon  & (!ONflag || (currentMode == EFF_COLOR && modes[currentMode].Scale < 3))){
        FastLED.clear();
        delay(1);
        FastLED.show();
      }
      loadingFlag = true;
    }
    else
    {
      loadingFlag = true;
      String str = "Access Point 192.168.4.1";
      while(!fillString(str.c_str(), CRGB::White, false)) { delay(1); ESP.wdtFeed();}
      if (ColorTextFon  & (!ONflag || (currentMode == EFF_COLOR && modes[currentMode].Scale < 3))){
        FastLED.clear();
        delay(1);
        FastLED.show();
      }
      loadingFlag = true;
    }
    #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы или будильника
      digitalWrite(MOSFET_PIN, ONflag || (dawnFlag && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
    #endif
}

void Folder_Next_Prev(bool direction)    {
    #ifdef MP3_TX_PIN
    if (true) { //(!pause_on && !mp3_stop && eff_sound_on) {
    CurrentFolder = constrain(direction ? CurrentFolder + 1 : CurrentFolder - 1, 0, 99);
    jsonWrite(configSetup, "fold_sel", CurrentFolder);
    if (!pause_on && !mp3_stop && eff_sound_on) {
      send_command(0x17,FEEDBACK,0,CurrentFolder);           // Включить непрерывное воспроизведение указанной папки
      delay(mp3_delay);
    }
    }
    #ifdef GENERAL_DEBUG
     LOG.print (F("\nCurrent folder "));
     LOG.println (CurrentFolder);
    #endif
    #ifdef TM1637_USE
    DisplayFlag = 0;
    Display_Timer();
    #endif
     #ifdef USE_MULTIPLE_LAMPS_CONTROL
    repeat_multiple_lamp_control = true;
    #endif  //USE_MULTIPLE_LAMPS_CONTROL
    #endif  //MP3_TX_PIN
}

void Current_Eff_Rnd_Def(bool direction)   {
    if (direction) {
    selectedSettings = 1U;
    updateSets();
    #ifdef USE_MULTIPLE_LAMPS_CONTROL
    repeat_multiple_lamp_control = true;
    #endif  //USE_MULTIPLE_LAMPS_CONTROL
    }
    else {
    setModeSettings();
    updateSets();    
    #ifdef USE_MULTIPLE_LAMPS_CONTROL
    repeat_multiple_lamp_control = true;
    #endif  //USE_MULTIPLE_LAMPS_CONTROL
    }
    if (direction) showWarning(CRGB::Blue, 500, 250U);  // мигание синим цветом 0.5 секунды
    else showWarning(CRGB::Red, 500, 250U);             // мигание красным цветом 0.5 секунды
}

void IR_Equalizer()   {     // Устанавливаем эквалайзер
    #ifdef MP3_TX_PIN
    Equalizer++;
    if (Equalizer > 5) Equalizer = 0;
    jsonWrite(configSetup, "eq", Equalizer);
    send_command(0x07, FEEDBACK, 0, Equalizer); 
    timeout_save_file_changes = millis();
    bitSet (save_file_changes, 0);
    #ifdef TM1637_USE
    DisplayFlag = 3;
    Display_Timer(Equalizer);
    #endif
    #endif  //MP3_TX_PIN
}

void Favorit_Add_Del(bool direction)   {
    String configCycle = readFile(F("cycle_config.json"), 2048);
    String e = "e" + String (currentMode);
    jsonWrite(configCycle, e, direction ? 1 : 0);
    FavoritesManager::FavoriteModes[currentMode] = (direction ? 1 : 0);
    //writeFile("cycle_config.json", configCycle );
    timeout_save_file_changes = millis();
    bitSet (save_file_changes, 2);
    if (direction) showWarning(CRGB::Blue, 500, 250U);  // мигание синим цветом 0.5 секунды
    else showWarning(CRGB::Red, 500, 250U);             // мигание красным цветом 0.5 секунды
}

void Digit_Handle (uint8_t digit)   {
    if (!Enter_Digit_1){
        Enter_Digit_1 = 1;
        IR_Dgit_Enter_Timer = millis();
        Enter_Number = digit;
        #ifdef TM1637_USE
        DisplayFlag = 3;
        Display_Timer(digit);
        #endif
       //Serial.println(Enter_Number);
    }
    else {
        Enter_Digit_1 = 0;
        Enter_Number = Enter_Number * 10 + digit;
        currentMode = eff_num_correct[Enter_Number];
        #ifdef TM1637_USE
        DisplayFlag = 3;
        Display_Timer(Enter_Number);
        #endif
	    jsonWrite(configSetup, "eff_sel", Enter_Number);
	    jsonWrite(configSetup, "br", modes[currentMode].Brightness);
        jsonWrite(configSetup, "sp", modes[currentMode].Speed);
        jsonWrite(configSetup, "sc", modes[currentMode].Scale);
        SetBrightness(modes[currentMode].Brightness);
        loadingFlag = true;
        //timeout_save_file_changes = millis();
        //bitSet (save_file_changes, 0);
        if (random_on && FavoritesManager::FavoritesRunning)
            selectedSettings = 1U;
        #if (USE_MQTT)
           if (espMode == 1U) MqttManager::needToPublish = true;
        #endif
        #ifdef USE_BLYNK
          updateRemoteBlynkParams();
        #endif
        #ifdef USE_MULTIPLE_LAMPS_CONTROL
          repeat_multiple_lamp_control = true;
        #endif  //USE_MULTIPLE_LAMPS_CONTROL
        //Serial.println(Enter_Number);
        //Serial.println("  2 цифры");
    }
}

#endif //IR_RECEIVER_USE
