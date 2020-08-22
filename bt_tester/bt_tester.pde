import processing.serial.*;
Serial myPort;
String ledStatus="LED: OFF";
void setup(){
  size(450, 500);
  myPort = new Serial(this, "/dev/cu.HC05-SPPDev", 38400); // Starts the serial communication
  myPort.bufferUntil('\n'); // Defines up to which character the data from the serial port will be read. The character '\n' or 'New Line'
}
void serialEvent (Serial myPort){ // Checks for available data in the Serial Port
  ledStatus = myPort.readStringUntil('\n'); //Reads the data sent from the Arduino (the String "LED: OFF/ON) and it puts into the "ledStatus" variable
}
void draw(){
  background(237, 240, 241);
  fill(20, 160, 133); // Green Color
  stroke(33);
  strokeWeight(1);
  rect(50, 100, 150, 50, 10);  // Turn ON Button
  rect(250, 100, 150, 50, 10); // Turn OFF Button
  fill(255);
  
  textSize(32);
  text("Turn ON",60, 135);
  text("Turn OFF", 255, 135);
  textSize(24);
  fill(33);
  text("Status:", 180, 200);
  textSize(30);
  textSize(16);
  text("Program made by Dejan Nedelkovski,\n     www.HowToMechatronics.com", 80, 320);
  text(ledStatus, 155, 240); // Prints the string comming from the Arduino
  
  // If the button "Turn ON" is pressed
  if(mousePressed && mouseX>50 && mouseX<200 && mouseY>100 && mouseY<150){
    myPort.write('1'); // Sends the character '1' and that will turn on the LED
    // Highlighs the buttons in red color when pressed
    stroke(255,0,0);
    strokeWeight(2);
    noFill();
    rect(50, 100, 150, 50, 10);
  }
  // If the button "Turn OFF" is pressed
  if(mousePressed && mouseX>250 && mouseX<400 && mouseY>100 && mouseY<150){
    myPort.write('0'); // Sends the character '0' and that will turn on the LED
    stroke(255,0,0);
    strokeWeight(2);
    noFill();
    rect(250, 100, 150, 50, 10);
  }
}
