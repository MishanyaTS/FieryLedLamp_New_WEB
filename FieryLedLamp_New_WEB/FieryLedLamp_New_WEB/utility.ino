// служебные функции

// залить все
void fillAll(CRGB color)
{
  for (int16_t i = 0; i < NUM_LEDS; i++)
    leds[i] = color;
}

// функция отрисовки точки по координатам X Y
void drawPixelXY(int8_t x, int8_t y, CRGB color)
{
  if (x < 0 || x > (WIDTH - 1) || y < 0 || y > (HEIGHT - 1)) return;
  uint32_t thisPixel = XY((uint8_t)x, (uint8_t)y) * SEGMENTS;
  for (uint8_t i = 0; i < SEGMENTS; i++)
  {
    leds[thisPixel + i] = color;
  }
}

// функция получения цвета пикселя по его номеру
uint32_t getPixColor(uint32_t thisSegm)
{
  uint32_t thisPixel = thisSegm * SEGMENTS;
  if (thisPixel > NUM_LEDS - 1) return 0;
  return (((uint32_t)leds[thisPixel].r << 16) | ((uint32_t)leds[thisPixel].g << 8 ) | (uint32_t)leds[thisPixel].b); // а почему не просто return (leds[thisPixel])?
}

// функция получения цвета пикселя в матрице по его координатам
uint32_t getPixColorXY(uint8_t x, uint8_t y)
{
  return getPixColor(XY(x, y));
}

// ************* НАСТРОЙКА МАТРИЦЫ *****
/*
#if (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y

#elif (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y x

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y x

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y (WIDTH - x - 1)

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y y

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y (WIDTH - x - 1)

#else
!!!!!!!!!!!!!!!!!!!!!!!!!!!   смотрите инструкцию: https://alexgyver.ru/wp-content/uploads/2018/11/scheme3.jpg
!!!!!!!!!!!!!!!!!!!!!!!!!!!   такого сочетания CONNECTION_ANGLE и STRIP_DIRECTION не бывает
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y
#pragma message "Wrong matrix parameters! Set to default"

#endif
*/
// получить номер пикселя в ленте по координатам
// библиотека FastLED тоже использует эту функцию
uint16_t XY(uint8_t x, uint8_t y)
{
 uint8_t THIS_X;
 uint8_t THIS_Y;
 uint8_t _WIDTH = WIDTH;
 
 switch (ORIENTATION)
 {
  case 0: THIS_X = x;                   //CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 0
          THIS_Y =y;
          break;
  case 1: _WIDTH = HEIGHT;              //CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 1
          THIS_X = y;
          THIS_Y = x;
          break;
  case 2: THIS_X = x;                   //CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 0
          THIS_Y = (HEIGHT - y - 1);
          break;
  case 3: _WIDTH = HEIGHT;              //CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3
          THIS_X = (HEIGHT - y - 1);
          THIS_Y = x;
          break;
  case 4: THIS_X = (WIDTH - x - 1);     //CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 2
          THIS_Y = (HEIGHT - y - 1);
          break;
  case 5: _WIDTH = HEIGHT;              //CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 3
          THIS_X = (HEIGHT - y - 1);
          THIS_Y = (WIDTH - x - 1);
          break;
  case 6: THIS_X = (WIDTH - x - 1);     //CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 2
          THIS_Y =y;
          break;
  case 7: _WIDTH = HEIGHT;              //CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 1
          THIS_X = y;
          THIS_Y = (WIDTH - x - 1);
          break;
  default : THIS_X = x;                 // !! смотрите инструкцию: https://alexgyver.ru/wp-content/uploads/2018/11/scheme3.jpg
            THIS_Y =y;                  // !! такого сочетания CONNECTION_ANGLE и STRIP_DIRECTION не бывает
            break;
 }
 
  if (!(THIS_Y & 0x01) || MATRIX_TYPE)               // Even rows run forwards
    return (THIS_Y * _WIDTH + THIS_X);
  else                                                  
    return (THIS_Y * _WIDTH + _WIDTH - THIS_X - 1);  // Odd rows run backwards
}

// если у вас матрица необычной формы с зазорами/вырезами, либо просто маленькая, тогда вам придётся переписать функцию XY() под себя
// массив для переадресации можно сформировать на этом онлайн-сервисе: https://macetech.github.io/FastLED-XY-Map-Generator/
// или ту по-русски: https://firelamp.pp.ua/matrix_generator/

// ниже пример функции, когда у вас матрица 8х16, а вы хотите, чтобы эффекты рисовались, будто бы матрица 16х16 (рисуем по центру, а по бокам обрезано)
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  Х  Х  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  8  9  Х  Х  Х  Х  Х  Х  -  -  -  - 
//   -  -  -  -  7  6  5  4  3  2  1  0  -  -  -  -
/*
uint8_t XY (uint8_t x, uint8_t y) {
  // any out of bounds address maps to the first hidden pixel
  if ( (x >= 16) || (y >= 16) ) {
    return (128); //(LAST_VISIBLE_LED + 1);
  }

  const uint8_t XYTable[] = {
   248, 249, 250, 251, 120, 121, 122, 123, 124, 125, 126, 127, 252, 253, 254, 255,
   247, 246, 245, 244, 119, 118, 117, 116, 115, 114, 113, 112, 243, 242, 241, 240,
   232, 233, 234, 235, 104, 105, 106, 107, 108, 109, 110, 111, 236, 237, 238, 239,
   231, 230, 229, 228, 103, 102, 101, 100,  99,  98,  97,  96, 227, 226, 225, 224,
   216, 217, 218, 219,  88,  89,  90,  91,  92,  93,  94,  95, 220, 221, 222, 223,
   215, 214, 213, 212,  87,  86,  85,  84,  83,  82,  81,  80, 211, 210, 209, 208,
   200, 201, 202, 203,  72,  73,  74,  75,  76,  77,  78,  79, 204, 205, 206, 207,
   199, 198, 197, 196,  71,  70,  69,  68,  67,  66,  65,  64, 195, 194, 193, 192,
   184, 185, 186, 187,  56,  57,  58,  59,  60,  61,  62,  63, 188, 189, 190, 191,
   183, 182, 181, 180,  55,  54,  53,  52,  51,  50,  49,  48, 179, 178, 177, 176,
   168, 169, 170, 171,  40,  41,  42,  43,  44,  45,  46,  47, 172, 173, 174, 175,
   167, 166, 165, 164,  39,  38,  37,  36,  35,  34,  33,  32, 163, 162, 161, 160,
   152, 153, 154, 155,  24,  25,  26,  27,  28,  29,  30,  31, 156, 157, 158, 159,
   151, 150, 149, 148,  23,  22,  21,  20,  19,  18,  17,  16, 147, 146, 145, 144,
   136, 137, 138, 139,   8,   9,  10,  11,  12,  13,  14,  15, 140, 141, 142, 143,
   135, 134, 133, 132,   7,   6,   5,   4,   3,   2,   1,   0, 131, 130, 129, 128
  };

  uint8_t i = (y * 16) + x;
  return XYTable[i];
}
*/

// оставлено для совместимости со эффектами из старых прошивок
uint16_t getPixelNumber(uint8_t x, uint8_t y)
{
  return XY(x, y);
}


// восстановление настроек эффектов на настройки по умолчанию
void restoreSettings()
{
    for (uint8_t i = 0; i < MODE_AMOUNT; i++) {
      modes[i].Brightness = pgm_read_byte(&defaultSettings[i][0]);
      modes[i].Speed      = pgm_read_byte(&defaultSettings[i][1]);
      modes[i].Scale      = pgm_read_byte(&defaultSettings[i][2]);
    }
}

// неточный, зато более быстрый квадратный корень
float sqrt3(const float x)
{
  union
  {
    int i;
    float x;
  } u;

  u.x = x;
  u.i = (1<<29) + (u.i >> 1) - (1<<22);
  return u.x;
}
