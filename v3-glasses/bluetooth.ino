void handleBluetooth(){
  
  //respond to incoming messages
  while (bluetooth.available()) {
    byte val = bluetooth.read();
    Serial.print("Received 0x");
    Serial.print(val, HEX);
    Serial.print(": ");
    if(!isAppConnected){
      lighting_cleared();
    }
    isAppConnected = true;
    switch(val){
      case 0x84:
        makeFriendAvailable(1);
        break;
      case 0x85:
        makeFriendBusy(1);
        break;
      case 0x86:
        makeFriendDnD(1);
        break;
      case 0x87:
        makeFriendCallingOtherFriend(1);
        break;
      case 0x88:
        makeFriendCallingMe(1);
        break;
      case 0x89:
        makeFriendCalledByMe(1);
        break;
      case 0x8A:
        makeFriendInCallWithMe(1);
        break;
      case 0x8B:
        makeFriendOffline(1);
        break;
      case 0x8C:
        makeFriendAvailable(2);
        break;
      case 0x8D:
        makeFriendBusy(2);
        break;
      case 0x8E:
        makeFriendDnD(2);
        break;
      case 0x8F:
        makeFriendCallingOtherFriend(2);
        break;
      case 0x90:
        makeFriendCallingMe(2);
        break;
      case 0x91:
        makeFriendCalledByMe(2);
        break;
      case 0x92:
        makeFriendInCallWithMe(2);
        break;
      case 0x93:
        makeFriendOffline(2);
        break;
      case 0x94:
        makeFriendAvailable(3);
        break;
      case 0x95:
        makeFriendBusy(3);
        break;
      case 0x96:
        makeFriendDnD(3);
        break;
      case 0x97:
        makeFriendCallingOtherFriend(3);
        break;
      case 0x98:
        makeFriendCallingMe(3);
        break;
      case 0x99:
        makeFriendCalledByMe(3);
        break;
      case 0x9A:
        makeFriendInCallWithMe(3);
        break;
      case 0x9B:
        makeFriendOffline(3);
        break;
      case 0x9C:
        makeMeAvailable();
        break;
      case 0x9D:
        makeMeBusy();
        break;
      case 0x9E:
        makeMeDnD();
        break;
      case 0x9F:
        makeMeInACall();
        break;
      default:
        isAppConnected = false;
        Serial.println("Command not recognized");
    }
  }
}

void bt_sendMessage(byte m){
  bluetooth.write(m);
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
  byte message[] = {0x04,0x05,0x06};
  bt_sendMessage(message[i]);    
}


