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
int hapticPin[] = {10,11,13};
int yellowLED = 12;

int buttonPin = A0;

//=================================

Adafruit_NeoPixel strip = Adafruit_NeoPixel(3, neoPin, NEO_GRB + NEO_KHZ800);
SoftwareSerial bluetooth(btRX, btTX); // RX, TX

//=================================
int timestep = 0;
int timerate = 1;

int appCountdown = 0;
bool debugMode = false;

//0 - Unassigned (Off)
//1 - Busy (Red)
//2 - Interruptable (Yellow)
//3 - Free (Green)
//4 - Calling (Blue)

int currentStatus = 3; 
int prevStatus = 0;
int prevFriendStatus = 0;

const int numFriends = 3;
int friendStatus[] = {0,0,0};

int buttonState[] = {0,0,0,0,0,0};
int buttonRelease[] = {0,0,0,0,0,0};

int hapticState[] = {0,0,0};

int buttonCenter[] = {138, 243, 416, 598, 775, 931};
int buttonPadding = 25;
int maxBrightness = 20;
int minHaptics = 255;
int maxHaptics = 20;

int hapticDuration = 500;

int myButtonIndex[] = {3,4,5}; //which button corresponds to each status
int friendButtonIndex[] = {2,1,0}; //which button corresponds to each friend

//=================================
void setup() {
  actualSetup();
   //testingSetup();
}

void testingSetup(){
  Serial.begin(9600);

    analogWrite(hapticPin[0], minHaptics);
    analogWrite(hapticPin[1], minHaptics);
    analogWrite(hapticPin[2], minHaptics);

    testAnalogButtons();

//  testHaptics();
//  testButtons();
//  testRGB();
//  testNeo();
//  testBT();

}


void testAnalogButtons(){
  strip.begin();
  while(true){
    lighting_isPairing();
    timestep = (timestep + 50) % 1000;
    int buttonValue = analogRead(buttonPin);
    Serial.print("Analog: ");
    Serial.println(buttonValue);
    
    int buttonInRange = -1;
    for(int i = 0; i < 6; i++){
      if(buttonInRange < 0){
        if((buttonValue >= buttonCenter[i] - buttonPadding)&&(buttonValue < buttonCenter[i] + buttonPadding)){
          buttonInRange = i;
          Serial.print("Button pressed: ");
          Serial.println(i);
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
    delay(50);
  }
}

void testHaptics(){
  Serial.println("testing haptics");
  int test_countdown = 3;
  while(test_countdown > 0){
    Serial.print("Countdown: ");
    Serial.println(test_countdown);
    analogWrite(hapticPin[0], minHaptics);
    analogWrite(hapticPin[1], minHaptics);
    analogWrite(hapticPin[2], minHaptics);
    delay(500);
    analogWrite(hapticPin[0], maxHaptics);
    analogWrite(hapticPin[1], maxHaptics);
    analogWrite(hapticPin[2], maxHaptics);
    delay(500);
    analogWrite(hapticPin[0], maxHaptics);
    analogWrite(hapticPin[1], minHaptics);
    analogWrite(hapticPin[2], minHaptics);
    delay(500);
    analogWrite(hapticPin[0], minHaptics);
    analogWrite(hapticPin[1], maxHaptics);
    analogWrite(hapticPin[2], minHaptics);
    delay(500);
    analogWrite(hapticPin[0], minHaptics);
    analogWrite(hapticPin[1], minHaptics);
    analogWrite(hapticPin[2], maxHaptics);
    delay(500);
    test_countdown--;
  }
}

void testRGB(){
  Serial.println("testing RGB");
  int test_countdown = 3;
  while(test_countdown > 0){
    Serial.println("Off");
    pinMode(greenLED, OUTPUT);
    digitalWrite(greenLED, LOW);
    
    pinMode(redLED, OUTPUT);
    digitalWrite(redLED, LOW);
    
    pinMode(yellowLED, OUTPUT);
    digitalWrite(yellowLED, LOW);  
    delay(2000);
    
    Serial.println("Green");
    pinMode(greenLED, OUTPUT);
    digitalWrite(greenLED, HIGH);
    
    pinMode(redLED, OUTPUT);
    digitalWrite(redLED, LOW);
    
    pinMode(yellowLED, OUTPUT);
    digitalWrite(yellowLED, LOW);  
    delay(2000);

    Serial.println("Yellow");
    pinMode(greenLED, OUTPUT);
    digitalWrite(greenLED, LOW);
    
    pinMode(redLED, OUTPUT);
    digitalWrite(redLED, LOW);
    
    pinMode(yellowLED, OUTPUT);
    digitalWrite(yellowLED, HIGH);  
    delay(2000);

    Serial.println("Red");
    pinMode(greenLED, OUTPUT);
    digitalWrite(greenLED, LOW);
    
    pinMode(redLED, OUTPUT);
    digitalWrite(redLED, HIGH);
    
    pinMode(yellowLED, OUTPUT);
    digitalWrite(yellowLED, LOW);  
    delay(2000);
    
    test_countdown--;
  }  
}

void testButtons(){
  Serial.println("testing buttons");
  
  while(true){
    Serial.println(analogRead(buttonPin));
    delay(500);
  }
//  
//    if(myButtonIndex[0]){
//      test_countdown--;
//      Serial.println("Status is Busy");
//    }
//    if(myButtonIndex[1]){
//      test_countdown--;
//      Serial.println("Status is Interruptable");
//    }
//    if(myButtonIndex[2]){
//      test_countdown--;
//      Serial.println("Status is Free");
//    }
//
//    if(friendButtonIndex[0]){
//      test_countdown--;
//      Serial.println("Calling first friend");
//    }
//    if(friendButtonIndex[1]){
//      test_countdown--;
//      Serial.println("Calling second friend");
//    }
//    if(friendButtonIndex[2]){
//      test_countdown--;
//      Serial.println("Calling third friend");
//    }
}

void actualSetup(){
  
  if(debugMode)
    Serial.begin(9600);
    Serial.println("> beginning setup");

  //haptics
  analogWrite(hapticPin[0], minHaptics);
  analogWrite(hapticPin[1], minHaptics);
  analogWrite(hapticPin[2], minHaptics);

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
  if(debugMode)
    Serial.println("> completed setup");

  checkButtons();
  bool skipPairing = false;
  for(int i = 0; i < 6; i++){
    if(buttonState[i]){
        skipPairing = true;
    }
  }
  if(skipPairing){
    Serial.println("> Skipping pairing!");
    appCountdown = 500;
    friendStatus[0] = 1;
    friendStatus[1] = 3;
    friendStatus[2] = 2;
  }
}

void loop() {
  if(isAppConnected()){
    if(debugMode)
      Serial.println("> if");    
    checkButtons();
  
    if(!inCall()){
      //update my status based on buttons
      for(int i = 0; i < 3; i++){
        if(buttonRelease[myButtonIndex[i]]){
          currentStatus = i + 1; //this happens to be the mapping
          bt_updateStatus(currentStatus);
          break;
        }
      }
    
      //call friends based on buttons
      for(int i = 0; i < numFriends; i++){
        if(buttonRelease[friendButtonIndex[i]]){
          if(friendStatus[i] > 1){ //if not busy, allow call
            startCall(i);
            bt_startCall(i);
            break;
          }
        }
      }
    }
    else{
      int i = whichCall();
      if(i < 0){
        bool errorButtonPressed = false;
        while(!errorButtonPressed){
          Serial.println(" something went wrong! here's some context:");
          printDebug();

          for(int i = 0; i < 3; i++){
            analogWrite(hapticPin[i], minHaptics);
          }
          
          flashErrorLights();

          checkButtons();

          for(int i = 0; i < 3; i++){
            if(buttonRelease[friendButtonIndex[i]]){
              errorButtonPressed = true;
            }
          }

          for(int i = 0; i < 3; i++){
            if(buttonRelease[myButtonIndex[i]]){
              errorButtonPressed = true;
            }
          }
          
        }
        endCall();
      }
      else{
        if(buttonRelease[friendButtonIndex[i]]){
          endCall();
          bt_endCall();
        }
      }
    }
  
    handleBluetooth();
    updateStatusLight();
    updateFriendStatusLights();
    updateHaptics();
  }
  else{
    if(debugMode){
      Serial.print("> else ");
      Serial.println(timestep);
    }
    for(int i = 0; i < 3; i++){
      analogWrite(hapticPin[i], minHaptics); 
    }
    handleBluetooth();
    lighting_isPairing();
  }
  
  timestep = (timestep + 1) % 1000;
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

void updateStatusLight(){
  if(debugMode)
    Serial.println("> updateStatusLight");
  
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
  if(debugMode)
    Serial.println("> updateFriendStatusLights");
  
  float brightness;
  for(int i = 0; i < numFriends; i++){
    switch(friendStatus[i]){
      case 1:
        strip.setPixelColor(i, strip.Color(maxBrightness,0,0)); //red
        break;        
      case 2:
        strip.setPixelColor(i, strip.Color(maxBrightness, 0.5 * maxBrightness,0)); //yellow
        break;  
      case 3:
        strip.setPixelColor(i, strip.Color(0,maxBrightness,0)); //green
        break;
      case 4:
        brightness = maxBrightness * fabs(sin(timestep * PI / 1000.0));
        strip.setPixelColor(i, strip.Color(0,0,int(brightness))); //blue
        break;      
      default:
        strip.setPixelColor(i, strip.Color(0,0,0));
        break;  
    }
  }
  strip.show();
}

void lighting_isPairing(){
  //  if(debugMode)
  //    Serial.println("> lighting_isPairing");
  //  
  float b1 = maxBrightness * (0.5 + 0.5 * (sin(PI + (timestep * PI / 500.0))));
  float b2 = maxBrightness * (0.5 + 0.5 * (sin((0.5 * PI) + timestep * PI / 500.0)));
  float b3 = maxBrightness * (0.5 + 0.5 * (sin(timestep * PI / 500.0)));
  strip.setPixelColor(2, strip.Color(b1,0,b1));
  strip.setPixelColor(1, strip.Color(b2,0,b2));
  strip.setPixelColor(0, strip.Color(b3,0,b3));
  strip.show();
}

void flashErrorLights(){
  strip.setPixelColor(2, strip.Color(maxBrightness,0,0));
  strip.setPixelColor(1, strip.Color(maxBrightness,0,0));
  strip.setPixelColor(0, strip.Color(maxBrightness,0,0));
  strip.show();
  delay(100);
  strip.setPixelColor(2, strip.Color(0,0,0));
  strip.setPixelColor(1, strip.Color(0,0,0));
  strip.setPixelColor(0, strip.Color(0,0,0));
  strip.show();
  delay(100);
}

void updateHaptics(){
  if(debugMode)
    Serial.println("> updateHaptics");
  for(int i = 0; i < 3; i++){
    if(hapticState[i] > 0){
      if((hapticState[i] < (0.3 * hapticDuration)) || (hapticState[i] > (0.7 * hapticDuration))){
        analogWrite(hapticPin[i], maxHaptics);  
      }
      else{
        analogWrite(hapticPin[i], minHaptics);
      }
      hapticState[i] = hapticState[i] - 1;
    }
    else{
      analogWrite(hapticPin[i], minHaptics); 
    }
  }
}

void startCall(int i){
  if(debugMode){
    Serial.print("> starting call! with friend ");
    Serial.println(i);
  }
  prevStatus = currentStatus;
  currentStatus = 4;
  prevFriendStatus = friendStatus[i];
  friendStatus[i] = 4;
  hapticState[i] = hapticDuration;
}

bool inCall(){
  if(debugMode)
    Serial.println("> inCall");
  if(currentStatus == 4)
    return true;
  else
    return false;
}

int whichCall(){
  if(debugMode)
    Serial.println("> whichCall");
  for(int i = 0; i < numFriends; i++){
    if(friendStatus[i] == 4)
      return i;
  }
  return -1;
}

void endCall(){
  if(debugMode)
    Serial.println("> endCall");
  if(inCall()){
    int i = whichCall();
    currentStatus = prevStatus;
    if(i >= 0){
      friendStatus[i] = prevFriendStatus;
      hapticState[i] = 0;
    }
  }
}

bool isAppConnected(){
  if(debugMode)
    Serial.println("> isAppConnected");
  if(appCountdown > 0){
//    appCountdown = appCountdown - 1;
    return true;
  }
  else{
    return false;
  }
}

void resetAppCountdown(){
  if(debugMode)
    Serial.println("> resetAppCountdown");
  appCountdown = 500;
}

