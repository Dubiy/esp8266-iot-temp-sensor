#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "WIFIlibGARY.h"
#include <DHT.h>
#define DHTTYPE DHT22
#define DHTPIN  2

char ssid[32];
char pass[32];
char apihost[64];
char apiurl[64];
char apikey[32];
uint16_t port = 80;
unsigned long previousMillis = 0;
const long interval = 60000;
float humidity, temp_f;
  
// HTTP server will listen at port 80
ESP8266WebServer server(80);
WiFiClient client;
DHT dht(DHTPIN, DHTTYPE, 30);

void setup(void) {
  EEPROM.begin(250);
  Serial.begin(9600);
  delay(500);

  String _ssid = getEEPROMString(0, 32);
  String _pass = getEEPROMString(32, 64);
  String _apihost = getEEPROMString(64, 128);
  String _apiurl = getEEPROMString(128, 192);
  String _apikey = getEEPROMString(192, 224);
  for (int i = 0; i < 32; i++) {
    ssid[i] =  _ssid.charAt(i);  
    if (_ssid.charAt(i) == 0) {
      break;
    }
  }

  for (int i = 0; i < 32; i++) {
    pass[i] =  _pass.charAt(i);  
    if (_pass.charAt(i) == 0) {
      break;
    }
  }

  for (int i = 0; i < 64; i++) {
    apihost[i] =  _apihost.charAt(i);  
    if (_apihost.charAt(i) == 0) {
      break;
    }
  }
  
  for (int i = 0; i < 64; i++) {
    apiurl[i] =  _apiurl.charAt(i);  
    if (_apiurl.charAt(i) == 0) {
      break;
    }
  }

  for (int i = 0; i < 32; i++) {
    apikey[i] =  _apikey.charAt(i);  
    if (_apikey.charAt(i) == 0) {
      break;
    }
  }

  // Connect to WiFi network
  WiFi.begin(ssid, pass);

  // Wait for connection
  int tryCount = 40;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (tryCount-- <= 0) {
      break;
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP);
    // Do a little work to get a unique-ish name. Append the
    // last two bytes of the MAC (HEX'd) to "GARY METEO-":
    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                   String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
    macID.toUpperCase();
    String AP_NameString = "ESP8266 GARY METEO" + macID;
  
    char AP_NameChar[AP_NameString.length() + 1];
    memset(AP_NameChar, 0, AP_NameString.length() + 1);
  
    for (int i=0; i<AP_NameString.length(); i++)
      AP_NameChar[i] = AP_NameString.charAt(i);
  
    WiFi.softAP(AP_NameChar, "");  
  }

  server.on("/", []() {
    server.send(200, "text/html", form(ssid, pass, apihost, apiurl, apikey));
  });
  server.on("/config", handle_config);
  server.begin();
}

void loop(void) {
  // check for incomming client connections frequently in the main loop:
  server.handleClient();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval || currentMillis < previousMillis) { //на випадок, якщо дійде до максимального значення, і почне рахувати заново 
    previousMillis = currentMillis;
  
    if (!client.connect(apihost, port)) {
      Serial.println("connection failed");
      return;
    }

    
    temp_f = dht.readTemperature(false);    // Read temperature as Fahrenheit
    humidity = dht.readHumidity();          // Read humidity (percent)

    Serial.print("humidity=");
    Serial.println(humidity);
    
    Serial.print("temp=");
    Serial.println(temp_f);
    
    if (isnan(humidity) || isnan(temp_f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    
    // We now create a URI for the request
    Serial.print("Requesting URL: ");
    Serial.println(apiurl);

    Serial.print("humidity=");
    Serial.println(humidity);
    
    Serial.print("temp=");
    Serial.println(temp_f);
    
    String data = "&temp=" + String((int)temp_f) + "&humidity=" + String((int)humidity);
    client.print(String("GET ") + apiurl + "?apikey=" + apikey + data + " HTTP/1.1\r\n" +
                 "Host: " + apihost + "\r\n" + 
                 "Connection: close\r\n\r\n");
    delay(10);
    
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
   
    Serial.println();
    Serial.println("closing connection");
  }
}

void handle_config() {
  String _ssid = server.arg("ssid");
  String _pass = server.arg("pass"); 
  String _apihost = server.arg("apihost");
  String _apiurl = server.arg("apiurl");
  _apiurl.replace("%2F", "/");
  String _apikey = server.arg("apikey");
 
  setEEPROMString(0,  32, _ssid);
  setEEPROMString(32, 64, _pass);
  setEEPROMString(64, 128, _apihost);
  setEEPROMString(128, 192, _apiurl);
  setEEPROMString(192, 224, _apikey);
  EEPROM.commit();
  
  ESP.reset();
}


String form(String ssid, String pass, String apihost, String apiurl, String apikey) {
  return 
  "<html><head><title>WiFi Weather-station [sender] : Admin-panel</title></head><body>"
  "<p>"
  "<center>"
  "<h2>Welcome to WiFi Weather-Station: sender</h2>"
  "<div>"
  "<form action='config'>"
  "<label for='ssid'>SSID</label><input id='ssid' type='text' name='ssid' maxlength=32 value='" + ssid + "'><br />"
  "<label for='pass'>Password</label><input id='pass' type='password' name='pass' maxlength=32 value='" + pass + "'><br />"
  "<label for='apihost'>API HOST</label><input id='apihost' type='text' name='apihost' maxlength=64 value='" + apihost + "'><br />"
  "<label for='apiurl'>API URL</label><input id='apiurl' type='text' name='apiurl' maxlength=64 value='" + apiurl + "'><br />"
  "<label for='apikey'>API KEY</label><input id='ssid' type='text' name='apikey' maxlength=32 value='" + apikey + "'><br />"
  "<input type='submit' value='Save & reboot'>"
  "</form>"
  "</div>"
  "<a href='http://garik.pp.ua'>Igor Dubiy</a> (<a href='mailto:netzver@gmail.com'>netzver@gmail.com</a>), 2015"
  "</center>"
  "<style>*{font-size:120%;font-family: monospace;}form * {display: block; width: 100%; margin: 5px 0;}div{padding: 20px; margin: 5px 20px; border: 1px solid gray;}</style>"
  "</body></html>"
  ;  
}

//todo
//config interval
//config IDLE mode (go sleep, wake up)