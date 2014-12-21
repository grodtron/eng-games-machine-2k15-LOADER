
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
#define FIRST_POS      2
#define SECOND_POS     3
#define RANDOM_DELAY   700
#define SERVO_MID      95
#define SERVO_RIGHT    120
#define SERVO_LEFT     70

#define DOWN_DELAY 875
#define UP_DELAY 950
#define SERVO_ROTATE_DELAY 30

// Number of steps for position
/*
  home to bag: ~430 steps
*/
#define MID_STEPS 256 
#define PICKUP_STEPS 175 // number of steps from mid point

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
const int weightSenseThreshold = 400; // 480 !
const int twoBagThreshold = 590;

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

void setup() {
  Wire.begin(LOADER_ADDRESS);
  Wire.onReceive(receiveEvent);

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
  Wire.onReceive(receiveEvent);

  Serial.begin (9600);
  
  myservo.attach(9); 
  for(servoPos = 80; servoPos < 90; servoPos += 1)  // goes from 0 degrees to 180 degrees
  {                                  // in steps of 1 degree
    myservo.write(servoPos);              // tell servo to go to position in variable 'pos'
    delay(30);                       // waits 15ms for the servo to reach the position
  }
  
  motorIsON = true;
  motorMove(3500, directionUP, 255, stopMotorsOnLowerHallSensor);
  motorMove(3500, directionDOWN, 255, NULL);  
  startCountingUpTo(MID_STEPS);

//  configureCounter();
/*
  Serial.print("Homing magnet... ");
  motorMove(3500, directionUP, 255, stopMotorsOnLowerHallSensor);
  Serial.println("done");
*/
  linearMotorToHomePosition();
}

void configureCounter(){
 TCCR1A = 0;
 TCCR1B =  (1 << WGM12) // CTC mode using T1 as the input for counting events (page 136)
           | (1 << CS12) | (1 << CS11) | (1 << CS10) // Clock on T1 rising edge (page 137)
           ;
 TCCR1C = 0;
 
 // initially disable timer coutner interrupt;
 TIMSK1 = 0;
 // TIMSK1 = (1 << OCIE1A); // interrupt on compare match;
 // OCR1AH and OCR1AL used to set n of ticks
}

//ISR(TIMER1_COMPA_vect) { MOTORS_OFF(); }

void stopMotorsOnLowerHallSensor(){
      motorIsON = true;    

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
     Serial.println(steps);
     steps++; 
  }
  MOTORS_OFF();
}

void loop() {
  

//  pickMethodOne();
//  pickMethodTwo();

//  triplePickUp();
  while(1);
}

void pickMethodOne() {
  for (int i = 0; i < 3; ++i) {
    rotationalLoaderMove(simplePickUp);
    
    /*
    motorIsON = true;
    motorMove(3500, directionUP, 255, stopMotorsOnLowerHallSensor);
    motorMove(3500, directionDOWN, 255, NULL);
    startCountingUpTo(MID_STEPS);
    */
    
    linearMotorToNewPickingPosition();
    delay(1000);
  }
  linearMotorToHomePosition();  
}
 
void pickMethodTwo() {
  for (int i = 0; i < 3; ++i) {
    rotationalLoaderMove(triplePickUp);
    linearMotorToNewPickingPosition();
    delay(1000);
  }
  linearMotorToHomePosition(); 
}

void triplePickUp() {
  motorMove(3500, directionDOWN, 255, NULL);
  startCountingUpTo(PICKUP_STEPS); 
  // small pickups
  for(int i = 0; i < 2; ++i) {
    motorMove(3500, directionUP, 255, NULL);
    startCountingUpTo(130);
    delay(250);  
    motorMove(3500, directionDOWN, 255, NULL);
    startCountingUpTo(120);     
  }
  
  motorMove(3500, directionUP, 255, NULL);
  startCountingUpTo(PICKUP_STEPS + 30); 
}

void simplePickUp() { // simple up down
  motorMove(3500, directionDOWN, 255, NULL);
  startCountingUpTo(PICKUP_STEPS);
  motorMove(3500, directionUP, 255, NULL);
  startCountingUpTo(PICKUP_STEPS+10);
}

void rotationalLoaderMove(void(*pickUpFunc)()) {
  delay(500);
  pickUpFunc();  
  for(servoPos = SERVO_MID; servoPos < SERVO_RIGHT; servoPos += 1)  // goes from 0 degrees to 180 degrees
  {                                  // in steps of 1 degree
    myservo.write(servoPos);              // tell servo to go to position in variable 'pos'
    delay(SERVO_ROTATE_DELAY);                       // waits 15ms for the servo to reach the position
  }
  delay(500);
  pickUpFunc();
  for(servoPos = SERVO_RIGHT; servoPos >= SERVO_MID; servoPos -= 1)  // goes from 0 degrees to 180 degrees
  {                                  // in steps of 1 degree
    myservo.write(servoPos);              // tell servo to go to position in variable 'pos'
    delay(SERVO_ROTATE_DELAY);                       // waits 15ms for the servo to reach the position
  }
  delay(500);
  pickUpFunc();

  for(servoPos = SERVO_MID; servoPos >= SERVO_LEFT; servoPos -= 1)  // goes from 0 degrees to 180 degrees
  {                                  // in steps of 1 degree
    myservo.write(servoPos);              // tell servo to go to position in variable 'pos'
    delay(SERVO_ROTATE_DELAY);                       // waits 15ms for the servo to reach the position
  }
  delay(500);
  pickUpFunc();

  for(servoPos = SERVO_LEFT; servoPos < SERVO_MID; servoPos += 1)  // goes from 0 degrees to 180 degrees
  {                                  // in steps of 1 degree
    myservo.write(servoPos);              // tell servo to go to position in variable 'pos'
    delay(SERVO_ROTATE_DELAY);                       // waits 15ms for the servo to reach the position
  }
  delay(500);
  pickUpFunc();
}
/*
  Serial.println("== loop ==");
  Serial.println("Picking");
  motorMove(3000, directionDOWN);
  delay(200);
  Serial.println("Homing");

  motorMove(pickDistance, directionUP);
  delay(100);
  int count = 0;
  while(analogRead(weightSensePin) < weightSenseThreshold){
    if(++count % 3 == 0){
      linearMotorToHomePosition();
    }else{
      linearMotorToNewRandomPickingPosition();
    }

    motorMove(pickDistance, directionDOWN);
    motorMove(pickDistance, directionUP);
    Serial.print("read value: ");
    Serial.print(analogRead(weightSensePin));
    Serial.print(" vs threshold " );
    Serial.println(weightSenseThreshold);

    delay(200);
  }

  motorMove(3500, directionUP, 255, stopMotorsOnLowerHallSensor);
  delay(500);

  int weight = analogRead(weightSensePin);
  Serial.print("weight pulled value: ");
  Serial.println(weight);

  if(weight > weightSenseThreshold){/*
    if(weight > twoBagThreshold){
      Serial.println("Got two bags, Dropping!");
      motorMove(300, directionUP, 255, stopMotorsOnUpperHallSensor);      
    }else{
      Serial.println("Dropping the one bag");
      linearMotorToDroppingPosition();
      motorMove(300, directionUP, 255, stopMotorsOnUpperHallSensor);
      sendFlapToClose(0);
      linearMotorToHomePosition();      
  }
  else
  {    
    Serial.println("no bag :(");
  }*/

void linearMotorToNewPickingPosition () {
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, HIGH);
  delay(RANDOM_DELAY);
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, LOW);
}

void linearMotorToHomePosition () {
  Serial.print("Homing linear... ");
  digitalWrite(linearMotorHomePin, HIGH);
  digitalWrite(linearMotorAwayPin, LOW);
  while(! digitalRead(linearHomePin) );
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, LOW);
  Serial.println("done");
}

void linearMotorToDroppingPosition (int pos) {
  Serial.print("to dropping position linear... ");
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, HIGH);
  if (pos == FIRST_POS) {
    while( digitalRead(linearMidPin) );
  } else if (pos == SECOND_POS) {
    while(! digitalRead(linearAwayPin) );
  }
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, LOW);
  Serial.println("done");
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
//  startCountingUpTo(steps);
  while(motorIsON){
    if (loopFunc) loopFunc();
  }

  return true;
}

void sendFlapToClose(int tower)
{
  Wire.beginTransmission(BADGER_ADDRESS); // Master writer
  Wire.write(tower);
  Wire.endTransmission();
  Serial.println(tower);
}

// function that executes when data is received from badger/slave
void receiveEvent(int bytes)
{
  while(Wire.available() > 0)
  {
    int x = Wire.read();   
    Serial.println(x);             
  }
}
