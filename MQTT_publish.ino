/*
   MQTT reconnect-Routine
*/
bool reconnectNB() {
  if (!MQTT_Client.connected()) {
    MQTT_Client.connect(ClientName, "Batt", "OnOff");
    if (MQTT_Client.connected())
      Serial.println(F("MQTT Connected"));
    else
      Serial.println(F("MQTT NOT connected!"));

    return MQTT_Client.connected();
  }
}

/*
   hier wird auf den MQTT-Broker veröffentlicht.
*/

bool PublishData (void) {
  const size_t capacity = JSON_OBJECT_SIZE(22);
  DynamicJsonDocument doc(capacity);
  char JSONBuffer[512];
  bool PubState = false;
  doc["temperatur"] = Temperatur;
  doc["druck"] = 0.0;
  doc["feuchte"] = 0.0;
  doc["supply"] = ESP.getVcc();
  doc["ip-address"] = HostIP;
  doc["messung_um"] = MyTime;
  doc["timestamp"] = timeClient.getEpochTime();
  serializeJson(doc, JSONBuffer);
  PubState = MQTT_Client.publish(TOPIC_7, JSONBuffer, true);
  Serial.printf("JSON: %s\n\n", JSONBuffer);
  /*delay(500);
    MQTT_Client.disconnect();
    delay(500);*/
  return (PubState);
}
