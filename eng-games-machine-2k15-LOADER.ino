// Both boards are slaves, wait until event to change to master mode, send data to the slave board, then changes back from master to slave mode
#include <Wire.h>

#define BADGER_ADDRESS 0x9
#define LOADER_ADDRESS 0x8  

void setup() {

  pinMode(13, OUTPUT);
  Serial.begin(9600);
  Wire.begin(BADGER_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

void loop() 
{
  
  Wire.beginTransmission(LOADER_ADDRESS); // Master writer
  Wire.write("badger (master) writing to loader (slave)");  
  Wire.endTransmission();    
  delay(1000);

  Wire.requestFrom(LOADER_ADDRESS, 28); // Master reader
  
  while(Wire.available())    // slave may send less than requested
  { 
    char c = Wire.read(); // receive a byte as character
    Serial.print(c);         // print the character
  }
  Serial.println();
  
  
}

// function that executes when data is received from loader
void receiveEvent(int bytes)
{
  while(Wire.available())
  {
    char y = Wire.read();   
    Serial.print(y);             
  }
  Serial.println();
}

// function that executes when data is requested by loader
void requestEvent()
{
  Wire.write("Responding to loader request"); // 28 bytes
}
