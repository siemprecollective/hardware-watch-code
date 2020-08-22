void handleBluetooth(){
  
  //respond to incoming messages
  while (ble.available()) {
    byte val = ble.read();
    Serial.print("Received 0x");
    Serial.print(val, HEX);
    Serial.print(": ");
    if(!isAppConnected){
      analogWrite(hapticPin[0], maxHaptics);
      delay(200);
      analogWrite(hapticPin[0], minHaptics);
      delay(200);
      analogWrite(hapticPin[0], maxHaptics);
      delay(400);
      analogWrite(hapticPin[0], minHaptics);  
      lighting_cleared();   
    }
    isAppConnected = true;
    switch(val){
      case 0x84:
        makeMeAvailable();
        break;
      case 0x85:
        makeMeBusy();
        break;
      case 0x86:
        makeMeDnD();
        break;
      case 0x87:
        makeMeInACall();
        break;
      default:
        if((val >= 0x88) && (val < 0x88 + (numFriends * 8))){
          int temp = val - 0x88;
          int i = temp / 8;
          switch(temp % 8){
            case 0:
              makeFriendAvailable(i);
              break;
            case 1:
              makeFriendBusy(i);
              break;
            case 2:
              makeFriendDnD(i);
              break;
            case 3:
              makeFriendCallingOtherFriend(i);
              break;
            case 4:
              makeFriendCallingMe(i);
              break;
            case 5:
              makeFriendCalledByMe(i);
              break;
            case 6:
              makeFriendInCallWithMe(i);
              break;
            case 7:
              makeFriendOffline(i);
              break;
          }
        }
        else{
          isAppConnected = false;
          Serial.println("Command not recognized");  
        }      
    }
  }
}

void bt_sendMessage(byte m){
  ble.print(char(m));
  Serial.print("Sending ");
  Serial.println(m);
}

void bt_sendMyButton(int i){
  Serial.print("> Pressing My Button ");
  Serial.println(i);
  byte message[] = {0x01,0x02,0x03};
  bt_sendMessage(message[i]);    
}

void bt_sendFriendButton(int i){
  Serial.print("> Pressing Friend Button ");
  Serial.println(i);
  byte message = 0x04 + i;
  bt_sendMessage(message);    
}

void bt_getUpdate(){
  ble.print(char(0x00));
}

