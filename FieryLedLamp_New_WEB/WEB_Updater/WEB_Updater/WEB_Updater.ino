#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LittleFS.h>

    #define STA_SSID "LOGIN"           // Имя Вашей WiFi сети
    #define PASSWORD "PASSWORD"     // Пароль Вашей WiFi сети

const char* host = "ESP8266";
const char* ssid = STA_SSID;
const char* password = PASSWORD;


ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void setup(void) {

  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");

  // Попытка подключения к точке доступа
  WiFi.mode(WIFI_STA);
  byte tries = 20;
  String _ssid = STA_SSID;
  String _password = PASSWORD;

   WiFi.begin(_ssid.c_str(), _password.c_str());
  
  // Делаем проверку подключения до тех пор пока счетчик tries
  // не станет равен нулю или не получим подключение
  while (--tries && WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    // Если не удалось подключиться запускаем в режиме AP
    Serial.println("");
    Serial.println("WiFi up ACCESS POINT");
    StartAPMode();
  }
  else {
    // Иначе удалось подключиться отправляем сообщение
    // о подключении и выводим адрес IP
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  MDNS.begin(host);

  httpUpdater.setup(&httpServer);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.print("HTTPUpdateServer ready! Open   http://");
  Serial.print(WiFi.localIP());
  Serial.println("/update   in your browser\n");
}

void loop(void) {
  httpServer.handleClient();
  MDNS.update();
}

void StartAPMode() {
  IPAddress apIP(192, 168, 4, 1);
  // Отключаем WIFI
  WiFi.disconnect();
  // Меняем режим на режим точки доступа
  WiFi.mode(WIFI_AP);
  // Задаем настройки сети
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  // Включаем WIFI в режиме точки доступа с именем и паролем
  // хронящихся в переменных _ssidAP _passwordAP
  String _ssidAP = "WEB_Updater";
  //String _passwordAP = "";
  WiFi.softAP(_ssidAP.c_str());   //WiFi.softAP(_ssidAP.c_str(), _passwordAP.c_str());
}
