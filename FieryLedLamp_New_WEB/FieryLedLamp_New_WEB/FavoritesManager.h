#pragma once
#include <EEPROM.h>
#include "EepromManager.h"
#include "Constants.h"

#define DEFAULT_FAVORITES_INTERVAL           (300U)         // значение по умолчанию для интервала переключения избранных эффектов в секундах
#define DEFAULT_FAVORITES_DISPERSION         (0U)           // значение по умолчанию для разброса интервала переключения избранных эффектов в секундах

#ifdef USE_SHUFFLE_FAVORITES // для режима последовательной смены эффектов будет такой массив (максимум поддерживается 255 эффектов)
    //пытался прописать его в private: static bool isDeactFavorite[MODE_AMOUNT];, но компилироваться не захотело.
    uint8_t shuffleFavoriteModes[MODE_AMOUNT];
    uint8_t shuffleCurrentIndex = MODE_AMOUNT; // начальное значение увеличивается на 1 и затем сравнивается с MODE_AMOUNT, чтобы создать первоначальное перемешивание режимов
#endif


ModeType modes[MODE_AMOUNT];

class FavoritesManager
{
  public:
    static uint8_t FavoritesRunning;                        // флаг "работает режим автоматической смены избранных эффектов"
    static uint16_t Interval;                               // статический интервал (время между сменами эффектов)
    static uint16_t Dispersion;                             // дополнительный динамический (случайный) интервал (время между сменами эффектов)
    static uint8_t UseSavedFavoritesRunning;                // флаг, определяющий, нужно ли использовать сохранённое значение FavoritesRunning при перезапуске; еслин нет, "избранное" будет выключено при старте
    static uint8_t FavoriteModes[MODE_AMOUNT];              // массив, каждый элемент которого соответствует флагу "эффект №... добавлен в избранные"
    static uint32_t nextModeAt;                             // ближайшее время переключения на следующий избранный эффект (millis())
	  static bool rndCycle;                                // Перемешивать цикл или нет

    static void SetStatus(char* statusText)                 // помещает в statusText состояние режима работы избранных эффектов
    {
      char buff[6];
      uint8_t n;
      strcpy_P(statusText, PSTR("FAV "));

      itoa(FavoritesRunning, buff, 10);
      strcat(statusText, buff);
      strcat_P(statusText, PSTR(" "));
      buff[0] = '\0';

      itoa(Interval, buff, 10);
      strcat(statusText, buff);
      strcat_P(statusText, PSTR(" "));
      buff[0] = '\0';

      itoa(Dispersion, buff, 10);
      strcat(statusText, buff);
      strcat_P(statusText, PSTR(" "));
      buff[0] = '\0';

      itoa(UseSavedFavoritesRunning, buff, 10);
      strcat(statusText, buff);
      strcat_P(statusText, PSTR(" "));
      buff[0] = '\0';

      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      {
        itoa(FavoriteModes[eff_num_correct[i]], buff, 10);
        strcat(statusText, buff);
        if (i < MODE_AMOUNT - 1) strcat_P(statusText, PSTR(" "));
        buff[0] = '\0';
      }

      statusText += '\0';
    }

    static void ConfigureFavorites(const char* statusText)  // принимает statusText, парсит его и инициализирует свойства класса значениями из statusText'а
    {
      FavoritesRunning = getFavoritesRunning(statusText);
      if (FavoritesRunning == 0)
      {
        nextModeAt = 0;
      }
      Interval = getInterval(statusText);
      Dispersion = getDispersion(statusText);
      UseSavedFavoritesRunning = getUseSavedFavoritesRunning(statusText);
      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      {
        FavoriteModes[eff_num_correct[i]] = getModeOnOff(statusText, i);
      }
#ifdef USE_SHUFFLE_FAVORITES //пускай список воспроизведения перемешается, раз список избранного изменился
      shuffleCurrentIndex = MODE_AMOUNT;
#endif
    }

    static bool HandleFavorites(                            // функция, обрабатывающая циклическое переключение избранных эффектов; возвращает true, если эффект был переключен
      bool* ONflag,
      uint8_t* currentMode,
      bool* loadingFlag
      //#ifdef USE_NTP
      #if defined(USE_NTP) || defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE)
      , bool* dawnFlag
      #endif
      , uint8_t* random_on
      , uint8_t* selectedSettings
    )
    {
      if (FavoritesRunning == 0 ||
          !*ONflag                                          // лампа не переключается на следующий эффект при выключенной матрице
          //#ifdef USE_NTP
          #if defined(USE_NTP) || defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE)
          || *dawnFlag                                      // лампа не переключается на следующий эффект при включенном будильнике
          #endif
          || *currentMode == EFF_WHITE_COLOR && FavoriteModes[EFF_WHITE_COLOR] == 0U // лампа не переключается на следующий эффект, если выбран режим Белый свет, и он не в списке режима Цикл
      )
      {
        return false;
      }

      if (nextModeAt == 0)                                  // лампа не переключается на следующий эффект сразу после включения режима избранных эффектов
      {
        nextModeAt = getNextTime();
        return false;
      }

      if (millis() >= nextModeAt)
      {
        *currentMode = getNextFavoriteMode(currentMode);
        
		//jsonWrite(configSetup, "eff_sel", *currentMode);
		//jsonWrite(configSetup, "br", modes[*currentMode].Brightness);
		//jsonWrite(configSetup, "sp", modes[*currentMode].Speed);
		//jsonWrite(configSetup, "sc", modes[*currentMode].Scale);
        #ifdef USE_MULTIPLE_LAMPS_CONTROL
        //multiple_lamp_control ();
        repeat_multiple_lamp_control = true;
        #endif //USE_MULTIPLE_LAMPS_CONTROL
        
        *loadingFlag = true;
        nextModeAt = getNextTime();

          if (*random_on) *selectedSettings = 1U;

        #ifdef GENERAL_DEBUG
        LOG.printf_P(PSTR("Переключение на следующий избранный режим: %d\n\n"), (*currentMode));
        #endif
        
      
        return true;
      }

      return false;
    }
/*
    static void ReadFavoritesFromEeprom()
    {
      Interval = EepromManager::ReadUint16(EEPROM_FAVORITES_START_ADDRESS + 1);
      Dispersion = EepromManager::ReadUint16(EEPROM_FAVORITES_START_ADDRESS + 3);
      UseSavedFavoritesRunning = EEPROM.read(EEPROM_FAVORITES_START_ADDRESS + 5);
      FavoritesRunning = UseSavedFavoritesRunning > 0 ? EEPROM.read(EEPROM_FAVORITES_START_ADDRESS) : FavoritesRunning;

      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      {
        FavoriteModes[i] = EEPROM.read(EEPROM_FAVORITES_START_ADDRESS + i + 6);
        FavoriteModes[i] = FavoriteModes[i] > 0 ? 1 : 0;
      }
    }

    static void SaveFavoritesToEeprom()
    {
      EEPROM.put(EEPROM_FAVORITES_START_ADDRESS, FavoritesRunning);
      EepromManager::WriteUint16(EEPROM_FAVORITES_START_ADDRESS + 1, Interval);
      EepromManager::WriteUint16(EEPROM_FAVORITES_START_ADDRESS + 3, Dispersion);
      EEPROM.put(EEPROM_FAVORITES_START_ADDRESS + 5, UseSavedFavoritesRunning);

      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      {
        EEPROM.put(EEPROM_FAVORITES_START_ADDRESS + i + 6, FavoriteModes[i] > 0 ? 1 : 0);
      }

      EEPROM.commit();
    }
*/
    static void TurnFavoritesOff()
    {
      FavoritesRunning = 0;
      nextModeAt = 0;
    }

  private:
    
    static bool isStatusTextCorrect(const char* statusText) // валидирует statusText (проверяет, правильное ли количество компонентов он содержит)
    {
      char buff[MAX_UDP_BUFFER_SIZE];
      strcpy(buff, statusText);

      uint8_t lexCount = 0;
      char* p = strtok(buff, " ");
      while (p != NULL)                                   // пока есть лексемы...
      {
        lexCount++;
        p = strtok(NULL, " ");
      }

      return lexCount == getStatusTextNormalComponentsCount();
    }

    static uint8_t getStatusTextNormalComponentsCount()     // возвращает правильное ли коичество компонентов для statusText в зависимости от определённого формата команды и количества эффектов
    {
      // "FAV 0/1 <цифра> <цифра> 0/1 <массив цифр 0/1 для каждого режима>" (вкл/выкл, интервал в секундах, разброс в секундах, использовать ли хранимый вкл/выкл, вкл/выкл каждого эффекта в избранные)
      return
        1 +          // "FAV"
        1 +          // вкл/выкл
        1 +          // интервал
        1 +          // разброс
        1 +          // использовать ли хранимый вкл/выкл
        MODE_AMOUNT; // 0/1 для каждого эффекта
    }

    static uint8_t getFavoritesRunning(const char* statusText)        // возвращает признак вкл/выкл режима избранных эффектов из statusText
    {
      char lexem[2];
      memset(lexem, 0, 2);
      strcpy(lexem, getLexNo(statusText, 1));
      return lexem != NULL
        ? !strcmp(lexem, "1")
        : 0;
    }

    static uint16_t getInterval(const char* statusText)     // возвращает интервал (постоянную составляющую) переключения избранных эффектов из statusText
    {
      char lexem[6];
      memset(lexem, 0, 6);
      strcpy(lexem, getLexNo(statusText, 2));
      return lexem != NULL
        ? atoi((const char*)lexem)
        : DEFAULT_FAVORITES_INTERVAL;
    }

    static uint16_t getDispersion(const char* statusText)   // возвращает разброс (случайную составляющую) интервала переключения избранных эффектов из statusText
    {
      char lexem[6];
      memset(lexem, 0, 6);
      strcpy(lexem, getLexNo(statusText, 3));
      return lexem != NULL
        ? atoi((const char*)lexem)
        : DEFAULT_FAVORITES_DISPERSION;
    }

    static uint8_t getUseSavedFavoritesRunning(const char* statusText)// возвращает признак вкл/выкл режима избранных эффектов из statusText
    {
      char lexem[2];
      memset(lexem, 0, 2);
      strcpy(lexem, getLexNo(statusText, 4));
      return lexem != NULL
        ? !strcmp(lexem, "1")
        : 0;
    }

    static bool getModeOnOff(const char* statusText, uint8_t modeId)  // возвращает признак включения указанного эффекта в избранные эффекты
    {
      char lexem[2];
      memset(lexem, 0, 2);
      strcpy(lexem, getLexNo(statusText, modeId + 5));
      return lexem != NULL
        ? !strcmp(lexem, "1")
        : false;
    }

    static char* getLexNo(const char* statusText, uint8_t pos)        // служебная функция, разбивает команду statusText на лексемы ("слова", разделённые пробелами) и возвращает указанную по счёту лексему
    {
      if (!isStatusTextCorrect(statusText))
      {
        return NULL;
      }

      const uint8_t buffSize = MAX_UDP_BUFFER_SIZE;
      char buff[buffSize];
      memset(buff, 0, buffSize);
      strcpy(buff, statusText);

      uint8_t lexPos = 0;
      char* p = strtok(buff, " ");
      while (p != NULL)                                   // пока есть лексемы...
      {
        if (lexPos == pos)
        {
          return p;
        }

        p = strtok(NULL, " ");
        lexPos++;
      }

      return NULL;
    }

#ifdef USE_SHUFFLE_FAVORITES
    static uint8_t getNextFavoriteMode(uint8_t* currentMode)   {  // возвращает следующий (случайный) включенный в избранные эффект
      uint8_t result;
      uint8_t count = MODE_AMOUNT;// считаем в этом счётчике, есть ли вообще в наличии избранные режимы. хотя бы два
      do {
        shuffleCurrentIndex++;
        if (shuffleCurrentIndex >= MODE_AMOUNT){        // если достигнут предел количества режимов
          count = MODE_AMOUNT;// считаем в этом счётчике, есть ли вообще в наличии избранные режимы, кроме одного
         if (rndCycle)   {
          for (uint8_t i = 0; i < MODE_AMOUNT; i++){    // перемешиваем режимы
            //swap(shuffleFavoriteModes[i], shuffleFavoriteModes[random8(MODE_AMOUNT)]); одной строчкой не получилось поменять местами
            uint8_t j = random8(MODE_AMOUNT);
            result = shuffleFavoriteModes[i];
            shuffleFavoriteModes[i] = shuffleFavoriteModes[j];
            shuffleFavoriteModes[j] = result;
            if (FavoriteModes[i] == 0) // заодно считаем, вдруг нет избранных режимов, кроме одного
              count--;
          }
         } 
        else   {
          for (uint8_t i = 0; i < MODE_AMOUNT; i++)   {     //расставляем очередь по порядку, начиная от текущего эффекта
          shuffleFavoriteModes[i] = (*currentMode + i + 1U) % MODE_AMOUNT;
          if (FavoriteModes[i] == 0) // заодно считаем, вдруг нет избранных режимов, кроме одного
            count--;
          }
        }
          shuffleCurrentIndex = 0;
        }
      } while ((FavoriteModes[shuffleFavoriteModes[shuffleCurrentIndex]] == 0U || shuffleFavoriteModes[shuffleCurrentIndex] == *currentMode) && count > 1U);
      if (count > 1U)
        result = shuffleFavoriteModes[shuffleCurrentIndex];
      else
        result = *currentMode + 1U < MODE_AMOUNT ? *currentMode + 1U : 0U;
      return result;
    }
#else
    static uint8_t getNextFavoriteMode(uint8_t* currentMode)  // возвращает следующий (случайный) включенный в избранные эффект
    {
      uint8_t result = *currentMode;
      
      for (uint8_t n = 0; n < MODE_AMOUNT; n++)
      {
          if (eff_num_correct[n] == result)
          {
              result = n;
              break;
          }
      }
      
      for (uint8_t tryNo = 0; tryNo <= random(0, MODE_AMOUNT); tryNo++)// случайное количество попыток определения следующего эффекта; без этого будет выбран следующий (избранный) по порядку после текущего
      {
        for (uint8_t i = (result + 1); i <= (result + MODE_AMOUNT); i++)
        {
          if (FavoriteModes[eff_num_correct[i < MODE_AMOUNT ? i : i - MODE_AMOUNT]] > 0)
          {
            result = i < MODE_AMOUNT ? i : i - MODE_AMOUNT;
            break;
          }
        }
		if (!rndCycle) break;
      }
      return eff_num_correct[result];
    }
#endif

    static uint32_t getNextTime()                           // определяет время следующего переключения на следующий избранный эффект
    {
      return millis() + Interval * 1000 + random(0, Dispersion + 1) * 1000;
    }
};
