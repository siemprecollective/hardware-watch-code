/*
AUTHOR: Hazim Bitar (techbitar)
DATE: Aug 29, 2013
LICENSE: Public domain (use at your own risk)
CONTACT: techbitar at gmail dot com (techbitar.com)
*/

//https://forum.chumbaka.asia/blog/2018/03/23/renaming-hc-05/

//Solder key pin on BT on Pin 6 on Arduino
//Desolder 5V connection to BT and connect through breadboard
//Make sure hello sends
//Then type AT and wait for okay
//then, AT+NAME=SiempreWatch-000X
//then, AT+PSWD="XXXX"

//#include <SoftwareSerial.h>
//
//SoftwareSerial Serial1(MISO, 3);   // RX | TX

void setup() 
{

  analogWrite(10, 255);
  analogWrite(11, 255);
  analogWrite(13, 255);
  
  pinMode(6, OUTPUT);    // this pin will pull the HC-05 
                         // pin 34 (KEY pin) HIGH to switch
                         // module to AT mode

  digitalWrite(6, HIGH); 
  Serial.begin(9600);
  while(!Serial);
  Serial.println("hello");
  delay(5000);
  Serial1.begin(38400);  // HC-05 default speed in AT 
                          // command mode
}

void loop()
{
  // Keep reading from HC-05 and send to Arduino 
  // Serial Monitor
  if (Serial1.available())
    Serial.write(Serial1.read());

  // Keep reading from Arduino Serial Monitor 
  //  and send to HC-05
  if (Serial.available())
    Serial1.write(Serial.read());
}
 



