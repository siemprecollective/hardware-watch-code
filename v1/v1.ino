//============= Questions ==============
//What if I change my status during a call?
//=================================

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include <SoftwareSerial.h>

//============= PINOUT ==============


int btRX = MISO;
int btTX = 3;

int neoPin = 5;
int greenLED = 7;
int redLED = 9; 
int pwm1 = 10;
int pwm2 = 11;
int yellowLED = 12;
int pwm3 = 13;

int buttonPin = A0;

//=================================

Adafruit_NeoPixel strip = Adafruit_NeoPixel(3, neoPin, NEO_GRB + NEO_KHZ800);
SoftwareSerial bluetooth(btRX, btTX); // RX, TX

//=================================
int maxBrightness = 20;
int timestep = 0;
int timerate = 1;

char data = 0;

//0 - Unassigned (Off)
//1 - Busy (Red)
//2 - Interruptable (Yellow)
//3 - Free (Green)
//4 - Calling (Blue)

int currentStatus = 3; 
int prevStatus = 0;
int prevFriendStatus = 0;

const int numFriends = 3;
int friendStatus[] = {1,2,3};

int buttonState[] = {0,0,0,0,0,0};
int buttonRelease[] = {0,0,0,0,0,0};
int motorState[] = {0,0,0};

int buttonCenter[] = {141, 270, 419, 579, 746, 931};
int buttonPadding = 25;

int myButtonIndex[] = {3,4,5}; //which button corresponds to each status
int friendButtonIndex[] = {2,1,0}; //which button corresponds to each friend

//=================================
void setup() {
  Serial.begin(9600);
//  while (!Serial) {
//    ;
//  }
  Serial.println("> beginning setup");
  
  //haptics
  analogWrite(pwm1, 255);
  analogWrite(pwm2, 255);
  analogWrite(pwm3, 255);

  //mono LED's
  pinMode(greenLED, OUTPUT);
  digitalWrite(greenLED, LOW);
  
  pinMode(redLED, OUTPUT);
  digitalWrite(redLED, LOW);
  
  pinMode(yellowLED, OUTPUT);
  digitalWrite(yellowLED, LOW);

  //neo LED's
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  //no configuration needed for analog button reading

  //bluetooth
  bluetooth.begin(9600);
  Serial.println("> completed setup");
}

void loop() {
  checkButtons();

  if(!inCall()){
    //update my status based on buttons
    for(int i = 0; i < 3; i++){
      if(buttonRelease[myButtonIndex[i]]){
        currentStatus = i + 1;
        break;
      }
    }
  
    //call friends based on buttons
    for(int i = 0; i < numFriends; i++){
      if(buttonRelease[friendButtonIndex[i]]){
        if(friendStatus[i] > 1){
          startCall(i);
          break;
        }
      }
    }
  }
  else{
    int i = whichCall();
    if(i < 0){
      while(true){
        Serial.println(" something went wrong! here's some context:");
        printDebug();
        delay(1000);
      }
    }
    else{
      if(buttonRelease[friendButtonIndex[i]]){
        endCall();
      }
    }
  }

  handleBluetooth();
  updateStatusLight();
  updateFriendStatusLights();
  
  timestep = (timestep + 1) % 1000;
}

void checkButtons(){
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

void updateStatusLight(){
  switch(currentStatus){
    case 1:
      digitalWrite(redLED, HIGH);
      digitalWrite(yellowLED, LOW);
      digitalWrite(greenLED, LOW);
      break;
    case 2:
      digitalWrite(redLED, LOW);
      digitalWrite(yellowLED, HIGH);
      digitalWrite(greenLED, LOW);
      break;
    case 3:
      digitalWrite(redLED, LOW);
      digitalWrite(yellowLED, LOW);
      digitalWrite(greenLED, HIGH);
      break;
    case 4:
      digitalWrite(redLED, HIGH);
      digitalWrite(yellowLED, LOW);
      digitalWrite(greenLED, LOW);
      break;      
    default:
      digitalWrite(redLED, LOW);
      digitalWrite(yellowLED, LOW);
      digitalWrite(greenLED, LOW);
  }
}

void updateFriendStatusLights(){
  float brightness;
  for(int i = 0; i < numFriends; i++){
    switch(friendStatus[i]){
      case 1:
        strip.setPixelColor(2-i, strip.Color(maxBrightness,0,0)); //red
        break;        
      case 2:
        strip.setPixelColor(2-i, strip.Color(maxBrightness, 0.5 * maxBrightness,0)); //yellow
        break;  
      case 3:
        strip.setPixelColor(2-i, strip.Color(0,maxBrightness,0)); //green
        break;
      case 4:
        brightness = maxBrightness * fabs(sin(timestep * PI / 1000.0));
        strip.setPixelColor(2-i, strip.Color(0,0,int(brightness))); //blue
        break;      
      default:
        strip.setPixelColor(2-i, strip.Color(0,0,0));
        break;  
    }
  }
  strip.show();
}

void startCall(int i){
  Serial.print("> starting call! with friend ");
  Serial.println(i);
  prevStatus = currentStatus;
  currentStatus = 4;
  prevFriendStatus = friendStatus[i];
  friendStatus[i] = 4;
}

//void takeCall(int i){
//  if(currentStatus > 1){
//    prevStatus = currentStatus;
//    currentStatus = 4;
//    prevFriendStatus = friendStatus[i];
//    friendStatus[i] = 4;
//  }
//}

bool inCall(){
  if(currentStatus == 4)
    return true;
  else
    return false;
}

int whichCall(){
  for(int i = 0; i < numFriends; i++){
    if(friendStatus[i] == 4)
      return i;
  }
  return -1;
}

void endCall(){
  //assert(inCall());
  int i = whichCall();
  currentStatus = prevStatus;
  friendStatus[i] = prevFriendStatus;
  //TODO: send a message over bluetooth saying the call has ended
}

void handleBluetooth(){
  //INPUT from app
  //update friendStatus[] based on messages from phone
  //also tell us if there is an incoming call
  //if a call ended

  //OUTPUT to app
  //starting a call
  //we update our status
  //ending a call
      //assert that the person who ended the call is who we're actually on a call with

//  if(!inCall()){
//    if(timestep % 50 == 0){
//      Serial.println("------> shifting friends status!");
//      for(int i = 0; i < numFriends; i++){
//        friendStatus[i] = (friendStatus[i] % 3) + 1;
//      }
//    }
//  }
//  else{
//    if(timestep % 100 == 0){
//      Serial.println("------> ending the call for you!");
//      endCall();
//    }
//  }
  if(bluetooth.available()) {
    byte val = bluetooth.read();
    Serial.print("received a message: ");
    Serial.println(val);
  } 
}


void printDebug(){
  Serial.println("---debug---");
  
  Serial.print("time: ");
  Serial.println(timestep);
  
  Serial.print("current status: ");
  Serial.println(currentStatus);

  Serial.print("status of friends: ");
  for(int i = 0; i < numFriends; i++){
    Serial.println(friendStatus[i]);
  }

  Serial.print("current status of buttons: ");
  for(int i = 0; i < 6; i++){
    Serial.println(buttonState[i]);
  }

  Serial.print("which buttons were released: ");
  for(int i = 0; i < 6; i++){
    Serial.println(buttonRelease[i]);
  }

  Serial.print("in call: ");
  Serial.println(inCall());

  Serial.print("on call with: ");
  Serial.println(whichCall());
  
}

