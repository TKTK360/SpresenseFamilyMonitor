#if (SUBCORE != 1)
#error "Core selection is wrong!!"
#endif

#include <MP.h>
#include <Arduino.h>
#include <vector>
//Lora
#include "spresense_e220900t22s_jp_lib.h"

#define MY_MSGID    10

// LORA
CLoRa lora;
struct RecvFrameE220900T22SJP_t g_data;

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

float g_flat;
float g_flot;
int g_rssi;

struct MyPacket {
  float lat;
  float lot;
  int rssi;
};

MyPacket packet;


void errorLoop(int num)
{
  int i;

  while (1) {
    for (i = 0; i < num; i++) {
      ledOn(LED0);
      delay(300);
      ledOff(LED0);
      delay(300);
    }
    delay(1000);
  }
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
//GpsDataFunction
//-------------------------------------------------------------
int GpsDataFunction(String data)
{
  String cmds[3] = {"\0"}; // 分割された文字列を格納する配列 
  int index = StrSplit(data, ',', cmds);

  if (2 > index) {
    return -1;
  }

  // 結果表示
  String now_lat = cmds[0];
  String now_lot = cmds[1];

  g_flat = atof(now_lat.c_str());
  g_flot = atof(now_lot.c_str());

  return 0;
}


//-------------------------------------------------------------
//LoraFunction
//-------------------------------------------------------------
bool LoraFunction()
{
  MPLog("lora function\n");
  if (lora.RecieveFrame(&g_data) != 0) {
    return false;
  }
  
  String strRev;
  int ret = 0;
  
  MPLog("lora recv data\n");
  for (int i = 0; i < g_data.recv_data_len; i++) {
    strRev += (char)g_data.recv_data[i];
  }

  // GPS Update
  g_rssi = g_data.rssi;
  ret = GpsDataFunction(strRev);  
  if (ret == 0) {
    return true;
  }
  return false;
}


//-------------------------------------------------------------
//SendGps
//-------------------------------------------------------------
void SendGps(float lat, float lot, int rssi)
{
  packet.lat = lat;
  packet.lot = lot;
  packet.rssi = rssi;

  MPLog("Gps:%d, %f, %f\n", rssi, lat, lot);  
  int ret = MP.Send(MY_MSGID, &packet);
  if (ret < 0) {
    MPLog("fail lora send\n");
  }
}


//-------------------------------------------------------------
//Setup
//-------------------------------------------------------------
void setup() 
{
  int ret = MP.begin();
  if (ret < 0) {
    errorLoop(2);
  }

  //------------------------------------------
  // Lora
  MPLog("Lora\n");
  // E220-900T22S(JP)へのLoRa初期設定
  if (lora.InitLoRaModule(config)) {
    MPLog("init error\n");
  } else {
    MPLog("init ok\n");
  }

  // ノーマルモード(M0=0,M1=0)へ移行する
  MPLog("switch to normal mode\n");
  lora.SwitchToNormalMode();

  // LoRa受信
  while (1) {
    if (LoraFunction()) {
      // Send Gps Data
      SendGps(g_flat, g_flot, g_rssi);  
    }
    delay(100);
  }
}


//-------------------------------------------------------------
//loop
//-------------------------------------------------------------
void loop() 
{
}
