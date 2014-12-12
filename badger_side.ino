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

int close_order[NUM_SERVOS] = {0, 1, 2, 3, 4, 5, 6, 7};
int count = 0;

void setup() {

  pinMode(13, OUTPUT);
  Serial.begin(9600);
  Wire.begin(BADGER_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  
  servos.begin();
  servos.setPWMFreq(100);
  for(int i = 0; i < 230; i += 5){
    servos.setPWM(0, 0, map( i, 0, 1000, 0, 4096 ));
    delay(100);  
  }
  servos.setPWM(0, 0, map( 0, 0, 1000, 0, 4096 ));  
}

boolean flag = true;

void loop() 
{ 
  delay(500);
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
// testFlaps();
}

// toggle a a servo to open or close
void toggleFlaps(int servonum, int state)
{
  uint16_t end = 1100;  
  if(state == OPEN) 
     end = 400;

  servos.setPWM(servonum, 0, end);
}

void testFlaps()
{
    count = (count + 1) % 8;
    switch(count){
      case 0:
        servos.setPWM(4, 0, 1100);
        Serial.println("Close Top");
        break;
      case 1:
        servos.setPWM(1, 0, 400);
        Serial.println("Open bottom");
        break;
      case 2:
        servos.setPWM(2, 0, 400);
        Serial.println("Open Second from bottom");
        break;
      case 3:
        servos.setPWM(3, 0, 400);
        Serial.println("Open second from top");
        break;
      case 4:
        servos.setPWM(4, 0, 400);
        Serial.println("Open top");
        break;
      case 5:
        servos.setPWM(1, 0, 1100);
        Serial.println("Close bottom");
        break;
      case 6:
        servos.setPWM(2, 0, 1100);
        Serial.println("Close second from bottom");
        break;
      case 7: 
        servos.setPWM(3, 0, 1100);
        Serial.println("Close second from top");
        break;
    } 
}

void closeAllFlaps()
{
   for(int i = 0; i < NUM_SERVOS; ++i)
  {
     servos.setPWM(i, 0, 1100);
  } 
}

// function that executes when data is requested by loader
void requestEvent()
{
  Wire.write("Responding to loader request"); // 28 bytes
}
