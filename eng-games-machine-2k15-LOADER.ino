// Both boards are slaves, wait until event to change to master mode, send data to the slave board, then changes back from master to slave mode
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define BADGER_ADDRESS 0x9
#define LOADER_ADDRESS 0x8  

#define OPEN 1
#define CLOSE 0

#define NUM_SERVOS 8

void setup() {

  pinMode(13, OUTPUT);
  Serial.begin(9600);
  Wire.begin(LOADER_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
}

int state = 0;

void loop() 
{
     sendFlapToClose(0, state);
     state = (state + 1) % 2;
     delay(5000); 
}

void sendFlapToClose(int servonum, int state)
{
  Wire.beginTransmission(BADGER_ADDRESS); // Master writer
  Wire.write(servonum);  
  Wire.write(state);
  Wire.endTransmission();
  Serial.print(servonum);
  Serial.println(state);  
}

// function that executes when data is received from badger/slave
void receiveEvent(int bytes)
{
  while(Wire.available() > 0)
  {
    char y = Wire.read();   
    Serial.print(y);             
  }
  Serial.println();
}

// function that executes when data is requested by badger/master
void requestEvent()
{
  Wire.write("Responding to badger request"); // 28 bytes
}
