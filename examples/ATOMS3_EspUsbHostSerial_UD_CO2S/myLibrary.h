#define _SKETCH_NAME_ String(__FILE__).substring(String(__FILE__).lastIndexOf('\\') + 1 + String(__FILE__).lastIndexOf('/') + 1, String(__FILE__).lastIndexOf('.')).c_str()

void waitWiFi(const int timeout) {
  auto start_millis = millis();
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
#ifdef __M5UNIFIED_HPP__    
    M5.Display.print(".");
#endif
    if(millis() - start_millis > timeout)
      ESP.restart();
  }
  Serial.println(WiFi.localIP().toString());
#ifdef __M5UNIFIED_HPP__    
    M5.Display.println(WiFi.localIP().toString());
#endif
  
  static auto ntpServer = WiFi.gatewayIP().toString();
  configTime(9 * 60 * 60, 0, ntpServer.c_str()); 
}

void beginWiFi(const char *ssid, const char *pass, const int timeout = 10000) {
  WiFi.begin(ssid, pass);
  waitWiFi(timeout);
}

void beginWiFi(const int timeout = 10000) {
  WiFi.begin();
  waitWiFi(timeout);
}