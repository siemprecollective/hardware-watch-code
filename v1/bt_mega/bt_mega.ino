#define ledPin 13
int state = 0;

#include <SoftwareSerial.h>
SoftwareSerial bluetooth(2, 3); // RX, TX

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(9600); // Default communication rate of the Bluetooth module
  bluetooth.begin(9600);

}
void loop() {
 if(bluetooth.available() > 0){ // Checks whether data is comming from the serial port
    state = bluetooth.read(); // Reads the data from the serial port
   
   if (state == '0') {
    digitalWrite(ledPin, LOW); // Turn LED OFF
    bluetooth.println('0');
    Serial.println("LED: OFF"); // Send back, to the phone, the String "LED: ON"
   }
   else if (state == '1') {
    digitalWrite(ledPin, HIGH);
    bluetooth.println('1'); 
    Serial.println("LED: ON");;
   }
 }
}
