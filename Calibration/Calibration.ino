//地磁気センサ QMC5883L キャリブレーション
/*手順
1.実行してSerial Monitorの表示に従い、地磁気センサを動かす
2.地磁気センサの方角(角度)が表示される
*/

#include <QMC5883LCompass.h>

QMC5883LCompass compass;

int geo = 0;
float a,b,c,d,e,f;

//QMC5883L
void QMC5883Setup(){
  compass.init();
  compass.setCalibrationOffsets(a,b,c);
  compass.setCalibrationScales(d,e,f);
}

void QMC5883Get(){
  compass.read();
  geo = compass.getAzimuth(); //-180<geo<180で与えられる
  if(geo<0){
   geo+=360; //0<geo<360に変更
  }
  Serial.print(" Azimuth: ");
	Serial.println(geo);
}

void setup() {
  Serial.begin(115200);
  compass.init();


  Serial.println("This will provide calibration settings for your QMC5883L chip. When prompted, move the magnetometer in all directions until the calibration is complete.");
  Serial.println("Calibration will begin in 5 seconds.");
  delay(5000);

  Serial.println("CALIBRATING. Keep moving your sensor...");
  compass.calibrate();

  Serial.println("DONE. Copy the lines below and paste it into your projects sketch.);");
  Serial.println();
  Serial.print("compass.setCalibrationOffsets(");
  Serial.print(compass.getCalibrationOffset(0));
  Serial.print(", ");
  Serial.print(compass.getCalibrationOffset(1));
  Serial.print(", ");
  Serial.print(compass.getCalibrationOffset(2));
  Serial.println(");");
  Serial.print("compass.setCalibrationScales(");
  Serial.print(compass.getCalibrationScale(0));
  Serial.print(", ");
  Serial.print(compass.getCalibrationScale(1));
  Serial.print(", ");
  Serial.print(compass.getCalibrationScale(2));
  Serial.println(");");
  a=compass.getCalibrationOffset(0);
  b=compass.getCalibrationOffset(1);
  c=compass.getCalibrationOffset(2);
  d=compass.getCalibrationScale(0);
  e=compass.getCalibrationScale(1);
  f=compass.getCalibrationScale(2);
  QMC5883Setup();
}

void loop() {
  QMC5883Get();
  delay(1000);
}