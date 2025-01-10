#ifdef TM1637_USE
void Display_Timer (uint8_t argument)   {
    
    if (DisplayFlag ==0 && LastEffect != currentMode) {
        LastEffect = currentMode;
        DisplayTimer = millis();
        DisplayFlag = 1;
        uint8_t n;
        for (n = 0; n < MODE_AMOUNT; n++)
        {
            if (eff_num_correct[n] == currentMode) break;
        }
        if (n < 100) {
        display.point(1);
        display.displayByte(_E_, _F_, _empty, _empty);               // отображаем EF
        display.showNumberDec (n, 0, 2, 2);                // отображаем номер эффекта
        }
        else {
        display.point(0);
        display.displayByte(_E_, _empty, _empty, _empty);               // отображаем EF
        display.showNumberDec (n, 0, 3, 1);                // отображаем номер эффекта
        }
    }
    if (DisplayFlag == 1 && (millis() - DisplayTimer > 3000)) {
        DisplayFlag = 0;
        if (timeSynched) clockTicker_blink ();                   // отображаем  время
        else display.displayByte(_dash, _dash, _dash, _dash);    // отображаем прочерки
    }
        
    #ifdef MP3_PLAYER_USE
    if (DisplayFlag ==0 && LastCurrentFolder != CurrentFolder) {
        LastCurrentFolder = CurrentFolder;
        DisplayTimer = millis();
        DisplayFlag = 2;
        display.point(1);
        display.displayByte(_F_, _o_, _empty, _empty);               // отображаем Fo
        display.showNumberDec (CurrentFolder, 0, 2, 2);              // отображаем номер папки
    }
    if (DisplayFlag == 2 && (millis() - DisplayTimer > 3000)) {
        DisplayFlag = 0;
        if (timeSynched) clockTicker_blink ();                   // отображаем  время
        else display.displayByte(_dash, _dash, _dash, _dash);    // отображаем прочерки
    }
    #endif  // MP3_PLAYER_USE
    
    if(DisplayFlag == 3) {
        DisplayTimer = millis();
        DisplayFlag = 4;
        display.point(0);
        display.displayByte(_empty, _empty, _empty, _empty);               // отображаем ничего
        display.showNumberDec (argument, 0, 3, 1);              // отображаем значение argument (br, sc, sp)
    }
    if (DisplayFlag == 4 && (millis() - DisplayTimer > 3000)) {
        DisplayFlag = 0;
        if (timeSynched) clockTicker_blink ();                   // отображаем  время
        else display.displayByte(_dash, _dash, _dash, _dash);    // отображаем прочерки
    }   
}
#endif  //TM1637_USE