#ifdef SUBCORE
#error "Core selection is wrong!!"
#endif

#include <SPI.h>
#include <Arduino.h>
#include <vector>
#include <MP.h>
#include <SoftwareSerial.h>

//Display
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "IMG_WALK.h"
#include "IMG_SCHOOL.h"
#include "IMG_HOME.h"

SoftwareSerial SerialAT(5, 6);  // RX, TX

// -----------------------------------------------------------------------
// parameter
#define SPACE_LAT  0.0015
#define SPACE_LOT  0.0015

#define home_lat 34.XXXXXXXXXXXXXX
#define home_lot 135.XXXXXXXXXXXXXX

#define school_lat 34.XXXXXXXXXXXXXX
#define school_lot 135.XXXXXXXXXXXXXX
// -----------------------------------------------------------------------

//Button
#define BTN_PIN A0

// BUTTONN KEY ID
#define CMD_1    1
#define CMD_2    2
#define CMD_3    3
#define CMD_4    4
#define CMD_5    5
#define CMD_6    6

//TFT
#define TFT_RST 8
#define TFT_DC  9
#define TFT_CS  10

//Dispaly
#define TFT_BACKLIGHT_PIN 7

#define IMG_TYPE_HOME   1
#define IMG_TYPE_SCHOOL 2
#define IMG_TYPE_MOVE   3

Adafruit_ILI9341 tft = Adafruit_ILI9341(&SPI, TFT_DC, TFT_CS, TFT_RST);
bool g_isTftLight = false;

int subcore = 1; // Communication with SubCore1

struct MyPacket {
  float lat;
  float lot;
  int rssi;
};

MyPacket packet;

float g_flat = home_lat;
float g_flot = home_lot;
float g_flat2 = 0;
float g_flot2 = 0;
float g_flat3 = 0;
float g_flot3 = 0;
float g_flat4 = 0;
float g_flot4 = 0;
int g_rssi;

int g_imgType = 0;
int g_cnt_gps = 0;
int g_push_index = 0;
int g_sendCmd = 0;
int g_cnt_send = 0;

//-------------------------------------------------------------
//GpsDataFunction
//-------------------------------------------------------------
int GpsDataFunction(float flat, float flot, int rssi)
{
  int ret = 0;
  // 自宅近く
  if (abs(flat - home_lat) < SPACE_LAT &&
      abs(flot - home_lot) < SPACE_LOT) {
    ret = IMG_TYPE_HOME;
    MPLog("HOME\n");
  }
  // 学校近く
  else if (abs(flat - school_lat) < SPACE_LAT &&
           abs(flot - school_lot) < SPACE_LOT) {
    ret = IMG_TYPE_SCHOOL;
    MPLog("SCHOOL\n");
  }  
  // 動いていない
  else if (abs(flat - g_flat) < SPACE_LAT &&
           abs(flot - g_flot) < SPACE_LOT) {
    ret = IMG_TYPE_MOVE;
    MPLog("MOVE\n");
  }

  // Now Parameter
  g_rssi = rssi;
  g_flat = flat;
  g_flot = flot;

  switch(g_cnt_gps) {
    case 2:
      g_flat2 = flat;
      g_flot2 = flot;
      break;
    case 3:
      g_flat3 = flat;
      g_flot3 = flot;
      break;
    case 4:
      g_flat4 = flat;
      g_flot4 = flot;
      break;
  }
  
  MPLog("Gps:%d, %f, %f\n", g_rssi, g_flat, g_flot);  

  return ret;
}


//-------------------------------------------------------------
//DisplayImageFunction
//-------------------------------------------------------------
bool DisplayImageFunction(int mode, bool isForce)
{
  if (!isForce && g_imgType == mode) {
    return false;
  }

  g_imgType = mode;

  switch(g_imgType) {
    case IMG_TYPE_HOME:
      tft.drawRGBBitmap(170, 73, IMG_HOME, 150, 137); 
      //Serial.println("Draw IMG_HOME");
      break;

    case IMG_TYPE_SCHOOL:
      tft.drawRGBBitmap(170, 94, IMG_SCHOOL, 150, 95); 
      //Serial.println("Draw IMG_SCHOOL");
      break;

    case IMG_TYPE_MOVE:
      tft.drawRGBBitmap(170, 76, IMG_WALK, 150, 132);
      //Serial.println("Draw IMG_TYPE_MOVE");
      break;
  }
  yield();

  return true;
}


//-------------------------------------------------------------
//DisplayTextFunction
//-------------------------------------------------------------
void DisplayTextFunction()
{
  tft.setTextColor(ILI9341_WHITE);  
  tft.setTextSize(2);

  // rssi
  tft.setCursor(10, 18);
  tft.print("RSSI:");
  tft.setCursor(97, 18);
  tft.print(g_rssi);

  // lat, lot
  int yPos = 18 + 35;
  int i = 0;
  char buffer[10];

  tft.setCursor(10, yPos);
  snprintf(buffer, sizeof(buffer), "%f", g_flat);
  tft.print(buffer);
  tft.setCursor(125, yPos);
  tft.print(",");
  tft.setCursor(138, yPos);
  snprintf(buffer, sizeof(buffer), "%f", g_flot);
  yPos += 45;
  tft.print(buffer);
  
  tft.setTextSize(1);
  while (i++ < 3)
  {
    tft.setCursor(10, yPos);
    if (i == 1)      snprintf(buffer, sizeof(buffer), "%f", g_flat2);
    else if (i == 2) snprintf(buffer, sizeof(buffer), "%f", g_flat3);
    else if (i == 3) snprintf(buffer, sizeof(buffer), "%f", g_flat4);
    tft.print(buffer);
  
    tft.setCursor(70, yPos);
    tft.print(",");
    
    tft.setCursor(90, yPos);
    if (i == 1)      snprintf(buffer, sizeof(buffer), "%f", g_flot2);
    else if (i == 2) snprintf(buffer, sizeof(buffer), "%f", g_flot3);
    else if (i == 3) snprintf(buffer, sizeof(buffer), "%f", g_flot4);
    tft.print(buffer);
    
    yPos += 25;
  }
  yield();
}


//-------------------------------------------------------------
//DispRefresh
//-------------------------------------------------------------
void DispRefresh()
{
  tft.fillScreen(ILI9341_BLACK);

  DisplayImageFunction(g_imgType, true);

  DisplayTextFunction();
}


//-------------------------------------------------------------
//DispLight
//-------------------------------------------------------------
void DispLight(bool isTftLight)
{
  if (isTftLight) {
    Serial.println("light on");
    //digitalWrite(TFT_BACKLIGHT_PIN,HIGH);

    DispRefresh();
  }
  else {
    Serial.println("light off");
    //digitalWrite(TFT_BACKLIGHT_PIN,LOW);
  }
}


//-------------------------------------------------------------
//SetupDisplay
//-------------------------------------------------------------
void SetupDisplay()
{
  tft.begin(); 
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  yield();
}


//-------------------------------------------------------------
//SendGps
//-------------------------------------------------------------
void SendGps(float lat, float lot)
{
  if (g_rssi == 0 || lat < 1 || lot < 1) {
    Serial.println("NO GPS");
    return;
  }
  Serial.println("START SendGps.");

  // Make a HTTP request:
  char buffer[35];
  snprintf(buffer, sizeof(buffer), "?lat=%f&lot=%f", lat, lot);
  
  Serial.println(buffer);
  SerialAT.println(buffer);

  Serial.println("END SendGps.");
}


//-------------------------------------------------------------
//getButtonKey
//-------------------------------------------------------------
int getButtonKey()
{
  int index = 0;
  int data = analogRead(BTN_PIN);
  //Serial.println(data);

  if (5 <= data && data <= 70) {
    g_push_index = 1;
  } else if (90 <= data && data <= 150) {
    g_push_index = 2;
  } else if (300 <= data && data <= 350) {
    g_push_index = 3;
  } else if (360 <= data && data <= 500) {
    g_push_index = 4;
  } else if (530 <= data && data <= 700) {
    g_push_index = 5;
  } else {    
    if (g_push_index != 0) {
      index = g_push_index;
      g_push_index = 0;
      Serial.print("btn= "); Serial.println(index);
    }    
  }
  return index;
}

//-------------------------------------------------------------
//commandFunction
//-------------------------------------------------------------
void commandFunction(int cmd)
{
  if (cmd == CMD_1) {
    g_sendCmd = CMD_1;
    SendGps(g_flat, g_flot);
  }
  else if (cmd == CMD_2) {
    g_sendCmd = CMD_2;
    DispRefresh();
  }
  else if (cmd == CMD_3) {
    g_sendCmd = CMD_3;

    tft.fillScreen(ILI9341_BLACK);
    DisplayImageFunction(IMG_TYPE_HOME, true);
    DisplayTextFunction();
  }
  else if (cmd == CMD_4) {
    g_sendCmd = CMD_4;

    tft.fillScreen(ILI9341_BLACK);
    DisplayImageFunction(IMG_TYPE_SCHOOL, true);
    DisplayTextFunction();
  }
  else if (cmd == CMD_5) {
    g_sendCmd = CMD_5;

    tft.fillScreen(ILI9341_BLACK);
    DisplayImageFunction(IMG_TYPE_MOVE, true);
    DisplayTextFunction();
  }
}


//-------------------------------------------------------------
//Setup
//-------------------------------------------------------------
void setup() 
{  
  Serial.begin(9600);
  //Serial2.begin(115200, SERIAL_8N1);
  SerialAT.begin(9600);
  
  //------------------------------------------
  // Button
  Serial.println("Button");
  //pinMode(BTN_PIN, INPUT);

  //------------------------------------------
  // TFT
  Serial.println("Tft");
  SetupDisplay();
  DispLight(g_isTftLight);

  DisplayImageFunction(IMG_TYPE_HOME, true);
  DisplayTextFunction();

  //------------------------------------------
  // Launch SubCore1
  MP.RecvTimeout(100);
  int ret = MP.begin(subcore);
  if (ret < 0) {
    printf("MP.begin error = %d\n", ret);
  }

  Serial.println("=== START ===");
}


//-------------------------------------------------------------
//loop
//-------------------------------------------------------------
void loop() 
{
  // KEY CODE
  int key = getButtonKey();
  commandFunction(key);

  // Subcore
  MyPacket *ppacket;
  int8_t   rcvid;  
  int ret = MP.Recv(&rcvid, &ppacket, subcore);
  if (ret >= 0) {
    ret = GpsDataFunction(ppacket->lat, ppacket->lot, ppacket->rssi);
    if (ret > 0) {
      g_cnt_gps++;
      if (g_cnt_gps > 4) {
        g_cnt_gps = 1;
      }

      tft.fillScreen(ILI9341_BLACK);
      DisplayImageFunction(ret, true);
      DisplayTextFunction();

      g_cnt_send++;
      if (g_cnt_send > 3) {
        g_cnt_send = 0;      
        commandFunction(CMD_1);
      }
    }
    delay(10);
    return;
  }
  
  delay(100);
}
