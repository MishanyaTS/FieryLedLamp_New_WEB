// Инициализация FFS
void FS_init(void) {  
  LittleFS.begin();
  #ifdef ESP32_USED
  {
    File root = LittleFS.open("/");
    File file = root.openNextFile();
    while (file) {
      //Serial.print("FILE: ");
      //Serial.println(file.name());
      String fileName = file.name();
      //size_t fileSize = file.size();
      file = root.openNextFile();
    }
  }
  #else
  {
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      //size_t fileSize = dir.fileSize();
    }
  }
  #endif
  //HTTP страницы для работы с FFS
  //list directory
  HTTP.on("/list", HTTP_GET, handleFileList);
  //загрузка редактора editor
  HTTP.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) HTTP.send(404, F("text/plain"), F("FileNotFound"));
  });
  //Создание файла
  HTTP.on("/edit", HTTP_PUT, handleFileCreate);
  //Удаление файла
  HTTP.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  HTTP.on("/edit", HTTP_POST, []() {
    HTTP.send(200, F("text/plain"), "");
  }, handleFileUpload);
  //called when the url is not defined here
  //use it to load content from LittleFS
  HTTP.onNotFound([]() {
    if (!handleFileRead(HTTP.uri()))
      HTTP.send(404, F("text/plain"), F("FileNotFound"));
  });
}
// Здесь функции для работы с файловой системой
String getContentType(String filename) {
  if (HTTP.hasArg("download")) return F("application/octet-stream");
  else if (filename.endsWith(".htm")) return F("text/html");
  else if (filename.endsWith(".html")) return F("text/html");
  else if (filename.endsWith(".json")) return F("application/json");
  else if (filename.endsWith(".css")) return F("text/css");
  else if (filename.endsWith(".js")) return F("application/javascript");
  else if (filename.endsWith(".png")) return F("image/png");
  else if (filename.endsWith(".gif")) return F("image/gif");
  else if (filename.endsWith(".jpg")) return F("image/jpeg");
  else if (filename.endsWith(".ico")) return F("image/x-icon");
  else if (filename.endsWith(".xml")) return F("text/xml");
  else if (filename.endsWith(".pdf")) return F("application/x-pdf");
  else if (filename.endsWith(".zip")) return F("application/x-zip");
  else if (filename.endsWith(".gz")) return F("application/x-gzip");
  return F("text/plain");
}

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += F("index.htm");
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (LittleFS.exists(pathWithGz) || LittleFS.exists(path)) {
    if (LittleFS.exists(pathWithGz))
      path += ".gz";
    File file = LittleFS.open(path, "r");
    HTTP.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (HTTP.uri() != "/edit") return;
  HTTPUpload& upload = HTTP.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    fsUploadFile = LittleFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
  }
}

void handleFileDelete() {
  if (HTTP.args() == 0) return HTTP.send(500, F("text/plain"), F("BAD ARGS"));
  String path = HTTP.arg(0);
  if (path == "/")
    return HTTP.send(500, F("text/plain"), F("BAD PATH"));
  if (!LittleFS.exists(path))
    return HTTP.send(404, F("text/plain"), F("FileNotFound"));
  LittleFS.remove(path);
  HTTP.send(200, F("text/plain"), "");
  path = String();
}

void handleFileCreate() {
  if (HTTP.args() == 0)
    return HTTP.send(500, F("text/plain"), F("BAD ARGS"));
  String path = HTTP.arg(0);
  if (path == "/")
    return HTTP.send(500, F("text/plain"), F("BAD PATH"));
  if (LittleFS.exists(path))
    return HTTP.send(500, F("text/plain"), F("FILE EXISTS"));
  File file = LittleFS.open(path, "w");
  if (file)
    file.close();
  else
    return HTTP.send(500, F("text/plain"), F("CREATE FAILED"));
  HTTP.send(200, F("text/plain"), "");
  path = String();

}

#ifdef ESP32_USED
void handleFileList() {
  if (!HTTP.hasArg("dir")) {
    HTTP.send(500, F("text/plain"), F("BAD ARGS"));
    return;
  }
  String path = HTTP.arg("dir");
  File root = LittleFS.open(path);
  File file = root.openNextFile();
  path = String();
  String output = "[";
  while (file) {
    //entry = file.read();
    if (output != "[") output += ',';
#if defined (USE_LittleFS)
    bool isDir = file.isDirectory();
#else
    bool isDir = false;
#endif
    output += F("{\"type\":\"");
    output += (isDir) ? F("dir") : F("file");
    output += F("\",\"name\":\"");
    output += String(file.name()).substring(1);
    output += "\"}";
    file.close();
//    isDir = false;

    file = root.openNextFile();
  }
  output += "]";
  HTTP.send(200, F("text/json"), output);
}
#else
void handleFileList() {
  if (!HTTP.hasArg("dir")) {
    HTTP.send(500, F("text/plain"), F("BAD ARGS"));
    return;
  }
  String path = HTTP.arg("dir");
  Dir dir = LittleFS.openDir(path);
  path = String();
  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
#if defined (USE_LittleFS)
    bool isDir = entry.isDirectory();
#else
    bool isDir = false;
#endif
    output += F("{\"type\":\"");
    output += (isDir) ? F("dir") : F("file");
    output += F("\",\"name\":\"");
#if defined (USE_LittleFS)
    output += String(entry.name()).substring(0);
#else
    output += String(entry.name()).substring(1);
#endif
    output += "\"}";
    entry.close();
//    isDir = false;
  }
  output += "]";
  HTTP.send(200, F("text/json"), output);
}
#endif
