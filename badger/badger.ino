// Both boards are slaves, wait until event to change to master mode, send data to the slave board, then changes back from master to slave mode
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define BADGER_ADDRESS 0x20
#define LOADER_ADDRESS 0x21

#define NUM_SERVOS 4
#define OPEN 1
#define CLOSE 0
#define MAX_BAG_COUNT 8

Adafruit_PWMServoDriver servos = Adafruit_PWMServoDriver();

// Order to close/open flaps
//const int closeOrder[8] = {1, 5, 2, 6, 3, 7, 4, 8};
const int closeOrder[MAX_BAG_COUNT] = {1, 2, 3, 4, 5, 6, 7, 8};

int count = 0;
const int buttonOne = A0;
const int buttonTwo = 3;

boolean all_flaps_closed = false;

int bag_count = 1;

void setup() {
  pinMode(buttonOne, INPUT);
  pinMode(buttonTwo, INPUT);

  Serial.begin(9600);
  
  servos.begin();
  servos.setPWMFreq(100); 
  
  // Open all but bottom flaps
  for(int i = 1; i <= 4; ++i) {
    servos.setPWM(closeOrder[i], 0, 400);
    delay(500);
  }
  
  for(int i = 0; i < 1; ++i) {
    servos.setPWM(closeOrder[i], 0, 1100);
    delay(500);
  }
  
  Wire.begin(BADGER_ADDRESS);
  Wire.onReceive(receiveEvent);
}

boolean flag = true;

volatile byte received = false;

void loop() 
{     
  static boolean ready = true;
  
  if (received) {
    Serial.println("got something");
    delay(750);
    closeFlaps();
    received = false;
  }
  else {
    Serial.println("Nothing yet");
  }
  delay(500);

}

// function that executes when data is received from loader
void receiveEvent(int bytes)
{
  int towerToClose;
  received = true;
  while(Wire.available())
  {
    towerToClose = Wire.read();
  }  
}

// Cycles through all flaps to close in order, closes 1 flap
void closeFlaps()
{
  if(bag_count >= MAX_BAG_COUNT) {
     Serial.println("wtf got more than 8 bags"); 
     return;
  }
  
  Serial.print("Closing servo ");
  Serial.println(closeOrder[bag_count]);
  servos.setPWM(closeOrder[bag_count++], 0, 1100);
  // Open back flaps for testing
  if(bag_count == 5) {
    bag_count = 1;
    for(int i = 1; i <= 4; ++i) {
      servos.setPWM(closeOrder[i], 0, 400);
      delay(500);
    }    
  }
  delay(500);
}

// Opens a flap
void openFlaps()
{
  if(bag_count < 0) {
    Serial.println("wtf got negative bags");
  }
  
  Serial.print("Opening servo ");
  Serial.println(closeOrder[MAX_BAG_COUNT - bag_count]);
  servos.setPWM(closeOrder[MAX_BAG_COUNT - bag_count--], 0, 400);
  delay(500);
}

void closeAllFlaps()
{
   for(int i = 0; i < NUM_SERVOS; ++i)
  {
     servos.setPWM(i, 0, 1100);
  } 
}

// Confirm to loader arduino that we closed the flap
void confirmMessage()
{
  Wire.beginTransmission(LOADER_ADDRESS); 
  Wire.write(254);
  Wire.endTransmission();
}
/*
// function that executes when data is requested by loader
void requestEvent()
{
  Wire.write("Responding to loader request"); // 28 bytes
}
*/
