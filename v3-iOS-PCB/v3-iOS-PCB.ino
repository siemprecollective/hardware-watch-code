
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"

#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <Adafruit_NeoPixel.h>

//============= PINOUT ==============
int neoPin = A3;
int greenLED = 9;
int yellowLED = 7;
int redLED = 5; 
int hapticPin[] = {10,10,10};
int stripOrder[] = {2,1,0};


int buttonPin = A0;
//=================================

Adafruit_NeoPixel strip = Adafruit_NeoPixel(3, neoPin, NEO_GRB + NEO_KHZ800);

Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);
//=================================
int timestep = 0;
int timerate = 1;

bool isAppConnected = false;
bool debugMode = false;

const int numFriends = 3;
int friendStatus[] = {0,0,0};

int buttonState[] = {0,0,0,0,0,0};
int buttonRelease[] = {0,0,0,0,0,0};

int hapticState[] = {0,0,0};

int buttonCenter[] = {175, 271, 416, 579, 745, 931}; //Watch 2

int buttonPadding = 40;
int maxBrightness = 10;
int minHaptics = 255;
int maxHaptics = 40;

int ringing = 0;
int buzz = 0;
int ringingDuration = 500;
int buzzDuration = 20;

int myButtonIndex[] = {3,4,5}; //which button corresponds to each status
int friendButtonIndex[] = {2,1,0}; //which button corresponds to each friend

//=================================

// A small helper
void error(const __FlashStringHelper*err) {
  if(debugMode){
    Serial.println(err);
    while (1);
  }
}

void pcbTest(){
  //requires USB connection and Serial Monitor
  Serial.begin(9600);
  while(!Serial);

  int testcase = 0;
  while(true){
    Serial.println("Type the test case number");
    if(Serial.available() > 0){
      testcase = Serial.parseInt();
    }
    switch(testcase){
      case 1:
        Serial.println("Testing status lights");
        testStatusLights();
        testcase++;
        break;
      case 2:
        Serial.println("Testing neo pixels");
        testNeoPixels();
        testcase++;
        break;
      case 3:
        Serial.println("Testing buttons");
        testButtons();
        testcase++;
      case 4:
        Serial.println("Testing haptics");
        testHaptics();
        testcase++;
      default:
        testcase = 0;
        delay(500);
        break;
    }
  }
}

void testNeoPixels(){
  int b = maxBrightness;
  strip.begin();
  while(Serial.parseInt() == 0){
    strip.setPixelColor(0, strip.Color(0,b,0));
    strip.setPixelColor(1, strip.Color(0,b,0));
    strip.setPixelColor(2, strip.Color(0,b,0));
    strip.show();
    delay(2000);  
    strip.setPixelColor(0, strip.Color(b,b,0));
    strip.setPixelColor(1, strip.Color(b,b,0));
    strip.setPixelColor(2, strip.Color(b,b,0));
    strip.show();
    delay(2000);  
    strip.setPixelColor(0, strip.Color(b,0,0));
    strip.setPixelColor(1, strip.Color(b,0,0));
    strip.setPixelColor(2, strip.Color(b,0,0));
    strip.show();
    delay(2000);  
  }
}

void testStatusLights(){
  while(Serial.parseInt() == 0){
    Serial.println("Green");
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
    delay(200);  
    Serial.println("Yellow");
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);
    delay(200);  
    Serial.println("Red");
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);
    delay(200);  
  }
}

void testButtons(){
  while(true){
    checkButtons();
    int buttonValue = analogRead(buttonPin);
    Serial.println(buttonValue);
    delay(1);

    for(int i = 0; i < 3; i++){
      if(buttonRelease[myButtonIndex[i]]){
        Serial.print("my button: ");
        Serial.println(i);
      }
    }
    
    for(int i = 0; i < numFriends; i++){
      if(buttonRelease[friendButtonIndex[i]]){
        Serial.print("friend button: ");
        Serial.println(i);
      }
    }
  }
}

void testHaptics(){
  while(Serial.parseInt() == 0){
    Serial.println("max");
    analogWrite(hapticPin[0], maxHaptics);
    delay(500);

  }
    Serial.println("min");
    analogWrite(hapticPin[0], minHaptics);
    delay(500);
}


void setup() {

  
  //haptics
  analogWrite(hapticPin[0], minHaptics);
  analogWrite(hapticPin[1], minHaptics);
  analogWrite(hapticPin[2], minHaptics);

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);

//  pcbTest();
    
  //neo LED's
  strip.begin();
  lighting_isPairing();
  strip.show();

  if(debugMode){
    Serial.begin(115200);
    Serial.println(F("Adafruit Bluefruit Command <-> Data Mode Example"));
    Serial.println(F("------------------------------------------------"));
    /* Initialise the module */
    Serial.print(F("Initialising the Bluefruit LE module: "));
  }
  
  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  if(debugMode)
    Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    if(debugMode)
      Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  if(debugMode){
    Serial.println("Requesting Bluefruit info:");
    /* Print Bluefruit information */
    ble.info();
  
    Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
    Serial.println(F("Then Enter characters to send to Bluefruit"));
    Serial.println();
  
    ble.verbose(false);  // debug info is a little annoying after this point!
  }
  /* Wait for connection */
  while (!ble.isConnected()) {
    lighting_isPairing();
    timestep = (timestep + 10) % 10000;
  }

  if(debugMode)
    Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    if(debugMode)
      Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set module to DATA mode
  if(debugMode)
    Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  if(debugMode)
    Serial.println(F("******************************"));
  
}

void loop() {
  if(isAppConnected){
      
    checkButtons();

    for(int i = 0; i < 3; i++){
      if(buttonRelease[myButtonIndex[i]]){
        bt_sendMyButton(i);
      }
    }
    
    for(int i = 0; i < numFriends; i++){
      if(buttonRelease[friendButtonIndex[i]]){
        bt_sendFriendButton(i);
      }
    }

    
    if(ringing > 0){
      float b = maxBrightness * (0.5 + 0.5 * (sin((0.0 * PI)  + (timestep * PI / 50.0))));
      strip.setPixelColor(stripOrder[2-(ringing - 1)], strip.Color(0.25*b,0.25*b,b)); //blue
      strip.show();
    }

    
    handleBluetooth();
    updateHaptics();
    if(timestep == 1000){
      bt_getUpdate();
    }
  }
  else{
    Serial.println("pairing");
    for(int i = 0; i < 3; i++){
      analogWrite(hapticPin[i], minHaptics); 
    }
    lighting_isPairing();
    handleBluetooth();
  }
  
  timestep = (timestep + 1) % 10000;
}

void checkButtons(){
  if(debugMode)
    Serial.println("> checkButtons");
  
  int buttonValue = analogRead(buttonPin);

  int buttonInRange = -1;
  for(int i = 0; i < 6; i++){
    if(buttonInRange < 0){
      if((buttonValue >= buttonCenter[i] - buttonPadding)&&(buttonValue < buttonCenter[i] + buttonPadding)){
        buttonInRange = i;
      }
    }
  }

  for(int i = 0; i < 6; i++){
    buttonRelease[i] = 0;
    if(i == buttonInRange){
      buttonState[buttonInRange] = 1;
    }
    else{
      if(buttonState[i] == 1){
        buttonRelease[i] = 1;
      }
      buttonState[i] = 0;
    }
  }
}

void makeFriendAvailable(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is Available");
  float b = maxBrightness;
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(0, b, 0));
  strip.show();

  if(buzz = i)
    buzz = 0;

  if(ringing == i)
    ringing = 0;
}

void makeFriendBusy(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is Busy");
  float b = maxBrightness;
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(b,0,0));  
  strip.show();

  if(buzz = i)
    buzz = 0;

  if(ringing == i)
    ringing = 0;
}

void makeFriendDnD(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is on Do Not Disturb");
  float b = maxBrightness;
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(0.7*b,0,b));
  strip.show();

  if(buzz = i)
    buzz = 0;

  if(ringing == i)
    ringing = 0;
}

void makeFriendCallingOtherFriend(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is calling someone else");
  float b = maxBrightness;
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(0.75 * b,0,0.25 * b));
  strip.show();

  if(buzz = i)
    buzz = 0;

  if(ringing == i)
    ringing = 0;
}

void makeFriendCallingMe(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is calling me");

  if(buzz = i)
    buzz = 0;

  if(ringing != i){
    hapticState[i-1] = ringingDuration;
    ringing = i;
  }
}

void makeFriendCalledByMe(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is being called by me");
  float b = maxBrightness;
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(0.25 * b,0.25 * b,0.5 * b));
  strip.show();

  if(buzz = i)
    buzz = 0;

  if(ringing == i)
    ringing = 0;
}

void makeFriendInCallWithMe(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is in a Call with me");
  float b = maxBrightness;
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(0,0,b));
  strip.show();

  if(buzz != i){
    Serial.println("starting  buzz");
    hapticState[i-1] = buzzDuration;
    buzz = i;
  }
  
  if(ringing == i)
    ringing = 0;
}

void makeFriendOffline(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is Offline");
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(0,0,0));
  strip.show();

  if(buzz = i)
    buzz = 0;

  if(ringing == i)
    ringing = 0;
}

void makeMeAvailable(){
  Serial.println("I'm Available!");
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, HIGH);
}

void makeMeBusy(){
  Serial.println("I'm Busy!");
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, HIGH);
  digitalWrite(greenLED, LOW);
}

void makeMeDnD(){
  Serial.println("I'm in Do Not Disturb!");
  digitalWrite(redLED, HIGH);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, LOW);
}

void makeMeInACall(){
  Serial.println("I'm in a call!");
  digitalWrite(redLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(greenLED, LOW);
}

void lighting_isPairing(){

  float b1 = maxBrightness * (0.55 + 0.45 * (sin(PI + (timestep * PI / 500.0))));
  float b2 = maxBrightness * (0.55 + 0.45 * (sin((0.5 * PI) + timestep * PI / 500.0)));
  float b3 = maxBrightness * (0.55 + 0.45 * (sin(timestep * PI / 500.0)));
  
//  float b1 = maxBrightness * (0.7 + 0.3 * (sin((0.0 * PI)  + (timestep * PI / 500.0))));
//  float b2 = maxBrightness * (0.7 + 0.3 * (sin((0.66 * PI) + (timestep * PI / 500.0))));
//  float b3 = maxBrightness * (0.7 + 0.3 * (sin((1.23 * PI) + (timestep * PI / 500.0))));

  float c1 = (0.8 + 0.2 * (sin((timestep * PI / 500.0))));
  float c2 = 0.0;
  float c3 = (0.4 + 0.3 * (sin(PI + (timestep * PI / 500.0))));
  
  strip.setPixelColor(stripOrder[0], strip.Color(b1*c1,b1*c2,b1*c3));
  strip.setPixelColor(stripOrder[1], strip.Color(b2*c1,b2*c2,b2*c3));
  strip.setPixelColor(stripOrder[2], strip.Color(b3*c1,b3*c2,b3*c3));
  strip.show();
}

void lighting_cleared(){
  strip.setPixelColor(stripOrder[0], strip.Color(0,0,0));
  strip.setPixelColor(stripOrder[1], strip.Color(0,0,0));
  strip.setPixelColor(stripOrder[2], strip.Color(0,0,0));
  strip.show();
}

void updateHaptics(){
  if(debugMode)
    Serial.println("> updateHaptics");
  for(int i = 0; i < 3; i++){
    if(hapticState[i] > 0){
      if(ringing > 0){
        if(hapticState[i] > 250){
          analogWrite(hapticPin[i], maxHaptics);
        }
        else{
          analogWrite(hapticPin[i], minHaptics);
        }
      }
      else if(buzz > 0){
        analogWrite(hapticPin[0], maxHaptics);
        delay(10);    
      }
      hapticState[i] = hapticState[i] - 1;
    }
    else{
      analogWrite(hapticPin[i], minHaptics); 
      if(ringing > 0){
        hapticState[i] = ringingDuration;
      }      
    }
  }
}

