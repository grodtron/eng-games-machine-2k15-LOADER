
/* Read Quadrature Encoder
 * Connect Encoder to Pins encoder0PinA, encoder0PinB, and +5V.
 *
 * Sketch by max wolf / www.meso.net
 * v. 0.1 - very basic functions - mw 20061220
 *
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

#define SERVO_MID      95
#define SERVO_RIGHT    120
#define SERVO_LEFT     70
#define SERVO_ROTATE_DELAY 30

// Number of steps for position
#define MID_STEPS 1400 
#define PICKUP_STEPS 950 // number of steps from mid point
#define JAB_STEPS 250
#define HOVER_STEPS 295

Servo myservo;
int servoPos = 0;

int encoderPin = 5; // sensor is on T1 and is counted using the 16 bit timer/counter 1 
int DOWNPin = A0;   // blue
int UPPin = 6;      // white

const int linearMotorAwayPin = 7;
const int linearMotorHomePin = 2;

const int linearHomePin = 4;
const int linearAwayPin = 3;
const int linearMidPin = A4;

const int weightSensePin = A3;
const int weightSenseThreshold = 518  ; // 480 !
const int twoBagThreshold = 700;

const int upperHallSensePin = A1;
const int lowerHallSensePin = A2;

volatile boolean motorIsON;
//distance the magnet goes up/down by to check/get bags
const int pickDistance = 800;

#define MOTORS_OFF() do {     \
  digitalWrite(DOWNPin, LOW); \
  digitalWrite(UPPin, LOW);   \
  motorIsON = false;          \
} while(0)

bool motorMove(int steps, int direction, int speed=255, void(*loopFunc)() = NULL);
void rotationalLoaderMove(void(*pickupFunc)() = NULL);
int motorCount = 0;
int linearCount = 0;
void(*pickUpFunc)();

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
  digitalWrite(linearMidPin, HIGH);
  
  Wire.begin(LOADER_ADDRESS);

  Serial.begin (9600);
  
  myservo.attach(9); 
  for(servoPos = 80; servoPos < 90; servoPos += 1)  
  {                                
    myservo.write(servoPos);              
    delay(30);
  }
  
  linearMotorToHomePosition();
  homeMagnet();
//  motorMove(MID_STEPS*2, directionDOWN, 255, NULL); while(1);
  pickUpFunc = jabRotate;
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

void loop() {   
//  TestLinearMotor();
  //  while(1);
//TestWeightSensor();

//  TestDrop();
//   runLoop();

}

void homeMagnet() {
  Serial.print("Homing magnet... ");
  analogWrite(UPPin, 255);
  digitalWrite(DOWNPin, LOW);  
  stopMotorsOnLowerHallSensor();
}

void jabRotate() {
  static int count = 0;
  if(count > 0) rotateLoader();
  
  triplePickUp();
  if(++count % 9 == 0) {
    pickUpFunc = swirlLeanback;
    rotateLoader(); // So that we return servo to middle before we try swirling
    centerServo();
    linearMotorToHomePosition();       
  }  
}

// TODO: Run this twice and go back to triple jab
void swirlLeanback() {
  static int count = 0;
  swirlPickUp();
  linearMotorToNewPickingPosition(RANDOM_DELAY);
  if(++count % 3 == 0) {
    linearMotorToHomePosition(); 
    homeMagnet();
    motorMove(MID_STEPS, directionDOWN, 255, NULL);  
  }
}

void centerServo() {
  if(servoPos > SERVO_MID) {
    for(;servoPos >= SERVO_MID; servoPos -= 1) 
    {                                  
      myservo.write(servoPos);             
      delay(SERVO_ROTATE_DELAY);                       
    }  
  } else {
    for(; servoPos < SERVO_MID; servoPos += 1) 
    {                            
      myservo.write(servoPos);              
      delay(SERVO_ROTATE_DELAY);                       
    }    
  }

}

void runLoop() {
  Serial.println("== loop ==");
  linearMotorToHomePosition();
  motorMove(MID_STEPS, directionDOWN, 255, NULL);  
  Serial.println("Picking");
  delay(300);
  static int count = 0;
  while(isBagPicked() == false){
    pickUpFunc();
    delay(300);
  }
  centerServo();
  homeMagnet();
  delay(500);
  int static tower_count = 0;
  int weight = analogRead(weightSensePin);
  Serial.print("weight pulled value: ");
  Serial.println(weight);

  if(isBagPicked() == true){
    if(weight > twoBagThreshold){
      Serial.println("Got two bags, Dropping!");
      motorMove(300, directionUP, 255, stopMotorsOnUpperHallSensor);      
    }else{
      Serial.println("Dropping the one bag");
      linearMotorToDroppingPosition(SECOND_POS);
      homeMagnet();     
      delay(500);
      dropBag();
      delay(250);
//      sendFlapToClose(tower_count);
      tower_count = (tower_count + 1 ) % 2;
      motorMove(200, directionDOWN);
    }
  }
  else
  {    
    Serial.println("no bag :(");
  }
  
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
//  if(isBagPicked) {
    Serial.print("Dropping bag... ");
    analogWrite(UPPin, 255);
    digitalWrite(DOWNPin, LOW);  
    stopMotorsOnUpperHallSensor();
    return true;
/*  } else {
    return false;
  }*/  
}

void sendFlapToClose(int tower)
{
  Wire.beginTransmission(BADGER_ADDRESS); // Master writer
  Wire.write(tower);
  Wire.endTransmission();
  Serial.println(tower);
}
