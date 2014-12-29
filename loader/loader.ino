/* TODO:
- Need to find a way to periodically check weight sensor while were pulling up. Right now it pulls up all the way end then checks it.
X Improve pull up (home magnet) while moving linear motor (needs testing with real badger height)
- Explore other grabbing techniques
- Find ways to reduce grabbing time (currently ~3-4 minutes)

*/
#include <Wire.h>
#include <Servo.h>

#define BADGER_ADDRESS 0x20
#define LOADER_ADDRESS 0x21
#define directionUP    0
#define directionDOWN  1
#define FIRST_POS      0
#define SECOND_POS     1
#define RANDOM_DELAY   700

#define SERVO_MID      75
#define SERVO_RIGHT    100
#define SERVO_LEFT     50
#define SERVO_ROTATE_DELAY 30

// Number of steps for position
#define MID_STEPS 1775 
#define PICKUP_STEPS 575 //distance the magnet goes up/down by to check/get bags
#define JAB_STEPS 250
#define HOVER_STEPS 305
/*
      O HOME
      | 
      |
      | MID_STEPS
      |______________
      | PICKUP_STEPS
      |
      | HOVER / JABS 
  ===BAGS====
                        */
                        
Servo myservo;
int servoPos = 0;

int encoderPin = 12; // sensor is on T1 and is counted using the 16 bit timer/counter 1 

int DOWNPin = 6;   // blue
int UPPin = 9;      // white

const int linearMotorAwayPin = 10;
const int linearMotorHomePin = 11;

const int linearHomePin = 2;
const int linearAwayPin = 3;
const int linearMidPin  = 4;

const int weightSensePin = A0;
const int weightSenseThreshold = 420;
const int twoBagThreshold = 590;

const int upperHallSensePin = 7;
const int lowerHallSensePin = 8;


int bag_count = 0;
int failed_attempts = 0;

#define MOTORS_OFF() do {     \
  digitalWrite(DOWNPin, LOW); \
  digitalWrite(UPPin, LOW);   \
} while(0)

bool motorMove(int steps, int direction, int speed=255, void(*loopFunc)() = NULL);
void centerServo(int rotate_delay = 15);
void linearMotorToHomePosition (boolean lowerMagnetWhileMoving = false);
void linearMotorToDroppingPosition (int pos, boolean homeMagnetWhileMoving = false);

void(*pickUpFunc)();

#define RUN_TESTS 0

void setup() {
  Wire.begin(LOADER_ADDRESS);

  Serial.begin (9600);
 
  pinMode (encoderPin,INPUT);
  pinMode (lowerHallSensePin,INPUT);
  pinMode (upperHallSensePin,INPUT);
  pinMode (DOWNPin,OUTPUT);
  pinMode (UPPin,OUTPUT);

  pinMode(linearMotorAwayPin, OUTPUT);
  pinMode(linearMotorHomePin, OUTPUT);
  pinMode(linearAwayPin, INPUT);
  pinMode(linearHomePin, INPUT);
  pinMode(linearMidPin, INPUT);
//  digitalWrite(linearMidPin, HIGH);
  
  Wire.begin(LOADER_ADDRESS);

  Serial.begin (9600);
  
  myservo.attach(9); 
  for(servoPos = 70; servoPos < SERVO_MID; servoPos += 1)  
  {                                
    myservo.write(servoPos);              
    delay(30);
  }
  
  linearMotorToHomePosition();
  homeMagnet();
  //motorMove(MID_STEPS, directionDOWN, 255, NULL); while(1);
  pickUpFunc = jabRotate;
#if RUN_TESTS
  for(;;) runTests();
  while(1);
#endif
}

void loop() {
//  while(bag_count < 8) {  
   runLoop();
//  }
}

void runLoop() {
  Serial.println("== loop ==");
  linearMotorToHomePosition(true);
  //homeMagnet();
  Serial.print("Bags left: ");
  Serial.print(8 - bag_count);
  Serial.print(", Failed attempts: ");
  Serial.println(failed_attempts);
  
  Serial.println("Picking");

  boolean doPicking = true;
  while(doPicking){
    pickUpFunc();
    delay(150);
    if(isBagPicked() == true) {
      doPicking = false;
    } else {
      failed_attempts++;       
    }
  }

  delay(200);
  int static tower_to_drop = 0;
  
  if(isBagPicked() == true){
    centerServo(25);
    //homeMagnet();
    int weight = analogRead(weightSensePin);
    if(weight > twoBagThreshold){
      Serial.print("Two bags pulled of value: ");
      Serial.print(weight);
      Serial.println("...dropping!");
      dropBag();
    }else{
      linearMotorToDroppingPosition(SECOND_POS, true);
      delay(150);
      if(isBagPicked() == true) { // check one more time
        dropBag();
//      sendFlapToClose(tower_to_drop);
        tower_to_drop = (tower_to_drop + 1 ) % 2;
        bag_count++;        
      }
    }
  }
  else
  {    
    failed_attempts++;
    Serial.println("no bag :(");
  }  
}

// TODO: When to home, and when to change pickup technique
void jabRotate() {
  static int count = 0;
  
  if(count > 0) rotateLoader(); 
  triplePickUp();  

  if(++count % 9 == 0) {
    //pickUpFunc = swirlLeanback;
    centerServo();
//    homeMagnet();
//    motorMove(MID_STEPS, directionDOWN, 255, NULL);  
    linearMotorToHomePosition();     
  }else if (count % 4 == 0) {
    linearMotorToNewPickingPosition(RANDOM_DELAY); // Move back to a new position if we tried all 3 spots   
  } 
}

// TODO: When to home
void swirlLeanback() {
  static int count = 0;
    
  swirlPickUp();
  
  if(++count % 3 == 0) {
    linearMotorToHomePosition(); 
//    homeMagnet();
//    motorMove(MID_STEPS, directionDOWN, 255, NULL);  
  } else if (count == 7) {
    pickUpFunc = dropSpin;
    centerServo();
    linearMotorToHomePosition();   
  } else {
    linearMotorToNewPickingPosition(RANDOM_DELAY);     
  }
}


void dropSpin() {
  static int count = 0;
  
  if(++count % 2 == 0) {
    linearMotorToHomePosition(); 
    homeMagnet();
    motorMove(MID_STEPS, directionDOWN, 255, NULL);  
  } 
  
  spinPickUp();

  linearMotorToNewPickingPosition(RANDOM_DELAY);     
}

void stopMotorsOnLowerHallSensor(){
  while(! digitalRead(lowerHallSensePin) );
  Serial.println("Lower hall sensor detected");

   MOTORS_OFF();
}

void stopMotorsOnUpperHallSensor(){

  while(! digitalRead(upperHallSensePin) );
  Serial.println("Upper hall sensor detected");

   MOTORS_OFF();
}

void startCountingUpTo(int count){
  int steps = 0;
  long start = millis();
  while (steps < count) {
     while(digitalRead(encoderPin));
     while( !digitalRead(encoderPin));
     steps++; 
  }
  MOTORS_OFF();
}

void homeMagnet() {
  Serial.print("Homing magnet... ");
  analogWrite(UPPin, 255);
  digitalWrite(DOWNPin, LOW);  
  stopMotorsOnLowerHallSensor();
}

// Move n steps, then stop
bool motorMove(int steps, int direction, int speed, void(*loopFunc)()) {
  
  Serial.print("motorMove "); Serial.println(steps);
  int count = 0;
  speed = speed % 256;
  
  if(direction == directionDOWN) { // move DOWN
    digitalWrite(DOWNPin, HIGH);
    analogWrite(UPPin, 255 - speed);
  }
  else { // move UPs
    analogWrite(UPPin, speed);
    digitalWrite(DOWNPin, LOW);
  }
  startCountingUpTo(steps);

  return true;
}

boolean isBagPicked() {
  Serial.print("read value: ");
  Serial.print(analogRead(weightSensePin));
  Serial.print(" vs threshold " );
  Serial.println(weightSenseThreshold);
  return (analogRead(weightSensePin) > weightSenseThreshold);
}

boolean dropBag() {
    Serial.print("Dropping bag... ");
    analogWrite(UPPin, 255);
    digitalWrite(DOWNPin, LOW);  
    stopMotorsOnUpperHallSensor();
    delay(250);
    motorMove(200, directionDOWN);
    return true;
}

void sendFlapToClose(int tower)
{
  Wire.beginTransmission(BADGER_ADDRESS); // Master writer
  Wire.write(tower);
  Wire.endTransmission();
  Serial.println(tower);
}
