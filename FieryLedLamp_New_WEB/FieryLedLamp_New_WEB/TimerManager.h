#pragma once


class TimerManager
{
  public:
    static bool TimerRunning;                               // флаг "таймер взведён"
    static bool TimerHasFired;                              // флаг "таймер отработал"
    static uint8_t TimerOption;                             // индекс элемента в списке List Picker'а
    static uint32_t TimeToFire;                             // время, в которое должен сработать таймер (millis)

    static void HandleTimer(                                // функция, обрабатывающая срабатывание таймера, гасит матрицу
      bool* ONflag,
      //bool* settChanged,
      //uint32_t* eepromTimeout,
      uint32_t* timeout_save_file_changes,
      uint8_t* save_file_changes,
      void (*changePower)())
    {
      if (!TimerManager::TimerHasFired &&
           TimerManager::TimerRunning &&
           millis() >= TimerManager::TimeToFire)
      {
        #ifdef GENERAL_DEBUG
        LOG.print(F("Выключение по таймеру\n\n"));
        #endif

        TimerManager::TimerRunning = false;
        TimerManager::TimerHasFired = true;
        FastLED.clear();
        delay(2);
        FastLED.show();
        *ONflag = !(*ONflag);
        jsonWrite(configSetup, "Power", (uint8_t)*ONflag);

        if (!(*ONflag))  {
            //*eepromTimeout = millis() - EEPROM_WRITE_DELAY;
            *timeout_save_file_changes = millis() - SAVE_FILE_DELAY_TIMEOUT;
            if (!FavoritesManager::FavoritesRunning) EepromManager::EepromPut(modes);
            *save_file_changes = 7;
            timeTick();
        }
        else EepromManager::EepromGet(modes);
        changePower();        
        #ifdef USE_MULTIPLE_LAMPS_CONTROL
        multiple_lamp_control ();
        #endif  //USE_MULTIPLE_LAMPS_CONTROL        

//        #ifdef USE_BLYNK короче, раз в Блинке нет управления таймером, то и это мы поддерживать не будем
//        updateRemoteBlynkParams();
//        #endif
      }
    }
};
