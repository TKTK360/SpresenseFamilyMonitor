#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>

//Speaker
#include <HTTPClient.h>
#include <base64.h>       // for http basic auth 

// parameter
#define URL_GET_PARAMETER "GET /NODE_NAME/"
#define HOST_SERVER       "lcdpXXX.enebular.com"
#define HOST_SERVER_TAG   "Host: lcdpXXX.enebular.com"
#define JST     3600* 9

// Your WiFi credentials.
const char* ssid = "XXXXXXXXXXXXXX";
const char* pass = "XXXXXXXXXXXXXX";

//参考：https://www.mgo-tec.com/blog-entry-arduino-esp32-ssl-stable-root-ca.html/2
const char* test_root_ca= \
     "-----BEGIN CERTIFICATE-----\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n" \
     "MrY=\n" \
     "-----END CERTIFICATE-----\n";

WiFiClientSecure client;

//------------------------------------------------------------------
//setup
void setup() 
{
  Serial.begin(9600);
  Serial2.begin(9600);
  Serial.println("setup-start");
  
  // WiFi setup
  WiFi.mode(WIFI_STA);  // Disable Access Point
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  
  Serial.println("configTime");
  configTime( JST, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  client.setCACert(test_root_ca);

  Serial.println("setup-end");
}


//-------------------------------------------------------------
//ReadDataFromConsole
//-------------------------------------------------------------
bool ReadDataFromConsole(char *msg)
{
  if (Serial.available() > 0) {
    char incoming_byte = Serial.read();
    if (incoming_byte == 0x00 || incoming_byte > 0x7F)
      return false;
      
    return true;
  }
  return false;
}


//-------------------------------------------------------------
//SendGps
//-------------------------------------------------------------
void SendGps(String posData)
{
  if (posData.length() < 5) {
    return;
  }
  if ((WiFi.status() != WL_CONNECTED)) {
    return;
  }

  // if you get a connection, report back via serial:
  if (!client.connect(HOST_SERVER, 443)) {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
    return;
  }
  
  Serial.print("connected to ");

  // Make a HTTP request:
  String urlParam = URL_GET_PARAMETER;
  posData.trim();
  urlParam += posData;
  urlParam += " HTTP/1.1";
  Serial.println(urlParam);
  
  client.println(urlParam);
  client.println(HOST_SERVER_TAG);
  client.println("Connection: close");
  client.println();

  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  // if there are incoming bytes available
  // from the server, read them and print them:
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  client.stop();
  Serial.println("\nSendGps");
}


//------------------------------------------------------------------
//loop
//------------------------------------------------------------------
void loop()
{
/*  
  // Test
  char msg[200] = { 0 };
  // コンソールから読み込む
  if (ReadDataFromConsole(msg)) {
    String posData = "?lat=";
    posData += shome_lat;
    posData += "&lot=";
    posData += shome_lot;
    SendGps(posData);
  }
/*/  
  //受信
  if(Serial2.available() > 0) {    
    String data = Serial2.readString();
    SendGps(data);
  }
  delay(100);
}
