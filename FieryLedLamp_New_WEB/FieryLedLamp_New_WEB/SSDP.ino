void SSDP_init(void) {
  //String chipID = String( ESP.getChipId() ) + "-" + String( ESP.getFlashChipId() );
  // SSDP дескриптор
  HTTP.on(F("/description.xml"), HTTP_GET, []() {
    SSDP.schema(HTTP.client());
  });
  // --------------------Получаем SSDP со страницы
  //HTTP.on("/ssdp", HTTP_GET, []() {
    //String ssdp = HTTP.arg("ssdp");
 // configSetup=jsonWrite(configJson, "SSDP", ssdp);
  //jsonWrite(configSetup, "SSDP", HTTP.arg("ssdp"));
  //saveConfig();                 // Функция сохранения данных во Flash
  //HTTP.send(200, F("text/plain"), "OK"); // отправляем ответ о выполнении
  //});
  //Если версия  2.0.0 закаментируйте следующую строчку
  LAMP_NAME = jsonRead(configSetup, "SSDP");
  SSDP.setName(jsonRead(configSetup, "SSDP"));
  SSDP.setDeviceType(F("upnp:rootdevice"));
  SSDP.setSchemaURL(F("description.xml"));
  SSDP.setHTTPPort(80);
  SSDP.setName(jsonRead(configSetup, "SSDP"));
  SSDP.setSerialNumber(ESP.getChipId());
  SSDP.setURL("/");
  SSDP.setModelName(F("FieryLedLamp"));
  SSDP.setModelNumber(jsonRead(configSetup, "SSDP") + FLL_VERSION);  
  SSDP.setModelURL(F("https://github.com/alvikskor/FieryLedLampMultilingual"));
  SSDP.setManufacturer(F("alvikskor"));
  SSDP.setManufacturerURL(F("https://github.com/alvikskor/FieryLedLampMultilingual"));
  SSDP.begin();
}
