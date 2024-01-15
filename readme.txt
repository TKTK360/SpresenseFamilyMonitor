■電子パーツ
【Sender】
・SONY SPRESENSE メインボード
・ソニー SPRESENSE?用 LoRa Add-onボード（DTH-SSLR）

【Reciever】
・SONY SPRESENSE メインボード
・SONY SPRESENSE 拡張ボード
・ESP32Dev
・AnalogMultiButton
・ILI9341搭載2.8インチSPI制御TFT液晶

■ソフトウェア
・Arduino

■Cloud
・Enebular
　クラウド実行環境を使用
　GPS値をデータストアに貯蓄
　クライアント閲覧時にデータストア値を元に、Openstreetmapで位置情報の表示


■配線
SPRESENSE    ILI9341
--------------------------------------------
AREF         VCC
GND          GND
SCK          SCK
MISO         MISO
MOSI         MOSI
CS           CS
PWM2         DC
GPIO         RESET
3.3V         VCC


SPRESENSE    ad keyboard simulate five key
--------------------------------------------
A0           OUT
GND          GND
Vout         VCC


SPRESENSE   ESP32
--------------------------------------------
D05(RX)     16(TX)
D06(TX)     17(RX)
