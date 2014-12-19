#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
 
Adafruit_PWMServoDriver servos = Adafruit_PWMServoDriver();

#define irSensorPin 7      // IR sensor on this pin

const int buttonOne = 0;
const int startButton = 2;
const int targetPin = 3;
const int motorPin = 6;
int holePeriod;
int woodPeriod;

/* Servos to Pins
front  -   back tower
     8 - 1
     9 - 2
    10 - 3
    11 - 4
*/

#define MAX_BAG_COUNT 8
const int servoOrder[MAX_BAG_COUNT] = {11, 4, 10, 3, 9, 2, 8, 1}; // Order to cloe and open flaps, alternating each tower

const int ShootingSpeed = 1300; // Time to reach the target after turning motors on

#define CLOSE 1025
#define OPEN 400

int bag_count = MAX_BAG_COUNT;
boolean button1_ready = true, startButtonReady = true;

void setup() {
  Serial.begin(9600);
  servos.begin();
  servos.setPWMFreq(100);
  pinMode(irSensorPin, INPUT);
  pinMode(targetPin, INPUT);
  digitalWrite(targetPin, HIGH);
  pinMode(buttonOne, INPUT);
  pinMode(startButton, INPUT);
  
  // open all flaps
  for(int i = 0; i < MAX_BAG_COUNT; ++i) {
    servos.setPWM(servoOrder[i], 0, OPEN);
    delay(500);
  }

  servos.setPWM(servoOrder[0], 0, CLOSE+25);
  servos.setPWM(servoOrder[1], 0, CLOSE);
  
  holePeriod = 0;
  woodPeriod = 0;
  while(0 == woodPeriod) {
    calculateWoodPeriod();
  }
  while(0 == holePeriod) {
    calculateHolePeriod();
  }
  Serial.println("Done measuring target periods");
}

static boolean ready = true;
unsigned long initialTime = 0;
unsigned long finishTime = 0;

void loop() {  
  static int count = 2;
  if(!button1_ready || digitalRead(buttonOne)) {
     button1_ready = true; 
  }else {
    Serial.print("Closing servo ");
    Serial.println(servoOrder[count]);
      button1_ready = false;
    if(servoOrder[count] == 2 || servoOrder[count] == 3)
      servos.setPWM(servoOrder[count], 0, CLOSE-75);
    else if (servoOrder[count] == 11)
      servos.setPWM(servoOrder[count], 0, CLOSE+25);
    else if (servoOrder[count] == 10)
      servos.setPWM(servoOrder[count], 0, CLOSE+125);
    else
       servos.setPWM(servoOrder[count], 0, CLOSE);
      count = (count + 1) % MAX_BAG_COUNT;
      delay(500);   
  }
  
  if(!startButtonReady || digitalRead(startButton)) {
     startButtonReady = true; 
  }else {
    startButtonReady = false;
    Serial.println("Starting shooting process");
    shootAllBags();
  }
}

// Loop to shoot all 8 bags
void shootAllBags() {
  int bottomServo; // Bottom servo that will release the bean bang
  while(bag_count > 0) {   
   if(bag_count % 2 == 0) { // Alternate bottom servo from which bean bags will be dropped
     bottomServo = servoOrder[0];
   }
   else {
     bottomServo = servoOrder[1];
   }
   Serial.print("Toggling bottom servo: ");
   Serial.println(bottomServo);
   servos.setPWM(bottomServo, 0, CLOSE);
   delay(1000);   
   openNextFlap();
   delay(1250);
   servos.setPWM(bottomServo, 0, OPEN);
   timeBagShooting();
 } 
}

// Close specific servo the right amount
void closeServoFix(int servonum)
{
  switch(servonum) {
    case 2:
    case 3:
      servos.setPWM(servonum, 0, CLOSE-75);
      break;
    case 10:
      servos.setPWM(servonum, 0, CLOSE+25);
      break;
    case 11:
      servos.setPWM(servonum, 0, CLOSE+125);
      break;
    default:
      servos.setPWM(servonum, 0, CLOSE);
      break;
  } 
}

// Opens a flap in order
void openNextFlap()
{
  if(bag_count < 0) {
    Serial.println("wtf got negative bags");
    bag_count = MAX_BAG_COUNT;
  }
  
  Serial.print("Opening servo ");
  Serial.println(servoOrder[MAX_BAG_COUNT - bag_count]);
  servos.setPWM(servoOrder[MAX_BAG_COUNT - bag_count--], 0, OPEN);
  delay(500);
}

// Cycles through all flaps to close in order, closes 1 flap at a time
void closeNextFlap()
{
  if(bag_count >= MAX_BAG_COUNT) {
     Serial.println("wtf got more than 8 bags"); 
     return;
  }
  
  Serial.print("Closing servo ");
  Serial.println(servoOrder[bag_count]);
  servos.setPWM(servoOrder[bag_count++], 0, 1100);
  
  // Open back flaps for testing
  if(bag_count == 5) {
    bag_count = 1;
    for(int i = 1; i <= MAX_BAG_COUNT; ++i) {
      servos.setPWM(servoOrder[i], 0, 400);
      delay(500);
    }    
  }
  delay(500);
}

void calculateWoodPeriod() {
  if (!digitalRead(irSensorPin)) { // detecting object
    while(!digitalRead(irSensorPin));
    while(digitalRead(irSensorPin));
  } else { // if it's one of the Halls
    while(digitalRead(irSensorPin));
    while(!digitalRead(irSensorPin));
    while(digitalRead(irSensorPin));
  }
  initialTime = millis();
  while(!digitalRead(irSensorPin));
  finishTime = millis();
  if (finishTime-initialTime>50) {
    woodPeriod = finishTime-initialTime;
    Serial.print("num of readings till next Hall: ");
    Serial.println(finishTime-initialTime);
  }
}

void calculateHolePeriod() {
  if (!digitalRead(irSensorPin)) { // detecting object
    while(!digitalRead(irSensorPin));
    while(digitalRead(irSensorPin));
    while(!digitalRead(irSensorPin));
  } else { // if it's one of the Halls
    while(digitalRead(irSensorPin));
    while(!digitalRead(irSensorPin));
  }
  initialTime = millis();
  while(digitalRead(irSensorPin));
  finishTime = millis();
  if (finishTime-initialTime>50) {
    holePeriod+=finishTime-initialTime;
    Serial.print("num of readings for a Hall: ");
    Serial.println(finishTime-initialTime);
  }
}

// Signal to start spinning conveyor belt
void shootBag() {     // for ~1300 msecs
  for(int i = 0; i < 130; i += 10){
    servos.setPWM(0, 0, map( i, 0, 1000, 0, 4096 ));
    delay(100);
  }
  servos.setPWM(0, 0, map( 0, 0, 1000, 0, 4096 ));
}

// Waiting for optimal time to shoot
void timeBagShooting() {
  long time_to_wait = holePeriod + woodPeriod;
  while (time_to_wait < ShootingSpeed) {
    time_to_wait = time_to_wait + holePeriod + woodPeriod;
  }
  time_to_wait -= (holePeriod/2 + ShootingSpeed);
  if (!digitalRead(irSensorPin)) { // detecting object
    while(!digitalRead(irSensorPin));
    while(digitalRead(irSensorPin));
  } else { // if it's one of the Halls
    while(digitalRead(irSensorPin));
    while(!digitalRead(irSensorPin));
    while(digitalRead(irSensorPin));
  }
  long start_Time = millis();
  while( (millis() - start_Time) < time_to_wait);
  shootBag();
}
