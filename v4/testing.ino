
// A small helper
void error(const __FlashStringHelper*err) {
  if(debugMode){
    Serial.println(err);
    while (1);
  }
}

void pcbTest(){
  //requires USB connection and Serial Monitor
  strip.begin();
  analogWrite(hapticPin[0], minHaptics);
  Serial.begin(9600);
  while(!Serial);

  Serial.println("Beginning testing!");
  int testcase = 1;
  while(true){
    switch(testcase){
      case 1:
        Serial.println("Testing button values");
        displayButtonValues();
        testcase++;
        break;
      case 2:
        Serial.println("Testing button ranges");
        checkButtonRanges();
        testcase++;
        break;
      case 3:
        Serial.println("Testing neo pixels");
        testNeoPixels();
        testcase++;
        break;
      case 4:
        Serial.println("Testing haptics");
        testHaptics();
        testcase++;
      default:
        testcase = 0;
        delay(500);

//        Serial.println("Type the test case number");
//        if(Serial.available() > 0){
//          testcase = Serial.parseInt();
//        }
        
        break;
    }
  }
  Serial.println("Completed testing!");
}

void displayButtonValues(){
  while(Serial.available()==0){
    checkButtons();
    int buttonValue = analogRead(buttonPin);
    Serial.println(buttonValue);
    delay(10);
  }
  Serial.readString();
}

void checkButtonRanges(){
  while(Serial.available()==0){
    
    checkButtons();
    
    for(int i = 0; i < numMyButtons; i++){
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
  Serial.readString();
}

void testNeoPixels(){
  int b = 0.5*maxBrightness;
  while(Serial.available()==0){
    for(int i = 0; i < stripLength; i++){
      strip.setPixelColor(i, strip.Color(b,0,0));  
    }
    strip.show();
    delay(2000);
    for(int i = 0; i < stripLength; i++){
      strip.setPixelColor(i, strip.Color(0,b,0));  
    }
    strip.show();
    delay(2000);
    for(int i = 0; i < stripLength; i++){
      strip.setPixelColor(i, strip.Color(0,0,b));  
    }
    strip.show();
    delay(2000);
    
  }
  Serial.readString();
}

void testHaptics(){
  while(Serial.available()==0){
    Serial.println("max");
    analogWrite(hapticPin[0], maxHaptics);
    delay(500);
    Serial.println("min");
    analogWrite(hapticPin[0], minHaptics);
    delay(500);
  }
  Serial.readString();
}


