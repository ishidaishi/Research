//UWBセンサデータを受信，振動デバイスの制御

//タグ番号の変更(2 or 3)
int tag = 2;

//DFplayer(振動デバイス制御、音楽プレイヤー)
#include <SoftwareSerial.h>
#include "DFRobotDFPlayerMini.h"
//nRF24L01(無線通信)
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
//QMC5883L(地磁気センサ)
#include <QMC5883LCompass.h>
QMC5883LCompass compass;

//DF player
SoftwareSerial mySerial_1(8, 7);    // RX, TX
SoftwareSerial mySerial_2(9, 6);    // RX, TX
SoftwareSerial mySerial_3(10, 13);  // RX, TX
SoftwareSerial mySerial_4(11, 12);  // RX, TX  (Arduino microでRXピンに指定できるのは8,9,10,11)
DFRobotDFPlayerMini myDFPlayer_1;
DFRobotDFPlayerMini myDFPlayer_2;
DFRobotDFPlayerMini myDFPlayer_3;
DFRobotDFPlayerMini myDFPlayer_4;


//変数
int folder = 4, file_1F = 100, file_2L = 100, file_3B = 100, file_4R = 100;  
//folder4はホワイトノイズ,file7は40秒,file100はデータなし(停止)
//1F:Front,2L:Left,3B:Back,4R:Right
int prefile_1F = file_1F;
int prefile_2L = file_2L;
int prefile_3B = file_3B;
int prefile_4R = file_4R;
int area = 0;
int prearea = 0;
int angle = -1;
int receive = -1;
int prereceive = -1;
int vol = 30;
char inkey;
int geo = 0;
int geodiff = 0;
unsigned long pretime = 0;


void DFplayerSetup() {
  mySerial_1.begin(9600);
  mySerial_2.begin(9600);
  mySerial_3.begin(9600);
  mySerial_4.begin(9600);
  mySerial_1.listen();
  if (!myDFPlayer_1.begin(mySerial_1)) {
    Serial.println(F("Unable to begin1:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }
  delay(1000);
  mySerial_2.listen();
  if (!myDFPlayer_2.begin(mySerial_2)) {
    Serial.println(F("Unable to begin2:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }
  delay(1000);
  mySerial_3.listen();
  if (!myDFPlayer_3.begin(mySerial_3)) {
    Serial.println(F("Unable to begin3:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }
  delay(1000);
  mySerial_4.listen();
  if (!myDFPlayer_4.begin(mySerial_4)) {
    Serial.println(F("Unable to begin4:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
  }
  myDFPlayer_1.volume(vol);
  / myDFPlayer_2.volume(vol);
  myDFPlayer_3.volume(vol);
  myDFPlayer_4.volume(vol);
  Serial.println("OK");
}
void DFplayerPlay() {
  if (prefile_1F != file_1F) {
    prefile_1F = file_1F;
    Serial.print("file1F:");
    Serial.print(file_1F);
    mySerial_1.listen();  // mySerial_1をアクティブに
    myDFPlayer_1.playFolder(folder, file_1F);
    myDFPlayer_1.readState();
  }

  if (prefile_2L != file_2L) {
    prefile_2L = file_2L;
    Serial.print("file2L:");
    Serial.print(file_2L);
    mySerial_2.listen();  // mySerial_2をアクティブに
    myDFPlayer_2.playFolder(folder, file_2L);
    myDFPlayer_2.readState();
  }

  if (prefile_3B != file_3B) {
    prefile_3B = file_3B;
    Serial.print("file3B:");
    Serial.println(file_3B);
    mySerial_3.listen();  // mySerial_3をアクティブに
    myDFPlayer_3.playFolder(folder, file_3B);
    myDFPlayer_3.readState();
  }
  if (prefile_4R != file_4R) {
    prefile_4R = file_4R;
    Serial.print("file4R:");
    Serial.print(file_4R);
    mySerial_4.listen();  // mySerial_4をアクティブに
    myDFPlayer_4.playFolder(folder, file_4R);
    myDFPlayer_4.readState();
  }
}
void DeviceChoice() {
  if (prearea != area) {
    if (area == 1) {  //1F振動
      file_1F = 7;
      file_2L = 100;
      file_3B = 100;
      file_4R = 100;
    }
    if (area == 2) {  //1F,2L振動
      file_1F = 7;
      file_2L = 7;
      file_3B = 100;
      file_4R = 100;
    }
    if (area == 3) {  //2L振動
      file_1F = 100;
      file_2L = 7;
      file_3B = 100;
      file_4R = 100;
    }
    if (area == 4) {  //2L,3B振動
      file_1F = 100;
      file_2L = 7;
      file_3B = 7;
      file_4R = 100;
    }
    if (area == 5) {  //3B振動
      file_1F = 100;
      file_2L = 100;
      file_3B = 7;
      file_4R = 100;
    }
    if (area == 6) {  //3B,4R振動
      file_1F = 100;
      file_2L = 100;
      file_3B = 7;
      file_4R = 7;
    }
    if (area == 7) {  //4R振動
      file_1F = 100;
      file_2L = 100;
      file_3B = 100;
      file_4R = 7;
    }
    if (area == 8) {  //4R,1F振動
      file_1F = 7;
      file_2L = 100;
      file_3B = 100;
      file_4R = 7;
    }
    if (area == 9) {  //振動停止
      file_1F = 100;
      file_2L = 100;
      file_3B = 100;
      file_4R = 100;
    }
    prearea = area;
  }
}
void Area() {
  unsigned long time = millis();  
  if (time - pretime > 1000) {
    area = 9;  //1秒間(1000ms)データ受信しなかったら振動を停止
  }
  if (receive != prereceive) {
    pretime = time;
    prereceive = receive;
    angle = receive + geo;
    if (angle >= 360 && angle < 720) {
      angle -= 360;  //0<angle<360の範囲に収める
    }
    Serial.print("angle:");
    Serial.println(angle);
    if (angle >= 0 && angle < 30 || angle >= 330 && angle < 360) {
      area = 1;
      Serial.print("1F振動");
    }
    if (angle >= 30 && angle < 60) {
      area = 2;
      Serial.print("1F,2L振動");
    }
    if (angle >= 60 && angle < 120) {
      area = 3;
      Serial.print("2L振動");
    }
    if (angle >= 120 && angle < 150) {
      area = 4;
      Serial.print("2L,3B振動");
    }
    if (angle >= 150 && angle < 210) {
      area = 5;
      Serial.print("3B振動");
    }
    if (angle >= 210 && angle < 240) {
      area = 6;
      Serial.print("3B,4R振動");
    }
    if (angle >= 240 && angle < 300) {
      area = 7;
      Serial.print("4R振動");
    }
    if (angle >= 300 && angle < 330) {
      area = 8;
      Serial.print("4R,1F振動");
    }
    Serial.print(", ");
    Serial.print("area:");
    Serial.println(area);
  }
}
//キーボード制御用
void KeyboardPlay() {
  if (Serial.available() > 0) {
    inkey = Serial.read();
    switch (inkey) {
      case 'a':
        file_1F = 5;  
        Serial.println("1F振動");
        break;
      case 'b':
        file_2L = 5;  
        Serial.println("2L振動");
        break;
      case 'c':
        file_3B = 5;  
        Serial.println("3B振動");
        break;
      case 'd':
        file_4R = 5;
        Serial.println("4R振動");
        break;
      case 'e':
        file_1F = 100;
        Serial.println("1F振動停止");
        break;
      case 'f':
        file_2L = 100;
        Serial.println("2L振動停止");
        break;
      case 'g':
        file_3B = 100;
        Serial.println("3B振動停止");
        break;
      case 'h':
        file_4R = 100;
        Serial.println("4R振動停止");
        break;
    }
  }
}


//nRF24L01
RF24 radio(22, 23);  // CE, CSN 
const byte address[6] = "00001";

void nRF24Setup() {
  radio.begin();
  radio.setChannel(1);
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.startListening();
}
void nRF24Receive() {
  if (radio.available()) {
    radio.read(&receive, sizeof(receive));
    Serial.print("Receive: ");
    Serial.println(receive);  
    if (receive >= 1 && receive <= 361) {  
      receive -= 1;
    }

    //Tag3の条件
    if (tag == 3) {
      receive -= 180;
      geo = 180;
      if (receive < 0) {
        receive += 360;
      }
    }

    Serial.print(" Azimuth: ");
    Serial.println(geo);
  }
}



//QMC5883L
void QMC5883Setup() {
  compass.init();
  compass.setCalibrationOffsets(212.00, -358.00, -857.00);
  compass.setCalibrationScales(0.89, 0.91, 1.27);  //実験環境のキャリブレーション結果
  geodiff = 45;  //x軸方向を向いた時の地磁気センサの値                                  
}
void QMC5883Get() {
  compass.read();
  geo = compass.getAzimuth();  //-180<geo<180
  if (geo < 0) {
    geo += 360;  //0<geo<360
  }
  geo -= geodiff;  //地磁気センサのずれを補正
  if (geo < 0) {
    geo += 360;
  }
}


//----------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  DFplayerSetup();
  nRF24Setup();
  QMC5883Setup();
  pretime = millis();
}

void loop() {
  nRF24Receive();
  QMC5883Get();
  Area();
  DeviceChoice();
  DFplayerPlay();
  KeyboardPlay();
  delay(10);
}