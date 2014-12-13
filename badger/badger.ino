// Both boards are slaves, wait until event to change to master mode, send data to the slave board, then changes back from master to slave mode
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define BADGER_ADDRESS 0x9
#define LOADER_ADDRESS 0x8

#define NUM_SERVOS 8
#define OPEN 1
#define CLOSE 0
#define SERVOMIN  150 
#define SERVOMAX  600 

Adafruit_PWMServoDriver servos = Adafruit_PWMServoDriver();

// Order to close/open flaps
int index = 0;
int close_order[NUM_SERVOS] = {0, 1, 2, 3, 4, 5, 6, 7};
int open_order[NUM_SERVOS] = {0, 1, 2, 3, 4, 5, 6, 7};

int count = 0;
const int buttonOne = 2;
const int buttonTwo = 3;

boolean all_flaps_closed = false;

void setup() {
  pinMode(13, OUTPUT);
  pinMode(2, INPUT);
  Serial.begin(9600);
  Wire.begin(BADGER_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  servos.begin();
  servos.setPWMFreq(100);

  servos.setPWM(0, 0, map( 0, 0, 1000, 0, 4096 ));  
}

boolean flag = true;

void loop() 
{        
  static int count = 0;
  static boolean ready = true;
  if( !ready || digitalRead(buttonOne) ){
    ready = true;
  } else{
    ready = false;
    count = (count + 1) % 2;
    switch(count){
      case 0:
        servos.setPWM(0, 0, 1000);
        Serial.println("Close");
        break;
      case 1:
        servos.setPWM(0, 0, 400);
        Serial.println("Open");
        break;
    }    
    delay(500);
  }
}

// function that executes when data is received from loader
void receiveEvent(int bytes)
{
  int servo_to_set, servo_state;

  while(Wire.available())
  {
    servo_to_set = Wire.read();   
    servo_state = Wire.read();   
  }
  Serial.print("Servo: ");
  Serial.print(servo_to_set);
  Serial.print(" set to ");
  Serial.print(servo_state);   
  Serial.println();
  
// toggleFlaps(servo_to_set, servo_state);
 closeFlaps();
  confirmMessage();
}

// toggle a a servo to open or close
void toggleFlaps(int servonum, int state)
{
  uint16_t pulselen = 1100;  
  if(state == OPEN) 
     pulselen = 400;

  servos.setPWM(servonum, 0, pulselen);
}

// Cycles through all flaps to close in order, closes 1 flap
void closeFlaps()
{
  servos.setPWM(close_order[index], 0, 1100);
  index++;
  
  if(index == 8)
  {
    all_flaps_closed = true;
    index = 0;
  }
}

// Opens a flap
void openFlaps()
{
  servos.setPWM(close_order[index], 0, 400);
  index++;
  
  if(index == 8)
  {
    all_flaps_closed = true;   
    index = 0;
  }
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

// function that executes when data is requested by loader
void requestEvent()
{
  Wire.write("Responding to loader request"); // 28 bytes
}
