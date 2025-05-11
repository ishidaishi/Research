//Serial通信でUWBセンサの測位に基づく角度情報をchar型文字列で受け取り、int型に変換してnRF24L01で送信

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);  // CE, CSN

const byte address[6] = "00001";

int data = 0, predata = 0;


void serialreadstring() {
  String line;   // 受信文字列
  int line_len;  // 受信文字列の長さ

  // シリアル通信で1行（改行コードまで）読み込む
  line = Serial.readStringUntil('\n');
  // 文字列の長さを取得する
  line_len = line.length();
  // 文字列の長さが1文字以上の場合
  if (line_len > 0) {
    // 文字列を整数に変換する
    data = line.toInt();
  }
}

void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.setChannel(1);
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
}

void loop() {
  serialreadstring();
  if (data > 0 && data < 999) {
    if (data != predata) {
      radio.write(&data, sizeof(data));
      predata = data;
    }
  }
  delay(10);
}