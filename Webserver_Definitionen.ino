void DefServerFunctions(void) {
  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/readJSON", sendJSON);
  server.on("/getJSON", readJSON);
  server.on("/TestPumpe", TestPumpen);
  server.on("/runPumpe", TestPumpen);
  server.on("/hostReboot", RebootHost);
  //Initialize Webserver
  server.on("/", handleRoot);
  server.onNotFound(handleWebRequests); //Set setver all paths are not found so we can handle as per URI
}
