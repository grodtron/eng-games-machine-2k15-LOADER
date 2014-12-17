#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
 
Adafruit_PWMServoDriver servos = Adafruit_PWMServoDriver();

#define irSensorPin 7      // IR sensor on this pin

const int buttonOne = 2;
const int targetPin = 3;
const int motorPin = 6;
int holePeriod;
int woodPeriod;

void setup() {
  Serial.begin(9600);
  servos.begin();
  servos.setPWMFreq(100);
  pinMode(irSensorPin, INPUT);
  pinMode(targetPin, INPUT);
  digitalWrite(targetPin, HIGH);
  pinMode(buttonOne, INPUT);/*
  for(int i = 0; i < 130; i += 10){
    servos.setPWM(0, 0, map( i, 0, 1000, 0, 4096 ));
    delay(100);
  }
  servos.setPWM(0, 0, map( 0, 0, 1000, 0, 4096 ));*/
}

static boolean ready = true;
unsigned long initialTime = 0;
unsigned long finishTime = 0;

void loop() {
  holePeriod = 0;
  woodPeriod = 0;
  while(0 == woodPeriod) {
    calculateWoodPeriod();
  }
  while(0 == holePeriod) {
    calculateHolePeriod();
  }
  delay(5000);
  if (!digitalRead(irSensorPin)) { // detecting object
    while(!digitalRead(irSensorPin));
    while(digitalRead(irSensorPin));
  } else { // if it's one of the Halls
    while(digitalRead(irSensorPin));
    while(!digitalRead(irSensorPin));
    while(digitalRead(irSensorPin));
  }
  shootBag();
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

void shootBag() {     // for 800 secs
  for(int i = 0; i < 130; i += 10){
    servos.setPWM(0, 0, map( i, 0, 1000, 0, 4096 ));
    delay(100);
  }
  servos.setPWM(0, 0, map( 0, 0, 1000, 0, 4096 ));
}
