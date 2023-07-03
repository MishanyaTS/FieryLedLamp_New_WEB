#pragma once

/*
 * Схема использования EEPROM памяти:
 * Используются адреса:
 * Начало    Длина    Описание
 * 0-63      64       Пароль wi-fi вашего роутера
 * 64        1        Признак первого запуска
 * 65-79     15       Специальные настройки
 * ***************** массив modes (эффекты)
 * 80-82     3        режим №1:  яркость, скорость, масштаб (по одному байту)
 * 83-85     3        режим №2:  яркость, скорость, масштаб (по одному байту)
 * 86-88     3        режим №3:  яркость, скорость, масштаб (по одному байту)
 * ...
*/


class EepromManager
{
  public:
    static void InitEepromSettings(ModeType modes[], void (*restoreDefaultSettings)())
    {
      EEPROM.begin(EEPROM_TOTAL_BYTES_USED);
      delay(50);

      // записываем в EEPROM начальное состояние настроек, если их там ещё нет
      if (EEPROM.read(EEPROM_FIRST_RUN_ADDRESS) != EEPROM_FIRST_RUN_MARK)
      {
        restoreDefaultSettings(); // а почему бы нам не восстановить настройки по умолчанию в этом месте?

        EEPROM.write(EEPROM_FIRST_RUN_ADDRESS, EEPROM_FIRST_RUN_MARK);
        EEPROM.commit();

        for (uint8_t i = 0; i < MODE_AMOUNT; i++)
        {
          EEPROM.put(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * i, modes[i]);
          EEPROM.commit();
        }
      }
      else
      // инициализируем настройки лампы значениями из EEPROM
      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      {
        EEPROM.get(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * i, modes[i]);
      }
    }
/*
    static void HandleEepromTick(bool* settChanged, uint32_t* eepromTimeout, ModeType modes[])
    {
      if (*settChanged && millis() - *eepromTimeout >= EEPROM_WRITE_DELAY)
      {
        *settChanged = false;
        *eepromTimeout = millis();
        for (uint8_t i = 0; i < MODE_AMOUNT; i++)
        {
          EEPROM.put(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * i, modes[i]);
          EEPROM.commit();
        }
        #ifdef GENERAL_DEBUG
        LOG.println (F("Сохраненные настройки эффектов"));
        #endif
      }
    }
*/
    static void EepromGet(ModeType modes[]) {
      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      {
        EEPROM.get(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * i, modes[i]);
      }
    }

    static void EepromPut(ModeType modes[]) {
      for (uint8_t i = 0; i < MODE_AMOUNT; i++)
      {
          EEPROM.put(EEPROM_MODES_START_ADDRESS + EEPROM_MODE_STRUCT_SIZE * i, modes[i]);
          EEPROM.commit();
      }
    }

  private:
};
