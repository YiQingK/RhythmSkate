/*
  DigitalReadSerial connected to Unity using Ardity
  
  Reads a digital input on pin 2, prints the result to Serial 
  On my mac, it uses /dev/cu.usbmodem1451301
  The main modification is the delay for 100ms
This example code is in the public domain and based on http://www.arduino.cc/en/Tutorial/DigitalReadSerial
*/
// digital pin 2 has a pushbutton attached to it. Give it a name:
int buttonRight = 2;
int buttonLeft = 3;
// the setup routine runs once when you press reset:
void setup() {
  Serial.begin(9600);
  pinMode(buttonRight, INPUT_PULLUP);
  pinMode(buttonLeft,INPUT_PULLUP);
}
// the loop routine runs over and over again forever:
void loop() {
  if(!digitalRead(buttonRight))
  {
    Serial.println(2);
  }
  else if(!digitalRead(buttonLeft))
  {
    Serial.println(3);
  }
  delay(100);  // IMPORTANT ---> delay in between reads
}