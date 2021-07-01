/*
   Hier werden die persistenten Daten
   der Rest-of-Work-Uhr im EEPROM
   behandelt.
*/
void InitEE (void) {
  EEPROM.get(0, Config);
  if (Config.check != 0x55aa) {
    Config.check = 0x55aa;
    for (int i = 0; i < 6; i++) {
      Config.PumpeDa[i] = false;
      strcpy(Config.Pflanzen[i], "Noch nix");
      Config.PumpeZeit[i] = 10;
    }
    Config.PumpeDa[6] = true;
    Config.PumpeZeit[6] = 0;
    strcpy(Config.Pflanzen[6], "Überlauf");
    Config.PumpeDa[7] = true;
    Config.PumpeZeit[7] = 0;
    strcpy(Config.Pflanzen[7], "Wassereimer");
    EEPROM.put(0, Config);
    EEPROM.commit();
    Serial.println("EEPROM initialisiert");
  }
  else {
    Serial.println("EEPROM ok");
  }
}

void GetEE (void) {
  EEPROM.get(0, Config);  
}

void PutEE (void) {
  EEPROM.put(0, Config);
  EEPROM.commit();
}
