// Если вы хотите добавить эффекты или сделать им копии для демонстрации на разных настройках, нужно делать это в 5 местах:
// 1. в файле effects.ino - добавляется программный код самого эффекта.
// 2. в файле Constants.h - придумываются названия "EFF_......" и задаются порядковые номера эффектам. В конце указывается общее количество MODE_AMOUNT.
// 3. там же в файле Constants.h ниже - задаётся Реестр эффектов для передачи в приложение. 
//    Он живёт отдельно.  Если у вас приложение не поддерживает запрос списка эффектов у лампы, реестр можно не менять.
// 4. там же в файле Constants.h ещё ниже - задаётся Массив настроек эффектов по умолчанию.
//    Просто добавьте строчку для своего нового эффекта в нужное место. Это тоже не обязательно.
// 5. здесь в файле effectTicker.ino - подключается процедура вызова эффекта на соответствующий ей "EFF_......"
//    Можно подключать один и тот же эффект под разными номерами. Например: EFF_FIRE (24U), EFF_FIRE2 (25U), EFF_FIRE3 (26U). Будет три огня для разных цветов.
// Для удобства изменения всех этих списков и настроек в архиве с прошивкой есть файл "таблица_эффектов.xls". 
// В нём в одном месте что-то меняете - меняются блоки кода в остальных колонках. Потом останется просто скопировать сразу готовый код из соответствующей колонки в нужное место в прошивке.

uint32_t effTimer;

void effectsTick()
{
  if (!dawnFlag)
  {
    // ------------------------------------- у эффектов до EFF_MATRIX (все перед Матрицей) бегунок Скорость не регулирует задержку между кадрами
    if (ONflag )
    {
      switch (currentMode)
      {

        case EFF_WHITE_COLOR:         HIGH_DELAY_TICK { effTimer = millis(); whiteColorStripeRoutine();    Eff_Tick (); }  break;  // (  0U) Бeлый cвeт
        case EFF_WATERCOLOR:          DYNAMIC_DELAY_TICK { effTimer = millis(); Watercolor();                 Eff_Tick (); }  break;  // (  1U) Акварель
        case EFF_EFF_POOL:            DYNAMIC_DELAY_TICK { effTimer = millis(); poolRoutine();                Eff_Tick (); }  break;  // (  2U) Бассейн
        case EFF_MADNESS:             HIGH_DELAY_TICK { effTimer = millis(); madnessNoiseRoutine();        Eff_Tick (); }  break;  // (  3U) Безумие
        case EFF_WATERFALL:           DYNAMIC_DELAY_TICK { effTimer = millis(); fire2012WithPalette();        Eff_Tick (); }  break;  // (  4U) Водопад
        case EFF_WATERFALL_4IN1:      DYNAMIC_DELAY_TICK { effTimer = millis(); fire2012WithPalette4in1();    Eff_Tick (); }  break;  // (  5U) Водопад 4в1
        case EFF_WAVES:               DYNAMIC_DELAY_TICK { effTimer = millis(); WaveRoutine();                Eff_Tick (); }  break;  // (  6U) Волны
        case EFF_MAGICLANTERN:        DYNAMIC_DELAY_TICK { effTimer = millis(); MagicLantern();               Eff_Tick (); }  break;  // (  7U) Волшебный Фонарик
        case EFF_WINE:                DYNAMIC_DELAY_TICK { effTimer = millis(); colorsWine();                 Eff_Tick (); }  break;  // (  8U) Вино
        case EFF_WHIRL:               DYNAMIC_DELAY_TICK { effTimer = millis(); whirlRoutine(true);           Eff_Tick (); }  break;  // (  9U) Вихри пламени
        case EFF_WHIRL_MULTI:         DYNAMIC_DELAY_TICK { effTimer = millis(); whirlRoutine(false);          Eff_Tick (); }  break;  // ( 10U) Вихри разноцветные
        case EFF_STARFALL:            DYNAMIC_DELAY_TICK { effTimer = millis(); stormRoutine2();              Eff_Tick (); }  break;  // ( 11U) Вьюга
        case EFF_STORMY_RAIN:         DYNAMIC_DELAY_TICK { effTimer = millis(); stormyRain();                 Eff_Tick (); }  break;  // ( 12U) Гроза в банке
        case EFF_DNA:                 LOW_DELAY_TICK { effTimer = millis(); DNARoutine();                 Eff_Tick (); }  break;  // ( 13U) ДНК
        case EFF_SMOKE:               DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStreamSmoke(false);   Eff_Tick (); }  break;  // ( 14U) Дым
        case EFF_SMOKE_COLOR:         DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStreamSmoke(true);    Eff_Tick (); }  break;  // ( 15U) Дым разноцветный
        case EFF_SMOKEBALLS:          LOW_DELAY_TICK { effTimer = millis(); smokeballsRoutine();          Eff_Tick (); }  break;  // ( 16U) Дымовые шашки
        case EFF_LIQUIDLAMP:          LOW_DELAY_TICK { effTimer = millis(); LiquidLampRoutine(true);      Eff_Tick (); }  break;  // ( 17U) Жидкая лампа
        case EFF_LIQUIDLAMP_AUTO:     LOW_DELAY_TICK { effTimer = millis(); LiquidLampRoutine(false);     Eff_Tick (); }  break;  // ( 18U) Жидкая лампа авто
        case EFF_SWIRL:               DYNAMIC_DELAY_TICK { effTimer = millis(); Swirl();                      Eff_Tick (); }  break;  // ( 19U) Завиток
        case EFF_STARS:               DYNAMIC_DELAY_TICK { effTimer = millis(); EffectStars();                 Eff_Tick (); }  break;  // ( 20U) Звезды
        case EFF_ZEBRA:               HIGH_DELAY_TICK { effTimer = millis(); zebraNoiseRoutine();          Eff_Tick (); }  break;  // ( 21U) Зебра
        case EFF_SNAKES:              LOW_DELAY_TICK { effTimer = millis(); snakesRoutine();              Eff_Tick (); }  break;  // ( 22U) Змейки
        case EFF_FOUNTAIN:            DYNAMIC_DELAY_TICK { effTimer = millis(); starfield2Routine();          Eff_Tick (); }  break;  // ( 23U) Источник
        case EFF_DROPS:               LOW_DELAY_TICK { effTimer = millis(); newMatrixRoutine();           Eff_Tick (); }  break;  // ( 24U) Капли на стекле
        case EFF_LLAND:               DYNAMIC_DELAY_TICK { effTimer = millis(); LLandRoutine();               Eff_Tick (); }  break;  // ( 25U) Кипение
        case EFF_RINGS:               DYNAMIC_DELAY_TICK { effTimer = millis(); ringsRoutine();               Eff_Tick (); }  break;  // ( 26U) Кодовый замок
        case EFF_COMET:               DYNAMIC_DELAY_TICK { effTimer = millis(); RainbowCometRoutine();        Eff_Tick (); }  break;  // ( 27U) Комета
        case EFF_COMET_COLOR:         DYNAMIC_DELAY_TICK { effTimer = millis(); ColorCometRoutine();          Eff_Tick (); }  break;  // ( 28U) Комета одноцветная
        case EFF_COMET_TWO:           DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStream();             Eff_Tick (); }  break;  // ( 29U) Комета двойная
        case EFF_COMET_THREE:         DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStream2();            Eff_Tick (); }  break;  // ( 30U) Комета тройная
        case EFF_CONTACTS:            DYNAMIC_DELAY_TICK { effTimer = millis(); Contacts();                   Eff_Tick (); }  break;  // ( 31U) Контакты
        case EFF_SPARKLES:            DYNAMIC_DELAY_TICK { effTimer = millis(); sparklesRoutine();            Eff_Tick (); }  break;  // ( 32U) Конфетти
        case EFF_CUBE2D:              DYNAMIC_DELAY_TICK { effTimer = millis(); cube2dRoutine();              Eff_Tick (); }  break;  // ( 33U) Кубик Рубика
        case EFF_LAVA:                HIGH_DELAY_TICK { effTimer = millis(); lavaNoiseRoutine();           Eff_Tick (); }  break;  // ( 34U) Лава
        case EFF_LAVALAMP:            LOW_DELAY_TICK { effTimer = millis(); LavaLampRoutine();            Eff_Tick (); }  break;  // ( 35U) Лавовая лампа
        case EFF_BUTTERFLYS_LAMP:     LOW_DELAY_TICK { effTimer = millis(); butterflysRoutine(false);     Eff_Tick (); }  break;  // ( 36U) Лампа с мотыльками
        case EFF_FOREST:              HIGH_DELAY_TICK { effTimer = millis(); forestNoiseRoutine();         Eff_Tick (); }  break;  // ( 37U) Лес
        case EFF_LUMENJER:            DYNAMIC_DELAY_TICK { effTimer = millis(); lumenjerRoutine();            Eff_Tick (); }  break;  // ( 38U) Люмeньep
        case EFF_MAGMA:               DYNAMIC_DELAY_TICK { effTimer = millis(); magmaRoutine();               Eff_Tick (); }  break;  // ( 39U) Магма
        case EFF_PAINTS:              DYNAMIC_DELAY_TICK { effTimer = millis(); OilPaints();                  Eff_Tick (); }  break;  // ( 40U) Масляные краски
        case EFF_MATRIX:              DYNAMIC_DELAY_TICK { effTimer = millis(); matrixRoutine();              Eff_Tick (); }  break;  // ( 41U) Матрица
        case EFF_TWINKLES:            DYNAMIC_DELAY_TICK { effTimer = millis(); twinklesRoutine();            Eff_Tick (); }  break;  // ( 42U) Мерцание
        case EFF_METABALLS:           LOW_DELAY_TICK { effTimer = millis(); MetaBallsRoutine();           Eff_Tick (); }  break;  // ( 43U) Метоболз
        case EFF_WEB_TOOLS:           SOFT_DELAY_TICK { effTimer = millis(); WebTools();                   Eff_Tick (); }  break;  // ( 44U) Мечта дизайнера
        case EFF_MOSAIC:              DYNAMIC_DELAY_TICK { effTimer = millis(); squaresNdotsRoutine();        Eff_Tick (); }  break;  // ( 45U) Мозайка
        case EFF_BUTTERFLYS:          LOW_DELAY_TICK { effTimer = millis(); butterflysRoutine(true);      Eff_Tick (); }  break;  // ( 46U) Moтыльки
        case EFF_BBALLS:              LOW_DELAY_TICK { effTimer = millis(); BBallsRoutine();              Eff_Tick (); }  break;  // ( 47U) Мячики
        case EFF_BALLS_BOUNCE:        LOW_DELAY_TICK { effTimer = millis(); bounceRoutine();              Eff_Tick (); }  break;  // ( 48U) Мячики без границ
        case EFF_CHRISTMAS_TREE:      DYNAMIC_DELAY_TICK { effTimer = millis(); ChristmasTree();              Eff_Tick (); }  break;  // ( 49U) Новогодняя Елка
        case EFF_FIRE:                DYNAMIC_DELAY_TICK { effTimer = millis(); fireRoutine(true);            Eff_Tick (); }  break;  // ( 50U) Огонь
        case EFF_FIRE_2012:           DYNAMIC_DELAY_TICK { effTimer = millis(); fire2012again();              Eff_Tick (); }  break;  // ( 51U) Огонь 2012
        case EFF_FIRE_2018:           DYNAMIC_DELAY_TICK { effTimer = millis(); Fire2018_2();                 Eff_Tick (); }  break;  // ( 52U) Огонь 2018
        case EFF_FIRE_2020:           DYNAMIC_DELAY_TICK { effTimer = millis(); fire2020Routine2();           Eff_Tick (); }  break;  // ( 53U) Огонь 2020
        case EFF_FIRE_2021:           LOW_DELAY_TICK { effTimer = millis(); Fire2021Routine();            Eff_Tick (); }  break;  // ( 54U) Огонь 2021
        case EFF_FIREFLY_TOP:         DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStream5();            Eff_Tick (); }  break;  // ( 55U) Огoнь верховой
        case EFF_FIREFLY:             DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStream3();            Eff_Tick (); }  break;  // ( 56U) Огoнь парящий
        case EFF_FIRESPARKS:          HIGH_DELAY_TICK { effTimer = millis(); FireSparks();                 Eff_Tick (); }  break;  // ( 57U) Огонь с искрами
        case EFF_COLOR_RAIN:          DYNAMIC_DELAY_TICK { effTimer = millis(); coloredRain();                Eff_Tick (); }  break;  // ( 58U) Осадки
        case EFF_OSCILLATING:         DYNAMIC_DELAY_TICK { effTimer = millis(); oscillatingRoutine();         Eff_Tick (); }  break;  // ( 59U) Осциллятор
        case EFF_CLOUDS:              HIGH_DELAY_TICK { effTimer = millis(); cloudsNoiseRoutine();         Eff_Tick (); }  break;  // ( 60U) Облака
        case EFF_OCEAN:               HIGH_DELAY_TICK { effTimer = millis(); oceanNoiseRoutine();          Eff_Tick (); }  break;  // ( 61U) Океан
        case EFF_RAINBOW_STRIPE:      HIGH_DELAY_TICK { effTimer = millis(); rainbowStripeNoiseRoutine();  Eff_Tick (); }  break;  // ( 62U) Павлин
        case EFF_HOURGLASS:           DYNAMIC_DELAY_TICK { effTimer = millis(); Hourglass();                  Eff_Tick (); }  break;  // ( 63U) Песочные часы
        case EFF_PAINTBALL:           DYNAMIC_DELAY_TICK { effTimer = millis(); lightBallsRoutine();          Eff_Tick (); }  break;  // ( 64U) Пейнтбол
        case EFF_PICASSO:             DYNAMIC_DELAY_TICK { effTimer = millis(); picassoSelector();            Eff_Tick (); }  break;  // ( 65U) Пикассо
        case EFF_PLASMA:              HIGH_DELAY_TICK { effTimer = millis(); plasmaNoiseRoutine();         Eff_Tick (); }  break;  // ( 66U) Плазма
        case EFF_SPIDER:              LOW_DELAY_TICK { effTimer = millis(); spiderRoutine();              Eff_Tick (); }  break;  // ( 67U) Плазменная лампа
        case EFF_FLAME:               LOW_DELAY_TICK { effTimer = millis(); execStringsFlame();           Eff_Tick (); }  break;  // ( 68U) Пламя
        case EFF_PLANETEARTH:         SOFT_DELAY_TICK { effTimer = millis(); PlanetEarth();                Eff_Tick (); }  break;  // ( 69U) Планета Земля
        case EFF_BY_EFFECT:           DYNAMIC_DELAY_TICK { effTimer = millis(); ByEffect();                   Eff_Tick (); }  break;  // ( 70U) Побочный эффект
        case EFF_POPCORN:             LOW_DELAY_TICK { effTimer = millis(); popcornRoutine();             Eff_Tick (); }  break;  // ( 71U) Попкорн
        case EFF_PRISMATA:            LOW_DELAY_TICK { effTimer = millis(); PrismataRoutine();            Eff_Tick (); }  break;  // ( 72U) Призмата
        case EFF_ATTRACT:             DYNAMIC_DELAY_TICK { effTimer = millis(); attractRoutine();             Eff_Tick (); }  break;  // ( 73U) Притяжение
        case EFF_LEAPERS:             DYNAMIC_DELAY_TICK { effTimer = millis(); LeapersRoutine();             Eff_Tick (); }  break;  // ( 74U) Пpыгyны
        case EFF_PULSE:               DYNAMIC_DELAY_TICK { effTimer = millis(); pulseRoutine(2U);             Eff_Tick (); }  break;  // ( 75U) Пульс
        case EFF_PULSE_WHITE:         LOW_DELAY_TICK     { effTimer = millis(); pulseRoutine(8U);             Eff_Tick (); }  break;  // ( 76U) Пульс белый
        case EFF_PULSE_RAINBOW:       DYNAMIC_DELAY_TICK { effTimer = millis(); pulseRoutine(4U);             Eff_Tick (); }  break;  // ( 77U) Пульс радужный
        case EFF_RADIAL_WAWE:         DYNAMIC_DELAY_TICK { effTimer = millis(); RadialWave();                 Eff_Tick (); }  break;  // ( 78U) Радиальная волна
        case EFF_RAINBOW_VER:         DYNAMIC_DELAY_TICK { effTimer = millis(); rainbowRoutine();             Eff_Tick (); }  break;  // ( 79U) Радуга
        case EFF_RAINBOW:             HIGH_DELAY_TICK { effTimer = millis(); rainbowNoiseRoutine();        Eff_Tick (); }  break;  // ( 80U) Радуга 3D
        case EFF_SNAKE:               DYNAMIC_DELAY_TICK { effTimer = millis(); MultipleStream8();            Eff_Tick (); }  break;  // ( 81U) Радужный змей
        case EFF_RAIN:                DYNAMIC_DELAY_TICK { effTimer = millis(); RainRoutine();                Eff_Tick (); }  break;  // ( 82U) Разноцветный дождь
        case EFF_RIVERS:              DYNAMIC_DELAY_TICK { effTimer = millis(); BotswanaRivers();             Eff_Tick (); }  break;  // ( 83U) Реки Ботсваны
        case EFF_LIGHTERS:            DYNAMIC_DELAY_TICK { effTimer = millis(); lightersRoutine();            Eff_Tick (); }  break;  // ( 84U) Светлячки
        case EFF_LIGHTER_TRACES:      DYNAMIC_DELAY_TICK { effTimer = millis(); ballsRoutine();               Eff_Tick (); }  break;  // ( 85U) Светлячки со шлейфом
        case EFF_FEATHER_CANDLE:      DYNAMIC_DELAY_TICK { effTimer = millis(); FeatherCandleRoutine();       Eff_Tick (); }  break;  // ( 86U) Свеча
        case EFF_AURORA:              HIGH_DELAY_TICK { effTimer = millis(); polarRoutine();               Eff_Tick (); }  break;  // ( 87U) Северное сияние
        case EFF_SINUSOID3:           HIGH_DELAY_TICK { effTimer = millis(); Sinusoid3Routine();           Eff_Tick (); }  break;  // ( 88U) Синусоид
        case EFF_COLORS:              HIGH_DELAY_TICK { effTimer = millis(); colorsRoutine2();             Eff_Tick (); }  break;  // ( 89U) Смена цвета
        case EFF_SNOW:                DYNAMIC_DELAY_TICK { effTimer = millis(); Snowfall();                   Eff_Tick (); }  break;  // ( 90U) Снегопад
        case EFF_SPECTRUM:            DYNAMIC_DELAY_TICK { effTimer = millis(); Spectrum();                   Eff_Tick (); }  break;  // ( 91U) Спектрум
        case EFF_SPIRO:               LOW_DELAY_TICK { effTimer = millis(); spiroRoutine();               Eff_Tick (); }  break;  // ( 92U) Спирали
        case EFF_FLOCK:               LOW_DELAY_TICK { effTimer = millis(); flockRoutine(false);          Eff_Tick (); }  break;  // ( 93U) Стая
        case EFF_FLOCK_N_PR:          LOW_DELAY_TICK { effTimer = millis(); flockRoutine(true);           Eff_Tick (); }  break;  // ( 94U) Стая и хищник
        case EFF_ARROWS:              DYNAMIC_DELAY_TICK { effTimer = millis(); arrowsRoutine();              Eff_Tick (); }  break;  // ( 95U) Стрелки
        case EFF_STROBE:              LOW_DELAY_TICK { effTimer = millis(); StrobeAndDiffusion();         Eff_Tick (); }  break;  // ( 96U) Строб.Хаос.Дифузия
        case EFF_SHADOWS:             LOW_DELAY_TICK { effTimer = millis(); shadowsRoutine();             Eff_Tick (); }  break;  // ( 97U) Тени
        case EFF_PACIFIC:             LOW_DELAY_TICK { effTimer = millis(); pacificRoutine();             Eff_Tick (); }  break;  // ( 98U) Тихий океан
        case EFF_SIMPLE_RAIN:         DYNAMIC_DELAY_TICK { effTimer = millis(); simpleRain();                 Eff_Tick (); }  break;  // ( 99U) Tyчкa в банке
        case EFF_FIREWORK:            SOFT_DELAY_TICK { effTimer = millis(); Firework();                   Eff_Tick (); }  break;  // (100U) Фейерверк
        case EFF_FIREWORK_2:          DYNAMIC_DELAY_TICK { effTimer = millis(); fireworksRoutine();           Eff_Tick (); }  break;  // (101U) Фейерверк 2
        case EFF_FAIRY:               DYNAMIC_DELAY_TICK { effTimer = millis(); fairyRoutine();               Eff_Tick (); }  break;  // (102U) Фея
        case EFF_COLOR:               HIGH_DELAY_TICK { effTimer = millis(); colorRoutine();               Eff_Tick (); }  break;  // (103U) Цвет
        case EFF_EFF_SAND:            DYNAMIC_DELAY_TICK { effTimer = millis(); sandRoutine();                Eff_Tick (); }  break;  // (104U) Цветные драже
        case EFF_COLOR_FRIZZLES:      SOFT_DELAY_TICK { effTimer = millis(); ColorFrizzles();              Eff_Tick (); }  break;  // (105U) Цветные кудри
        case EFF_EFF_LOTUS:           DYNAMIC_DELAY_TICK { effTimer = millis(); LotusFlower();                Eff_Tick (); }  break;  // (106U) Цветок лотоса
        case EFF_SPHERES:             LOW_DELAY_TICK { effTimer = millis(); spheresRoutine();             Eff_Tick (); }  break;  // (107U) Шapы
        case EFF_NEXUS:               LOW_DELAY_TICK { effTimer = millis(); nexusRoutine();               Eff_Tick (); }  break;  // (108U) Nexus
        case EFF_CLOCK:               DYNAMIC_DELAY_TICK { effTimer = millis(); clockRoutine();               Eff_Tick (); }  break;  // (109U) Часы
        case EFF_TEXT:                DYNAMIC_DELAY_TICK { effTimer = millis(); text_running();               Eff_Tick (); }  break;  // (110U) Бeгyщaя cтpoкa
        

      }
      #ifdef WARNING_IF_NO_TIME_ON_EFFECTS_TOO
        if (!timeSynched)
          noTimeWarning();
      #endif
    }
    #ifdef WARNING_IF_NO_TIME
    else if (!timeSynched && !ONflag && !((uint8_t)millis())){
      noTimeWarningShow();
    }
    #endif
  }
}

void changePower()
{
  if (ONflag)
  {
    effectsTick();
    #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором
    digitalWrite(MOSFET_PIN, MOSFET_LEVEL);
    #endif    
    for (uint8_t i = 0U; i < modes[currentMode].Brightness; i = constrain(i + (modes[currentMode].Brightness < 60 ? 1 : 4), 0, modes[currentMode].Brightness))
    {
      FastLED.setBrightness(i);
      delay(1);
      FastLED.show();
    }
    FastLED.setBrightness(modes[currentMode].Brightness);
    delay(2);
    FastLED.show();
  }
  else
  {
    effectsTick();
    for (uint8_t i = modes[currentMode].Brightness; i > 0; i = constrain(i - (modes[currentMode].Brightness < 60 ? 1 : 4), 0, modes[currentMode].Brightness))
    {
      FastLED.setBrightness(i);
      delay(1);
      FastLED.show();
    }
    FastLED.clear();
    delay(2);
    FastLED.show();
    #if defined(MOSFET_PIN) && defined(MOSFET_LEVEL)      // установка сигнала в пин, управляющий MOSFET транзистором
    digitalWrite(MOSFET_PIN, !MOSFET_LEVEL);
    #endif
  }
  TimerManager::TimerRunning = false;
  TimerManager::TimerHasFired = false;
  TimerManager::TimeToFire = 0ULL;
  jsonWrite(configSetup, "tmr", 0);
    if (ONflag && AUTOMATIC_OFF_TIME) {
      TimerManager::TimerRunning = true;
      TimerManager::TimeToFire = millis() + AUTOMATIC_OFF_TIME;
       }  
  if (!ONflag && FavoritesManager::UseSavedFavoritesRunning == 0U) // если выбрана опция Сохранять состояние (вкл/выкл) "избранного",
  {                                                                // то ни выключение модуля, ни выключение матрицы не сбрасывают текущее состояние (вкл/выкл) "избранного"
      FavoritesManager::TurnFavoritesOff();
      jsonWrite(configSetup, "cycle_on", 0);
  }

  #if (USE_MQTT)
  if (espMode == 1U)
  {
    MqttManager::needToPublish = true;
  }
  #endif
}

#ifdef WARNING_IF_NO_TIME
void noTimeWarning(){
  for (uint8_t i = 0; i < WIDTH; i++) leds[XY(i, 0U)] = CRGB::Black;
  uint8_t z = millis() / 1000U;
  #ifdef USE_NTP
    leds[XY(z % WIDTH , 0U)] = espMode ? CRGB::Red : CRGB::Blue; // если при включенном NTP время не получено, будем красным цветом мигать
    leds[XY((z + WIDTH / 2U) % WIDTH , 0U)] = espMode ? CRGB::Red : CRGB::Blue;
  #else  
    leds[XY(z % WIDTH , 0U)] = espMode ? CRGB::Yellow : CRGB::Blue; // иначе скромно жёлтым - нормальная ситуация при отсутствии NTP
    leds[XY((z + WIDTH / 2U) % WIDTH , 0U)] = espMode ? CRGB::Yellow : CRGB::Blue;
  #endif //USE_NTP
}
void noTimeWarningShow(){
  noTimeWarning();
  FastLED.setBrightness(WARNING_IF_NO_TIME);
  FastLED.show();
}
void noTimeClear(){
  if (!timeSynched){ 
    for (uint8_t i = 0; i < WIDTH; i++) 
       leds[XY(i, 0U)] = CRGB::Black; 
    FastLED.show();
  }
}
#endif //WARNING_IF_NO_TIME

void Eff_Tick () {
    #ifdef MP3_TX_PIN
    mp3_folder=effects_folders[currentMode];
    #endif  //MP3_TX_PIN
    FastLED.show();
      #ifdef USE_MULTIPLE_LAMPS_CONTROL
      if (repeat_multiple_lamp_control)  {
          for ( uint8_t n=0; n< MODE_AMOUNT; n++)
          {
              if (eff_num_correct[n] == currentMode) {
                  jsonWrite(configSetup, "eff_sel", n);
                  break;
              }
          } 
      jsonWrite(configSetup, "br", modes[currentMode].Brightness);
      jsonWrite(configSetup, "sp", modes[currentMode].Speed);
      jsonWrite(configSetup, "sc", modes[currentMode].Scale);          
          multiple_lamp_control ();
          repeat_multiple_lamp_control = false;
      }
      #endif  //USE_MULTIPLE_LAMPS_CONTROL
}
