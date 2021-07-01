String MakeDateString(time_t DateTime) {
  String DateString = "";
  DateString = day(DateTime);
  DateString += ".";
  DateString += month(DateTime);
  DateString += ".";
  DateString += year(DateTime);
  return DateString;
}

String MakeTimeString(time_t DateTime) {
  String TimeString = "";
  TimeString += hour(DateTime);
  TimeString += ":";
  if (minute(DateTime) < 10)
    TimeString += "0";
  TimeString += minute(DateTime);
  return TimeString;
}

int MakeDateDisplay (void) {
  int Day, Month;
  Day = day(now());
  Month = month(now());
  return (Day * 100 + Month);
}

time_t SyncTimeToNTP1(void) {
  uint32_t CurrentTime, NewTime;
  static bool NTPSync = false;
  CurrentTime = now();
  while (!NTPSync) {
    Serial.println("******Time-Sync******");
    while (!timeClient.update()) {
      Serial.print(".");
      delay(100);
    }
    NewTime = timeClient.getEpochTime();
    if (abs(NewTime - CurrentTime) < 10000)
      NTPSync = true;
  }
  Serial.println("");
  NTPSync = false;
  return (LTZ.toLocal(timeClient.getEpochTime(), &tcr));
}

time_t SyncTimeToNTP(void) {
  Serial.println("******Time-Sync******");
  while(!timeClient.update())
    Serial.print(".");
  Serial.println("");
  return (LTZ.toLocal(timeClient.getEpochTime(), &tcr));
}


time_t TimeCalc (int Td, int Tm, int Ty) {
  tmElements_t Tin;
  Tin.Second = 0;
  Tin.Minute = 0;
  Tin.Hour = 0;
  Tin.Day = Td;
  Tin.Month = Tm;
  Tin.Year = Ty;
  return (makeTime(Tin));
}

void I2CScanner (void) {
  static int nDevices;
  static int ScanCount = 0;
  static byte address;
  byte error;
  Serial.print("I2C-Scan #");
  Serial.println(ScanCount);

  nDevices = 0;
  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.print("I2C Baustein gefunden unter 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unbekannter Fehler bei Sdresse 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("Keine I2C Bausteine gefunden\n");
  else
    Serial.println("fertig\n");
  LastScan = millis();           // wait 1 second for next scan
  ScanCount++;
}

void ReadAnalog(void) {
  static int MessCount = 0;
  static unsigned long TimeReadAnalog = 0;
  if (millis() > TimeReadAnalog) {
    Vcc = (float)ESP.getVcc() / 1000;
    sensor.requestTemperatures();
    Temperatur = sensor.getTempCByIndex(0);
    for (int i = 0; i < 4; i++) {
      int AnaVal0 = ads0.readADC_SingleEnded(i);
      int AnaVal1 = ads1.readADC_SingleEnded(i);

      // ADC 0
      AnalogRaw[i] = AnaVal0;
      AnalogRaw[i + 4] = AnaVal1;
      if (AnaVal0 < 11600)
        Analog[i] = 100;
      else if (AnaVal0 > 21500)
        Analog[i] = 0;
      else
        Analog[i] = map(AnaVal0, 11600, 21500, 100, 0);

      // ADC 1
      if (AnaVal1 < 11600)
        Analog[i + 4] = 100;
      else if (AnaVal1 > 21500)
        Analog[i + 4] = 0;
      else
        Analog[i + 4] = map(AnaVal1, 11600, 21500, 100, 0);

    }
    for (int i = 0; i < 8; i++) {
      Volt[i] = (int)(AnalogRaw[i] * 0.125);
    }
    TimeReadAnalog = millis() + 1000;
  }
}

void DoPumpenTest (void) {
  static bool Running = false;
  if (DoTest) {
    if (millis() < TestRunTime[TestPumpe]) {
      PumpeStatus[TestPumpe] = 99;
      mcp.digitalWrite(TestPumpe, HIGH);
      if (!SayTestOnce) {
        Serial.println("Geht los");
        SayTestOnce = true;
      }
    }
    else {
      mcp.digitalWrite(TestPumpe, LOW);
      PumpeStatus[TestPumpe] = 0;
      DoTest = false;
      SayTestOnce = false;
      Serial.println("Fertg");
    }
  }
}

void RunPumpe(void) {
  for (int i = 0; i < 6; i++) {
    if (TestRunTime[i] > millis()) {
      mcp.digitalWrite(i, HIGH);
      PumpeStatus[i] = true;
    }
    else {
      mcp.digitalWrite(i, LOW);
      PumpeStatus[i] = false;
    }
  }
}
