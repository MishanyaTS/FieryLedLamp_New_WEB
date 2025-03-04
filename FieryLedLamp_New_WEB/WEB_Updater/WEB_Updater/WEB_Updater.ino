#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <WiFiClient.h>
  #include <ESP8266WebServer.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
#endif
#include <TM1637Display.h>
#include <ElegantOTA.h>

#define CLK 27  // Пин, подключенный к CLK на TM1637
#define DIO 13  // Пин, подключенный к DIO на TM1637
#define BUTTON_PIN 35  // Пин, подключенный к TTP223

const char* ssid = "ssid";
const char* password = "password";

#if defined(ESP8266)
  ESP8266WebServer server(80);
#elif defined(ESP32)
  WebServer server(80);
#endif
  TM1637Display display(CLK, DIO);

unsigned long ota_progress_millis = 0;
bool isAccessPoint = false; // Флаг, определяющий, работает ли устройство в режиме точки доступа
int buttonPressCount = 0; // Счетчик нажатий кнопки

void displayIP(IPAddress ip) {
  // Показать IP-адрес на дисплее TM1637 по октетам
  for (int i = 0; i < 4; i++) {
    display.showNumberDec(ip[i], false, 3, 0); // Показываем октет без передних нулей
    delay(2000); // Короткая пауза для видимости
    display.clear(); // Очистка дисплея перед следующим октетом
    delay(500); // Пауза перед следующим показом
  }
}

void setup() {
  Serial.begin(115200);
  
  // Инициализация дисплея TM1637 и пина кнопки
  display.setBrightness(0x0f); // Установка максимальной яркости
  pinMode(BUTTON_PIN, INPUT); // Установка пина TTP223 как входящего

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  
  // Ожидание подключения
  for (int attempts = 0; attempts < 10; attempts++) {
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("");
    Serial.println("Не удалось подключиться к WiFi, создается точка доступа...");
    
    // Создание точки доступа
    WiFi.softAP("WEB_Updater", "password");
    IPAddress IP(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(IP, IP, subnet);
    Serial.print("Точка доступа создана. IP адрес: ");
    Serial.println(WiFi.softAPIP());
    
    isAccessPoint = true; // Устанавливаем флаг точки доступа
    displayIP(WiFi.softAPIP()); // Отобразить IP адрес точки доступа
  } else {
    Serial.println("");
    Serial.print("Подключено к WiFi: ");
    Serial.print(ssid);
    Serial.print(", IP адрес: ");
    String ipAddress = WiFi.localIP().toString();
    Serial.println(ipAddress);
    
    isAccessPoint = false; // Устанавливаем флаг для WiFi
    displayIP(WiFi.localIP()); // Отобразить IP адрес подключенного WiFi
  }

  // Обработка корневого URL
  server.on("/", []() {
    String html = "<html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<title>ElegantOTA</title>";
    html += "</head><body>";
    html += "<h1>ElegantOTA</h1>";
    html += "<button onclick=\"location.href='/update'\">Обновление прошивки</button>";
    html += "</body></html>";

    server.send(200, "text/html", html);
  });

  ElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // Проверка нажатий кнопки
  if (digitalRead(BUTTON_PIN) == HIGH) { // Если кнопка нажата
    delay(200); // антидребезг
    buttonPressCount++;

    if (buttonPressCount >= 5) {
      buttonPressCount = 0; // Сбросить счетчик нажатий
      Serial.println("Кнопка нажата 5 раз, вывод IP адреса...");
      
      // Вывод IP адреса на дисплей
      if (isAccessPoint) {
        displayIP(WiFi.softAPIP()); // Если точка доступа
      } else {
        displayIP(WiFi.localIP()); // Если подключен к WiFi
      }
    }
    
    // Ждем пока кнопка не будет отпущена
    while (digitalRead(BUTTON_PIN) == HIGH) {
      delay(10);
    }
  }
}
