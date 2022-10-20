
void HTTP_init(void) {
/*
  // --------------------Выдаем данные configJson
  HTTP.on("/config.live.json", HTTP_GET, []() {
    outData();
    HTTP.send(200, F("application/json"), configJson);
  });
*/  
  // -------------------Выдаем данные configSetup
  HTTP.on("/config.setup.json", HTTP_GET, []() {
    HTTP.send(200, F("application/json"), configSetup);
  });
 
  // -------------------Обработка Restart
  HTTP.on("/restart", HTTP_GET, []() {
    String restart = HTTP.arg("device");          // Получаем значение device из запроса
    if (restart == "ok") {                         // Если значение равно Ок
      HTTP.send(200, F("text / plain"), F("Reset OK")); // Oтправляем ответ Reset OK
      ESP.restart();                                // перезагружаем модуль
    }
    else {                                        // иначе
      HTTP.send(200, F("text / plain"), F("No Reset")); // Oтправляем ответ No Reset
    }
  });
  
  httpUpdater.setup(&HTTP);    // Добавляем функцию Update для перезаписи прошивки по WiFi при 4М(1M SPIFFS) и выше
  HTTP.begin();                // Запускаем HTTP сервер
  
}
