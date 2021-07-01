void handleRoot() {
  server.sendHeader("Location", "/index.html", true);  //Redirect to our html web page
  server.send(302, "text/plane", "");
}

bool loadFromSPIFFS(String path) {
  Serial.print("path ist ");
  String dataType = "text/plain";
  if (path.endsWith("/"))
    path += "index.html";
  if (path.endsWith(".src"))
    path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".jpg")) dataType = "image/jpeg";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";
  else if (path.endsWith(".xml")) dataType = "text/xml";
  else if (path.endsWith(".pdf")) dataType = "application/pdf";
  else if (path.endsWith(".zip")) dataType = "application/zip";
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }
  dataFile.close();
  Serial.println(path);
  return true;
}

void handleWebRequests() {
  //Serial.println(server.client().remoteIP());
  if (loadFromSPIFFS(server.uri())) return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " NAME:" + server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
}

void sendJSON(void) {
  static int CallCounter = 0;
  const size_t capacity = JSON_OBJECT_SIZE(80);
  DynamicJsonDocument doc(capacity);
  char JSONBuffer[2048];
  char OutputBuffer[20];
  ClientIP = server.client().remoteIP();
  GetEE();
  doc["compile"] = __DATE__ " " __TIME__;
  sprintf(OutputBuffer, "%2.2f", Vcc);
  doc["vcc"] = OutputBuffer;
  sprintf(OutputBuffer, "%3.1f", Temperatur);
  doc["temperatur"] = OutputBuffer;
  doc["wochentag"] = Wochentag[weekday(now())];
  doc["tag"] = MakeDateString(now());
  doc["uhrzeit"] = MakeTimeString(now());
  doc["hostIP"] = HostIP;
  sprintf(OutputBuffer, "%d.%d.%d.%d", ClientIP[0], ClientIP[1], ClientIP[2], ClientIP[3]);
  doc["clientIP"] = OutputBuffer;
  doc["mqtt"] = PublishOK;
  //Serial.println(OutputBuffer);

  JsonArray pumpenConfig = doc.createNestedArray("pumpenConfig");
  JsonArray pumpeStatus = doc.createNestedArray("pumpeStatus");
  JsonArray pflanzenNamen = doc.createNestedArray("pflanzenNamen");
  JsonArray pumpeZeit = doc.createNestedArray("pumpeZeit");
  JsonArray pflanzenFeuchte = doc.createNestedArray("pflanzenFeuchte");
  JsonArray sensorFeuchte = doc.createNestedArray("sensorFeuchte");
  for (int i = 0; i < 8; i++) {
    pumpenConfig.add(Config.PumpeDa[i]);
    pumpeStatus.add(PumpeStatus[i]);
    pflanzenNamen.add(Config.Pflanzen[i]);
    pumpeZeit.add(Config.PumpeZeit[i]);
    pflanzenFeuchte.add(Analog[i]);
    sensorFeuchte.add(Volt[i]);
  }

  serializeJson(doc, JSONBuffer);
  //Serial.println(JSONBuffer);
  server.send(200, "text/plane", JSONBuffer); //Send JSON Data to client ajax request
}

void readJSON() {
  DynamicJsonDocument doc(2048);

  if (server.hasArg("plain") == false) { //Check if body received
    server.send(200, "text/plain", "JSON not received");
    Serial.println("Nix empfangen");
    return;
  }
  server.send(200, "text/plain", "JSON received");
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  for (int i = 0; i < 6; i++) {
    Config.PumpeDa[i] = doc["pumpenConfig"][i];
    strcpy(Config.Pflanzen[i], doc["pflanzeName"][i]);
    Config.PumpeZeit[i] = doc["pumpeZeit"][i];
    Serial.printf("Name: %s, Da: %d, Zeit; %d\n", Config.Pflanzen[i], Config.PumpeDa[i], Config.PumpeZeit[i]);
  }
  Serial.println("---+++---");
  PutEE();
  Serial.println("Daten gespeichert");
}

void TestPumpen() {
  if (server.hasArg("pumpe") == false) { //Check if body received
    server.send(200, "text/plain", "kein Parameter empfangen");
    Serial.println("Nix empfangen");
    return;
  }
  /*if (DoTest) {
    server.send(200, "text/plain", "Test laeuft, warten");
    Serial.println("Test laeuft, warten");
    return;
    }*/
  TestPumpe = server.arg("pumpe").toInt();
  DoTest = true;
  TestRunTime[TestPumpe] = millis() + (Config.PumpeZeit[TestPumpe] * 1000);
  server.send(200, "text/plain", "Teste Pumpe");
}

void RebootHost() {
  Serial.println("Reboot ping");
  if (server.hasArg("reboot") == 1) {
    Serial.println("ESP Reboot");
    server.send(200, "text/plain", "reboot ok");
    delay(1000);
    ESP.restart();
    delay(1000);
  }
}
