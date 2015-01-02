/* TODO:
- Need to find a way to periodically check weight sensor while were pulling up. Right now it pulls up all the way end then checks it.
- Test pullup and dropping magnet while moving linear motor
*/
#include <Wire.h>
#include <Servo.h>

#define BADGER_ADDRESS 0x20
#define LOADER_ADDRESS 0x21
#define directionUP    0
#define directionDOWN  1
#define FIRST_POS      0
#define SECOND_POS     1
#define RANDOM_DELAY   250

#define SERVO_MID      100
#define SERVO_RIGHT    115
#define SERVO_LEFT     65
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
const int servoPin = 5;
int servoPos = 0;

int encoderPin = 12; // sensor is on T1 and is counted using the 16 bit timer/counter 1 

const int DOWNPin = 6;   // blue
const int UPPin = 9;      // white

const int linearMotorAwayPin = 10;
const int linearMotorHomePin = 11;

const int linearHomePin = 2;
const int linearAwayPin = 3;
const int linearMidPin  = 4;

const int weightSensePin = A0;
const int weightSenseThreshold = 430;
const int twoBagThreshold = 485;

const int upperHallSensePin = 7;
const int lowerHallSensePin = 8;

int bag_count = 0;
int failed_attempts = 0;

#define MOTORS_OFF() do {     \
  analogWrite(DOWNPin, 0); \
  analogWrite(UPPin, 0);   \
} while(0)

bool motorMove(int steps, int direction, int speed=255, void(*loopFunc)() = NULL);
void centerServo(int rotate_delay = 15);
void linearMotorToHomePosition (boolean lowerMagnetWhileMoving = false);
void linearMotorToDroppingPosition (int pos, boolean homeMagnetWhileMoving = false);

void(*pickUpFunc)();

#define RUN_TESTS 0
int tower_to_drop = 1;

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
  
  Wire.begin(LOADER_ADDRESS);

  Serial.begin (9600);
  
  myservo.attach(servoPin); 
                          
  servoPos = SERVO_MID;   
  myservo.write(servoPos);              
  
#if RUN_TESTS
  //for(;;) runTests();
  while(1){
      delay(2000);
      int weight = analogRead(weightSensePin);
      Serial.print("weight that is pulled: "); Serial.println(weight);
  }
#endif
  tower_to_drop = getTowerToDrop();
  Serial.print("Tower to drop: ");
  Serial.println(tower_to_drop);
  homeMagnet();
  linearMotorToHomePosition();
  //motorMove(MID_STEPS, directionDOWN, 255, NULL); while(1);
  pickUpFunc = jabRotate;

  homeMagnet();
}

void loop() {
  while(bag_count < 8) {  
   runLoop();
  }
}

void runLoop() {
  Serial.println("== loop ==");
  linearMotorToHomePosition(false);
  homeMagnet();
  motorMove(MID_STEPS, directionDOWN, 255, NULL);  

  Serial.print("Bags left: ");
  Serial.print(8 - bag_count);
  Serial.print(", Failed attempts: ");
  Serial.println(failed_attempts);
  
  Serial.println("Picking");

  boolean doPicking = true;
  while(doPicking){
    pickUpFunc();
    delay(150);
    if(isBagPicked()) {
      doPicking = false;
    } else {
      failed_attempts++;       
    }
    
    if(failed_attempts > 10) {
      homeMagnet();
      motorMove(MID_STEPS, directionDOWN, 255, NULL);    
      failed_attempts = 0;  
    }
  }

  delay(450);
  
  if(isBagPicked()){
    centerServo(25);
    int weight = analogRead(weightSensePin);
    if(weight > twoBagThreshold){
      Serial.print("Two bags pulled of value: ");
      Serial.print(weight);
      Serial.println("...dropping!");
      dropBag();
      failed_attempts++;
    }else{
      failed_attempts = 0;
      if(tower_to_drop == FIRST_POS) {
        homeMagnet();
        linearMotorToDroppingPosition(tower_to_drop, false);
        servoTurnLeft(30);
        
      } else if (tower_to_drop == SECOND_POS) {
        homeMagnet();
        linearMotorToDroppingPosition(tower_to_drop, false);         
      }
      delay(400);
      if(isBagPicked()) { // check one more time
        dropBag();
        sendFlapToClose(tower_to_drop);
        tower_to_drop = (tower_to_drop + 1 ) % 2;
        bag_count++; 
        centerServo();       
      }
    }
  }
  else
  {    
    failed_attempts++;
    Serial.println("no bag :(");
  }  
}

void jabRotate() {
  static int count = 0;
  
  if(count > 0) rotateLoader(); 
  triplePickUp();  

  if(++count % 9 == 0) {
    //pickUpFunc = swirlLeanback;
    centerServo();
//    homeMagnet();
    linearMotorToHomePosition();     
  }else if (count % 4 == 0) {
    linearMotorToNewPickingPosition(RANDOM_DELAY); // Move back to a new position if we tried all 3 spots   
  } 
}

void swirlLeanback() {
  static int count = 0;
    
  swirlPickUp();
  
  if(++count % 3 == 0) {
    linearMotorToHomePosition(); 
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
  analogWrite(DOWNPin, LOW);  
  stopMotorsOnLowerHallSensor();
}

// Move n steps, then stop
bool motorMove(int steps, int direction, int speed, void(*loopFunc)()) {
  
  Serial.print("motorMove "); Serial.println(steps);
  int count = 0;
  speed = speed % 256;
  
  if(direction == directionDOWN) { // move DOWN
    analogWrite(DOWNPin, 255);
    analogWrite(UPPin, 255 - speed);
  }
  else { // move UPs
    analogWrite(UPPin, speed);
    analogWrite(DOWNPin, 0);
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

int getTowerToDrop() {
  Wire.requestFrom(BADGER_ADDRESS, 1);    // request last tower dropped
  delay(100);
  int tower;
  while(Wire.available()) {     
    tower = Wire.read(); 
    Serial.print("Received from request: ");
    Serial.println(tower);         
  }
  return ((tower + 1) % 2); // got last drop, so tower to drop is next
}
