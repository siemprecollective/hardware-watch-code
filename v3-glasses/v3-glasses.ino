#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <SoftwareSerial.h>

//============= PINOUT ==============


int btRX = MISO;
int btTX = 3;

int neoPin = A2;
int greenLED = 9;
int yellowLED = 7;
int redLED = 5; 
int hapticPin[] = {10,10,10};
int stripOrder[] = {2,1,0};


int buttonPin = A0;
//=================================

Adafruit_NeoPixel strip = Adafruit_NeoPixel(3, neoPin, NEO_GRB + NEO_KHZ800);

SoftwareSerial bluetooth(btRX, btTX); // RX, TX

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

int buttonCenter[] = {177, 290, 438, 579, 746, 931};
int buttonPadding = 25;
int maxBrightness = 20;
int minHaptics = 255;
int maxHaptics = 0;

int hapticDuration = 500;
int ringing = 0;

int myButtonIndex[] = {3,4,5}; //which button corresponds to each status
int friendButtonIndex[] = {0,1,2}; //which button corresponds to each friend

//=================================

void setup() {
  //haptics
  analogWrite(hapticPin[0], minHaptics);
  analogWrite(11, minHaptics);
  analogWrite(13, minHaptics);

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  int b = maxBrightness;
  int toggle = 0;
  strip.begin();

  Serial.begin(115200);
  while(true){
    
    if(analogRead(buttonPin) > 900){
      toggle = (toggle + 1)%3;
      delay(500);
    }
    Serial.println(toggle);
    switch (toggle) {
      case 0:
        digitalWrite(greenLED, HIGH);
        digitalWrite(yellowLED, LOW);
        digitalWrite(redLED, LOW);
        strip.setPixelColor(0, strip.Color(0,b,0));
        strip.setPixelColor(1, strip.Color(0,b,0));
        strip.setPixelColor(2, strip.Color(0,b,0));
        strip.show();
        break;
      case 1:
        digitalWrite(greenLED, LOW);
        digitalWrite(yellowLED, HIGH);
        digitalWrite(redLED, LOW);
        strip.setPixelColor(0, strip.Color(b,b,0));
        strip.setPixelColor(1, strip.Color(b,b,0));
        strip.setPixelColor(2, strip.Color(b,b,0));
        strip.show();
        break;
      case 2:
        digitalWrite(greenLED, LOW);
        digitalWrite(yellowLED, LOW);
        digitalWrite(redLED, HIGH);
        strip.setPixelColor(0, strip.Color(b,0,0));
        strip.setPixelColor(1, strip.Color(b,0,0));
        strip.setPixelColor(2, strip.Color(b,0,0));
        strip.show();
        break;
    }
  }
  
  Serial.begin(9600);
  if(debugMode)
    Serial.println("> beginning setup");

  //mono LED's
  strip.show(); // Initialize all pixels to 'off'

  //no configuration needed for analog button reading

  //bluetooth
  bluetooth.begin(9600);
  if(debugMode)
    Serial.println("> completed setup");

  checkButtons();
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
  }
  else{
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
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(0, maxBrightness, 0));
  strip.show();

  if(ringing == i)
    ringing = 0;
}

void makeFriendBusy(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is Busy");
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(maxBrightness,0,0));  
  strip.show();

  if(ringing == i)
    ringing = 0;
}

void makeFriendDnD(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is on Do Not Disturb");
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(0.7*maxBrightness,0,maxBrightness));
  strip.show();

  if(ringing == i)
    ringing = 0;
}

void makeFriendCallingOtherFriend(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is calling someone else");
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(0.75 * maxBrightness,0,0.25 * maxBrightness));
  strip.show();

  if(ringing == i)
    ringing = 0;
}

void makeFriendCallingMe(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is calling me");

  if(ringing != i){
    hapticState[i-1] = 500;
    ringing = i;
  }
}

void makeFriendCalledByMe(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is being called by me");
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(0,0,0.5 * maxBrightness));
  strip.show();

  if(ringing == i)
    ringing = 0;
}

void makeFriendInCallWithMe(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is in a Call with me");
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(0,0,maxBrightness));
  strip.show();

  if(ringing == i)
    ringing = 0;
}

void makeFriendOffline(int i){
  Serial.print("Friend ");
  Serial.print(i);
  Serial.println(" is Offline");
  strip.setPixelColor(stripOrder[2-(i-1)], strip.Color(0,0,0));
  strip.show();

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

  float c1 = (0.6 + 0.4 * (sin((timestep * PI / 1000.0))));
  float c2 = 0.0;
  float c3 = (0.8 + 0.2 * (sin(PI + (timestep * PI / 1000.0))));
  
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
      if(hapticState[i] > 250){
        analogWrite(hapticPin[i], maxHaptics);
      }
      else{
        analogWrite(hapticPin[i], minHaptics);
      }
      hapticState[i] = hapticState[i] - 1;
    }
    else{
      analogWrite(hapticPin[i], minHaptics); 
      if(ringing > 0){
        hapticState[i] = 500;
      }
    }
  }
}

