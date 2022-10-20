// работа с бегущим текстом

// --- НАСТРОЙКИ -----------------------
#define TEXT_DIRECTION        (1U)                          // 1 - по горизонтали, 0 - по вертикали
#define MIRR_V                (0U)                          // отразить текст по вертикали (0 / 1)
#define MIRR_H                (0U)                          // отразить текст по горизонтали (0 / 1)

#define TEXT_HEIGHT           (2U)                          // высота, на которой бежит текст (от низа матрицы)
#define LET_WIDTH             (5U)                          // ширина буквы шрифта
#define LET_HEIGHT            (8U)                          // высота буквы шрифта
#define SPACE                 (1U)                          // пробел
#define LETTER_COLOR          (CRGB::White)                 // цвет букв по умолчанию


// --- ДЛЯ РАЗРАБОТЧИКОВ ---------------

int16_t offset = WIDTH;
uint32_t scrollTimer = 0LL;

boolean fillString(const char* text, CRGB letterColor, boolean itsText)
{
  //CRGB letterColor = CHSV(modes[EFF_TEXT].Scale * 2.5 * 2.5, 255U, 255U);
//Serial.println(text);
  if (!text || !strlen(text)) { return true; }
  if (loadingFlag && !itsText) {
    offset = WIDTH;                                         // перемотка в правый край
    loadingFlag = false;
  }

  if (millis() - scrollTimer >= 255U - modes[EFF_TEXT].Speed)
  {
    scrollTimer = millis();
    FastLED.clear();
    uint8_t i = 0, j = 0;
    while (text[i] != '\0')
    {
      if ((uint8_t)text[i] > 191)                           // работаем с русскими буквами
      {
        i++;
      }
      else
      {
        drawLetter(text[i-1], text[i], offset + j * (LET_WIDTH + SPACE), letterColor);
        i++;
        j++;
      }
    }

    offset--;
    if (offset < (int16_t)(-j * (LET_WIDTH + SPACE)))       // строка убежала
    {
      offset = WIDTH + 3;
      return true;
    }
    FastLED.show();    
  }

  return false;
}


void printTime(uint32_t thisTime, bool onDemand, bool ONflag) // периодический вывод времени бегущей строкой; onDemand - по требованию, вывод текущего времени; иначе - вывод времени по расписанию
{
  //#if defined(USE_NTP) && defined(PRINT_TIME)               // вывод, только если используется синхронизация времени и если заказан его вывод бегущей строкой
  #if defined(USE_NTP) || defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE)

//  if (espMode != 1U || !ntpServerAddressResolved || !timeSynched)     // вывод только в режиме WiFi клиента и только, если имя сервера времени разрезолвлено
  if (!timeSynched)     // хз зачем было так сложно
  {
    showWarning(CRGB::Red, 4000U, 500U);                    // мигание красным цветом 4 секунды
    return;
  }

  CRGB letterColor = CRGB::Black;
  bool needToPrint = false;
  
  if (PRINT_TIME >= 1U)  
  {                                  
    if (thisTime % 60U == 0U)    // вывод  каждый час (красным цветом)
     {
        needToPrint = true;
        letterColor = CRGB::Red;
     }
    else
     {
      if ( thisTime % PRINT_TIME == 0U)      // вывод каждый час (красным цветом) + каждые PRINT_TIME минут (синим цветом)
       {
        needToPrint = true;
        letterColor = CRGB::Blue;
       }
     }
  }

  if (onDemand)
  {
    letterColor = CRGB::White;
  }

  if (((ONflag || time_always) && needToPrint && thisTime != lastTimePrinted) || onDemand)
  {
    lastTimePrinted = thisTime;
    char stringTime[10U];                                   // буффер для выводимого текста, его длина должна быть НЕ МЕНЬШЕ, чем длина текста + 1
    sprintf_P(stringTime, PSTR("-> %u:%02u"), (uint8_t)((thisTime - thisTime % 60U) / 60U), (uint8_t)(thisTime % 60U));
    loadingFlag = true;
    FastLED.setBrightness(getBrightnessForPrintTime());
    delay(1);

    #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)        // установка сигнала в пин, управляющий MOSFET транзистором, матрица должна быть включена на время вывода текста
    digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
    #endif
    
    #ifdef MP3_TX_PIN
    if (mp3_player_connect == 4){
        first_entry = 1;
        advert_hour = true;
    }
    #endif  //MP3_TX_PIN

    while (!fillString(stringTime, letterColor, false)) {
        parseUDP();
        delay (1);
        #ifdef MP3_TX_PIN
        if (day_night) {
           if ((day_advert_sound_on && mp3_player_connect == 4 && !dawnFlag) || advert_flag) play_time_ADVERT();
        }
        else {
           if ((night_advert_sound_on && mp3_player_connect == 4 && !dawnFlag) || advert_flag) play_time_ADVERT();
        }
        #endif  //MP3_TX_PIN
        HTTP.handleClient();
        #ifdef ESP_USE_BUTTON
          buttonTick();
        #endif
        ESP.wdtFeed();
    }
    
    #ifdef MP3_TX_PIN
    while (advert_flag) {
        play_time_ADVERT();
        ESP.wdtFeed();
    }
    //first_entry = 0;
    #endif  //MP3_TX_PIN

    #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)        // установка сигнала в пин, управляющий MOSFET транзистором, соответственно состоянию вкл/выкл матрицы или будильника
    digitalWrite(MOSFET_PIN, ONflag || (dawnFlag && !manualOff) ? MOSFET_LEVEL : !MOSFET_LEVEL);
    #endif

    FastLED.setBrightness(modes[currentMode].Brightness);
    loadingFlag = true;
  }

  #endif
}


uint8_t getBrightnessForPrintTime()     // определение яркости для вывода времени бегущей строкой в зависимости от  успешности синхронизации времени,
                                                                      // текущего времени суток, настроек дневного/ночного времени 
{
  #if defined(USE_NTP) || defined(USE_MANUAL_TIME_SETTING) || defined(GET_TIME_FROM_PHONE)

  //if (!timeSynched || ONflag)     // хз зачем было так сложно
  if (!timeSynched)  
  {
    day_night = false;
    return modes[currentMode].Brightness;
  }

  if (NIGHT_HOURS_START >= NIGHT_HOURS_STOP)                          // ночное время включает переход через полночь
  {
    if (thisTime >= NIGHT_HOURS_START || thisTime <= NIGHT_HOURS_STOP)// период действия ночного времени
    {
        day_night = false;
        return NIGHT_HOURS_BRIGHTNESS;
    }
  }
  else                                                                // ночное время не включает переход через полночь
  {
    if (thisTime >= NIGHT_HOURS_START && thisTime <= NIGHT_HOURS_STOP)// период действия ночного времени
    {
        day_night = false;
        return NIGHT_HOURS_BRIGHTNESS;
    }
  }

  day_night = true;
  return DAY_HOURS_BRIGHTNESS;                                   // дневное время

  #endif

  day_night = false;
  return modes[currentMode].Brightness;
}


void drawLetter(uint8_t subleter, uint8_t letter, int8_t offset, CRGB letterColor)
{
 
  uint8_t start_pos = 0, finish_pos = LET_WIDTH;

  if (offset < (int8_t)-LET_WIDTH || offset > (int8_t)WIDTH)
  {
    return;
  }
  if (offset < 0)
  {
    start_pos = (uint8_t)-offset;
  }
  if (offset > (int8_t)(WIDTH - LET_WIDTH))
  {
    finish_pos = (uint8_t)(WIDTH - offset);
  }
  for (uint8_t i = start_pos; i < finish_pos; i++)
  {
    uint8_t thisByte;
    if (MIRR_V)
    {
      thisByte = getFont(subleter, letter, (uint8_t)(LET_WIDTH - 1 - i));
    }
    else
    {
      thisByte = getFont(subleter, letter, i);
    }

    for (uint8_t j = 0; j < LET_HEIGHT; j++)
    {
      bool thisBit = MIRR_H
        ? thisByte & (1 << j)
        : thisByte & (1 << (LET_HEIGHT - 1 - j));

      // рисуем столбец (i - горизонтальная позиция, j - вертикальная)
      if (TEXT_DIRECTION)
      {
        if (thisBit)
        {
          leds[XY(offset + i, TEXT_HEIGHT + j)] = letterColor;
        }
        else
        {
          drawPixelXY(offset + i, TEXT_HEIGHT + j, 0x000000);
        }
      }
      else
      {
        if (thisBit)
        {
          leds[XY(i, offset + TEXT_HEIGHT + j)] = letterColor;
        }
        else
        {
          drawPixelXY(i, offset + TEXT_HEIGHT + j, 0x000000);
        }
      }
    }
  }
}


// --- СЛУЖЕБНЫЕ ФУНКЦИИ ---------------
uint8_t getFont(uint8_t subasciiCode, uint8_t asciiCode, uint8_t row)             // интерпретатор кода символа в массиве fontHEX (для Arduino IDE 1.8.* и выше)
{
  asciiCode = asciiCode - '0' + 16;                         // перевод код символа из таблицы ASCII в номер согласно нумерации массива

  if (asciiCode <= 94)                                      // печатаемые символы и английские буквы
  {
    return pgm_read_byte(&fontHEX[asciiCode][row]);
  }
  else if (asciiCode >= 112 && asciiCode <= 159 && subasciiCode == 0xD0)  // А - Я а - п
  {
    return pgm_read_byte(&fontHEX[asciiCode - 17][row]);
  }
  else if (asciiCode >= 96 && asciiCode <= 111 && subasciiCode == 0xD1) // р - я
  {
    return pgm_read_byte(&fontHEX[asciiCode + 47][row]);
  }
  else if (asciiCode == 97 && subasciiCode == 0xD0)  // Ё
  {
    return pgm_read_byte(&fontHEX[159][row]);//return pgm_read_byte(&fontHEX[asciiCode + 62][row]);
  }
  else if (asciiCode == 113 && subasciiCode == 0xD1)  // ё
  {
    return pgm_read_byte(&fontHEX[163][row]); 
  }
  else if (asciiCode == 100 && subasciiCode == 0xD0)  // Є
  {
    return pgm_read_byte(&fontHEX[160][row]);
  }
  else if (asciiCode == 116 && subasciiCode == 0xD1)  // є
  {
    return pgm_read_byte(&fontHEX[164][row]);
  }
  else if (asciiCode == 102 && subasciiCode == 0xD0)  // І
  {
    return pgm_read_byte(&fontHEX[161][row]);
  }
  else if (asciiCode == 118 && subasciiCode == 0xD1)  // і
  {
    return pgm_read_byte(&fontHEX[165][row]);
  }
  else if (asciiCode == 103 && subasciiCode == 0xD0)  // Ї
  {
    return pgm_read_byte(&fontHEX[162][row]);
  }
  else if (asciiCode == 119 && subasciiCode == 0xD1)  // ї
  {
    return pgm_read_byte(&fontHEX[166][row]);
  }
  else if (asciiCode == 117 && subasciiCode == 0xD2)  // Г
  {
    return pgm_read_byte(&fontHEX[167][row]);
  }
  else if (asciiCode == 118 && subasciiCode == 0xD2)  // г
  {
    return pgm_read_byte(&fontHEX[168][row]);
  }

  return 0;
}
