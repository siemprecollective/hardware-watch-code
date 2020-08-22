
void handleBluetooth(){
  if(debugMode){
    Serial.println("> handleBluetooth");
  }

  //respond to incoming messages
  while (bluetooth.available()) {
    byte val = bluetooth.read();
    Serial.print("0x");
    Serial.println(val, HEX);

    // we want to confirm any valid
    // message we received except
    // for ack messages so we'll set it
    // to true and undo it otherwise
    bool sendAck = true; 
    
    switch(val){
      case 0x81: // - App confirmed receipt
        Serial.println(">: App confirmed receipt");
        sendAck = false;
        break;
      case 0x82: // - Friend 1 is calling
        Serial.println(">: Friend 1 is calling");
        startCall(0);
        break;
      case 0x83: // - Friend 2 is calling
        Serial.println(">: Friend 2 is calling");
        startCall(1);
        break;
      case 0x84: // - Friend 3 is calling
        Serial.println(">: Friend 3 is calling");
        startCall(2);
        break;
      case 0x85: // - Friend 1 is Available
        Serial.println(">: Friend 1 is Available");
        friendStatus[0] = 3;
        break;
      case 0x86: // - Friend 1 is Interruptable
        Serial.println(">: Friend 1 is Interruptable");
        friendStatus[0] = 2;
        break;
      case 0x87: // - Friend 1 is Busy
        Serial.println(">: Friend 1 is Busy");
        friendStatus[0] = 1;
        break;
      case 0x88: // - Friend 1 is Offline
        Serial.println(">: Friend 1 is Offline");
        friendStatus[0] = 0;
        break;
      case 0x89: // - Friend 2 is Available
        Serial.println(">: Friend 2 is Available");
        friendStatus[1] = 3;
        break;
      case 0x8A: // - Friend 2 is Interruptable
        Serial.println(">: Friend 2 is Interruptable");
        friendStatus[1] = 2;
        break;
      case 0x8B: // - Friend 2 is Busy
        Serial.println(">: Friend 2 is Busy");
        friendStatus[1] = 1;
        break;
      case 0x8C: // - Friend 2 is Offline
        Serial.println(">: Friend 2 is Offline");
        friendStatus[1] = 0;
        break;
      case 0x8D: // - Friend 3 is Available
        Serial.println(">: Friend 3 is Available");
        friendStatus[2] = 3;
        break;
      case 0x8E: // - Friend 3 is Interruptable
        Serial.println(">: Friend 3 is Interruptable");
        friendStatus[2] = 2;
        break;
      case 0x8F: // - Friend 3 is Busy
        Serial.println(">: Friend 3 is Busy");
        friendStatus[2] = 1;
        break;
      case 0x90: // - Friend 3 is Offline
        Serial.println(">: Friend 3 is Offline");
        friendStatus[2] = 0;
        break;
      case 0x91: // - Call disconnected by friend
        Serial.println(">: Call disconnected by friend");
        endCall();
        break;
      case 0x92: // - Still connected to app
        Serial.println(">: Still connected to app");
        resetAppCountdown();
        sendAck = false;
        break;
      default:
        Serial.println(">: Command not recognized");
        sendAck = false;
    }
    if(sendAck){
      bt_confirm();
    }
  }
}

void bt_confirm(){
  if(debugMode){
    Serial.println("> bt_confirm");
  }  
  bt_sendMessage(0x01);
}

void bt_startCall(int i){
  Serial.print("<: Starting call to friend: ");
  Serial.println(i);
  byte friendByte[] = {0x02, 0x03, 0x04};
  bt_sendMessage(friendByte[i]); 
}

void bt_updateStatus(int i){
  Serial.print("<: Setting my status to: ");
  Serial.println(i);
  if((i > 0)&&(i < 4)){
    byte statusByte[] = {0x07, 0x06, 0x05};
    bt_sendMessage(statusByte[i - 1]);  
  }
}

void bt_endCall(){
  Serial.print("<: Ending call!");
  bt_sendMessage(0x08);  
}

void bt_sendMessage(byte m){
  bluetooth.write(m);
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

  Serial.print("app count down: ");
  Serial.println(appCountdown);
}



