bool newBoard = true;

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
int neoPin = A1;
int greenLED = 9;
int yellowLED = 7;
int redLED = 5; 
int hapticPin[] = {13,13,13,13,13};
const int stripLength = 6;
int stripOrder[] = {0,1,2,3,4,5};

int buttonPin = A0;
//=================================

Adafruit_NeoPixel strip = Adafruit_NeoPixel(stripLength, neoPin, NEO_GRB + NEO_KHZ800);

Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);
//=================================
int timestep = 0;
int timerate = 1;

bool isAppConnected = false;
bool debugMode = false;

//buttons
const int numFriends = 5;
int friendButtonIndex[] = {0,1,2,3,4}; //which button corresponds to each friend

const int numMyButtons = 1;
int myButtonIndex[] = {5}; //which button corresponds to each status

const int numButtons = 6;
int buttonState[] = {0,0,0,0,0,0};
int buttonRelease[] = {0,0,0,0,0,0};
bool falseButton = true;

int buttonCenter[] = {953,785,658,512,354,218};
int buttonPadding = 40;

//haptics
int hapticState[] = {0,0,0,0,0};
int ringing = 0;
int buzz = 0;
int ringingDuration = 500;
int buzzDuration = 20;
int minHaptics = 255;
int maxHaptics = 40;

//lighting
int maxBrightness = 50;
//=================================

void setup() {
  if(newBoard){
    pcbTest();
    while(true);
  }
  
  //haptics
  analogWrite(hapticPin[0], minHaptics);
  analogWrite(hapticPin[1], minHaptics);
  analogWrite(hapticPin[2], minHaptics);

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

    for(int i = 0; i < numMyButtons; i++){
      if(buttonRelease[myButtonIndex[i]]){
        falseButton = !falseButton;
        if(falseButton){
          bt_sendMyButton(1);
        }
        else{
          bt_sendMyButton(2);
        }
      }
    }
    
    for(int i = 0; i < numFriends; i++){
      if(buttonRelease[friendButtonIndex[i]]){
        bt_sendFriendButton(i);
      }
    }

    
    if(ringing > 0){
      float b = maxBrightness * (0.5 + 0.5 * (sin((0.0 * PI)  + (timestep * PI / 50.0))));
      strip.setPixelColor(stripOrder[(ringing - 1)], strip.Color(0.25*b,0.25*b,b)); //blue
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
  for(int i = 0; i < numButtons; i++){
    if(buttonInRange < 0){
      if((buttonValue >= buttonCenter[i] - buttonPadding)&&(buttonValue < buttonCenter[i] + buttonPadding)){
        buttonInRange = i;
        
      }
    }
  }

  for(int i = 0; i < numButtons; i++){
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
  strip.setPixelColor(stripOrder[(i)], strip.Color(0, b, 0));
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
  strip.setPixelColor(stripOrder[(i)], strip.Color(b,0,0));  
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
  strip.setPixelColor(stripOrder[(i)], strip.Color(0.7*b,0,b));
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
  strip.setPixelColor(stripOrder[(i)], strip.Color(0.75 * b,0,0.25 * b));
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
    hapticState[i] = ringingDuration;
    ringing = i;
  }
}

void makeFriendCalledByMe(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is being called by me");
  float b = maxBrightness;
  strip.setPixelColor(stripOrder[(i)], strip.Color(0.25 * b,0.25 * b,0.5 * b));
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
  strip.setPixelColor(stripOrder[(i)], strip.Color(0,0,b));
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
  strip.setPixelColor(stripOrder[(i)], strip.Color(0,0,0));
  strip.show();

  if(buzz = i)
    buzz = 0;

  if(ringing == i)
    ringing = 0;
}

void makeMeAvailable(){
  Serial.println("I'm Available!");
  float b = maxBrightness;
  strip.setPixelColor(stripOrder[5], strip.Color(0,b,0));
}

void makeMeBusy(){
  Serial.println("I'm Busy!");
  float b = maxBrightness;
  strip.setPixelColor(stripOrder[5], strip.Color(b,0,0));
}

void makeMeDnD(){
  Serial.println("I'm in Do Not Disturb!");
  float b = 0.5*maxBrightness;
  strip.setPixelColor(stripOrder[5], strip.Color(b,b,b));
}

void makeMeInACall(){
  Serial.println("I'm in a call!");
  float b = maxBrightness;
  strip.setPixelColor(stripOrder[5], strip.Color(0,0,b));
}

void lighting_isPairing(){
  for(int i = 0; i < stripLength; i++){
    float r = 0.5 + (0.5*sin(0        + (float(i * PI) / stripLength) + (timestep * PI / 500.0)));
    float g = 0.5 + (0.5*sin((2*PI/3) + (float(i * PI) / stripLength) + (timestep * PI / 500.0))); 
    float b = 0.5 + (0.5*sin((4*PI/3) + (float(i * PI) / stripLength) + (timestep * PI / 500.0)));
    
    float br = maxBrightness * (0.6 + 0.4*sin((float(i * PI) / stripLength) - (timestep * PI / 1000.0)));
    strip.setPixelColor(stripOrder[i], strip.Color(br*r, br*g, br*b)); 
  }
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

