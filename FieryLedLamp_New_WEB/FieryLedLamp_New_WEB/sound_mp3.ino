#ifdef MP3_TX_PIN

#ifdef DF_PLAYER_IS_ORIGINAL
  #define ADVERT_TIMER_1 800UL        // Задержка между командой старт и адверт (если озвучка не играет)
//  #define ADVERT_TIMER_H 2050UL
//  #define ADVERT_TIMER_M 2000UL
  #define ADVERT_TIMER_2 700UL        // Задержка между произнесением минут и командой стоп (когда озвучка не играет)
  #define MP3_DELAY 500               // Задержка при настройке карты или флешки
//  #define mp3_delay 70                // Задержка между командами плееру

#else
  #define ADVERT_TIMER_1 1000UL        // Задержка между командой старт и адверт (если озвучка не играет)
//  #define ADVERT_TIMER_H 3000UL
//  #define ADVERT_TIMER_M 3200UL
  #define ADVERT_TIMER_2 2000UL
  #define MP3_DELAY 2000              // Задержка при настройке карты или флешки
//  #define mp3_delay 150               // Задержка между командами плееру

#endif

#define MP3_READ_TIMEOUT  (500UL)
/*
// При наступлении ночи NIGHT_HOURS_START MP3 переходит на ночной режим
// Воиспроиведение времени используем метод "ADVERT" или объявление

// Переменные, которые были использованиы в модуле для анализа
// dawnFlag - Идет рассвет
// dawnPosition - Яркость рассвета
// ONflag - включена/выключена лампа
*/

void mp3_setup()   {
  int16_t tmp;
  if ( first_entry == 5 ){
      first_entry = 0;
      delay(mp3_delay);
      send_command(0x0C,FEEDBACK,0,0);  //Сброс модуля
      #ifdef GENERAL_DEBUG
      LOG.println(F("\n mp3 Reset "));
      #endif
      mp3_player_connect = 2;
      return;
  }
  tmp = read_command (MP3_READ_TIMEOUT);
  send_command(6,FEEDBACK,0,0);                     // Устанавливаем громкость равной 0 (от 0 до 30)
  delay(mp3_delay);
  #ifndef CHECK_MP3_CONNECTION
    if (tmp == -1) tmp = 0;                         // Не проверяем, есть ли связь с МР3 плеером
  #endif  //CHECK_MP3_CONNECTION
  if (tmp != -1) {                                  // Проверяем, есть ли связь с плеером и, если есть, то...
      if (tmp == 1 || tmp == 3) {
          send_command(0x09,FEEDBACK,0,1);          // Устанавливаем источником Flash
          delay(MP3_DELAY);                         // ----------????????????---------
      }
      if (tmp == 2) {
          send_command(0x09,FEEDBACK,0,2);          // Устанавливаем источником SDкарту
          delay(MP3_DELAY);
      }
      //read_command (MP3_READ_TIMEOUT);
      send_command(0x0E,FEEDBACK,0,0);              //Пауза
      delay(mp3_delay);
        delay(mp3_delay);
        send_command(0x07,FEEDBACK,0,Equalizer);             // Устанавливаем эквалайзер в положение Equalizer
        delay(mp3_delay);
        send_command(6,FEEDBACK,0,eff_volume);               // Устанавливаем громкость равной eff_volume (от 0 до 30)
        mp3_player_connect = 4;
      LOG.print (F("\nMP3 плеер подключён. "));
      if (tmp == 2) LOG.println (F("Установлена ​​SD-карта\n"));
      if (tmp == 1 || tmp == 3) LOG.println (F("Установлена флешка\n"));
      if (tmp == 0) LOG.println (F("SD-карта или Флешка не установлена\n"));
    }
    else { LOG.println (F("\nSD-карта (флешка) не установлена ​​или МР3 плеер не подключен\n")); mp3_player_connect = 0; }
}

void play_time_ADVERT()   {
  if (mp3_player_connect == 4 && first_entry) {  
    if (first_entry !=2) {
       if (first_entry==1 && advert_hour) {
           advert_flag = true;
           first_entry = 3;
           send_command(0x06,FEEDBACK,0,0);
           delay(mp3_delay);
           if ((pause_on || mp3_stop) && !alarm_sound_flag) {  //+++++-----------+++++++++---------+++++++++
              send_command(0x0D,FEEDBACK,0,0);  //Старт
              delay(ADVERT_TIMER_1);
           }
           int pt_h=(uint8_t)((thisTime - thisTime % 60U) / 60U);
           if (pt_h==0) pt_h=24;
           send_command(0x13,FEEDBACK,0,pt_h);  //Старт Адверт №... Часы
           //Serial.print ("Start ADVERT Hour");
           mp3_timer = millis();
           delay(mp3_delay);
           if (day_night) send_command(0x06,FEEDBACK,0,day_advert_volume);  //Громкость днём
           else send_command(0x06,FEEDBACK,0,night_advert_volume);  //Громкость ночью
           }
        if (advert_hour && (millis() - mp3_timer > ADVERT_TIMER_H)) {
           advert_hour = false;
           int pt_m=(uint8_t)(thisTime % 60U);
           send_command(0x13,FEEDBACK,0,pt_m+100);  //Старт Адверт №... Минуты
           //Serial.print ("Start ADVERT Minute");
           mp3_timer = millis();
        }
        if (!advert_hour && millis() - mp3_timer > ADVERT_TIMER_M) {
           send_command(0x06,FEEDBACK,0,0);  //Громкость
           mp3_timer = millis();
           first_entry =2;
        }
    }
    else {
        if (millis() - mp3_timer > ADVERT_TIMER_2) {
           advert_flag = false;
           first_entry =0;
           delay(mp3_delay);    
            if ((pause_on || mp3_stop) && !alarm_sound_flag) {
               send_command(0x0E,FEEDBACK,0,0);  //Пауза
               delay(mp3_delay);
            }
        if (dawnflag_sound) send_command(0x06,FEEDBACK,0,alarm_volume);  //Громкость будильника
        else send_command(0x06,FEEDBACK,0,eff_volume);  //Громкость эффектов
        delay(mp3_delay);
        }
    }
  }
}

void play_sound(uint8_t folder)   {
    if (!mp3_folder) {
        delay(mp3_delay);
        send_command(0x0E,FEEDBACK,0,0);  //Пауза
        mp3_stop = true;
    }
    else {
        delay(mp3_delay);
        if ( folder >= 20 && folder <= 90 )
        {
            folder = (uint8_t) random (folder, constrain (folder + 10, 20, 99));
        }
        send_command(0x17,FEEDBACK,0,folder); // Включить непрерывное воспроизведение указанной папки
        mp3_stop = false;
    }
    CurrentFolder = folder;
    jsonWrite(configSetup, "fold_sel", CurrentFolder);
    #ifdef GENERAL_DEBUG
     LOG.print (F("\nCurrent folder "));
     LOG.println (CurrentFolder);
    #endif
}

void mp3_loop()   {
  if (dawnFlag) {                          // если наступает рассвет
      if (dawnflag_sound ) {
          if (alarm_sound_flag && (millis() - alarm_timer > 1000)) {
              alarm_timer = millis();
              send_command (0x06,FEEDBACK,0,constrain((uint8_t)(dawnPosition/8), 0, alarm_volume)); //Нарастание громкости в зависимости от стадии рассвета от 0 до alarm_volume
          }
          return;
     }
      send_command(0x0E,FEEDBACK,0,0);  //Пауза
      mp3_stop = true;
      dawnflag_sound = 1;
     if (alarm_sound_on) {
        delay(mp3_delay);
        mp3_folder = AlarmFolder;  // Папка будильника
        alarm_timer = millis();
        send_command(0x06,FEEDBACK,0,0);  //Громкость
        play_sound(mp3_folder);
        mp3_folder_last = mp3_folder;
        alarm_sound_flag = true;
     }
    return;
  }
  else {
      if (dawnflag_sound) {
        send_command(0x06,FEEDBACK,0,eff_volume);  //Громкость
        delay(mp3_delay);
        alarm_sound_flag = false;
        dawnflag_sound = 0;
        send_command(0x0E,FEEDBACK,0,0);  //Пауза
        mp3_stop = true;
        delay(mp3_delay);
      }
  }
  if (ONflag && eff_sound_on) {
    set_mp3_play_now=true;  // Указывает, что надо играть сейчас мелодии  
    }
  else
    {
    set_mp3_play_now=false;  // Указывает, что не надо играть сейчас мелодии      
    }
  
  if (!mp3_stop && !set_mp3_play_now && !pause_on) {
    send_command(0x0E,FEEDBACK,0,0);  //Пауза
    pause_on = true;
  }   
  if (!mp3_stop && set_mp3_play_now && pause_on) {
    send_command(0x0D,FEEDBACK,0,0);  //Старт
    pause_on = false;
  }
  
  
  if ((set_mp3_play_now) && (mp3_folder_last != mp3_folder)) {
        #ifdef MP3_DEBUG
          LOG.print (F("mp3_folder_last = "));
          LOG.println (mp3_folder_last);
          LOG.print (F("mp3_folder = "));
          LOG.println (mp3_folder);
        #endif   
    mp3_folder_last = mp3_folder;
    play_sound(mp3_folder);
  }
  
}

int16_t send_command(int8_t cmd, uint8_t feedback, uint8_t dat1, uint8_t dat2)
{
   uint8_t mp3_send_buf[8] = {0x7E, 0xFF, 06, 0x06, 00, 00, 00, 0xEF};
  // Посылка команды MP3 плееру
  //mp3_send_buf[0] = 0x7e; 
  //mp3_send_buf[1] = 0xFF; 
  //mp3_send_buf[2] = 0x06; 
  mp3_send_buf[3] = cmd;  // Команда
  mp3_send_buf[4] = feedback; // 0x00 = Без ответа, 0x01 = с ответом (подтверждением)
  mp3_send_buf[5] = dat1; // параметр 1
  mp3_send_buf[6] = dat2; // параметр 2
  //mp3_send_buf[7] = 0xEF;  
  //while (mp3.available()) mp3.read();
  for (uint8_t i = 0; i < 8; i++)
  {
    mp3.write(mp3_send_buf[i]);
    delay(3);
  }
#ifdef MP3_DEBUG
  LOG.println();
  LOG.print(F("mp3_sending:"));
  for (uint8_t i=0; i<8; i++) {
    LOG.print(mp3_send_buf[i],HEX);
    LOG.print(F(" "));
  }
  LOG.println();
#endif  //MP3_DEBUG
  
  if (!feedback && (cmd < 0x30)) {
      //Serial.println ("Нулевой");
      return 0xFF00;
  }
  else if ( feedback && (cmd < 0x30)) {
        //Serial.println ("Первый");
        return read_command (MP3_READ_TIMEOUT);
  }
  else if (feedback && (cmd >= 0x30)) {
        //Serial.println ("Второй");
        if (read_command (MP3_READ_TIMEOUT) == -1) return -1;
        if (read_command (MP3_READ_TIMEOUT) == -1) return -1;
        return (((int16_t)mp3_receive_buf[5]) << 8) + mp3_receive_buf[6];
      }
  else if (!feedback && (cmd >= 0x30)) {
        //Serial.println ("Третий");
        if (read_command (MP3_READ_TIMEOUT) == -1) return -1;
        return (((int16_t)mp3_receive_buf[5]) << 8) + mp3_receive_buf[6];
      }
    //Serial.println ("Ничего");
}


int16_t read_command (uint32_t mp3_read_timeout) {
    uint8_t tmp, flag = 0;
    int16_t Answer = 0;
    uint32_t mp3_read_timer = millis();
    while ((tmp = mp3.read()) != 0x7E)
        if (millis() - mp3_read_timer > mp3_read_timeout) return -1;
        
        mp3_receive_buf[0] = tmp;
        delay (3);
        for (uint8_t i = 1; i< 10; i++)
        {
            mp3_receive_buf[i] = mp3.read();
            delay(3);
        }
    #ifdef MP3_DEBUG
      LOG.println();
      LOG.print(F("mp3_received:"));
      for (uint8_t i=0; i<10; i++)
      {
          LOG.print(mp3_receive_buf[i],HEX);
          LOG.print(F(" "));
      }
      LOG.println();
    #endif  //MP3_DEBUG
if ( mp3_receive_buf[2] == 6 && mp3_receive_buf[9] == 0xEF && mp3_receive_buf[3] != 0x40) return (((int16_t)mp3_receive_buf[5]) << 8) + mp3_receive_buf[6];
    else return -1;
}
#endif
