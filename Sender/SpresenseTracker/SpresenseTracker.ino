#include <Arduino.h>
#include <vector>
#include <GNSS.h>
#include <GNSSPositionData.h>
#include "gnss_logger.h"
#include "gnss_nmea.h"
#include "spresense_e220900t22s_jp_lib.h"

// -----------------------------------------------------------------------
// parameter
#define POSITIONING_INTERVAL  10 // positioning interval in seconds
#define SEND_INTERVAL         10 // send interval in seconds
// -----------------------------------------------------------------------

// enum LoopState
enum LoopState {
  eStateActive  // Loop is activated
};

// GNSS
SpGnss Gnss;                            // SpGnss object

String g_lat;
String g_lot;

// LORA
CLoRa lora;

// LoRa設定値
struct LoRaConfigItem_t config = {
  0xa215,   // own_address 0
  0b011,    // baud_rate 9600 bps
  0b10000,  // air_data_rate SF:9 BW:125
  0b00,     // subpacket_size 200
  0b1,      // rssi_ambient_noise_flag 有効
  0b0,      // transmission_pause_flag 有効
  0b01,     // transmitting_power 13 dBm
  0x00,     // own_channel 0
  0b1,      // rssi_byte_flag 有効
  0b1,      // transmission_method_type 固定送信モード
  0b0,      // lbt_flag 有効
  0b011,    // wor_cycle 2000 ms
  0x0000,   // encryption_key 0
  0x0000,   // target_address 0
  0x00      // target_channel 0
};


//-------------------------------------------------------------
// Turn on / off the LED1 for positioning state notification.
// [in] state Positioning state
//-------------------------------------------------------------
static void Led_isPosfix(bool state)
{
  if (state == 1){
    ledOn(PIN_LED0);
  } else {
    ledOff(PIN_LED0);
  }
}


//-------------------------------------------------------------
// Turn on / off the LED2 for file SD access notification.
// [in] state SD access state
//-------------------------------------------------------------
static void Led_isStorageAccess(bool state)
{
  if (state == 1) {
    ledOn(PIN_LED3);
  } else {
    ledOff(PIN_LED3);
  }
}

//-------------------------------------------------------------
// Turn on / off the LED3 for error notification.
// [in] state Error state
//-------------------------------------------------------------
static void Led_isError(bool state)
{
  /*
  if (state == 1) {
    ledOn(PIN_LED3);
  } else {
    ledOff(PIN_LED3);
  }
  */
}


//-------------------------------------------------------------
// Setup positioning.
// return 0 if success, 1 if failure
//-------------------------------------------------------------
static int SetupPositioning(void)
{
  int error_flag = 0;

  // Set Gnss debug mode.
  Gnss.setDebugMode(PrintNone);
 
  if (Gnss.begin(Serial) != 0) {
    Serial.println("Gnss begin error!!");
    error_flag = 1;
  } else {
    Serial.println("Gnss begin OK.");

    // GPS + QZSS(L1C/A) + QZAA(L1S)
    Gnss.select(GPS);
    Gnss.select(QZ_L1CA);
    Gnss.select(QZ_L1S);
    Gnss.setInterval(POSITIONING_INTERVAL);

    if (Gnss.start(HOT_START) != OK) {
      Serial.println("Gnss start error!!");
      error_flag = 1;
    }
  }

  return error_flag;
}


//-------------------------------------------------------------
//StrSplit
//-------------------------------------------------------------
int StrSplit(String data, char delimiter, String *dst)
{
  int index = 0; 
  int datalength = data.length();
  
  for (int i = 0; i < datalength; i++) {
    char tmp = data.charAt(i);
    if ( tmp == delimiter ) {
      index++;
    } else {
      dst[index] += tmp;
    }
  }
  
  return (index + 1);
}


//-------------------------------------------------------------
//ConvertGps
//-------------------------------------------------------------
float ConvertGps(String data)
{
  float f = atof(data.c_str());
  float deg = (int)f / 100;
  float min = f - deg * 100;
   
  return deg + min / 60;
}

void floatToBytes(float val, uint8_t* bytes)
{
  memcpy(bytes, &val, sizeof(float));
}

void floatArrayToBytes(float* values, uint8_t* bytes)
{
  for (int i = 0; i < 2; ++i) {
    floatToBytes(values[i], bytes + i * sizeof(float));
  }
}

void floatValuesToCommaString(float* values, char* resultString)
{
  String floatStrings[2];
  for (int i = 0; i < 2; ++i) {
    floatStrings[i] = String(values[i], 5);  // Adjust precision as needed
  }

  sprintf(resultString, "%s,%s\0", floatStrings[0].c_str(), floatStrings[1].c_str());
}


//-------------------------------------------------------------
//SendGnssLoraData
//-------------------------------------------------------------
void SendGnssLoraData(String lat, String lot)
{
  float values[2] = { ConvertGps(lat), ConvertGps(lot)};
  char resultString[50];  // Adjust the size as needed
  uint8_t byteData[2 * sizeof(float)];  

  floatArrayToBytes(values, byteData);
  floatValuesToCommaString(values, resultString);
  Serial.print("result = ");
  Serial.println(resultString);

  if (lora.SendFrame(config, resultString, strlen(resultString)) == 0) {
    Serial.println("send succeeded.");
  } else {
    Serial.printf("send failed.");
  }
}


//-------------------------------------------------------------
//Setup
//-------------------------------------------------------------
void setup()
{
  int error_flag = 0;

  // Open serial communications and wait for port to open
  Serial.begin(9600);
  Serial.println("-- READY --");

  // Turn on all LED:Setup start.
  ledOn(PIN_LED0);
  ledOn(PIN_LED1);
  ledOn(PIN_LED2);
  ledOn(PIN_LED3);

  error_flag = SetupPositioning();

  // Turn off all LED:Setup done.
  ledOff(PIN_LED0);
  ledOff(PIN_LED1);
  ledOff(PIN_LED2);
  ledOff(PIN_LED3);

  // Set error LED.
  if (error_flag == 1) {
    Led_isError(true);
  }

  // E220-900T22S(JP)へのLoRa初期設定
  if (lora.InitLoRaModule(config)) {
    SerialMon.printf("init error\n");
    return;
  } else {
    Serial.printf("init ok\n");
  }

  // ノーマルモード(M0=0,M1=0)へ移行する
  lora.SwitchToNormalMode();

  Serial.println("-- START --");
}


//-------------------------------------------------------------
//loop
//-------------------------------------------------------------
void loop() 
{
  // static
  static bool PosFixflag = false;

  // Check update.
  if (Gnss.waitUpdate(POSITIONING_INTERVAL * 1000)) {
    // Get NavData.
    SpNavData NavData;
    Gnss.getNavData(&NavData);

    // Position Fixed
    bool LedSet = ((NavData.posDataExist) && (NavData.posFixMode != 0));
    if (PosFixflag != LedSet) {
      Led_isPosfix(LedSet);
      PosFixflag = LedSet;
    }
    
    if (PosFixflag) {
        //unsigned short sat = NavData.posSatelliteType;
        unsigned short sat = NavData.satelliteType;
        // GPS, QZ_L1CA --> LED1
        if (0 != (sat & (GPS | QZ_L1CA))) {
          ledOn(PIN_LED1);
        } else {
          ledOff(PIN_LED1);  
        }
        // QZ_L1S --> LED2
        if (0 != (sat & QZ_L1S)) {
          ledOn(PIN_LED2);
        } else {
          ledOff(PIN_LED2);  
        }
    }
    
    // Convert Nav-Data to Nmea-String.
    String NmeaString = getNmeaGga(&NavData);
    if (strlen(NmeaString.c_str()) == 0) {
      Serial.println("getNmea error");
      Led_isError(true);
    } else {
      Serial.print(NmeaString);

      // 分割数 = 分割処理(文字列, 区切り文字, 配列) 
      String cmds[15] = {"\0"}; // 分割された文字列を格納する配列 
      int index = StrSplit(NmeaString, ',', cmds);

      // 結果表示
      //for(int i = 0; i < index; i++){
      //  Serial.println(cmds[i]);
      //}
      if (4 < index) {
        g_lat = cmds[2];
        g_lot = cmds[4];

        if (strlen(g_lat.c_str()) != 0 &&
            strlen(g_lot.c_str()) != 0) {
          //Serial.print("lat=");   Serial.println(g_lat);
          //Serial.print("lot=");   Serial.println(g_lot);
          SendGnssLoraData(g_lat, g_lot);
          delay(SEND_INTERVAL * 1000);
          return;
        }
      }
    }
  }

  delay(SEND_INTERVAL * 1000);
}
