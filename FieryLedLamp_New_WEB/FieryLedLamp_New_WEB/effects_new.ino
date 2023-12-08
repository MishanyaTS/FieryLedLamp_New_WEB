//---------------------------------------
// My global variables
uint8_t custom_eff = 0;

static const uint8_t exp_gamma[256] PROGMEM = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
    1,   2,   2,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,
    4,   4,   4,   4,   4,   5,   5,   5,   5,   5,   6,   6,   6,   7,   7,
    7,   7,   8,   8,   8,   9,   9,   9,   10,  10,  10,  11,  11,  12,  12,
    12,  13,  13,  14,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,
    19,  20,  20,  21,  21,  22,  23,  23,  24,  24,  25,  26,  26,  27,  28,
    28,  29,  30,  30,  31,  32,  32,  33,  34,  35,  35,  36,  37,  38,  39,
    39,  40,  41,  42,  43,  44,  44,  45,  46,  47,  48,  49,  50,  51,  52,
    53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,
    68,  70,  71,  72,  73,  74,  75,  77,  78,  79,  80,  82,  83,  84,  85,
    87,  89,  91,  92,  93,  95,  96,  98,  99,  100, 101, 102, 105, 106, 108,
    109, 111, 112, 114, 115, 117, 118, 120, 121, 123, 125, 126, 128, 130, 131,
    133, 135, 136, 138, 140, 142, 143, 145, 147, 149, 151, 152, 154, 156, 158,
    160, 162, 164, 165, 167, 169, 171, 173, 175, 177, 179, 181, 183, 185, 187,
    190, 192, 194, 196, 198, 200, 202, 204, 207, 209, 211, 213, 216, 218, 220,
    222, 225, 227, 229, 232, 234, 236, 239, 241, 244, 246, 249, 251, 253, 254,
    255
};


//---------------------------------------
// --------------------------------------
/* выделяем в памяти масив для загружаемых бинарных изображений
  header= 16 | заголовок
  w=16, h=16 | ширина высота
  color  = 2 | байт на цвет
  frames = 5 | количество кадров
  масив на 5 кадров 16x16 | размером w * h * frames * color + header = 2 576
  размер можно увеличивать по мере надобности, постоянно занимает место в памяти
  возможно в будущем будет сделано динамическим */
//byte binImage[2576];

/* binImage буффер для бинарных img размер выбран по размеру подгружаемых картинок */
byte binImage[2336];
// ======================================
// espModeStat default lamp start effect
// ======================================
void  espModeState(uint8_t color) {
  if (loadingFlag) {
    loadingFlag = false;
    step = deltaValue;
    deltaValue = 1;
    hue2 = 0;
    deltaHue2 = 1;
    DrawLine(CENTER_X_MINOR, CENTER_Y_MINOR, CENTER_X_MAJOR + 1, CENTER_Y_MINOR, CHSV(color, 255, 210));
    DrawLine(CENTER_X_MINOR, CENTER_Y_MINOR - 1, CENTER_X_MAJOR + 1, CENTER_Y_MINOR - 1, CHSV(color, 255, 210));
    // setModeSettings(128U, 128U);
    pcnt = 1;
    FastLED.clear();
  }
  if (pcnt > 0 & pcnt < 200) {
    if (pcnt != 0) {
      pcnt++;
    }

    // animation esp state ===========
    dimAll(108);
    //    if (step % 2 == 0) {
    uint8_t w = validMinMax(hue2, 0, floor(WIDTH / 2) - 1);
    uint8_t posY = validMinMax(CENTER_Y_MINOR + deltaHue2, 0, HEIGHT - 1);
    DrawLine(CENTER_X_MINOR - w, posY, CENTER_X_MAJOR + w, posY, CHSV(color, 255, (210 - deltaHue2)));
    posY = validMinMax(CENTER_Y_MINOR - 1 - deltaHue2, 1, HEIGHT - 1);
    DrawLine(CENTER_X_MINOR - w, posY, CENTER_X_MAJOR + w, posY, CHSV(color, 255, (210 - deltaHue2)));

    if (deltaHue2 == 0) {
      deltaHue2 = 1;
    }
    hue2++;
    deltaHue2 = deltaHue2 << 1;
    if (deltaHue2 == 2) {
      deltaHue2 = deltaHue2 << 1;
    }
    if (CENTER_Y_MINOR + deltaHue2 > HEIGHT) {
      deltaHue2 = 0;
      hue2 = 0;
    }
    // LOG.printf_P(PSTR("espModeState | pcnt = %05d | deltaHue2 = %03d | step %03d | ONflag • %s\n"), pcnt, deltaHue2, step, (ONflag ? "TRUE" : "FALSE"));
  } else {

#ifdef USE_NTP
    // error ntp ------------------
    color = 255;        // если при включенном NTP время не получено, будем красным цветом мигать
#else
    color = 176U;       // иначе скромно синим - нормальная ситуация при отсутствии NTP
#endif //USE_NTP
    // animtion no time -----------
    leds[XY(CENTER_X_MINOR , 0U)] = CHSV( color, 255, (step % 4 == 0) ? 200 : 128);

  }
  // clear led lamp ---------------
  if ( pcnt >= 100) {
    pcnt = 0;
    //    FastLED.clear();
    //    FastLED.delay(2);
    FastLED.clear();
    delay(2);
    FastLED.show();
    loadingFlag = false;
  }
  step++;
}

//---------------------------------------
// Global Function
//---------------------------------------
void drawRec(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint32_t color) {
  for (uint8_t y = startY; y < endY; y++) {
    for (uint8_t x = startX; x < endX; x++) {
      drawPixelXY(x, y, color);
    }
  }
}

//---------------------------------------
void drawRecCHSV(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, CHSV color) {
  for (uint8_t y = startY; y < endY; y++) {
    for (uint8_t x = startX; x < endX; x++) {
      drawPixelXY(x, y, color);
    }
  }
}

//--------------------------------------
uint8_t validMinMax(float val, uint8_t minV, uint32_t maxV) {
  uint8_t result;
  if (val <= minV) {
    result = minV;
  } else if (val >= maxV) {
    result = maxV;
  } else {
    result = ceil(val);
  }
  //  LOG.printf_P(PSTR( "result: %f | val: %f \n\r"), result, val);
  return result;
}

// альтернативный градиент для ламп собраных из лент с вертикальной компоновкой
// gradientHorizontal | gradientVertical менее производительный но работает на всех видах ламп
//--------------------------------------
void gradientHorizontal(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint8_t start_color, uint8_t end_color, uint8_t start_br, uint8_t end_br, uint8_t saturate) {
  float step_color = 0;
  float step_br = 0;
  if (startX == endX) {
    endX++;
  }
  if (startY == endY) {
    endY++;
  }
  step_color = (end_color - start_color) / abs(startX - endX);
  if (start_color >  end_color) {
    step_color -= 1.2;
  } else {
    step_color += 1.2;
  }

  step_br = (end_br - start_br) / abs(startX - endX);
  if (start_br >  end_color) {
    step_br -= 1.2;
  } else {
    step_br += 1.2;
  }

  // LOG.printf_P(PSTR( "\n step_color: %f | step_br: %f \n\n\r"), step_color, step_br);
  for (uint8_t x = startX; x < endX; x++) {
    for (uint8_t y = startY; y < endY; y++) {
      CHSV thisColor = CHSV((uint8_t) validMinMax((start_color + (x - startX) * step_color), 1, 254), saturate,
                            (uint8_t) validMinMax((start_br + (x - startX) * step_br), 0, 255) );
      drawPixelXY(x, y, thisColor);
    }
  }
}

//--------------------------------------
void gradientVertical(uint8_t startX, uint8_t startY, uint8_t endX, uint8_t endY, uint8_t start_color, uint8_t end_color, uint8_t start_br, uint8_t end_br, uint8_t saturate) {
  float step_color = 0;
  float step_br = 0;
  if (startX == endX) {
    endX++;
  }
  if (startY == endY) {
    endY++;
  }
  step_color = (end_color - start_color) / abs(startY - endY);

  if (start_color >  end_color) {
    step_color -= 1.2;
  } else {
    step_color += 1.2;
  }

  step_br = (end_br - start_br) / abs(startY - endY);
  if (start_br >  end_color) {
    step_br -= 1.2;
  } else {
    step_br += 1.2;
  }
  for (uint8_t y = startY; y < endY; y++) {
    CHSV thisColor = CHSV( (uint8_t) validMinMax((start_color + (y - startY) * step_color), 0, 255), saturate,
                           (uint8_t) validMinMax((start_br + (y - startY) * step_br), 0, 255) );
    for (uint8_t x = startX; x < endX; x++) {
      drawPixelXY(x, y, thisColor);
    }
  }
}

//---------------------------------------
// gradientDownTop • более плавный градиент в отличие от gradientVertical
// но может некоректно работать на лампах собранных на ленточных светодиодах
//---------------------------------------
void gradientDownTop( uint8_t bottom, CHSV bottom_color, uint8_t top, CHSV top_color ) {
  //  FORWARD_HUES: hue always goes clockwise
  //  BACKWARD_HUES: hue always goes counter-clockwise
  //  SHORTEST_HUES: hue goes whichever way is shortest
  //  LONGEST_HUES: hue goes whichever way is longest
  if (STRIP_DIRECTION < 2) {
    // STRIP_DIRECTION to UP ========
    fill_gradient(leds, top * WIDTH, top_color, bottom * WIDTH, bottom_color, SHORTEST_HUES);
  } else {
    // STRIP_DIRECTION to DOWN ======
    fill_gradient(leds, NUM_LEDS - bottom * WIDTH - 1, bottom_color, NUM_LEDS - top * WIDTH, top_color, SHORTEST_HUES);
  }
}

// ======================================
// функции для работы с бинарными файлами
// ======================================

// --------------------------------------
// функция чтения бинарного файла изображения
//    из файловой системы лампы 
void readBinFile(String fileName, size_t len ) {

  File binFile = SPIFFS.open("/" + fileName, "r");
  if (!binFile) {
    LOG.println("File not found");
    return;
  }
  size_t size = binFile.size();
  if (size > len) {
    binFile.close();
    LOG.println("Large File");
    return;
  }

  byte buffer[size];
  uint16_t amount;

  if (binFile == NULL) exit (1);
  binFile.seek(0);

  while (binFile.available()) {
    amount = binFile.read(buffer, size);
  }

#ifdef GENERAL_DEBUG
  LOG.printf_P(PSTR("File size • %08d bytes\n"), amount);
#endif

  // binImage = malloc(amount);
  // byte *by = malloc(1024);
  // memset(binImage, 66, 1552);
  // byte *by = new byte[size];
  memcpy(binImage, buffer, amount);
  binFile.close();
}

// --------------------------------------
// функция получения размера изображения
//   из заголовка файла

uint16_t getSizeValue(byte* buffer, byte b ) {
  return  (buffer[b + 1] << 8) + buffer[b];
}

// --------------------------------------
// функция скрола изображения по оси X 
void scrollImage(uint16_t imgW, uint16_t imgH, uint16_t start_row) {
  const byte HEADER = 16;
  const uint16_t BYTES_PER_PIXEL = 2U;
  // const uint16_t imgSize = imgW * imgH * BYTES_PER_PIXEL + HEADER;
  uint8_t r, g, b;
  uint8_t padding = floor((HEIGHT - imgH) / 2);
  uint8_t topPos = HEIGHT - padding - 1;
  uint16_t pixIndex;
  uint8_t delta = 0;

  for (uint16_t x = 0; x < WIDTH; x++) {
    for (uint16_t y = 0; y < (imgH - 1); y++) {
      if ((start_row + x) > WIDTH) {
        delta = 1;
      }
      pixIndex = HEADER + (start_row + x + y * imgW) * BYTES_PER_PIXEL;

      // convert rgb565 to rgb888 -----------
      // masc rgb565  0xF800 | 0x07E0 | 0x001F 
      r = (binImage[pixIndex + 1] & 0xF8);
      g = ((binImage[pixIndex + 1] & 0x07) << 5) + ((binImage[pixIndex] & 0xE0) << 5);
      b = (binImage[pixIndex] & 0x1F) << 3;
      // вариант с изменением яркости ----
        //hue = abs(16 - start_row) * 4;
        //leds[XY(x, topPos - y - delta)] = CRGB(constrain(r - hue, 0, 255), constrain(g - hue, 0, 255), constrain(b - hue, 0, 255));
      // ------------------------------------
      
      leds[XY(x, topPos - y - delta)] = CRGB(r, g, b);
      // drawPixelXY(x, topPos - y - delta, CRGB(r, g, b));
      // draw background 
      if ((start_row == 0) && (y == 0) && (padding > 0)) {
        drawRec(0, HEIGHT - padding, WIDTH, HEIGHT, getPixColorXY(0, topPos));
        drawRec(0, 0, WIDTH, padding, getPixColorXY(0, topPos));
      }
    } // end for y
  }
}


// ======================================
// New Effects
// ======================================
uint32_t colorDimm(uint32_t colorValue, long lenght, long pixel) {

  uint8_t red = (colorValue & 0x00FF0000) >> 16;
  uint8_t green = (colorValue & 0x0000FF00) >> 8;
  uint8_t blue = (colorValue & 0x000000FF);

  double prozent = 100 / lenght;

  red = red - red * ((prozent * pixel) / 100);
  green = green - green * ((prozent * pixel) / 100);
  blue = blue - blue * ((prozent * pixel) / 100);

  // colorValue = strip.Color(red,green,blue);
  colorValue = red;
  colorValue = (colorValue << 8) + green;
  colorValue = (colorValue << 8) + blue;
  return colorValue;
}

// =============== Wine ================
//    © SlingMaster | by Alex Dovby
//               EFF_WINE
//--------------------------------------

void colorsWine() {
  uint8_t divider;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(20U + random8(200U), 200U);
    }
#endif
    loadingFlag = false;
    fillAll(CHSV(55U, 255U, 65U));
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    // minspeed 230 maxspeed 250 ============
    // minscale  40 maxscale  75 ============
    // красное вино hue > 0 & <=10
    // розовое вино hue > 10 & <=20
    // белое вино   hue > 20U & <= 40
    // шампанское   hue > 40U & <= 60

    deltaHue2 = 0U;                         // count для замедления смены цвета
    step = deltaValue;                      // чтообы при старте эффекта сразу покрасить лампу
    deltaHue = 1U;                          // direction | 0 hue-- | 1 hue++ |
    hue = 55U;                              // Start Color
    hue2 = 65U;                             // Brightness
    pcnt = 0;
  }

  deltaHue2++;
  // маштаб задает скорость изменения цвета 5 уровней
  divider = 5 - floor((modes[currentMode].Scale - 1) / 20);

  // возвращаем яркость для перехода к белому
  if (hue >= 10 && hue2 < 100U) {
    hue2++;
  }
  // уменьшаем яркость для красного вина
  if (hue < 10 && hue2 > 40U) {
    hue2--;
  }

  // изменение цвета вина -----
  if (deltaHue == 1U) {
    if (deltaHue2 % divider == 0) {
      hue++;
    }
  } else {
    if (deltaHue2 % divider == 0) {
      hue--;
    }
  }
  // --------

  // LOG.printf_P(PSTR("Wine | hue = %03d | Dir = %d | Brightness %03d | deltaHue2 %03d | divider %d | %d\n"), hue, deltaHue, hue2, deltaHue2, divider, step);

  // сдвигаем всё вверх -----------
  for (uint8_t x = 0U; x < WIDTH; x++) {
    for (uint8_t y = HEIGHT; y > 0U; y--) {
      drawPixelXY(x, y, getPixColorXY(x, y - 1U));
    }
  }

  if (hue > 40U) {
    // добавляем перляж для шампанского
    pcnt = random(0, WIDTH);
  } else {
    pcnt = 0;
  }

  // заполняем нижнюю строку с учетом перляжа
  for (uint8_t x = 0U; x < WIDTH; x++) {
    if ((x == pcnt) && (pcnt > 0)) {
      // с перляжем ------
      drawPixelXY(x, 0U, CHSV(hue, 150U, hue2 + 20U + random(0, 50U)));
    } else {
      drawPixelXY(x, 0U, CHSV(hue, 255U, hue2));
    }
  }

  // меняем направление изменения цвета вина от красного к шампанскому и обратно
  // в диапазоне шкалы HUE |0-60|
  if  (hue == 0U) {
    deltaHue = 1U;
  }
  if (hue == 60U) {
    deltaHue = 0U;
  }
  step++;
}

// ============== Swirl ================
//    © SlingMaster | by Alex Dovby
//              EFF_SWIRL
//--------------------------------------
void Swirl() {
    
  if (modes[currentMode].Scale > 50) Spindle(); // Якщо масштаб/колір більше 50 - тоді єфект "Веретено"
  else {
    
  uint8_t divider;
  uint8_t lastHue;
  static const uint32_t colors[5][6] PROGMEM = {
    {CRGB::Blue, CRGB::DarkRed, CRGB::Aqua, CRGB::Magenta, CRGB::Gold, CRGB::Green },
    {CRGB::Yellow, CRGB::LemonChiffon, CRGB::LightYellow, CRGB::Gold, CRGB::Chocolate, CRGB::Goldenrod},
    {CRGB::Green, CRGB::DarkGreen, CRGB::LawnGreen, CRGB::SpringGreen, CRGB::Cyan, CRGB::Black },
    {CRGB::Blue, CRGB::DarkBlue, CRGB::MidnightBlue, CRGB::MediumSeaGreen, CRGB::MediumBlue, CRGB:: DeepSkyBlue },
    {CRGB::Magenta, CRGB::Red, CRGB::DarkMagenta, CRGB::IndianRed, CRGB::Gold, CRGB::MediumVioletRed }
  };
  uint32_t color;

  if (loadingFlag) {

#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(50U + random8(190U), 250U);
    }
#endif
    loadingFlag = false;
    FastLED.clear();
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = deltaValue;                      // чтообы при старте эффекта сразу покрасить лампу
    deltaHue2 = 0U;                         // count для замедления смены цвета
    deltaHue = 0U;                          // direction | 0 hue-- | 1 hue++ |
    hue2 = 0U;                              // x
  }

  if (step >= deltaValue) {
    step = 0U;
  }
  divider = floor((modes[currentMode].Scale - 1) / 20); // маштаб задает смену палитры
  //  if (deltaValue > 50U && deltaHue2 == 0U) {
  //    hue = random8(6);                       // если низкая скорость меняем цвет после каждого витка
  //  }
  // задаем цвет и рисуем завиток --------
  color = colors[divider][hue];
  // drawPixelXY((hue2 + 1), (deltaHue2 - 1), 0x000000); // aded shadow
  drawPixelXY(hue2, deltaHue2, color);
  // LOG.printf_P(PSTR("Swirl | hue = %03d | x= %03d | y= %03d | divider %d | %d\n"), hue, hue2, deltaHue2, divider, step);
  // -------------------------------------

  hue2++;                     // x
  // два варианта custom_eff задается в сетапе лампы ----
  if (custom_eff == 1) {
    // blurScreen(beatsin8(5U, 20U, 5U));
    deltaHue2++;              // y
  } else {
    // blurScreen(10U);
    if (hue2 % 2 == 0) {
      deltaHue2++;            // y
    }
  }
  // -------------------------------------

  if  (hue2 > WIDTH) {
    hue2 = 0U;
  }

  if (deltaHue2 >= HEIGHT) {
    deltaHue2 = 0U;
    // new swirl ------------
    hue2 = random8(WIDTH - 2);
    // hue2 = hue2 + 2;
    // select new color -----
    hue = random8(6);

    if (lastHue == hue) {
      hue = hue + 1;
      if (hue >= 6) {
        hue = 0;
      }
    }
    lastHue = hue;
  }
  // blurScreen(beatsin8(5U, 20U, 5U));
  blurScreen(4U + random8(8));
  step++;
  } // else у пачатку функції
}

// -------------------------------------------
// for effect Ukraine
// -------------------------------------------
void drawCrest() {
  static const uint32_t data[9][5] PROGMEM = {
    {0x000000, 0x000000, 0xFFD700, 0x000000, 0x000000 },
    {0xFFD700, 0x000000, 0xFFD700, 0x000000, 0xFFD700 },
    {0xFFD700, 0x000000, 0xFFD700, 0x000000, 0xFFD700 },
    {0xFFD700, 0x000000, 0xFFD700, 0x000000, 0xFFD700 },
    {0xFFD700, 0x000000, 0xFFD700, 0x000000, 0xFFD700 },
    {0xFFD700, 0xFFD700, 0xFFD700, 0xFFD700, 0xFFD700 },
    {0xFFD700, 0x000000, 0xFFD700, 0x000000, 0xFFD700 },
    {0x000000, 0xFFD700, 0xFFD700, 0xFFD700, 0x000000 },
    {0x000000, 0x000000, 0xFFD700, 0x000000, 0x000000 }
  };

  uint8_t posX = CENTER_X_MAJOR - 3;
  uint8_t posY = 9;
  uint32_t color;
  if (HEIGHT > 16) {
    posY = CENTER_Y_MINOR - 1;
  }
  FastLED.clear();
  for (uint8_t y = 0U; y < 9; y++) {
    for (uint8_t x = 0U; x < 5; x++) {
      color = data[y][x];
      drawPixelXY(posX + x, posY - y, color);
    }
  }
}

// ************************** СТРЕЛКИ *************************
int8_t arrow_x[4], arrow_y[4], stop_x[4], stop_y[4];
uint8_t arrow_direction; // 0x01 - слева направо; 0x02 - снизу вверх; 0х04 - справа налево; 0х08 - сверху вниз
uint8_t arrow_mode, arrow_mode_orig;// 0 - по очереди все варианты
// 1 - по очереди от края до края экрана;
// 2 - одновременно по горизонтали навстречу к ентру, затем одновременно по вертикали навстречу к центру
// 3 - одновременно все к центру
// 4 - по два (горизонталь / вертикаль) все от своего края к противоположному, стрелки смещены от центра на 1/3
// 5 - одновременно все от своего края к противоположному, стрелки смещены от центра на 1/3
bool arrow_complete, arrow_change_mode;
uint8_t arrow_hue[4];
uint8_t arrow_play_mode_count[6]; // Сколько раз проигрывать полностью каждый режим если вариант 0 - текущий счетчик
uint8_t arrow_play_mode_count_orig[6]; // Сколько раз проигрывать полностью каждый режим если вариант 0 - исходные настройки

void arrowsRoutine() {
if (loadingFlag) {
loadingFlag = false;
//modeCode = MC_ARROWS;
FastLED.clear();
arrow_complete = false;
// arrow_mode_orig = (specialTextEffectParam >= 0) ? specialTextEffectParam : getEffectScaleParamValue2(MC_ARROWS);

arrow_mode = (arrow_mode_orig == 0 || arrow_mode_orig > 5) ? random8(1,5) : arrow_mode_orig;
arrow_play_mode_count_orig[0] = 0;
arrow_play_mode_count_orig[1] = 4; // 4 фазы - все стрелки показаны по кругу один раз - переходить к следующему ->
arrow_play_mode_count_orig[2] = 4; // 2 фазы - гориз к центру (1), затем верт к центру (2) - обе фазы повторить по 2 раза -> 4
arrow_play_mode_count_orig[3] = 4; // 1 фаза - все к центру (1) повторить по 4 раза -> 4
arrow_play_mode_count_orig[4] = 4; // 2 фазы - гориз к центру (1), затем верт к центру (2) - обе фазы повторить по 2 раза -> 4
arrow_play_mode_count_orig[5] = 4; // 1 фаза - все сразу (1) повторить по 4 раза -> 4
for (uint8_t i=0; i<6; i++) {
arrow_play_mode_count[i] = arrow_play_mode_count_orig[i];
}
arrowSetupForMode(arrow_mode, true);
}

//uint8_t effectBrightness;
//effectBrightness = modes[currentMode].Brightness;

// fader(65);
dimAll(160);
CHSV color;

// движение стрелки - cлева направо
if ((arrow_direction & 0x01) > 0) {
color = CHSV(arrow_hue[0], 255, modes[currentMode].Brightness);
for (int8_t x = 0; x <= 4; x++) {
for (int8_t y = 0; y <= x; y++) {
if (arrow_x[0] - x >= 0 && arrow_x[0] - x <= stop_x[0]) {
CHSV clr = (x < 4 || (x == 4 && y < 2)) ? color : CHSV(0,0,0);
drawPixelXY(arrow_x[0] - x, arrow_y[0] - y, clr);
drawPixelXY(arrow_x[0] - x, arrow_y[0] + y, clr);
}
}
}
arrow_x[0]++;
}

// движение стрелки - cнизу вверх
if ((arrow_direction & 0x02) > 0) {
color = CHSV(arrow_hue[1], 255, modes[currentMode].Brightness);
for (int8_t y = 0; y <= 4; y++) {
for (int8_t x = 0; x <= y; x++) {
if (arrow_y[1] - y >= 0 && arrow_y[1] - y <= stop_y[1]) {
CHSV clr = (y < 4 || (y == 4 && x < 2)) ? color : CHSV(0,0,0);
drawPixelXY(arrow_x[1] - x, arrow_y[1] - y, clr);
drawPixelXY(arrow_x[1] + x, arrow_y[1] - y, clr);
}
}
}
arrow_y[1]++;
}

// движение стрелки - cправа налево
if ((arrow_direction & 0x04) > 0) {
color = CHSV(arrow_hue[2], 255, modes[currentMode].Brightness);
for (int8_t x = 0; x <= 4; x++) {
for (int8_t y = 0; y <= x; y++) {
if (arrow_x[2] + x >= stop_x[2] && arrow_x[2] + x < WIDTH) {
CHSV clr = (x < 4 || (x == 4 && y < 2)) ? color : CHSV(0,0,0);
drawPixelXY(arrow_x[2] + x, arrow_y[2] - y, clr);
drawPixelXY(arrow_x[2] + x, arrow_y[2] + y, clr);
}
}
}
arrow_x[2]--;
}

// движение стрелки - cверху вниз
if ((arrow_direction & 0x08) > 0) {
color = CHSV(arrow_hue[3], 255, modes[currentMode].Brightness);
for (int8_t y = 0; y <= 4; y++) {
for (int8_t x = 0; x <= y; x++) {
if (arrow_y[3] + y >= stop_y[3] && arrow_y[3] + y < HEIGHT) {
CHSV clr = (y < 4 || (y == 4 && x < 2)) ? color : CHSV(0,0,0);
drawPixelXY(arrow_x[3] - x, arrow_y[3] + y, clr);
drawPixelXY(arrow_x[3] + x, arrow_y[3] + y, clr);
}
}
}
arrow_y[3]--;
}

// Проверка завершения движения стрелки, переход к следующей фазе или режиму

switch (arrow_mode) {

case 1:
// Последовательно - слева-направо -> снизу вверх -> справа налево -> сверху вниз и далее по циклу
// В каждый сомент времени сктивна только одна стрелка, если она дошла до края - переключиться на следующую и задать ее начальные координаты
arrow_complete = false;
switch (arrow_direction) {
case 1: arrow_complete = arrow_x[0] > stop_x[0]; break;
case 2: arrow_complete = arrow_y[1] > stop_y[1]; break;
case 4: arrow_complete = arrow_x[2] < stop_x[2]; break;
case 8: arrow_complete = arrow_y[3] < stop_y[3]; break;
}

arrow_change_mode = false;
if (arrow_complete) {
arrow_direction = (arrow_direction << 1) & 0x0F;
if (arrow_direction == 0) arrow_direction = 1;
if (arrow_mode_orig == 0) {
arrow_play_mode_count[1]--;
if (arrow_play_mode_count[1] == 0) {
arrow_play_mode_count[1] = arrow_play_mode_count_orig[1];
arrow_mode = random8(1,5);
arrow_change_mode = true;
}
}

arrowSetupForMode(arrow_mode, arrow_change_mode);
}
break;

case 2:
// Одновременно горизонтальные навстречу до половины экрана
// Затем одновременно вертикальные до половины экрана. Далее - повторять
arrow_complete = false;
switch (arrow_direction) {
case 5: arrow_complete = arrow_x[0] > stop_x[0]; break; // Стрелка слева и справа встречаются в центре одновременно - проверять только стрелку слева
case 10: arrow_complete = arrow_y[1] > stop_y[1]; break; // Стрелка снизу и сверху встречаются в центре одновременно - проверять только стрелку снизу
}

arrow_change_mode = false;
if (arrow_complete) {
arrow_direction = arrow_direction == 5 ? 10 : 5;
if (arrow_mode_orig == 0) {
arrow_play_mode_count[2]--;
if (arrow_play_mode_count[2] == 0) {
arrow_play_mode_count[2] = arrow_play_mode_count_orig[2];
arrow_mode = random8(1,5);
arrow_change_mode = true;
}
}

arrowSetupForMode(arrow_mode, arrow_change_mode);
}
break;

case 3:
// Одновременно со всех сторон к центру
// Завершение кадра режима - когда все стрелки собрались в центре.
// Проверять стрелки по самой длинной стороне
if (WIDTH >= HEIGHT)
arrow_complete = arrow_x[0] > stop_x[0];
else
arrow_complete = arrow_y[1] > stop_y[1];

arrow_change_mode = false;
if (arrow_complete) {
if (arrow_mode_orig == 0) {
arrow_play_mode_count[3]--;
if (arrow_play_mode_count[3] == 0) {
arrow_play_mode_count[3] = arrow_play_mode_count_orig[3];
arrow_mode = random8(1,5);
arrow_change_mode = true;
}
}

arrowSetupForMode(arrow_mode, arrow_change_mode);
}
break;

case 4:
// Одновременно слева/справа от края до края со смещением горизонтальной оси на 1/3 высоты, далее
// одновременно снизу/сверху от края до края со смещением вертикальной оси на 1/3 ширины
// Завершение кадра режима - когда все стрелки собрались в центре.
// Проверять стрелки по самой длинной стороне
switch (arrow_direction) {
case 5: arrow_complete = arrow_x[0] > stop_x[0]; break; // Стрелка слева и справа движутся и достигают края одновременно - проверять только стрелку слева
case 10: arrow_complete = arrow_y[1] > stop_y[1]; break; // Стрелка снизу и сверху движутся и достигают края одновременно - проверять только стрелку снизу
}

arrow_change_mode = false;
if (arrow_complete) {
arrow_direction = arrow_direction == 5 ? 10 : 5;
if (arrow_mode_orig == 0) {
arrow_play_mode_count[4]--;
if (arrow_play_mode_count[4] == 0) {
arrow_play_mode_count[4] = arrow_play_mode_count_orig[4];
arrow_mode = random8(1,5);
arrow_change_mode = true;
}
}

arrowSetupForMode(arrow_mode, arrow_change_mode);
}
break;

case 5:
// Одновременно со всех сторон от края до края со смещением горизонтальной оси на 1/3 высоты, далее
// Проверять стрелки по самой длинной стороне
if (WIDTH >= HEIGHT)
arrow_complete = arrow_x[0] > stop_x[0];
else
arrow_complete = arrow_y[1] > stop_y[1];

arrow_change_mode = false;
if (arrow_complete) {
if (arrow_mode_orig == 0) {
arrow_play_mode_count[5]--;
if (arrow_play_mode_count[5] == 0) {
arrow_play_mode_count[5] = arrow_play_mode_count_orig[5];
arrow_mode = random8(1,5);
arrow_change_mode = true;
}
}

arrowSetupForMode(arrow_mode, arrow_change_mode);
}
break;
}

}

void arrowSetupForMode(uint8_t mode, bool change) {
switch (mode) {
case 1:
if (change) arrow_direction = 1;
arrowSetup_mode1(); // От края матрицы к краю, по центру гориз и верт
break;
case 2:
if (change) arrow_direction = 5;
arrowSetup_mode2(); // По центру матрицы (гориз / верт) - ограничение - центр матрицы
break;
case 3:
if (change) arrow_direction = 15;
arrowSetup_mode2(); // как и в режиме 2 - по центру матрицы (гориз / верт) - ограничение - центр матрицы
break;
case 4:
if (change) arrow_direction = 5;
arrowSetup_mode4(); // От края матрицы к краю, верт / гориз
break;
case 5:
if (change) arrow_direction = 15;
arrowSetup_mode4(); // как и в режиме 4 от края матрицы к краю, на 1/3
break;
}
}
void arrowSetup_mode1() {
// Слева направо
if ((arrow_direction & 0x01) > 0) {
arrow_hue[0] = random8();
arrow_x[0] = 0;
arrow_y[0] = HEIGHT / 2;
stop_x [0] = WIDTH + 7; // скрывается за экраном на 7 пикселей
stop_y [0] = 0; // неприменимо
}
// снизу вверх
if ((arrow_direction & 0x02) > 0) {
arrow_hue[1] = random8();
arrow_y[1] = 0;
arrow_x[1] = WIDTH / 2;
stop_y [1] = HEIGHT + 7; // скрывается за экраном на 7 пикселей
stop_x [1] = 0; // неприменимо
}
// справа налево
if ((arrow_direction & 0x04) > 0) {
arrow_hue[2] = random8();
arrow_x[2] = WIDTH - 1;
arrow_y[2] = HEIGHT / 2;
stop_x [2] = -7; // скрывается за экраном на 7 пикселей
stop_y [2] = 0; // неприменимо
}
// сверху вниз
if ((arrow_direction & 0x08) > 0) {
arrow_hue[3] = random8();
arrow_y[3] = HEIGHT - 1;
arrow_x[3] = WIDTH / 2;
stop_y [3] = -7; // скрывается за экраном на 7 пикселей
stop_x [3] = 0; // неприменимо
}
}

void arrowSetup_mode2() {
// Слева направо до половины экрана
if ((arrow_direction & 0x01) > 0) {
arrow_hue[0] = random8();
arrow_x[0] = 0;
arrow_y[0] = HEIGHT / 2;
stop_x [0] = WIDTH / 2 - 1; // до центра экрана
stop_y [0] = 0; // неприменимо
}
// снизу вверх до половины экрана
if ((arrow_direction & 0x02) > 0) {
arrow_hue[1] = random8();
arrow_y[1] = 0;
arrow_x[1] = WIDTH / 2;
stop_y [1] = HEIGHT / 2 - 1; // до центра экрана
stop_x [1] = 0; // неприменимо
}
// справа налево до половины экрана
if ((arrow_direction & 0x04) > 0) {
arrow_hue[2] = random8();
arrow_x[2] = WIDTH - 1;
arrow_y[2] = HEIGHT / 2;
stop_x [2] = WIDTH / 2; // до центра экрана
stop_y [2] = 0; // неприменимо
}
// сверху вниз до половины экрана
if ((arrow_direction & 0x08) > 0) {
arrow_hue[3] = random8();
arrow_y[3] = HEIGHT - 1;
arrow_x[3] = WIDTH / 2;
stop_y [3] = HEIGHT / 2; // до центра экрана
stop_x [3] = 0; // неприменимо
}
}

void arrowSetup_mode4() {
// Слева направо
if ((arrow_direction & 0x01) > 0) {
arrow_hue[0] = random8();
arrow_x[0] = 0;
arrow_y[0] = (HEIGHT / 3) * 2;
stop_x [0] = WIDTH + 7; // скрывается за экраном на 7 пикселей
stop_y [0] = 0; // неприменимо
}
// снизу вверх
if ((arrow_direction & 0x02) > 0) {
arrow_hue[1] = random8();
arrow_y[1] = 0;
arrow_x[1] = (WIDTH / 3) * 2;
stop_y [1] = HEIGHT + 7; // скрывается за экраном на 7 пикселей
stop_x [1] = 0; // неприменимо
}
// справа налево
if ((arrow_direction & 0x04) > 0) {
arrow_hue[2] = random8();
arrow_x[2] = WIDTH - 1;
arrow_y[2] = HEIGHT / 3;
stop_x [2] = -7; // скрывается за экраном на 7 пикселей
stop_y [2] = 0; // неприменимо
}
// сверху вниз
if ((arrow_direction & 0x08) > 0) {
arrow_hue[3] = random8();
arrow_y[3] = HEIGHT - 1;
arrow_x[3] = WIDTH / 3;
stop_y [3] = -7; // скрывается за экраном на 7 пикселей
stop_x [3] = 0; // неприменимо
}
}

// ============ Oil Paints ==============
//      © SlingMaster | by Alex Dovby
//              EFF_PAINT
//           Масляные Краски
//---------------------------------------
void OilPaints() {

  uint8_t divider;
  uint8_t entry_point;
  uint16_t value;
  uint16_t max_val;
  if (loadingFlag) {

#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                          scale | speed 210
      setModeSettings(1U + random8(252U), 1 + random8(219U));
    }
#endif
    loadingFlag = false;
    FastLED.clear();
    // blurScreen(beatsin8(5U, 50U, 5U));
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = deltaValue;                    // чтообы при старте эффекта сразу покрасить лампу
    hue = floor(21.25 * (random8(11) + 1)); // next color
    deltaHue = hue - 22;                  // last color
    deltaHue2 = 80;                       // min bright
    max_val = pow(2, WIDTH);
    //    for ( int i = WIDTH; i < (NUM_LEDS - WIDTH); i++) {
    //      leds[i] = CHSV(120U, 24U, 64U);
    //    }
  }

  if (step >= deltaValue) {
    step = 0U;
    // LOG.printf_P(PSTR("%03d | log: %f | val: %03d\n\r"), modes[currentMode].Brightness, log(modes[currentMode].Brightness), deltaHue2);
  }

  // Create Oil Paints --------------
  // выбираем краски  ---------------
  if (step % CENTER_Y_MINOR == 0) {
    divider = floor((modes[currentMode].Scale - 1) / 10);             // маштаб задает диапазон изменения цвета
    deltaHue = hue;                                                   // set last color
    hue += 6 * divider;                                               // new color
    hue2 = 255;                                                       // restore brightness
    deltaHue2 = 80 - floor(log(modes[currentMode].Brightness) * 6);   // min bright
    entry_point = random8(WIDTH);                                     // start X position
    trackingObjectHue[entry_point] = hue;                             // set start position
    drawPixelXY(entry_point,  HEIGHT - 2, CHSV(hue, 255U, 255U));
    // !!! ********
    if (custom_eff == 1) {
      drawPixelXY(entry_point + 1,  HEIGHT - 3, CHSV(hue + 30, 255U, 255U));
    }
    // ************
    // LOG.printf_P(PSTR("BR %03d | SP %03d | SC %03d | hue %03d\n\r"), modes[currentMode].Brightness, modes[currentMode].Speed, modes[currentMode].Scale, hue);
  }

  // формируем форму краски, плавно расширяя струю ----
  if (random8(3) == 1) {
    // LOG.println("<--");
    for (uint8_t x = 1U; x < WIDTH; x++) {
      if (trackingObjectHue[x] == hue) {
        trackingObjectHue[x - 1] = hue;
        break;
      }
    }
  } else {
    // LOG.println("-->");
    for (uint8_t x = WIDTH - 1; x > 0U ; x--) {
      if (trackingObjectHue[x] == hue) {
        trackingObjectHue[x + 1] = hue;
        break;
      }
      // LOG.printf_P(PSTR("x = %02d | value = %03d | hue = %03d \n\r"), x, trackingObjectHue[x], hue);
    }
  }
  // LOG.println("------------------------------------");

  // выводим сформированную строку --------------------- максимально яркую в момент смены цвета
  for (uint8_t x = 0U; x < WIDTH; x++) {
    //                                                                                set color  next |    last  |
    drawPixelXY(x,  HEIGHT - 1, CHSV(trackingObjectHue[x], 255U, (trackingObjectHue[x] == hue) ? hue2 : deltaHue2));
  }
  //  LOG.println("");
  // уменьшаем яркость для следующих строк
  if ( hue2 > (deltaHue2 + 16)) {
    hue2 -= 16U;
  }
  // сдвигаем неравномерно поток вниз ---
  value = random16(max_val);
  //LOG.printf_P(PSTR("value = %06d | "), value);
  for (uint8_t x = 0U; x < WIDTH; x++) {
    if ( bitRead(value, x ) == 0) {
      //LOG.print (" X");
      for (uint8_t y = 0U; y < HEIGHT - 1; y++) {
        drawPixelXY(x, y, getPixColorXY(x, y + 1U));
      }
    }
  }
  // LOG.printf_P(PSTR("%02d | hue2 = %03d | min = %03d \n\r"), step, hue2, deltaHue2);
  // -------------------------------------

  step++;
}


// ========== Botswana Rivers ===========
//      © SlingMaster | by Alex Dovby
//              EFF_RIVERS
//            Реки Ботсваны

//---------------------------------------
void flora() {
  uint32_t  FLORA_COLOR = 0x2F1F00;
  uint8_t posX =  floor(CENTER_X_MINOR - WIDTH * 0.3);
  uint8_t h =  random8(HEIGHT - 6U) + 4U;
  DrawLine(posX + 1, 1U, posX + 1, h - 1, 0x000000);
  DrawLine(posX + 2, 1U, posX + 2, h, FLORA_COLOR );
  drawPixelXY(posX + 2, h - random8(floor(h * 0.5)), random8(2U) == 1 ? 0xFF00E0 :  random8(2U) == 1 ? 0xFFFF00 : 0x00FF00);
  drawPixelXY(posX + 1, h - random8(floor(h * 0.25)), random8(2U) == 1 ? 0xFF00E0 : 0xFFFF00);
  if (random8(2U) == 1) {
    drawPixelXY(posX + 1, floor(h * 0.5), random8(2U) == 1 ? 0xEF001F :  0x9FFF00);
  }
  h =  floor(h * 0.65);
  if (WIDTH > 8) {
    DrawLine(posX - 1, 1U, posX - 1, h - 1, 0x000000);
  }
  DrawLine(posX, 1U, posX, h, FLORA_COLOR);
  drawPixelXY(posX, h - random8(floor(h * 0.5)), random8(2U) == 1 ? 0xFF00E0 : 0xFFFF00);
}

//---------------------------------------
void animeBobbles() {
  // сдвигаем всё вверх ----
  for (uint8_t x = CENTER_X_MAJOR; x < WIDTH; x++) {
    for (uint8_t y = HEIGHT; y > 0U; y--) {
      if (getPixColorXY(x, y - 1) == 0xFFFFF7) {
        drawPixelXY(x, y, 0xFFFFF7);
        drawPixelXY(x, y - 1, getPixColorXY(0, y - 1));
      }
    }
  }
  // ----------------------
  if ( step % 4 == 0) {
    drawPixelXY(CENTER_X_MAJOR + random8(5), 0U, 0xFFFFF7);
    if ( step % 12 == 0) {
      drawPixelXY(CENTER_X_MAJOR + 2 + random8(3), 0U, 0xFFFFF7);
    }
  }
}

//---------------------------------------
void createScene(uint8_t idx) {
  switch (idx) {
    case 0:     // blue green ------
      gradientDownTop(floor((HEIGHT - 1) * 0.5), CHSV(96, 255, 100), HEIGHT, CHSV(160, 255, 255));
      gradientDownTop(0, CHSV(96, 255, 255), CENTER_Y_MINOR, CHSV(96, 255, 100));
      break;
    case 1:     // aquamarine green
      gradientDownTop(floor((HEIGHT - 1) * 0.3), CHSV(96, 255, 100), HEIGHT, CHSV(130, 255, 220));
      gradientDownTop(0, CHSV(96, 255, 255), floor(HEIGHT * 0.3), CHSV(96, 255, 100));
      break;
    case 2:     // blue aquamarine -
      gradientDownTop(floor((HEIGHT - 1) * 0.5), CHSV(170, 255, 100), HEIGHT, CHSV(160, 255, 200));
      gradientDownTop(0, CHSV(100, 255, 255), CENTER_Y_MINOR, CHSV(170, 255, 100));
      break;
    case 3:     // yellow green ----
      gradientDownTop(floor((HEIGHT - 1) * 0.5), CHSV(95, 255, 55), HEIGHT, CHSV(70, 255, 200));
      gradientDownTop(0, CHSV(95, 255, 255), CENTER_Y_MINOR, CHSV(100, 255, 55));
      break;
    case 4:     // sea green -------
      gradientDownTop(floor((HEIGHT - 1) * 0.3), CHSV(120, 255, 55), HEIGHT, CHSV(175, 255, 200));
      gradientDownTop(0, CHSV(120, 255, 255), floor(HEIGHT * 0.3), CHSV(120, 255, 55));
      break;
    default:
      gradientDownTop(floor((HEIGHT - 1) * 0.25), CHSV(180, 255, 85), HEIGHT, CHSV(160, 255, 200));
      gradientDownTop(0, CHSV(80, 255, 255), floor(HEIGHT * 0.25), CHSV(180, 255, 85));
      break;
  }
  flora();
}

//---------------------------------------
void createSceneM(uint8_t idx) {
  switch (idx) {
    case 0:     // blue green ------
      gradientVertical(0, CENTER_Y_MINOR, WIDTH, HEIGHT, 96, 150, 100, 255, 255U);
      gradientVertical(0, 0, WIDTH, CENTER_Y_MINOR, 96, 96, 255, 100, 255U);
      break;
    case 1:     // aquamarine green
      gradientVertical(0, floor(HEIGHT  * 0.3), WIDTH, HEIGHT, 96, 120, 100, 220, 255U);
      gradientVertical(0, 0, WIDTH, floor(HEIGHT  * 0.3), 96, 96, 255, 100, 255U);
      break;
    case 2:     // blue aquamarine -
      gradientVertical(0, CENTER_Y_MINOR, WIDTH, HEIGHT, 170, 160, 100, 200, 255U);
      gradientVertical(0, 0, WIDTH, CENTER_Y_MINOR, 100, 170, 255, 100, 255U);
      break;
    case 3:     // yellow green ----
      gradientVertical(0, CENTER_Y_MINOR, WIDTH, HEIGHT, 95, 65, 55, 200, 255U);
      gradientVertical(0, 0, WIDTH, CENTER_Y_MINOR, 95, 100, 255, 55, 255U);
      break;
    case 4:     // sea green -------
      gradientVertical(0, floor(HEIGHT  * 0.3), WIDTH, HEIGHT, 120, 160, 55, 200, 255U);
      gradientVertical(0, 0, WIDTH, floor(HEIGHT  * 0.3), 120, 120, 255, 55, 255U);
      break;
    default:
      drawRec(0, 0, WIDTH, HEIGHT, 0x000050);
      break;
  }
  flora();
}

//---------------------------------------
void BotswanaRivers() {
  // альтернативный градиент для ламп собраных из лент с вертикальной компоновкой
  // для корректной работы ALT_GRADIENT = true
  // для ламп из лент с горизонтальной компоновкой и матриц ALT_GRADIENT = false
  // ALT_GRADIENT = false более производительный и более плавная растяжка
  //------------------------------------------------------------------------------
  static const bool ALT_GRADIENT = true;

  uint8_t divider;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                          scale | speed 210
      setModeSettings(1U + random8(252U), 20 + random8(180U));
    }
#endif
    loadingFlag = false;
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = deltaValue;                                          // чтообы при старте эффекта сразу покрасить лампу
    divider = floor((modes[currentMode].Scale - 1) / 20);       // маштаб задает смену палитры воды
    if (ALT_GRADIENT) {
      createSceneM(divider);
    } else {
      createScene(divider);
    }
  }

  if (step >= deltaValue) {
    step = 0U;
  }

  // restore scene after power on ---------
  if (getPixColorXY(0U, HEIGHT - 2) == CRGB::Black) {
    if (ALT_GRADIENT) {
      createSceneM(divider);
    } else {
      createScene(divider);
    }
  }

  // light at the bottom ------------------
  if (!ALT_GRADIENT) {
    if (step % 2 == 0) {
      if (random8(6) == 1) {
        //fill_gradient(leds, NUM_LEDS - WIDTH, CHSV(96U, 255U, 200U), NUM_LEDS, CHSV(50U, 255U, 255U), SHORTEST_HUES);
        if (STRIP_DIRECTION < 2) {
          fill_gradient(leds, 0, CHSV(96U, 255U, 190U), random8(WIDTH + random8(6)), CHSV(90U, 200U, 255U), SHORTEST_HUES);
        } else {
          fill_gradient(leds, NUM_LEDS - random8(WIDTH + random8(6)), CHSV(96U, 255U, 190U), NUM_LEDS, CHSV(90U, 200U, 255U), SHORTEST_HUES);
        }
      } else {
        //fill_gradient(leds, NUM_LEDS - WIDTH, CHSV(50U, 128U, 255U), NUM_LEDS, CHSV(90U, 255U, 180U), SHORTEST_HUES);
        if (STRIP_DIRECTION < 2) {
          fill_gradient(leds, 0, CHSV(85U, 128U, 255U), random8(WIDTH), CHSV(90U, 255U, 180U), SHORTEST_HUES);
        } else {
          fill_gradient(leds, NUM_LEDS - random8(WIDTH), CHSV(85U, 128U, 255U), NUM_LEDS, CHSV(90U, 255U, 180U), SHORTEST_HUES);
        }
      }
    }
  }

  // LOG.printf_P(PSTR("%02d | hue2 = %03d | min = %03d \n\r"), step, hue2, deltaHue2);
  // -------------------------------------
  animeBobbles();
  if (custom_eff == 1) {
    blurRows(leds, WIDTH, 3U, 10U);
    // blurScreen(beatsin8(0U, 5U, 0U));
  }
  step++;
}

/*
// ========== Botswana Rivers New ===========
//      © SlingMaster | by Alex Dovby
//              EFF_RIVERS
//            Реки Ботсваны

//---------------------------------------
void flora() {
  if (WIDTH < 10) return;
  static const uint8_t POS_X = (CENTER_X_MINOR - 6);
  static const  uint32_t LEAF_COLOR = 0x1F2F00;
  static const bool data[5][5] = {
    { 0, 0, 0, 0, 1 },
    { 0, 0, 1, 1, 1 },
    { 0, 1, 1, 1, 1 },
    { 0, 1, 1, 1, 0 },
    { 1, 0, 0, 0, 0 }
  };

  uint8_t h =  random8(10U, 15U);
  uint8_t posX = 3; // floor(WIDTH * 0.5) - 3;
  byte deltaY = random8(2U);

  gradientVertical(POS_X - 1, 0, POS_X, 9U, 70, 75, 65U, 255U, 255U);
  gradientVertical(POS_X + 1, 0, POS_X + 2, 15U, 70, 75, 65U, 255U, 255U);
  drawPixelXY(POS_X + 2, h - random8(floor(h * 0.5)), random8(2U) == 1 ? 0xFF00E0 :  random8(2U) == 1 ? 0xFFFF00 : 0x00FF00);
  drawPixelXY(POS_X + 1, h - random8(floor(h * 0.25)), random8(2U) == 1 ? 0xFF00E0 : 0xFFFF00);
  if (random8(2U) == 1) {
    drawPixelXY(posX + 1, 5U, random8(2U) == 1 ? 0xEF001F :  0x9FFF00);
  }
  h =  floor(h * 0.65);
  drawPixelXY(POS_X, h - random8(5, h - 2), random8(2U) == 1 ? 0xFF00E0 : 0xFFFF00);

  // draw leafs -------------------
  for (uint8_t y = 0; y < 5; y++) {
    for (uint8_t x = 0; x < 5; x++) {
      if (data[y][x]) {
        leds[XY(POS_X + x, 7 + deltaY - y)] = LEAF_COLOR;
        if (WIDTH > 16) {
          leds[XY(POS_X - x, 15 - deltaY - y)] = LEAF_COLOR;
        }
      }
    }
  }
}

//---------------------------------------
void animeBobbles() {
  const uint32_t color = 0xF0F7FF;
  // сдвигаем Bobbles вверх ----
  for (uint8_t x = CENTER_X_MINOR; x < (CENTER_X_MINOR + 4); x++) {
    for (uint8_t y = HEIGHT; y > 0U; y--) {
      if (getPixColorXY(x, y - 1) == color) {
        drawPixelXY(x, y, color);
        drawPixelXY(x, y - 1, getPixColorXY(WIDTH - 1, y - 1));
      }
    }
  }
  // ----------------------
  if ( step % 4 == 0) {
    drawPixelXY(random8(CENTER_X_MINOR, CENTER_X_MINOR + 4), 0U, color);
    if ( step % 11 == 0) {
      drawPixelXY(random8(CENTER_X_MINOR, CENTER_X_MINOR + 4), 1U, color);
    }
  }
}

//---------------------------------------
void createScene(uint8_t idx) {
  switch (idx) {
    case 0:     // blue green ------
      gradientVertical(0, HEIGHT * 0.25, WIDTH, HEIGHT, 96, 160, 64, 255, 255U);
      gradientVertical(0, 0, WIDTH, HEIGHT * 0.25, 96, 96, 255, 64, 255U);
      break; //CENTER_Y_MINOR
    case 1:     // aquamarine green
      gradientVertical(0, 0, WIDTH, HEIGHT, 96, 130, 48, 255, 255U);
      break;
    case 2:     // blue aquamarine -
      gradientVertical(0, CENTER_Y_MINOR, WIDTH, HEIGHT, 170, 160, 100, 200, 255U);
      gradientVertical(0, 0, WIDTH, CENTER_Y_MINOR, 100, 170, 255, 100, 255U);
      break;
    case 3:     // yellow green ----
      gradientVertical(0, CENTER_Y_MINOR, WIDTH, HEIGHT, 100, 80, 60, 160, 255U);
      gradientVertical(0, 0, WIDTH, CENTER_Y_MINOR, 96, 100, 205, 60, 255U);
      break;
    case 4:     // sea green -------
      gradientVertical(0, floor(HEIGHT  * 0.3), WIDTH, HEIGHT, 120, 160, 64, 200, 255U);
      gradientVertical(0, 0, WIDTH, floor(HEIGHT  * 0.3), 120, 120, 225, 64, 255U);
      break;
    default:
      drawRec(0, 0, WIDTH, HEIGHT, 0x000050);
      break;
  }
  flora();
}

//---------------------------------------
void BotswanaRivers() {
  uint8_t divider;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                          scale | speed 210
      setModeSettings(1U + random8(252U), 20 + random8(180U));
    }
#endif
    loadingFlag = false;
    step = 0U;
    divider = floor((modes[currentMode].Scale - 1) / 20);       // маштаб задает смену палитры воды
    createScene(divider);
  }

  // restore scene after power on ---------
  if (getPixColorXY(0U, HEIGHT - 2) == CRGB::Black) {
    createScene(divider);
  }

  // LOG.printf_P(PSTR("%02d | hue2 = %03d | min = %03d \n\r"), step, hue2, deltaHue2);
  // -------------------------------------
  animeBobbles();
  step++;
}
*/

// ============ Watercolor ==============
//      © SlingMaster | by Alex Dovby
//            EFF_WATERCOLOR
//               Акварель
//---------------------------------------
void SmearPaint(uint8_t obj[trackingOBJECT_MAX_COUNT]) {
  uint8_t divider;
  int temp;
  static const uint32_t colors[6][8] PROGMEM = {
    {0x2F0000,  0xFF4040, 0x6F0000, 0xAF0000, 0xff5f00, CRGB::Red, 0x480000, 0xFF0030},
    {0x002F00, CRGB::LawnGreen, 0x006F00, 0x00AF00, CRGB::DarkMagenta, 0x00FF00, 0x004800, 0x00FF30},
    {0x002F1F, CRGB::DarkCyan, 0x00FF7F, 0x007FFF, 0x20FF5F, CRGB::Cyan, 0x004848, 0x7FCFCF },
    {0x00002F, 0x5030FF, 0x00006F, 0x0000AF, CRGB::DarkCyan, 0x0000FF, 0x000048, 0x5F5FFF},
    {0x2F002F, 0xFF4040, 0x6F004A, 0xFF0030, CRGB::DarkMagenta, CRGB::Magenta, 0x480048, 0x3F00FF},
    {CRGB::Blue, CRGB::Red, CRGB::Gold, CRGB::Green, CRGB::DarkCyan, CRGB::DarkMagenta, 0x000000, 0xFF7F00 }
  };
  if (trackingObjectHue[5] == 1) {  // direction >>>
    obj[1]++;
    if (obj[1] >= obj[2]) {
      trackingObjectHue[5] = 0;     // swap direction
      obj[3]--;                     // new line
      if (step % 2 == 0) {
        obj[1]++;
      } else {
        obj[1]--;
      }

      obj[0]--;
    }
  } else {                          // direction <<<
    obj[1]--;
    if (obj[1] <= (obj[2] - obj[0])) {
      trackingObjectHue[5] = 1;     // swap direction
      obj[3]--;                     // new line
      if (obj[0] >= 1) {
        temp = obj[0] - 1;
        if (temp < 0) {
          temp = 0;
        }
        obj[0] = temp;
        obj[1]++;
      }
    }
  }

  if (obj[3] == 255) {
    deltaHue = 255;
  }

  divider = floor((modes[currentMode].Scale - 1) / 16.7);
  if ( (obj[1] >= WIDTH) || (obj[3] == obj[4]) ) {
    // deltaHue value == 255 activate -------
    // set new parameter for new smear ------
    deltaHue = 255;
  }
  drawPixelXY(obj[1], obj[3], colors[divider][hue]);

  // alternative variant without dimmer effect
  // uint8_t h = obj[3] - obj[4];
  // uint8_t br = 266 - 12 * h;
  // if (h > 0) {
  // drawPixelXY(obj[1], obj[3], makeDarker(colors[divider][hue], br));
  // } else {
  // drawPixelXY(obj[1], obj[3], makeDarker(colors[divider][hue], 240));
  // }
}



//---------------------------------------
void Watercolor() {
  // #define DIMSPEED (254U - 500U / WIDTH / HEIGHT)
  uint8_t divider;
  if (loadingFlag) {

#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                          scale | speed 250
      setModeSettings(1U + random8(252U), 1 + random8(250U));
    }
#endif
    loadingFlag = false;
    FastLED.clear();
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = deltaValue;                    // чтообы при старте эффекта сразу покрасить лампу
    hue = 0;
    deltaHue = 255;                       // last color
    trackingObjectHue[1] = floor(WIDTH * 0.25);
    trackingObjectHue[3] = floor(HEIGHT * 0.25);
  }

  if (step >= deltaValue) {
    step = 0U;
    // LOG.printf_P(PSTR("%03d | log: %f | val: %03d | divider: %d \n\r"), modes[currentMode].Brightness, log(modes[currentMode].Brightness), deltaHue2, divider);
  }

  // ******************************
  // set random parameter for smear
  // ******************************
  if (deltaHue == 255) {

    trackingObjectHue[0] = 4 + random8(floor(WIDTH * 0.25));                // width
    trackingObjectHue[1] = random8(WIDTH - trackingObjectHue[0]);           // x
    int temp =  trackingObjectHue[1] + trackingObjectHue[0];
    if (temp >= (WIDTH - 1)) {
      temp = WIDTH - 1;
      if (trackingObjectHue[1] > 1) {
        trackingObjectHue[1]--;
      } else {
        trackingObjectHue[1]++;
      }
    }
    trackingObjectHue[2] = temp;                                            // x end
    trackingObjectHue[3] = 3 + random8(HEIGHT - 4);                         // y
    temp = trackingObjectHue[3] - random8(3) - 3;
    if (temp <= 0) {
      temp = 0;
    }
    trackingObjectHue[4] = temp;                                            // y end
    trackingObjectHue[5] = 1;
    divider = floor((modes[currentMode].Scale - 1) / 16.7);                 // маштаб задает смену палитры
    hue = random8(8);
    //    if (step % 127 == 0) {
    //      LOG.printf_P(PSTR("BR %03d | SP %03d | SC %03d | divider %d | [ %d ]\n\r"), modes[currentMode].Brightness, modes[currentMode].Speed, modes[currentMode].Scale, divider, hue);
    //    }
    hue2 = 255;
    deltaHue = 0;
  }
  // ******************************
  SmearPaint(trackingObjectHue);

  // LOG.printf_P(PSTR("%02d | hue2 = %03d | min = %03d \n\r"), step, hue2, deltaHue2);
  // -------------------------------------
  //  if (custom_eff == 1) {
  // dimAll(DIMSPEED);
  if (step % 2 == 0) {
    blurScreen(beatsin8(1U, 1U, 6U));
    // blurRows(leds, WIDTH, 3U, 10U);
  }
  //  }
  step++;
}

// =========== FeatherCandle ============
//         адаптация © SottNick
//    github.com/mnemocron/FeatherCandle
//      modify & design © SlingMaster
//           EFF_FEATHER_CANDLE
//                Свеча
//---------------------------------------
#include "data7x15flip.h"                       // FeatherCandle animation data
const uint8_t  level = 160;
const uint8_t  low_level = 110;
const uint8_t *ptr  = anim;                     // Current pointer into animation data
const uint8_t  w    = 7;                        // image width
const uint8_t  h    = 15;                       // image height
uint8_t        img[w * h];                      // Buffer for rendering image
uint8_t        deltaX = CENTER_X_MAJOR - 3;     // position img
uint8_t last_brightness;
void FeatherCandleRoutine() {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
  if (selectedSettings) {
    // brightness | scale | speed
    // { 21, 220,  40}
    setModeSettings(1U + random8(99U), 190U + random8(65U));
  }
#endif
  if (loadingFlag) {
    FastLED.clear();
    hue = 0;
    trackingObjectState[0] = low_level;
    trackingObjectState[1] = low_level;
    trackingObjectState[2] = low_level;
    trackingObjectState[4] = CENTER_X_MAJOR;
    loadingFlag = false;
  }

  uint8_t a = pgm_read_byte(ptr++);     // New frame X1/Y1
  if (a >= 0x90) {                      // EOD marker? (valid X1 never exceeds 8)
    ptr = anim;                         // Reset animation data pointer to start
    a   = pgm_read_byte(ptr++);         // and take first value
  }
  uint8_t x1 = a >> 4;                  // X1 = high 4 bits
  uint8_t y1 = a & 0x0F;                // Y1 = low 4 bits
  a  = pgm_read_byte(ptr++);            // New frame X2/Y2
  uint8_t x2 = a >> 4;                  // X2 = high 4 bits
  uint8_t y2 = a & 0x0F;                // Y2 = low 4 bits

  // Read rectangle of data from anim[] into portion of img[] buffer
  for (uint8_t y = y1; y <= y2; y++)
    for (uint8_t x = x1; x <= x2; x++) {
      img[y * w + x] = pgm_read_byte(ptr++);
    }
  int i = 0;
  uint8_t color = (modes[currentMode].Scale - 1U) * 2.57;



  // draw flame -------------------
  for (uint8_t y = 1; y < h; y++) {
    if ((HEIGHT < 15) || (WIDTH < 9)) {
      // for small matrix -----
      if (y % 2 == 0) {
        leds[XY(CENTER_X_MAJOR - 1, 7)] = CHSV(color, 255U, 55 + random8(200));
        leds[XY(CENTER_X_MAJOR, 6)] = CHSV(color, 255U, 160 + random8(90));
        leds[XY(CENTER_X_MAJOR + 1, 6)] = CHSV(color, 255U, 205 + random8(50));
        leds[XY(CENTER_X_MAJOR - 1, 5)] = CHSV(color, 255U, 155 + random8(100));
        leds[XY(CENTER_X_MAJOR, 5)] = CHSV(color - 10U , 255U, 120 + random8(130));
        leds[XY(CENTER_X_MAJOR, 4)] = CHSV(color - 10U , 255U, 100 + random8(120));
        DrawLine(0, 2U, WIDTH - 1, 2U, 0x000000);
      }
    } else {
      for (uint8_t x = 0; x < w; x++) {
        uint8_t brightness = img[i];
        leds[XY(deltaX + x, y)] = CHSV(brightness > 240 ? color : color - 10U , 255U, brightness);
        i++;
      }
    }

    // draw body FeatherCandle ------
    if (y <= 3) {
      if (y % 2 == 0) {
        gradientVertical(0, 0, WIDTH, 2, color, color, 48, 128, 20U);
      }
    }

    // drops of wax move -------------
    switch (hue ) {
      case 0:
        if (trackingObjectState[0] < level) {
          trackingObjectState[0]++;
        }
        break;
      case 1:
        if (trackingObjectState[0] > low_level) {
          trackingObjectState[0] --;
        }
        if (trackingObjectState[1] < level) {
          trackingObjectState[1] ++;
        }
        break;
      case 2:
        if (trackingObjectState[1] > low_level) {
          trackingObjectState[1] --;
        }
        if (trackingObjectState[2] < level) {
          trackingObjectState[2] ++;
        }
        break;
      case 3:
        if (trackingObjectState[2] > low_level) {
          trackingObjectState[2] --;
        } else {
          hue++;
          // set random position drop of wax
          trackingObjectState[4] = CENTER_X_MAJOR - 3 + random8(6);
        }
        break;
    }

    if (hue > 3) {
      hue++;
    } else {
      // LOG.printf_P(PSTR("[0] = %03d | [1] = %03d | [2] = %03d \n\r"), trackingObjectState[0], trackingObjectState[1], trackingObjectState[2]);
      if (hue < 2) {
        leds[XY(trackingObjectState[4], 2)] = CHSV(50U, 20U, trackingObjectState[0]);
      }
      if ((hue == 1) || (hue == 2)) {
        leds[XY(trackingObjectState[4], 1)] = CHSV(50U, 15U, trackingObjectState[1]); // - 10;
      }
      if (hue > 1) {
        leds[XY(trackingObjectState[4], 0)] = CHSV(50U, 5U, trackingObjectState[2]); // - 20;
      }
    }
  }

  // next -----------------
  if ((trackingObjectState[0] == level) || (trackingObjectState[1] == level) || (trackingObjectState[2] == level)) {
    hue++;
  }
}

// ============= Hourglass ==============
//             © SlingMaster
//             EFF_HOURGLASS
//             Песочные Часы
//---------------------------------------

void Hourglass() {
  uint8_t divider;
  uint8_t h;
  float SIZE = 0.43;
  const uint8_t topPos  = HEIGHT - floor(HEIGHT * SIZE);
  const uint8_t route = HEIGHT - floor(HEIGHT * SIZE) + 1;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                          scale | speed 210
      setModeSettings(15U + random8(225U), random8(255U));
    }
#endif
    loadingFlag = false;
    deltaValue = 255U - modes[currentMode].Speed + 1U;
    step = deltaValue;                                          // чтообы при старте эффекта сразу покрасить лампу
    deltaHue2 = 0;
    h = floor(NUM_LEDS * SIZE / WIDTH) * WIDTH;
    FastLED.clear();
    hue2 = 0;
  }
  if (hue2 == 0) {


    divider = modes[currentMode].Scale;
    if (divider < 10) {
      hue = step;
    } else {
      hue = divider * 2.55;
    }

    if (STRIP_DIRECTION < 2) {
      // STRIP_DIRECTION to UP ========
      fill_gradient(leds, NUM_LEDS - h, CHSV(hue, 255, 254), NUM_LEDS - deltaHue2, CHSV(hue, 255, 30), SHORTEST_HUES);
      fill_gradient(leds, 0, CHSV(hue, 255, 254), deltaHue2, CHSV(hue, 255, 30), SHORTEST_HUES);
    } else {
      // STRIP_DIRECTION to DOWN ======
      fill_gradient(leds, deltaHue2, CHSV(hue, 255, 30), h, CHSV(hue, 255, 254), SHORTEST_HUES);
      fill_gradient(leds, NUM_LEDS - deltaHue2, CHSV(hue, 255, 30), NUM_LEDS, CHSV(hue, 255, 254), SHORTEST_HUES);
    }

    if (custom_eff == 1) {
      if (deltaHue2 < WIDTH) {
        blurScreen(8U);
      }
    }
    deltaHue2++;
    if (deltaHue2 > h) {
      deltaHue2 = 0;
      hue2 = 1;
      // борьба с артифактом -----
      DrawLine(0, topPos, WIDTH - 1, topPos, 0x000000);
    }
    step++;
  }

  // сдвигаем всё вверх -----------
  // имитация переворота песочных часов
  if (hue2 > 0) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      for (uint8_t y = HEIGHT; y > 0U; y--) {
        drawPixelXY(x, y, getPixColorXY(x, y - 1U));
        drawPixelXY(x, y - 1, 0x000000);
      }
    }
    hue2++;
    if (hue2 > route) {
      hue2 = 0;
    }
  }
}
/*
// ============== Spectrum ==============
//             © SlingMaster
//              EFF_SPECTRUM
//                Spectrum
//---------------------------------------

void  Spectrum() {
  uint8_t divider;
  uint8_t deltaY;
  uint8_t posX;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed 210
      setModeSettings(random8(100U), 180U + random8(75U));
    }
#endif
    loadingFlag = false;
    step = deltaValue;
    hue = 120;
    hue2 = hue + 30;
    posX = 0;
    deltaHue2 = 0;
    divider = floor(modes[currentMode].Scale / 12.5);
    if (divider % 2 == 0) {
      blurScreen(dim8_raw(beatsin8(3, 64, 3)));
      DrawLine(3, 0, 4, 0 , 0x00FF00);
    } else {
      FastLED.clear();
      DrawLine(3, 0, 4, 0 , 0xFF3F00);
    }
  }

  deltaHue2++;
  if (deltaHue2 >= WIDTH) {
    deltaHue2 = 0;
  }
  divider = floor(modes[currentMode].Scale / 12.5);
  posX = deltaHue2;
  deltaY = floor((HEIGHT - validMinMax(random8(HEIGHT - 1), 3, HEIGHT)) * 0.5);

  switch (divider ) {
    case 0:
    case 1:
      if (step % 32 == 0) {
        hue++;
      }
      break;
    case 2:
    case 3:
      if (step % 2 == 0) {
        hue++;
      }
      break;
    case 4:
    case 5:
      if (step == 0) {
        hue += 30;
      }
      break;
    default:
      if (step == 0) {
        hue += 10;
        hue2 -= 10;
      }
      break;
  }

  if (divider < 6) {
    if (hue >= 225) {
      hue2 = 224;
    } else {
      hue2 = hue + 30;
    }
  }

  // if (step % 32 == 0) {
  //   LOG.printf_P(PSTR("divider = %01d | hue = %03d | hue2 = %03d | scale = %03d | BR = %03d\n\r"), divider, hue, hue2, modes[currentMode].Scale, (deltaValue));
  // }

  deltaValue = random8(128);          // delta brightness
  if (posX % 2 == 0) {
    if (divider < 6) {
      gradientVertical(posX, CENTER_Y_MINOR, posX + 1, CENTER_Y_MINOR + random8(2), hue + 15, hue2 - 15, 128 + deltaValue, 255 - deltaValue, 255U);
    }
    DrawLine(posX, HEIGHT - deltaY, posX, HEIGHT - 1, 0x000000);
    DrawLine(posX, 0, posX, deltaY , 0x000000);
  } else {
    gradientVertical(posX, deltaY, posX + 1, HEIGHT - deltaY, hue, hue2, 128 + deltaValue, 255 - deltaValue, 255U);
  }

  if (divider % 2 == 0) {
    blurScreen(6U);
  } else {
    DrawLine(posX, HEIGHT - deltaY, posX, HEIGHT - 1, 0x000000);
    DrawLine(posX, 0, posX, deltaY , 0x000000);
    dimAll(255 - posX);
  }
  step++;
}
*/


// ============ Spectrum New ==============
//             © SlingMaster
//         source code © kostyamat
//                Spectrum
//---------------------------------------
void  Spectrum() {
  static const byte COLOR_RANGE = 32;
  static uint8_t customHue;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random8(1, 100U), random8(215, 255U) );
    }
#endif // #if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)

    loadingFlag = false;
    ff_y = map(WIDTH, 8, 64, 310, 63);
    ff_z = ff_y;
    speedfactor = map(modes[currentMode].Speed, 1, 255, 32, 4); // _speed = map(speed, 1, 255, 128, 16);
    customHue = floor( modes[currentMode].Scale - 1U) * 2.55;
    FastLED.clear();
  }
  uint8_t color = customHue + hue;
  if (modes[currentMode].Scale >= 99) {
    if (hue2++ & 0x01 && deltaHue++ & 0x01 && deltaHue2++ & 0x01) hue += 8;
    fillMyPal16_2(customHue + hue, modes[currentMode].Scale & 0x01);
  } else {
    color = customHue;
    fillMyPal16_2(customHue + AURORA_COLOR_RANGE - beatsin8(AURORA_COLOR_PERIOD, 0U, AURORA_COLOR_RANGE * 2), modes[currentMode].Scale & 0x01);
  }

  for (byte x = 0; x < WIDTH; x++) {
    if (x % 2 == 0) {
      leds[XY(x, 0)] = CHSV( color, 255U, 128U);
    }

    emitterX = ((random8(2) == 0U) ? 545. : 390.) / HEIGHT;
    for (byte y = 2; y < HEIGHT - 1; y++) {
      polarTimer++;
      leds[XY(x, y)] =
        ColorFromPalette(myPal,
                         qsub8(
                           inoise8(polarTimer % 2 + x * ff_z,
                                   y * 16 + polarTimer % 16,
                                   polarTimer / speedfactor
                                  ),
                           fabs((float)HEIGHT / 2 - (float)y) * emitterX
                         )
                        ) ;
    }
  }
}


/*

// ============ Lotus Flower ============
//             © SlingMaster
//               EFF_LOTUS
//             Цветок Лотоса
//---------------------------------------
void  Flower() {
  uint8_t br;
  if (step < 128) {
    br = 255 - step;  // 255 >> 128
  } else {
    br = step;        // 128 >> 255
  }
  if (modes[currentMode].Scale > 10) {
    dimAll(90);
    hue = floor(modes[currentMode].Scale * 1.9) + floor(br / 4);
  } else {
    FastLED.clear();
    hue = step;
  }
  if (step > 190) {
    hue2 = validMinMax(hue - 64 + floor(br / 4), 190, 250);
  } else {
    hue2 = hue + 64 - floor(br / 4);
  }

  for (uint8_t x = 0U ; x < WIDTH ; x++) {
    if (x % 6 == 0) {
      gradientVertical( x - deltaValue, 2U, x + 1 - deltaValue, HEIGHT - floor((255 - br) / 24) - random8(2), hue, hue2, 255, floor(br * 0.5), 255U);
      gradientVertical( x + 3U - deltaValue, 0U, x + 4U - deltaValue, HEIGHT - floor(br / 24) + random8(3), hue, hue2, 255, floor((255 - br * 0.5)), 255U);
      drawPixelXY(x - deltaValue, 0, 0x005F00);
    }
  }
}

//---------------------------------------
void LotusFlower() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), 50U + random8(190U));
    }
#endif
    loadingFlag = false;
    deltaValue = 0;
    step = deltaValue;
    deltaValue = 0;
    hue = 120;
    hue2 = 0;
    deltaHue = 0;
    deltaHue2 = 0;
    FastLED.clear();
  }

  Flower();
  if (deltaHue == 0) {               // rotation
    deltaValue--;
    if (deltaValue <= 0) {
      deltaValue = 3;
    }
  } else {
    deltaValue++;
    if (deltaValue >= 3) {
      deltaValue = 0;
    }
  }
  deltaHue2++;
  if (deltaHue2 >= 18) {           // swap direction rotate
    deltaHue2 = 0;
    deltaHue = (deltaHue == 0) ? 1 : 0;
  }
  step++;
}



// =========== Christmas Tree ===========
//             © SlingMaster
//           EFF_CHRISTMAS_TREE
//            Новогодняя Елка
//---------------------------------------
void VirtualSnow() {
  for (uint8_t x = 0U; x < WIDTH; x++) {
    for (uint8_t y = 0U; y < HEIGHT - 1; y++) {
      noise3d[0][x][y] = noise3d[0][x][y + 1];
      if (noise3d[0][x][y] > 0) {
        drawPixelXY(x, y, CHSV(170, 5U, 127 + random8(128)));
      }
    }
  }
  uint8_t posX = random(WIDTH);
  for (uint8_t x = 0U; x < WIDTH; x++) {
    // заполняем случайно верхнюю строку
    if (posX == x) {
      if (step % 3 == 0) {
        noise3d[0][x][HEIGHT - 1U] = 1;
      } else {
        noise3d[0][x][HEIGHT - 1U]  = 0;
      }
    } else {
      noise3d[0][x][HEIGHT - 1U]  = 0;
    }
  }
}

//---------------------------------------
void GreenTree() {
  hue = floor(step / 32) * 32U;
  FastLED.clear();
  for (uint8_t x = 0U; x < WIDTH + 1 ; x++) {
    if (x % 8 == 0) {
      if (modes[currentMode].Scale < 60) {
        // nature -----
        DrawLine(x - 1U - deltaValue, floor(HEIGHT * 0.70), x + 1U - deltaValue, floor(HEIGHT * 0.70), 0x002F00);
        DrawLine(x - 1U - deltaValue, floor(HEIGHT * 0.55), x + 1U - deltaValue, floor(HEIGHT * 0.55), 0x004F00);
        DrawLine(x - 2U - deltaValue, floor(HEIGHT * 0.35), x + 2U - deltaValue, floor(HEIGHT * 0.35), 0x005F00);
        DrawLine(x - 2U - deltaValue, floor(HEIGHT * 0.15), x + 2U - deltaValue, floor(HEIGHT * 0.15), 0x007F00);
        drawPixelXY(x - 3U - deltaValue, floor(HEIGHT * 0.15), 0x001F00);
        drawPixelXY(x + 3U - deltaValue, floor(HEIGHT * 0.15), 0x001F00);
        gradientVertical( x - deltaValue, 0U, x + 1U - deltaValue, HEIGHT, 96U, 97U, 190U, 32U, 255U);
      } else {
        // holiday -----
        //        drawPixelXY(x - 1 - deltaValue, CENTER_Y_MINOR, CHSV(floor(random8(255) / 32) * 32U, 255U, 128 + random8(128)));
        //        drawPixelXY(x + 1 - deltaValue, CENTER_Y_MINOR, CHSV(floor(random8(90) / 32) * 32U, 255U, 128 + random8(128)));
        //        drawPixelXY(x - deltaValue, floor(HEIGHT * 0.2), CHSV(floor(random8(255) / 32) * 32U, 255U, 128 + random8(128)));
        //        drawPixelXY(x - 2 - deltaValue, floor(HEIGHT * 0.25), CHSV(floor(random8(90) / 32) * 32U, 255U, 128 + random8(128)));
        //        drawPixelXY(x + 2 - deltaValue, floor(HEIGHT * 0.25), CHSV(floor(random8(255) / 32) * 32U, 255U, 128 + random8(128)));
        //        drawPixelXY(x - deltaValue, 0U, CHSV(96U, 255U, 200U));

        drawPixelXY(x - 1 - deltaValue, floor(HEIGHT * 0.6), CHSV(step, 255U, 128 + random8(128)));
        drawPixelXY(x + 1 - deltaValue, floor(HEIGHT * 0.6), CHSV(step, 255U, 128 + random8(128)));

        drawPixelXY(x - deltaValue, floor(HEIGHT * 0.4), CHSV(step, 255U, 200U));

        drawPixelXY(x - deltaValue, floor(HEIGHT * 0.2), CHSV(step, 255U, 190 + random8(65)));
        drawPixelXY(x - 2 - deltaValue, floor(HEIGHT * 0.25), CHSV(step, 255U, 96 + random8(128)));
        drawPixelXY(x + 2 - deltaValue, floor(HEIGHT * 0.25), CHSV(step, 255U, 96 + random8(128)));

        drawPixelXY(x - 2 - deltaValue, 1U, CHSV(step, 255U, 200U));
        drawPixelXY(x - deltaValue, 0U, CHSV(step, 255U, 250U));
        drawPixelXY(x + 2 - deltaValue, 1U, CHSV(step, 255U, 200U));

        gradientVertical( x - deltaValue, floor(HEIGHT * 0.75), x + 1U - deltaValue, HEIGHT,  hue, hue + 2, 250U, 0U, 128U);
      }
    }
  }
}

//---------------------------------------
void ChristmasTree() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), 10U + random8(128U));
    }
#endif
    loadingFlag = false;
    deltaValue = 0;
    step = deltaValue;
    FastLED.clear();
  }

  GreenTree();
  if (modes[currentMode].Scale < 60) {
    VirtualSnow();
  }
  if (modes[currentMode].Scale > 30) {
    deltaValue++;
  }
  if (deltaValue >= 8) {
    deltaValue = 0;
  }
  step++;
}

*/


// =========== Christmas Tree ===========
//             © SlingMaster
//           EFF_CHRISTMAS_TREE
//            Новогодняя Елка
//---------------------------------------
void clearNoiseArr() {
  for (uint8_t x = 0U; x < WIDTH; x++) {
    for (uint8_t y = 0U; y < HEIGHT; y++) {
      noise3d[0][x][y] = 0;
      noise3d[1][x][y] = 0;
    }
  }
}

//---------------------------------------
void VirtualSnow(byte snow_type) {
  uint8_t posX = random8(WIDTH - 1);
  const uint8_t maxX = WIDTH - 1;
  static int deltaPos;
  byte delta = (snow_type == 3) ? 0 : 1;
  for (uint8_t x = delta; x < WIDTH - delta; x++) {

    // заполняем случайно верхнюю строку
    if ((noise3d[0][x][HEIGHT - 2] == 0U) &&  (posX == x) && (random8(0, 2) == 0U)) {
      noise3d[0][x][HEIGHT] = 1;
    } else {
      noise3d[0][x][HEIGHT] = 0;
    }

    for (uint8_t y = 0U; y < HEIGHT; y++) {
      switch (snow_type) {
        case 0:
          noise3d[0][x][y] = noise3d[0][x][y + 1];
          deltaPos = 0;
          break;
        case 1:
        case 2:
          noise3d[0][x][y] = noise3d[0][x][y + 1];
          deltaPos = 1 - random8(2);
          break;
        default:
          deltaPos = -1;
          if ((x == 0 ) & (y == 0 ) & (random8(2) == 0U)) {
            noise3d[0][WIDTH - 1][random8(CENTER_Y_MAJOR / 2, HEIGHT - CENTER_Y_MAJOR / 4)] = 1;
          }
          if (x > WIDTH - 2) {
            noise3d[0][WIDTH - 1][y] = 0;
          }
          if (x < 1)  {
            noise3d[0][x][y] = noise3d[0][x][y + 1];
          } else {
            noise3d[0][x - 1][y] = noise3d[0][x][y + 1];
          }
          break;
      }

      if (noise3d[0][x][y] > 0) {
        if (snow_type < 3) {
          if (y % 2 == 0U) {
            leds[XY(x - ((x > 0) ? deltaPos : 0), y)] = CHSV(160, 5U, random8(200U, 240U));
          } else {
            leds[XY(x + deltaPos, y)] = CHSV(160, 5U,  random8(200U, 240U));
          }
        } else {
          leds[XY(x, y)] = CHSV(160, 5U,  random8(200U, 240U));
        }
      }
    }
  }
}

//---------------------------------------
void GreenTree(uint8_t tree_h) {
  hue = floor(step / 32) * 32U;

  for (uint8_t x = 0U; x < WIDTH + 1 ; x++) {
    if (x % 8 == 0) {
      if (modes[currentMode].Scale < 60) {
        // nature -----
        DrawLine(x - 1U - deltaValue, floor(tree_h * 0.70), x + 1U - deltaValue, floor(tree_h * 0.70), 0x002F00);
        DrawLine(x - 1U - deltaValue, floor(tree_h * 0.55), x + 1U - deltaValue, floor(tree_h * 0.55), 0x004F00);
        DrawLine(x - 2U - deltaValue, floor(tree_h * 0.35), x + 2U - deltaValue, floor(tree_h * 0.35), 0x005F00);
        DrawLine(x - 2U - deltaValue, floor(tree_h * 0.15), x + 2U - deltaValue, floor(tree_h * 0.15), 0x007F00);

        drawPixelXY(x - 3U - deltaValue, floor(tree_h * 0.15), 0x001F00);
        drawPixelXY(x + 3U - deltaValue, floor(tree_h * 0.15), 0x001F00);
        if ((x - deltaValue ) >= 0) {
          gradientVertical( x - deltaValue, 0U, x - deltaValue, tree_h, 90U, 90U, 190U, 64U, 255U);
        }
      } else {
        // holiday -----
        drawPixelXY(x - 1 - deltaValue, floor(tree_h * 0.6), CHSV(step, 255U, 128 + random8(128)));
        drawPixelXY(x + 1 - deltaValue, floor(tree_h * 0.6), CHSV(step, 255U, 128 + random8(128)));

        drawPixelXY(x - deltaValue, floor(tree_h * 0.4), CHSV(step, 255U, 200U));

        drawPixelXY(x - deltaValue, floor(tree_h * 0.2), CHSV(step, 255U, 190 + random8(65)));
        drawPixelXY(x - 2 - deltaValue, floor(tree_h * 0.25), CHSV(step, 255U, 96 + random8(128)));
        drawPixelXY(x + 2 - deltaValue, floor(tree_h * 0.25), CHSV(step, 255U, 96 + random8(128)));

        drawPixelXY(x - 2 - deltaValue, 1U, CHSV(step, 255U, 200U));
        drawPixelXY(x - deltaValue, 0U, CHSV(step, 255U, 250U));
        drawPixelXY(x + 2 - deltaValue, 1U, CHSV(step, 255U, 200U));
        if ((x - deltaValue) >= 0) {
          gradientVertical( x - deltaValue, floor(tree_h * 0.75), x - deltaValue, tree_h,  hue, hue, 250U, 0U, 128U);
        }
      }
    }
  }
}

//---------------------------------------
void ChristmasTree() {
  static uint8_t tree_h = HEIGHT;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), 10U + random8(128));
    }
#endif
    loadingFlag = false;
    clearNoiseArr();
    deltaValue = 0;
    step = deltaValue;
    FastLED.clear();

    if (HEIGHT > 16) {
      tree_h = 16;
    }
  }

  if (HEIGHT > 16) {
    if (modes[currentMode].Scale < 60) {
      gradientVertical(0, 0, WIDTH, HEIGHT, 160, 160, 64, 128, 255U);
    } else {
      FastLED.clear();
    }
  } else {
    FastLED.clear();
  }
  GreenTree(tree_h);

  if (modes[currentMode].Scale < 60) {
    VirtualSnow(1);
  }
  if (modes[currentMode].Scale > 30) {
    deltaValue++;
  }
  if (deltaValue >= 8) {
    deltaValue = 0;
  }
  step++;
}



// ============== ByEffect ==============
//             © SlingMaster
//             EFF_BY_EFFECT
//            Побочный Эффект
// --------------------------------------
void ByEffect() {
  uint8_t saturation;
  uint8_t delta;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed 210
      setModeSettings(random8(100U), random8(200U));
    }
#endif
    loadingFlag = false;
    deltaValue = 0;
    step = deltaValue;
    FastLED.clear();
  }

  hue = floor(step / 32) * 32U;
  dimAll(180);
  // ------
  saturation = 255U;
  delta = 0;
  for (uint8_t x = 0U; x < WIDTH + 1 ; x++) {
    if (x % 8 == 0) {
      gradientVertical( x - deltaValue, floor(HEIGHT * 0.75), x + 1U - deltaValue, HEIGHT,  hue, hue + 2, 250U, 0U, 255U);
      if (modes[currentMode].Scale > 50) {
        delta = random8(200U);
      }
      drawPixelXY(x - 2 - deltaValue, floor(HEIGHT * 0.7), CHSV(step, saturation - delta, 128 + random8(128)));
      drawPixelXY(x + 2 - deltaValue, floor(HEIGHT * 0.7), CHSV(step, saturation, 128 + random8(128)));

      drawPixelXY(x - deltaValue, floor(HEIGHT * 0.6), CHSV(hue, 255U, 190 + random8(65)));
      if (modes[currentMode].Scale > 50) {
        delta = random8(200U);
      }
      drawPixelXY(x - 1 - deltaValue, CENTER_Y_MINOR, CHSV(step, saturation, 128 + random8(128)));
      drawPixelXY(x + 1 - deltaValue, CENTER_Y_MINOR, CHSV(step, saturation - delta, 128 + random8(128)));

      drawPixelXY(x - deltaValue, floor(HEIGHT * 0.4), CHSV(hue, 255U, 200U));
      if (modes[currentMode].Scale > 50) {
        delta = random8(200U);
      }
      drawPixelXY(x - 2 - deltaValue, floor(HEIGHT * 0.3), CHSV(step, saturation - delta, 96 + random8(128)));
      drawPixelXY(x + 2 - deltaValue, floor(HEIGHT * 0.3), CHSV(step, saturation, 96 + random8(128)));

      gradientVertical( x - deltaValue, 0U, x + 1U - deltaValue, floor(HEIGHT * 0.25),  hue + 2, hue, 0U, 250U, 255U);

      if (modes[currentMode].Scale > 50) {
        drawPixelXY(x + 3 - deltaValue, HEIGHT - 3U, CHSV(step, 255U, 255U));
        drawPixelXY(x - 3 - deltaValue, CENTER_Y_MINOR, CHSV(step, 255U, 255U));
        drawPixelXY(x + 3 - deltaValue, 2U, CHSV(step, 255U, 255U));
      }
    }
  }
  // ------
  deltaValue++;
  if (deltaValue >= 8) {
    deltaValue = 0;
  }
  step++;
}


// =====================================
//            Строб Хаос Дифузия
//          Strobe Haos Diffusion
//             © SlingMaster
// =====================================
/*должен быть перед эффектом Матрицf бегунок Скорость не регулирует задержку между кадрами,
  но меняет частоту строба*/
void StrobeAndDiffusion() {
  const uint8_t SIZE = 3U;
  const uint8_t DELTA = 1U;         // центровка по вертикали
  uint8_t STEP = 2U;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(1U + random8(100U), 1U + random8(150U));
    }
#endif
    loadingFlag = false;
    FPSdelay = 25U; // LOW_DELAY;
    hue2 = 1;
    FastLED.clear();
  }

  STEP = floor((255 - modes[currentMode].Speed) / 64) + 1U; // for strob
  if (modes[currentMode].Scale > 50) {
    // diffusion ---
    blurScreen(beatsin8(3, 64, 80));
    FPSdelay = LOW_DELAY;
    STEP = 1U;
    if (modes[currentMode].Scale < 75) {
      // chaos ---
      FPSdelay = 30;
      VirtualSnow(1);
    }

  } else {
    // strob -------
    if (modes[currentMode].Scale > 25) {
      dimAll(200);
      FPSdelay = 30;
    } else {
      dimAll(240);
      FPSdelay = 40;
    }
  }

  const uint8_t rows = (HEIGHT + 1) / 3U;
  deltaHue = floor(modes[currentMode].Speed / 64) * 64;
  bool dir = false;
  for (uint8_t y = 0; y < rows; y++) {
    if (dir) {
      if ((step % STEP) == 0) {   // small layers
        drawPixelXY(WIDTH - 1, y * 3 + DELTA, CHSV(step, 255U, 255U ));
      } else {
        drawPixelXY(WIDTH - 1, y * 3 + DELTA, CHSV(170U, 255U, 1U));
      }
    } else {
      if ((step % STEP) == 0) {   // big layers
        drawPixelXY(0, y * 3 + DELTA, CHSV((step + deltaHue), 255U, 255U));
      } else {
        drawPixelXY(0, y * 3 + DELTA, CHSV(0U, 255U, 0U));
      }
    }

    // сдвигаем слои  ------------------
    for (uint8_t x = 0U ; x < WIDTH; x++) {
      if (dir) {  // <==
        drawPixelXY(x - 1, y * 3 + DELTA, getPixColorXY(x, y * 3 + DELTA));
      } else {    // ==>
        drawPixelXY(WIDTH - x, y * 3 + DELTA, getPixColorXY(WIDTH - x - 1, y * 3 + DELTA));
      }
    }
    dir = !dir;
  }

  if (hue2 == 1) {
    step ++;
    if (step >= 254) hue2 = 0;
  } else {
    step --;
    if (step < 1) hue2 = 1;
  }
}

// =====================================
//               Фейерверк
//                Firework
//             © SlingMaster
// =====================================
void VirtualExplosion(uint8_t f_type, int8_t timeline) {
  const uint8_t DELAY_SECOND_EXPLOSION = HEIGHT * 0.25;
  uint8_t horizont = 1U; // HEIGHT * 0.2;
  const int8_t STEP = 255 / HEIGHT;
  uint8_t firstColor = random8(255);
  uint8_t secondColor = 0;
  uint8_t saturation = 255U;
  switch (f_type) {
    case 0:
      secondColor =  random(50U, 255U);
      saturation = random(245U, 255U);
      break;
    case 1: /* сакура */
      firstColor = random(210U, 230U);
      secondColor = random(65U, 85U);
      saturation = 255U;
      break;
    case 2: /* день Независимости */
      firstColor = random(160U, 170U);
      secondColor = random(25U, 50U);
      saturation = 255U;
      break;
    default: /* фризантемы */
      firstColor = random(30U, 40U);
      secondColor = random(25U, 50U);
      saturation = random(128U, 255U);
      break;
  }
  if ((timeline > HEIGHT - 1 ) & (timeline < HEIGHT * 1.75)) {
    for (uint8_t x = 0U; x < WIDTH; x++) {
      for (uint8_t y =  horizont; y < HEIGHT - 1; y++) {
        noise3d[0][x][y] = noise3d[0][x][y + 1];
        uint8_t bri = y * STEP;
        if (noise3d[0][x][y] > 0) {
          if (timeline > (HEIGHT + DELAY_SECOND_EXPLOSION) ) {
            /* second explosion */
            drawPixelXY((x - 2 + random8(4)), y - 1, CHSV(secondColor + random8(16), saturation, bri));
          }
          if (timeline < ((HEIGHT - DELAY_SECOND_EXPLOSION) * 1.75) ) {
            /* first explosion */
            drawPixelXY(x, y, CHSV(firstColor, 255U, bri));
          }
        } else {
          // drawPixelXY(x, y, CHSV(175, 255U, floor((255 - bri) / 4)));
        }
      }
    }
    uint8_t posX = random8(WIDTH);
    for (uint8_t x = 0U; x < WIDTH; x++) {
      // заполняем случайно верхнюю строку
      if (posX == x) {
        if (step % 2 == 0) {
          noise3d[0][x][HEIGHT - 1U] = 1;
        } else {
          noise3d[0][x][HEIGHT - 1U]  = 0;
        }
      } else {
        noise3d[0][x][HEIGHT - 1U]  = 0;
      }
    }
  }
}

// --------------------------------------
void Firework() {
  const uint8_t MAX_BRIGHTNESS = 40U;            /* sky brightness */
  const uint8_t DOT_EXPLOSION = HEIGHT * 0.95;
  const uint8_t HORIZONT = HEIGHT * 0.25;
  const uint8_t DELTA = 1U;                      /* центровка по вертикали */
  const float stepH = HEIGHT / 128.0;
  const uint8_t FPS_DELAY = 20U;
  const uint8_t STEP = 3U;
  const uint8_t skyColor = 156U;
  uint8_t sizeH;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(1U + random8(100U), 1U + random8(250U));
    }
#endif
    loadingFlag = false;
    deltaHue2 = 0;
    FPSdelay = 255U;
    clearNoiseArr();
    FastLED.clear();
    step = 0U;
    deltaHue2 = floor(modes[currentMode].Scale / 26);
    hue = 48U;            // skyBright
    sizeH = HEIGHT;
    if (modes[currentMode].Speed > 85U) {
      sizeH = HORIZONT;
      FPSdelay = FPS_DELAY;
    }
    if (modes[currentMode].Speed <= 85U) {
      gradientVertical(0, 0, WIDTH, HEIGHT,  skyColor,  skyColor, 96U, 0U, 255U);
    }
  }
  if (FPSdelay == 240U) {
    FPSdelay = FPS_DELAY;
  }
  if (FPSdelay > 230U) {
    //  if (FPSdelay > 128U) {
    /* вечерело */
    FPSdelay--;
    sizeH = (FPSdelay - 128U) * stepH;

    if (modes[currentMode].Speed <= 85U) {
      dimAll(225U);
      return;
    }
    if (sizeH > HORIZONT)  {
      dimAll(200);
      return;
    }
    if (sizeH == HORIZONT )  FPSdelay = FPS_DELAY;
  }

  if (step > DOT_EXPLOSION ) {
    blurScreen(beatsin8(3, 64, 80));
    //    FastLED.setBrightness(250);
  }
  if (step == DOT_EXPLOSION - 1) {
    /* включаем фазу затухания */
    FPSdelay = 70;
  }
  if (step > CENTER_Y_MAJOR) {
    dimAll(140);
  } else {
    dimAll(100);
  }


  /* ============ draw sky =========== */
  if ((modes[currentMode].Speed > 85U) & (modes[currentMode].Speed < 180U)) {
    gradientVertical(0, 0, WIDTH, HORIZONT, skyColor, skyColor, 48U, 0U, 255U);
  }

  /* deltaHue2 - Firework type */
  VirtualExplosion(deltaHue2, step);

  if ((step > DOT_EXPLOSION ) & (step < HEIGHT * 1.5)) {
    /* фаза взрыва */
    FPSdelay += 5U;
  }
  const uint8_t rows = (HEIGHT + 1) / 3U;
  deltaHue = floor(modes[currentMode].Speed / 64) * 64;
  if (step > CENTER_Y_MAJOR) {
    bool dir = false;
    for (uint8_t y = 0; y < rows; y++) {
      /* сдвигаем слои / эмитация разлета */
      for (uint8_t x = 0U ; x < WIDTH; x++) {
        if (dir) {  // <==
          drawPixelXY(x - 1, y * 3 + DELTA, getPixColorXY(x, y * 3 + DELTA));
        } else {    // ==>
          drawPixelXY(WIDTH - x, y * 3 + DELTA, getPixColorXY(WIDTH - x - 1, y * 3 + DELTA));
        }
      }
      dir = !dir;
      /* --------------------------------- */
    }
  }

  /* ========== фаза полета ========== */
  if (step < DOT_EXPLOSION ) {
    FPSdelay ++;
    if (HEIGHT < 20) {
      FPSdelay ++;
    }
    /* закоментируйте следующие две строки если плоская лампа
      подсветка заднего фона */
    if (custom_eff == 1) {
      DrawLine(0U, 0U, 0U, HEIGHT - step, CHSV(skyColor, 255U, 32U));
      DrawLine(WIDTH - 1, 0U, WIDTH - 1U, HEIGHT - step, CHSV(skyColor, 255U, 32U));
    }
    /* ------------------------------------------------------ */

    uint8_t saturation = (step > (DOT_EXPLOSION - 2U)) ? 192U : 20U;
    uint8_t rndPos = 3U * deltaHue2 * 0.5;
    drawPixelXY(CENTER_X_MINOR + rndPos, step,  CHSV(50U, saturation, 80U));                 // first
    drawPixelXY(CENTER_X_MAJOR + 1 - rndPos, step - HORIZONT,  CHSV(50U, saturation, 80U));  // second
    if (rndPos > 1) {
      drawPixelXY(CENTER_X_MAJOR + 4 - rndPos, step - HORIZONT + 2,  CHSV(50U, saturation, 80U));// three
    }
    /* sky brightness */
    if (hue > 2U) {
      hue -= 1U;
    }
  }
  if (step > HEIGHT * 1.25) {
    /* sky brightness */
    if (hue < MAX_BRIGHTNESS) {
      hue += 2U;
    }
  }

  if (step >= (HEIGHT * 2.0)) {
    step = 0U;
    // LOG.printf_P(PSTR("• Bright • [%03d]\n"), FastLED.getBrightness());
    FPSdelay = FPS_DELAY;
    if (modes[currentMode].Scale < 5) {
      deltaHue2++;
    }
    if (deltaHue2 >= 4U) deltaHue2 = 0U;  // next Firework type
  }
  // LOG.printf_P(PSTR("• [%03d] | %03d | sky Bright • [%03d]\n"), step, FPSdelay, hue);
  step ++;
}


// =====================================
//             Мечта Дизайнера
//                WebTools
//             © SlingMaster
// =====================================
/* --------------------------------- */
int getRandomPos(uint8_t STEP) {
  uint8_t val = floor(random(0, (STEP * 16 - WIDTH - 1)) / STEP) * STEP;
  return -val;
}

/* --------------------------------- */
int getHue(uint8_t x, uint8_t y) {
  return ( x * 32 +  y * 24U );
}

/* --------------------------------- */
uint8_t getSaturationStep() {
  return (modes[currentMode].Speed > 170U) ? ((HEIGHT > 24) ? 12 : 24) : 0;
}

/* --------------------------------- */
uint8_t getBrightnessStep() {
  return (modes[currentMode].Speed < 85U) ? ((HEIGHT > 24) ? 16 : 24) : 0;
}

/* --------------------------------- */
void drawPalette(int posX, int posY, uint8_t STEP) {
  int PX, PY;
  const uint8_t SZ = STEP - 1;
  const uint8_t maxY = floor(HEIGHT / SZ);
  uint8_t sat = getSaturationStep();
  uint8_t br  = getBrightnessStep();

  FastLED.clear();
  for (uint8_t y = 0; y < maxY; y++) {
    for (uint8_t x = 0; x < 16; x++) {
      PY = y * STEP;
      PX = posX + x * STEP;
      if ((PX >= - STEP ) && (PY >= - STEP) && (PX < WIDTH) && (PY < HEIGHT)) {
        // LOG.printf_P(PSTR("y: %03d | br • %03d | sat • %03d\n"), y, (240U - br * y), sat);
        drawRecCHSV(PX, PY, PX + SZ, PY + SZ, CHSV( getHue(x, y), (255U - sat * y), (240U - br * y)));
      }
    }
  }
}

/* --------------------------------- */
void selectColor(uint8_t sc) {
  uint8_t offset = (WIDTH >= 16) ? WIDTH * 0.25 : 0;
  hue = getHue(random(offset, WIDTH - offset), random(HEIGHT));
  uint8_t sat = getSaturationStep();
  uint8_t br  = getBrightnessStep();

  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = offset; x < (WIDTH - offset); x++) {
      CHSV curColor = CHSV(hue, (255U - sat * y), (240U - br * y));
      if (curColor == getPixColorXY(x, y)) {
        /* show srlect color */
        drawRecCHSV(x, y, x + sc, y + sc, CHSV( hue, 64U, 255U));
        FastLED.show();
        delay(400);
        drawRecCHSV(x, y, x + sc, y + sc, CHSV( hue, 255U, 255U));
        y = HEIGHT;
        x = WIDTH;
      }
    }
  }
}

/* --------------------------------- */
void WebTools() {
  const uint8_t FPS_D = 24U;
  static uint8_t STEP = 3U;
  static int posX = -STEP;
  static int posY = 0;
  static int nextX = -STEP * 2;
  static bool stop_moving = true;
  uint8_t speed = constrain (modes[currentMode].Speed, 65, 255);
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
       setModeSettings(random(10U, 90U), random(10U, 255U));
    }
#endif
    loadingFlag = false;
    FPSdelay = 1U;
    step = 0;
    STEP = 2U + floor(modes[currentMode].Scale / 35);
    posX = 0;
    posY = 0;
    drawPalette(posX, posY, STEP);
  }
  /* auto scenario */
  //switch (step) {
    if (step == 0){     /* restart ----------- */
      nextX = 0;
      FPSdelay = FPS_D;
    }
    else 
    if (step == speed/16+1){    /* start move -------- 16*/
      nextX = getRandomPos(STEP);
      FPSdelay = FPS_D;
    }
    else
    if (step == speed/10+1){    /* find --------------100 */
      nextX = getRandomPos(STEP);
      FPSdelay = FPS_D;
    }
    else
    if (step == speed/7+1){    /* find 2 ----------- 150*/
      nextX = getRandomPos(STEP);
      FPSdelay = FPS_D;
    }
    else
    if (step == speed/6+1){    /* find 3 -----------200 */
      nextX = - STEP * random(4, 8);
      // nextX = getRandomPos(STEP);
      FPSdelay = FPS_D;
    }
    else
    if (step == speed/5+1){   /* select color ------220 */
      FPSdelay = 200U;
      selectColor(STEP - 1);
    }
    else
    if (step == speed/4+1){   /* show color -------- 222*/
      FPSdelay = FPS_D;
      nextX = WIDTH;
    }
    else
    if (step == speed/4+3){
      step = 252;
    }
    
  //}
  if (posX < nextX) posX++;
  if (posX > nextX) posX--;

  if (stop_moving)   {
    FPSdelay = 80U;
    step++;
  } else {
    drawPalette(posX, posY, STEP);
    if ((nextX == WIDTH) || (nextX == 0)) {
      /* show select color bar gradient */
      // LOG.printf_P(PSTR("step: %03d | Next x: %03d • %03d | fps %03d\n"), step, nextX, posX, FPSdelay);
      if (posX > 1) {
        gradientHorizontal(0, 0, (posX - 1), HEIGHT, hue, hue, 255U, 96U, 255U);
      }
      if (posX > 3) DrawLine(posX - 3, CENTER_Y_MINOR, posX - 3, CENTER_Y_MAJOR, CHSV( hue, 192U, 255U));
    }
  }

  stop_moving = (posX == nextX); 
}

// ================Contacts==============
//             © Yaroslaw Turbin
//        Adaptation © SlingMaster
//          modifed © alvikskor
//              Контакты
// =====================================

void Contacts() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random(25U, 90U), random(5U, 250U));
    }
#endif
    loadingFlag = false;
    FPSdelay = 80U;
    FastLED.clear();
  }

  int a = millis() / floor((255 - modes[currentMode].Speed) / 10);
  hue = floor(modes[currentMode].Scale / 14);
  for (int x = 0; x < WIDTH; x++) {
    for (int y = 0; y < HEIGHT; y++) {
      int index = XY(x, y);
      uint8_t color1 = pgm_read_byte(&exp_gamma[sin8(cos8((x * 7 +a/5)) - cos8((y * 10) +a/3)/4+a )]);
      uint8_t color2 = pgm_read_byte(&exp_gamma[(sin8(x * 16 + a / 3) + cos8(y * 8 + a / 2)) / 2]);
      uint8_t color3 = pgm_read_byte(&exp_gamma[sin8(cos8(x * 8 + a / 3) + sin8(y * 8 + a / 4) + a)]);
      if (hue == 0) {
        leds[index].b = color3 >> 2;
        leds[index].g = color2;
        leds[index].r = 0;
      } else if (hue == 1) {
        leds[index].b = color1;
        leds[index].g = 0;
        leds[index].r = color3 >> 2;
      } else if (hue == 2) {
        leds[index].b = 0;
        leds[index].g = color1 >> 2;
        leds[index].r = color3;
      } else if (hue == 3) {
        leds[index].b = color1;
        leds[index].g = color2;
        leds[index].r = color3;
      } else if (hue == 4) {
        leds[index].b = color3;
        leds[index].g = color1;
        leds[index].r = color2;
      } else if (hue == 5) {
        leds[index].b = color2;
        leds[index].g = color3;
        leds[index].r = color1;
      } else if (hue >= 6) {
        leds[index].b = color3;
        leds[index].g = color1;
        leds[index].r = color2;
      }
    }
  }
}

// ============ Magic Lantern ===========
//             © SlingMaster
//            Волшебный Фонарик
// --------------------------------------
void MagicLantern() {
  static uint8_t saturation;
  static uint8_t brightness;
  static uint8_t low_br;
  uint8_t delta;
  const uint8_t PADDING = HEIGHT * 0.25;
  const uint8_t WARM_LIGHT = 55U;
  const uint8_t STEP = 4U;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed 210
      setModeSettings(random8(100U), random8(40, 200U));
    }
#endif
    loadingFlag = false;
    deltaValue = 0;
    step = deltaValue;
    if (modes[currentMode].Speed > 52) {
      // brightness = 50 + modes[currentMode].Speed;
      brightness = map(modes[currentMode].Speed, 1, 255, 50U, 250U);
      low_br = 50U;
    } else {
      brightness = 0U;
      low_br = 0U;
    }
    saturation = (modes[currentMode].Scale > 50U) ? 64U : 0U;
    if (abs (70 - modes[currentMode].Scale) <= 5) saturation = 170U;
    FastLED.clear();

  }
  dimAll(170);
  hue = (modes[currentMode].Scale > 95) ? floor(step / 32) * 32U : modes[currentMode].Scale * 2.55;

  // ------
  for (uint8_t x = 0U; x < WIDTH + 1 ; x++) {

    // light ---
    if (low_br > 0) {
      gradientVertical( x - deltaValue, CENTER_Y_MAJOR, x + 1U - deltaValue, HEIGHT - PADDING - 1,  WARM_LIGHT, WARM_LIGHT, brightness, low_br, saturation);
      gradientVertical( WIDTH - x + deltaValue, CENTER_Y_MAJOR, WIDTH - x + 1U + deltaValue, HEIGHT - PADDING - 1,  WARM_LIGHT, WARM_LIGHT, brightness, low_br, saturation);
      gradientVertical( x - deltaValue, PADDING + 1, x + 1U - deltaValue, CENTER_Y_MAJOR, WARM_LIGHT, WARM_LIGHT, low_br + 10, brightness, saturation);
      gradientVertical( WIDTH - x + deltaValue, PADDING + 1, WIDTH - x + 1U + deltaValue, CENTER_Y_MAJOR, WARM_LIGHT, WARM_LIGHT, low_br + 10, brightness, saturation);
    } else {
      if (x % (STEP + 1) == 0) {
        leds[XY(random8(WIDTH), random8(PADDING + 2, HEIGHT - PADDING - 2))] = CHSV(step - 32U, random8(128U, 255U), 255U);
      }
      if ((modes[currentMode].Speed < 25) & (low_br == 0)) {
        deltaValue = 0;
        if (x % 2 != 0) {
          gradientVertical( x - deltaValue, HEIGHT - PADDING, x + 1U - deltaValue, HEIGHT,  hue, hue + 2, 64U, 20U, 255U);
          gradientVertical( (WIDTH - x + deltaValue), 0U,  (WIDTH - x + 1U + deltaValue), PADDING,  hue, hue, 42U, 64U, 255U);
        }
        //        deltaValue = 0;
      }
    }
    if (x % STEP == 0) {
      // body --
      gradientVertical( x - deltaValue, HEIGHT - PADDING, x + 1U - deltaValue, HEIGHT,  hue, hue + 2, 255U, 20U, 255U);
      gradientVertical( (WIDTH - x + deltaValue), 0U,  (WIDTH - x + 1U + deltaValue), PADDING,  hue, hue, 42U, 255U, 255U);
    }
  }
  // ------

  deltaValue++;
  if (deltaValue >= STEP) {
    deltaValue = 0;
  }

  step++;
}

// ============ Octopus ===========
//        © Stepko and Sutaburosu
//    Adapted and modifed © alvikskor
//             Осьминог
// --------------------------------------
//Idea from https://www.youtube.com/watch?v=HsA-6KIbgto&ab_channel=GreatScott%21

void Octopus() {
    
  //FastLED.clear();
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random(10U, 101U), random(150U, 255U));
    }
#endif
    loadingFlag = false;
    for (int8_t x = -CENTER_X_MAJOR; x < CENTER_X_MAJOR + (WIDTH % 2); x++) {
      for (int8_t y = -CENTER_Y_MAJOR; y < CENTER_Y_MAJOR + (HEIGHT % 2); y++) {
        noise3d[0][x + CENTER_X_MAJOR][y + CENTER_Y_MAJOR] = (atan2(x, y) / PI) * 128 + 127; // thanks ldirko
        noise3d[1][x + CENTER_X_MAJOR][y + CENTER_Y_MAJOR] = hypot(x, y); // thanks Sutaburosu
      }
    }
  }
  
  uint8_t legs = modes[currentMode].Scale / 10;
  uint16_t color_speed;
  step = modes[currentMode].Scale % 10;
  if (step < 5) color_speed = scale / (3 - step/2);
  else color_speed = scale * (step/2 - 1);
  scale ++;
  for (uint8_t x = 0; x < WIDTH; x++) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      byte angle = noise3d[0][x][y];
      byte radius = noise3d[1][x][y];
      leds[XY(x, y)] = CHSV(color_speed - radius * (255 / WIDTH), 255,sin8(sin8((angle*4-(radius * (255 / WIDTH)))/4+scale) + radius * (255 / WIDTH) - scale*2 + angle * legs));
    }
  }
  //delay(255 - modes[currentMode].Speed);
}



// ============ Lotus Flower ============
//             © SlingMaster
//             Цветок Лотоса
//---------------------------------------
void drawLotusFlowerFragment(uint8_t posX, byte line) {
  const uint8_t h = (HEIGHT > 24) ? HEIGHT * 0.9 : HEIGHT;
  uint8_t flover_color = 128 + abs(128 - hue);                        // 128 -- 255
  uint8_t gleam = 255 - abs(128 - hue2);                              // 255 -- 128
  float f_size = (128 - abs(128 - deltaValue)) / 150.0;               // 1.0 -- 0.0
  const byte lowBri = 112U;
  // clear -----
  DrawLine(posX, 0, posX, h * 1.1, CRGB::Black);

  switch (line) {
    case 0:
      gradientVertical(posX, 0, posX + 1, h * 0.22, 96, 96, 32, 255, 255U);                             // green leaf c
      gradientVertical(posX, h * 0.9, posX + 1, h * 1.1, 64, 48, 64, 205, gleam);                       // pestle
      gradientVertical(posX, 8, posX + 1, h * 0.6, flover_color, flover_color, 128, lowBri, 255U);          // ---
      break;
    case 2:
    case 6:
      gradientVertical(posX, h * 0.2, posX + 1, h - 4, flover_color, flover_color, lowBri, 255, gleam);     //  -->
      gradientVertical(posX, h * 0.05, posX + 1, h * 0.15, 96, 96, 32, 255, 255U);                      // green leaf
      break;
    case 3:
    case 5:
      gradientVertical(posX, h * 0.5, posX + 1, h - 2, flover_color, flover_color, lowBri, 255, 255U);      // ---->
      break;
    case 4:
      gradientVertical(posX, 1 + h * f_size, posX + 1, h, flover_color, flover_color, lowBri, 255, gleam);  // ------>
      break;
    default:
      gradientVertical(posX, h * 0.05, posX + 1, h * 0.2, 80, 96, 160, 64, 255U);                       // green leaf m
      break;
  }
}

//---------------------------------------
void LotusFlower() {
  const byte STEP_OBJ = 8;
  static uint8_t deltaSpeed = 0;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), random8(1, 255U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    step = 0U;
    hue2 = 128U;
    deltaValue = 0;
    hue = 224;
    FPSdelay = SpeedFactor(160);
    FastLED.clear();
  }

  if (modes[currentMode].Speed > 128U) {
    if (modes[currentMode].Scale > 50) {
      deltaSpeed = 80U + (128U - abs(128U - deltaValue)) / 1.25;
      FPSdelay = SpeedFactor(deltaSpeed);
      if (step % 256 == 0U ) hue += 32;           /* color morph */
    } else {
      FPSdelay = SpeedFactor(160);
      hue = 28U;
    }
    deltaValue++;     /* size morph  */
    /* <==== scroll ===== */
    drawLotusFlowerFragment(WIDTH - 1, (step % STEP_OBJ));
    for (uint8_t y = 0U ; y < HEIGHT; y++) {
      for (uint8_t x = 0U ; x < WIDTH; x++) {
        drawPixelXY(x - 1, y,  getPixColorXY(x,  y));
      }
    }
  } else {
    /* <==== morph ===== */
    for (uint8_t x = 0U ; x < WIDTH; x++) {
      drawLotusFlowerFragment(x, (x % STEP_OBJ));
      if (x % 2U) {
        hue2++;         /* gleam morph */
      }
    }
    deltaValue++;       /* size morph  */
    if (modes[currentMode].Scale > 50) {
      hue += 8; /* color morph */
    } else {
      hue = 28U;
    }
  }
  step++;
}


// ============== Spindle ==============
//             © SlingMaster
//          adapted © alvikskor
//               Веретено
// =====================================
void Spindle() {
  static bool dark;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random8(1U, 100U), random8(100U, 255U));
    }
#endif
    loadingFlag = false;
    hue = random8(8) * 32; // modes[currentMode].Scale;
    hue2 = 255U;
    dark = modes[currentMode].Scale < 76U;
  }

  if  (modes[currentMode].Scale < 81) {
    blurScreen(128U);
  } else 
   if  (modes[currentMode].Scale < 86) {
    blurScreen(96U);
  } else 
  if  (modes[currentMode].Scale < 91) {
    blurScreen(64U);
  } else 
   if  (modes[currentMode].Scale < 96) {
    blurScreen(32U);
   }

  /* <==== scroll ===== */
  for (uint8_t y = 0U ; y < HEIGHT; y++) {
    for (uint8_t x = 0U ; x < WIDTH - 1; x++) {
      hue2--;
      if (dark) {   // black delimiter -----
        drawPixelXY(WIDTH - 1, y, CHSV(hue, 255, hue2));
      } else {      // white delimiter -----
        drawPixelXY(WIDTH - 1, y, CHSV(hue, 64 + hue2 / 2, 255 - hue2 / 4));
      }
      drawPixelXY(x, y,  getPixColorXY(x + 1,  y));
    }
  }
  if (modes[currentMode].Scale < 56) {

    return;
  }
  if (modes[currentMode].Scale < 61) {
    hue += 1;
  } else 
  if (modes[currentMode].Scale < 66) {
    hue += 2;
  } else 
  if (modes[currentMode].Scale < 71) {
    hue += 3;
  } else 
    if (modes[currentMode].Scale < 76) {
      hue += 4;
  } else {
      hue += 3;
    }
}

// =====================================
//           Rainbow Tornado
//  base code © Stepko, © Sutaburosu
//        and © SlingMaster
//   adapted and modifed © alvikskor
//              Торнадо
// =====================================

  const byte OFFSET = 1U;
  const uint8_t H = HEIGHT - OFFSET;
  //static uint8_t t;
  
void Tornado() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random8(100U, 255U), random8(20U, 100U));
    }
#endif
    //scale = 1;
    loadingFlag = 0;

    //FastLED.clear();
    for (int8_t x = -CENTER_X_MAJOR; x < CENTER_X_MAJOR; x++) {
      for (int8_t y = -OFFSET; y < H; y++) {
        noise3d[0][x + CENTER_X_MAJOR][y + OFFSET] = 128 * (atan2(y, x) / PI);
        noise3d[1][x + CENTER_X_MAJOR][y + OFFSET] = hypot(x, y);                    // thanks Sutaburosu
      }
    }
  }
  scale += modes[currentMode].Speed / 10;
  for (uint8_t x = 0; x < WIDTH; x++) {
    for (uint8_t y = 0; y < HEIGHT; y++) {
      byte angle = noise3d[0][x][y];
      byte radius = noise3d[1][x][y];
      leds[XY(x, y)] = CHSV((angle * modes[currentMode].Scale / 10) - scale + (radius * modes[currentMode].Scale / 10), constrain((uint16_t)y*512U/(uint16_t)HEIGHT,0,255), (y < (HEIGHT/8) ? 255 - (((HEIGHT/8) - y) * 16) : 255));
    }
  }
}

// ============ Plasma Waves ============
//              © Stepko
//        Adaptation © alvikskor
//             Плазменные Волны
// --------------------------------------

void Plasma_Waves() {
  static int64_t frameCount = 0;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), random8(40, 200U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    hue = modes[currentMode].Scale / 10;
  }
  //EVERY_N_MILLIS(modes[currentMode].Speed) {//(1000 / 60) {
  //  frameCount++;
  //}
  FPSdelay = 1;//64 - modes[currentMode].Speed / 4;

  frameCount++;
  uint8_t t1 = cos8((42 * frameCount) / (132 - modes[currentMode].Speed / 2));
  uint8_t t2 = cos8((35 * frameCount) / (132 - modes[currentMode].Speed / 2));
  uint8_t t3 = cos8((38 * frameCount) / (132 - modes[currentMode].Speed / 2));

  for (uint16_t y = 0; y < HEIGHT; y++) {
    for (uint16_t x = 0; x < WIDTH; x++) {
      // Calculate 3 seperate plasma waves, one for each color channel
      uint8_t r = cos8((x << 3) + (t1 >> 1) + cos8(t2 + (y << 3) + modes[currentMode].Scale));
      uint8_t g = cos8((y << 3) + t1 + cos8((t3 >> 2) + (x << 3)) +modes[currentMode].Scale);
      uint8_t b = cos8((y << 3) + t2 + cos8(t1 + x + (g >> 2) + modes[currentMode].Scale));

      // uncomment the following to enable gamma correction
      // r = pgm_read_byte_near(exp_gamma + r);
      switch (hue) {
          case 0:
              r = pgm_read_byte(&exp_gamma[r]);
              g = pgm_read_byte(&exp_gamma[g]);
              b = pgm_read_byte(&exp_gamma[b]);
              break;
          case 1:
              r = pgm_read_byte(&exp_gamma[r]);
              b = pgm_read_byte(&exp_gamma[g]);
              g = pgm_read_byte(&exp_gamma[b]);
              break;
          case 2:
              g = pgm_read_byte(&exp_gamma[r]);
              r = pgm_read_byte(&exp_gamma[g]);
              b = pgm_read_byte(&exp_gamma[b]);
              break;
          case 3:
              r = pgm_read_byte(&exp_gamma[r])/2;
              g = pgm_read_byte(&exp_gamma[g]);
              b = pgm_read_byte(&exp_gamma[b]);
              break;
          case 4:
              r = pgm_read_byte(&exp_gamma[r]);
              g = pgm_read_byte(&exp_gamma[g])/2;
              b = pgm_read_byte(&exp_gamma[b]);
              break;
          case 5:
              r = pgm_read_byte(&exp_gamma[r]);
              g = pgm_read_byte(&exp_gamma[g]);
              b = pgm_read_byte(&exp_gamma[b])/2;
              break;
          case 6:
              r = pgm_read_byte(&exp_gamma[r])*3;
              g = pgm_read_byte(&exp_gamma[g]);
              b = pgm_read_byte(&exp_gamma[b]);
              break;
          case 7:
              r = pgm_read_byte(&exp_gamma[r]);
              g = pgm_read_byte(&exp_gamma[g])*3;
              b = pgm_read_byte(&exp_gamma[b]);
              break;
          case 8:
              r = pgm_read_byte(&exp_gamma[r]);
              g = pgm_read_byte(&exp_gamma[g]);
              b = pgm_read_byte(&exp_gamma[b])*3;
              break;

      }
      // g = pgm_read_byte_near(exp_gamma + g);
      // b = pgm_read_byte_near(exp_gamma + b);

      leds[XY(x, y)] = CRGB(r, g, b);
    }
    //hue++;
  }
  // blurScreen(beatsin8(3, 64, 80));
}



// ============ Colored Python ============
//      base code WavingCell from © Stepko
//       Adaptation & modefed © alvikskor
//            Цветной Питон
// --------------------------------------

uint32_t color_timer = millis();

void Colored_Python() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
      if (selectedSettings) {
          //                     scale | speed
          setModeSettings(random8(100U), random8(1, 255U));
      }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
      loadingFlag = false;
      step = 0;
  }
  uint16_t  t = millis() / (128 - (modes[currentMode].Speed / 2));
  uint8_t palette_number = modes[currentMode].Scale / 10;
  uint8_t thickness;
  if (palette_number < 9)
      step = palette_number;
  else
      if (millis()- color_timer > 30000){
        color_timer = millis();
        step++;
        if(step > 8) step = 0;
      }
  switch (step) {
      case 0: currentPalette = CloudColors_p; break;
      case 1: currentPalette = AlcoholFireColors_p; break;
      case 2: currentPalette = OceanColors_p; break;
      case 3: currentPalette = ForestColors_p; break;
      case 4: currentPalette = RainbowColors_p; break;
      case 5: currentPalette = RainbowStripeColors_p; break;
      case 6: currentPalette = HeatColors_p; break;
      case 7: currentPalette = LavaColors_p; break;
      case 8: currentPalette = PartyColors_p;
    }
  switch (modes[currentMode].Scale % 5){
      case 0: thickness = 5; break;
      case 1: thickness = 10; break;
      case 2: thickness = 20; break;
      case 3: thickness = 30; break;
      case 4: thickness = 40; break;
  }
for(byte x =0; x < WIDTH; x++){
  for(byte y =0; y < HEIGHT; y++){
    leds[XY(x,y)]=ColorFromPalette(currentPalette,((sin8((x*thickness)+sin8(y*5+t*5))+cos8(y*10))+1)+t*(modes[currentMode].Speed%10)); //HeatColors_p -палитра, t*scale/10 -меняет скорость движения вверх, sin8(x*20) -меняет ширину рисунка
}}
}

// =====================================
//                Stars
//     © SottNick and  © Stepko
//      Adaptation © SlingMaster
//                Звезды
// =====================================
void drawStar(float xlocl, float ylocl, float biggy, float little, int16_t points, float dangle, uint8_t koler) { // random multipoint star
  float radius2 = 255.0 / points;
  for (int i = 0; i < points; i++) {
    DrawLine(xlocl + ((little * (sin8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), ylocl + ((little * (cos8(i * radius2 + radius2 / 2 - dangle) - 128.0)) / 128), xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), ColorFromPalette(*curPalette, koler));
    DrawLine(xlocl + ((little * (sin8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), ylocl + ((little * (cos8(i * radius2 - radius2 / 2 - dangle) - 128.0)) / 128), xlocl + ((biggy * (sin8(i * radius2 - dangle) - 128.0)) / 128), ylocl + ((biggy * (cos8(i * radius2 - dangle) - 128.0)) / 128), ColorFromPalette(*curPalette, koler));

  }
}

// --------------------------------------
void EffectStars() {
#define STARS_NUM (8U)
#define STAR_BLENDER (128U)
#define CENTER_DRIFT_SPEED (6U)
  static uint8_t spd;
  static uint8_t points[STARS_NUM];
  static float color[STARS_NUM] ;
  static int delay_arr[STARS_NUM];
  static float counter;
  static float driftx;
  static float  drifty;
  static float cangle;
  static float  sangle;
  static uint8_t stars_count;
  static uint8_t blur;

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), random8(80U, 255U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    counter = 0.0;
    // стартуем с центра
    driftx = (float)WIDTH / 2.0;
    drifty = (float)HEIGHT / 2.0;

    cangle = (float)(sin8(random8(25, 220)) - 128.0f) / 128.0f; //angle of movement for the center of animation gives a float value between -1 and 1
    sangle = (float)(sin8(random8(25, 220)) - 128.0f) / 128.0f; //angle of movement for the center of animation in the y direction gives a float value between -1 and 1
    spd = modes[currentMode].Speed;
    blur = modes[currentMode].Scale / 2;
    stars_count = WIDTH / 2U;

    if (stars_count > STARS_NUM) stars_count = STARS_NUM;
    for (uint8_t num = 0; num < stars_count; num++) {
      points[num] = map(modes[currentMode].Scale, 1, 255, 3U, 7U); //5; // random8(3, 6);                              // количество углов в звезде
      delay_arr[num] = spd / 5 + (num << 2) + 2U;               // задержка следующего пуска звезды
      color[num] = random8();
    }
  }
  // fadeToBlackBy(leds, NUM_LEDS, 245);
  fadeToBlackBy(leds, NUM_LEDS, 165);
  float speedFactor = ((float)spd / 380.0 + 0.05);
  counter += speedFactor;                                                   // определяет то, с какой скоростью будет приближаться звезда

  if (driftx > (WIDTH - spirocenterX / 2U)) cangle = 0 - fabs(cangle);      //change directin of drift if you get near the right 1/4 of the screen
  if (driftx < spirocenterX / 2U) cangle = fabs(cangle);                    //change directin of drift if you get near the right 1/4 of the screen
  if ((uint16_t)counter % CENTER_DRIFT_SPEED == 0) driftx = driftx + (cangle * speedFactor); //move the x center every so often
  if (drifty > ( HEIGHT - spirocenterY / 2U)) sangle = 0 - fabs(sangle);    // if y gets too big, reverse
  if (drifty < spirocenterY / 2U) sangle = fabs(sangle);                    // if y gets too small reverse

  if ((uint16_t)counter % CENTER_DRIFT_SPEED == 0) drifty = drifty + (sangle * speedFactor); //move the y center every so often

  for (uint8_t num = 0; num < stars_count; num++) {
    if (counter >= delay_arr[num]) {              //(counter >= ringdelay)
      if (counter - delay_arr[num] <= WIDTH + 5) {
        drawStar(driftx, drifty, 2 * (counter - delay_arr[num]), (counter - delay_arr[num]), points[num], STAR_BLENDER + color[num], color[num]);
        color[num] += speedFactor;                // в зависимости от знака - направление вращения
      } else {
        delay_arr[num] = counter + (stars_count << 1) + 1U; // задержка следующего пуска звезды
      }
    }
  }
  blur2d(leds, WIDTH, HEIGHT, blur);
}

// =====================================
//             Планета Земля
//              PlanetEarth
//             © SlingMaster
// =====================================
void PlanetEarth() {
  static uint16_t imgW = 0;
  static uint16_t imgH = 0;
  if (HEIGHT < 16U) {
    return;
  }
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(128U, 10U + random8(230U));
    }
#endif
    loadingFlag = false;
    FPSdelay = 96U;
    FastLED.clear();
    String file_name = (modes[currentMode].Scale < 50) ? "globe0" : (HEIGHT >= 24U) ? "globe_big" : "globe1";
    readBinFile("bin/" + file_name + ".img", 4112 );

    imgW = getSizeValue(binImage, 8 );
    imgH = getSizeValue(binImage, 10 );

#ifdef GENERAL_DEBUG
    LOG.printf_P(PSTR("Image • %03d x %02d px\n"), imgW, imgH);
#endif
    scrollImage(imgW, imgH, 0U);
    ff_x = 1U;
  }

  /* scrool index reverse --> */
  if (ff_x < 1) ff_x = (imgW - imgH);
  scrollImage(imgW, imgH, ff_x - 1);
  ff_x--;
  //  if (ff_x < 1) ff_x = (imgW - 1);
  if (ff_x == 0) {
    scrollImage(imgW, imgH, 0U);
    ff_x = imgW;
  } else {
    scrollImage(imgW, imgH, ff_x);
  }

  /* <-- scrool index ------- */
  //  if (ff_x > (imgW - imgH)) ff_x = 1U;
  //  scrollImage(imgW, imgH, ff_x - 1);
  //  ff_x++;
}

// =============== Bamboo ===============
//             © SlingMaster
//                 Бамбук
// --------------------------------------
uint8_t nextColor(uint8_t posY, uint8_t base, uint8_t next ) {
  const byte posLine = (HEIGHT > 16) ? 4 : 3;
  if ((posY + 1 == posLine) | (posY == posLine)) {
    return next;
  } else {
    return base;
  }
}

// --------------------------------------
void Bamboo() {
  const uint8_t gamma[7] = {0, 32, 144, 160, 196, 208, 230};
  static float index;
  const byte DELTA = 4U;
  const uint8_t VG_STEP = 64U;
  const uint8_t V_STEP = 32U;
  const byte posLine = (HEIGHT > 16) ? 4 : 3;
  const uint8_t SX = 5;
  const uint8_t SY = 10;
  static float deltaX = 0;
  static bool direct = false;
  uint8_t posY;
  static uint8_t colLine;
  const float STP = 0.2;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), random8(128, 255U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    index = STP;
    uint8_t idx = map(modes[currentMode].Scale, 5, 95, 0U, 6U);;
    colLine = gamma[idx];
    step = 0U;
  }

  // *** ---
  for (int y = 0; y < HEIGHT + SY; y++) {
    if (modes[currentMode].Scale < 50U) {
      if (step % 128 == 0U) {
        deltaX += STP * ((direct) ? -1 : 1);
        if ((deltaX > 1) | (deltaX < -1)) direct = !direct;
      }
    } else {
      deltaX = 0;
    }
    posY = y;
    for (int x = 0; x < WIDTH + SX; x++) {
      if (y == posLine) {
        drawPixelXYF(x , y - 1, CHSV(colLine, 255U, 128U));
        drawPixelXYF(x, y, CHSV(colLine, 255U, 96U));
        if (HEIGHT > 16) {
          drawPixelXYF(x, y - 2, CHSV(colLine, 10U, 64U));
        }
      }
      if ((x % SX == 0U) & (y % SY == 0U)) {
        for (int i = 1; i < (SY - 3); i++) {
          if (i < 3) {
            posY = y - i + 1 - DELTA + index;
            drawPixelXYF(x - 3 + deltaX, posY, CHSV(nextColor(posY, 96, colLine), 255U, 255 - V_STEP * i));
            posY = y - i + index;
            drawPixelXYF(x + deltaX, posY, CHSV(nextColor(posY, 96, colLine), 255U, 255 - VG_STEP * i));
          }
          posY = y - i - DELTA + index;
          drawPixelXYF(x - 4 + deltaX, posY , CHSV(nextColor(posY, 96, colLine), 180U, 255 - V_STEP * i));
          posY = y - i + 1 + index;
          drawPixelXYF(x - 1 + deltaX, posY , CHSV(nextColor(posY, ((i == 1) ? 96 : 80), colLine), 255U, 255 - V_STEP * i));
        }
      }
    }
    step++;
  }
  if (index >= SY)  {
    index = 0;
  }
  fadeToBlackBy(leds, NUM_LEDS, 60);
  index += STP;
}


// =====================================
//     Multicolored Dandelions
//      Base Code © Less Lam
//          © SlingMaster
//       Разноцветные одуванчики
// https://editor.soulmatelights.com/gallery/2007-amber-rain
// =====================================
class Circle {
  public:
    float thickness = 3.0;
    long startTime;
    uint16_t offset;
    int16_t centerX;
    int16_t centerY;
    int hue;
    int bpm = 10;

    void move() {
      centerX = random(0, WIDTH);
      centerY = random(0, HEIGHT);
    }

    void scroll() {
      centerX--; // = random(0, WIDTH);
      if (centerX < 1) {
        centerX = WIDTH - 1;
      }
      centerY++;
      if (centerY > HEIGHT) {
        centerY = 0;
      }
    }
    void reset() {
      startTime = millis();
      centerX = random(0, WIDTH);
      centerY = random(0, HEIGHT);
      hue = random(0, 255);
      offset = random(0, 60000 / bpm);
    }

    float radius() {
      float radius = beatsin16(modes[currentMode].Speed / 2.5, 0, 500, offset) / 100.0;
      return radius;
    }
};

// -----------------------------------
namespace Circles {
#define NUMBER_OF_CIRCLES WIDTH/2
Circle circles[NUMBER_OF_CIRCLES] = {};

void drawCircle(Circle circle) {
  int16_t centerX = circle.centerX;
  int16_t centerY = circle.centerY;
  int hue = circle.hue;
  float radius = circle.radius();

  int16_t startX = centerX - ceil(radius);
  int16_t endX = centerX + ceil(radius);
  int16_t startY = centerY - ceil(radius);
  int16_t endY = centerY + ceil(radius);

  for (int16_t x = startX; x < endX; x++) {
    for (int16_t y = startY; y < endY; y++) {
      int16_t index = XY(x, y);
      if (index < 0 || index > NUM_LEDS)
        continue;
      double distance = sqrt(sq(x - centerX) + sq(y - centerY));
      if (distance > radius)
        continue;

      uint16_t brightness;
      if (radius < 1) { // last pixel
        // brightness = 0; //255.0 * radius;
        deltaValue = 20;
        brightness = 180;
        // brightness = 0;
      } else {
        deltaValue = 200; // 155 + modes[currentMode].Scale;
        double percentage = distance / radius;
        double fraction = 1.0 - percentage;
        brightness = 255.0 * fraction;
      }
      leds[index] += CHSV(hue, deltaValue, brightness);
    }
  }
}

// -----------------------------
void draw(bool setup) {
  fadeToBlackBy(leds, NUM_LEDS, 100U);
  // fillAll(CRGB::Black);
  for (int i = 0; i < NUMBER_OF_CIRCLES; i++) {
    if (setup) {
      circles[i].reset();
    } else {
      if (circles[i].radius() < 0.5) {
        circles[i].scroll();
      }
    }
    drawCircle(circles[i]);
  }
}
}; // namespace Circles

// ==============
void Dandelions() {
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      // scale | speed
      setModeSettings(random8(1U, 100U), random8(10U, 255U));
    }
#endif
    loadingFlag = false;
    FastLED.clear();
    Circles::draw(true);
    // deltaValue = 150 + modes[currentMode].Scale;
    deltaValue = 155 + modes[currentMode].Scale;
  }

  // FPSdelay = SOFT_DELAY;
  Circles::draw(false);
}

// ======== Digital Тurbulence =========
//             © SlingMaster
//        Цифровая турбулентность
// =====================================
void drawRandomCol(uint8_t x, uint8_t y, uint8_t offset, uint32_t count) {
  const byte STEP = 32;
  const byte D = HEIGHT / 8;
  uint8_t color = floor(y / D) * STEP + offset;

  if (count == 0U) {
    drawPixelXY(x, y, CHSV(color, 255, random8(8U) == 0U ? (step % 2U ? 0 : 255) : 0));
  } else {
    drawPixelXY(x, y, CHSV(color, 255, (bitRead(count, y ) == 1U) ? (step % 5U ? 0 : 255) : 0));
  }
}

//---------------------------------------
void Turbulence() {
  const byte STEP_COLOR = 255 / HEIGHT;
  const byte STEP_OBJ = 8;
  const byte DEPTH = 2;
  static uint32_t count; // 16777216; = 65536
  uint32_t curColor;
  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      //                     scale | speed
      setModeSettings(random8(100U), random8(1, 255U));
    }
#endif //#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    loadingFlag = false;
    step = 0U;
    deltaValue = 0;
    hue = 0;
    if (modes[currentMode].Speed < 20U) {
      FPSdelay = SpeedFactor(30);
    }
    FastLED.clear();
  }

  deltaValue++;     /* size morph  */

  /* <==== scroll =====> */
  for (uint8_t y = HEIGHT; y > 0; y--) {
    drawRandomCol(0, y - 1, hue, count);
    drawRandomCol(WIDTH - 1, y - 1, hue + 128U, count);

    // left -----
    for (uint8_t x = CENTER_X_MAJOR - 1; x > 0; x--) {
      if (x > CENTER_X_MAJOR) {
        if (random8(2) == 0U) { /* scroll up */
          CRGB newColor = getPixColorXY(x, y - 1 );
        }
      }

      /* ---> */
      curColor = getPixColorXY(x - 1, y - 1);
      if (x < CENTER_X_MAJOR - DEPTH / 2) {
        drawPixelXY(x, y - 1, curColor);
      } else {
        if (curColor != 0U) drawPixelXY(x, y - 1, curColor);
      }
    }

    // right -----
    for (uint8_t x = CENTER_X_MAJOR + 1; x < WIDTH; x++) {
      if (x < CENTER_X_MAJOR + DEPTH ) {
        if (random8(2) == 0U)  {  /* scroll up */
          CRGB newColor = getPixColorXY(x, y - 1 );
        }
      }
      /* <---  */
      curColor = getPixColorXY(x, y - 1);
      if (x > CENTER_X_MAJOR + DEPTH / 2 ) {
        drawPixelXY(x - 1, y - 1, curColor);
      } else {
        if (curColor != 0U) drawPixelXY(x - 1, y - 1, curColor);
      }
    }

    /* scroll center up ---- */
    for (uint8_t x = CENTER_X_MAJOR - DEPTH; x < CENTER_X_MAJOR + DEPTH; x++) {
      drawPixelXY(x, y,  makeDarker(getPixColorXY(x, y - 1 ), 128 / y));
      if (y == 1) {
        drawPixelXY(x, 0, CRGB::Black);
      }
    }
    /* --------------------- */
  }

  if (modes[currentMode].Scale > 50) {
    count++;
    if (count % 256 == 0U) hue += 16U;
  } else {
    count = 0;
  }
  step++;
}

// ============ Serpentine =============
//             © SlingMaster
//              Серпантин
// =====================================
void Serpentine() {
  const byte PADDING = HEIGHT * 0.25;
  const byte BR_INTERWAL = 64 / HEIGHT;
  const byte DELTA = WIDTH  * 0.25;
  // ---------------------

  if (loadingFlag) {
#if defined(USE_RANDOM_SETS_IN_APP) || defined(RANDOM_SETTINGS_IN_CYCLE_MODE)
    if (selectedSettings) {
      setModeSettings(random8(4, 50), random8(4, 254U));
    }
#endif
    loadingFlag = false;
    deltaValue = 0;
    hue = 0;
    FastLED.clear();
  }
  // ---------------------

  byte step1 = map8(modes[currentMode].Speed, 10U, 60U);
  uint16_t ms = millis();
  double freq = 3000;
  float mn = 255.0 / 13.8;
  byte fade = 180 - abs(128 - step);
  fadeToBlackBy(leds, NUM_LEDS, fade);

  // -----------------
  for (uint16_t y = 0; y < HEIGHT; y++) {
    uint32_t yy = y * 256;
    uint32_t x1 = beatsin16(step1, WIDTH, (HEIGHT - 1) * 256, WIDTH, y * freq + 32768) / 2;

    // change color --------
    // CRGB col1 = CHSV(ms / 29 + y * 255 / (HEIGHT - 1) + 128, 255 - abs(128 - step)/4, qadd8(hue, beatsin8(step1, 60, 255U, 0, y * mn + 128)));
    // CRGB col3 = CHSV(ms / 29 + y * 255 / (HEIGHT - 1), 255, qadd8(hue, beatsin8(step1, 60, 255U, 0, y * mn + 128)) );
    CRGB col1 = CHSV(ms / 29 + y * 256 / (HEIGHT - 1) + 128, 255, 255 - (HEIGHT - y) * BR_INTERWAL);
    CRGB col2 = CHSV(ms / 29 + y * 256 / (HEIGHT - 1), 255, 255 - (HEIGHT - y) * BR_INTERWAL);
    // CRGB col3 = CHSV(ms / 29 + y * 256 / (HEIGHT - 1) + step, 255, 255 - (HEIGHT - y) * BR_INTERWAL - fade);

    wu_pixel( x1 + hue * DELTA, yy - PADDING * (255 - hue), &col1);
    wu_pixel( abs((WIDTH - 1) * 256 - (x1 + hue * DELTA)), yy - PADDING * hue, &col2);
    //    wu_pixel( x1 + hue * DELTA, yy - PADDING * 3 * (255 - hue), &col3);
  }

  step++;
  if (step % 64) {
    if (deltaValue == 0) {
      hue++;
      if (hue >= 255) {
        deltaValue = 1;
      }
    } else {
      hue--;
      if (hue < 1) {
        deltaValue = 0;
      }
    }
  }
}
