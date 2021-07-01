/*
   Elke's Wasserwerk für die Überwinterung
*/
#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h> //für http-Update
#include <ESP8266Ping.h>
#include <DNSServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266Ping.h>
#include <FS.h>   //Include File System Headers
//#include <LittleFS.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUDP.h>
#include <TimeLib.h>
#include <Timezone.h>     // from https://github.com/JChristensen/Timezone
#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1015.h>

ADC_MODE(ADC_VCC);

#define sdaPIN D2
#define sclPIN D1
Adafruit_MCP23017 mcp;

#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensor(&oneWire);
DeviceAddress insideThermometer;

// ADC ADS1115
Adafruit_ADS1115 ads0(0x49); // ADS1115, single ended
Adafruit_ADS1115 ads1(0x48); // ADS1115, single ended
float Temperatur, Vcc;

//MQTT
#define INTERVAL 60 //Interval Sekunden zwischen den Messungen
#define RECOVER 10  //Interval, wenn Publish nicht erfolgreich
#define TOPIC_1 "19a/wasserwerk/cpusupply"
#define TOPIC_2 "19a/wasserwerk/timestamp"
#define TOPIC_3 "19a/wasserwerk/temperatur"
#define TOPIC_6 "19a/wasserwerk/ip-adresse"
#define TOPIC_7 "19a/wasserwerk/daten"
char FloatString[30];

WiFiClient espClient;
PubSubClient MQTT_Client(espClient);
const char* mqtt_server = "192.168.2.13";
const char ClientName[] = "19a-wasserwerk";

// NTP
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone LTZ(CEST, CET);    // this is the Timezone object that will be used to calculate local time
TimeChangeRule *tcr;        //pointer to the time change rule, use to get the TZ abbrev
const long utcOffsetInSeconds = 0;
unsigned long UpdateIntervall = 1800;
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", utcOffsetInSeconds, UpdateIntervall);
tmElements_t TP;
const char* Wochentag[8] = {
  "Fehler",
  "Sonntag",
  "Montag",
  "Dienstag",
  "Mittwoch",
  "Donnerstag",
  "Freitag",
  "Samstag"
};
// Ende NTP

ESP8266WebServer server(80); //Server on port 80
ESP8266HTTPUpdateServer serverUpdater;

IPAddress ip, ClientIP;
char HostIP[15];
time_t LastShow = 0, LastSync = 0, LastPing = 0, LastPub = 0;
char MyTime[48];

struct EEData_t {
  int check;
  bool PumpeDa[8];
  int PumpeZeit[8];
  char Pflanzen[8][17];
};

time_t DaysLeft = 0;

EEData_t Config;

bool PumpeStatus[8] = {false, false, false, false, false, false, false, false};
int PublishOK = 0;

// I2C Scan
long LastScan;
// Ende

// ADC
int Analog[8];
int AnalogRaw[8];
int Volt[8];
// Ende

// Pumpentest
int TestPumpe;
bool DoTest = false;
bool SayTestOnce = false;
int TestZeit;
unsigned long TestRunTime[8] = {0, 0, 0, 0, 0, 0, 0, 0};
// Ende

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.printf("Wasserwerk, Compilerzeit: %s %s\n",__DATE__,__TIME__);
  WiFiManager wifiManager;
  SPIFFS.begin();
  EEPROM.begin(512);
  sensor.begin();
  Wire.begin(sdaPIN, sclPIN);

  // hir die I2C-Schnittstellen
  // Parallel Expander
  mcp.begin(); // use default address 0
  for (int i = 0; i < 8; i++) {
    mcp.pinMode(i, OUTPUT);
    mcp.digitalWrite(i, LOW);
  }
  // ADCs
  ads0.setGain(GAIN_ONE);
  ads0.begin();
  ads1.setGain(GAIN_ONE);
  ads1.begin();

  uint8_t macAddr[6];
  char Hostname[15];
  WiFi.macAddress(macAddr);
  sprintf(Hostname, "ottO-%02x-%02x-%02x", macAddr[3], macAddr[4], macAddr[5]);
  WiFi.hostname(Hostname);
  wifiManager.setDebugOutput(false);
  //wifiManager.resetSettings();
  if (!wifiManager.autoConnect("Wasserwerk-AP")) {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.restart();
    delay(5000);
  }
  ip = WiFi.localIP();
  sprintf(HostIP, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  Serial.printf("IP-Adresse ist %s\n", HostIP);
  setSyncProvider(SyncTimeToNTP);
  setSyncInterval(1800);
  LastSync = now();
  InitEE();
  GetEE();
  MQTT_Client.setServer(mqtt_server, 1883);
  DefServerFunctions();
  serverUpdater.setup(&server);
  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void loop() {
  server.handleClient();
  MDNS.update();
  //DoPumpenTest();
  RunPumpe();
  ReadAnalog();
  if (now() > LastPub) {
    sprintf(MyTime, "%02d-%02d-%04d %02d:%02d:%02d UTC", day(timeClient.getEpochTime()), month(timeClient.getEpochTime()), year(timeClient.getEpochTime()), \
            hour(timeClient.getEpochTime()), minute(timeClient.getEpochTime()), second(timeClient.getEpochTime()));
    if (reconnectNB()) {
      if (PublishData()) {
        LastPub = now() + INTERVAL;
        PublishOK = 1;
      }
      else {
        LastPub = now() + RECOVER;
        PublishOK = 0;
      }
    }
    else {
      LastPub = now() + RECOVER;
      PublishOK = 2;
    }
  }
}
