#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
 
Adafruit_PWMServoDriver servos = Adafruit_PWMServoDriver();

#define irSensorPin 7      // IR sensor on this pin

const int buttonOne = 0;
const int buttonTwo = 2;
const int targetPin = 3;
const int motorPin = 6;
int holePeriod;
int woodPeriod;

#define MAX_BAG_COUNT 4
const int closeOrder[MAX_BAG_COUNT] = {4, 3, 2, 1};
const int ShootingSpeed = 1300;

#define CLOSE 1100
#define OPEN 400

int bag_count = 4;
boolean button1_ready = true, button2_ready = true, button3_ready = true;

void setup() {
  Serial.begin(9600);
  servos.begin();
  servos.setPWMFreq(100);
  pinMode(irSensorPin, INPUT);
  pinMode(targetPin, INPUT);
  digitalWrite(targetPin, HIGH);
  pinMode(buttonOne, INPUT);
  pinMode(buttonTwo, INPUT);
  
  // open all flaps
  for(int i = 0; i < MAX_BAG_COUNT; ++i) {
    servos.setPWM(closeOrder[i], 0, OPEN);
    delay(500);
  }

  servos.setPWM(closeOrder[0], 0, CLOSE);

    holePeriod = 0;
    woodPeriod = 0;
    while(0 == woodPeriod) {
      calculateWoodPeriod();
    }
    while(0 == holePeriod) {
      calculateHolePeriod();
    }
  /*
  for(int i = 0; i < 130; i += 10){
    servos.setPWM(0, 0, map( i, 0, 1000, 0, 4096 ));
    delay(100);
  }
  servos.setPWM(0, 0, map( 0, 0, 1000, 0, 4096 ));*/
}

static boolean ready = true;
unsigned long initialTime = 0;
unsigned long finishTime = 0;

int count1 = 0, count2 = 0;

void loop() {  
  static int count1 = 0, count2 = 0;
  if(!button1_ready || digitalRead(buttonOne)) {
     button1_ready = true; 
  }else {
    Serial.print("Closing servo ");
    Serial.println(closeOrder[count1]);
      button1_ready = false;
      servos.setPWM(closeOrder[count1], 0, CLOSE);
      count1 = (count1 + 1) % 4;
      delay(500);   
  }
  
  if(!button2_ready || digitalRead(buttonTwo)) {
     button2_ready = true; 
  }else {
    button2_ready = false;
    Serial.println("Starting shooting process");
    startShooting();
  }
  
}

void startShooting() {
 while(bag_count > 0) {
   openFlaps();
   delay(1250);
   servos.setPWM(4, 0, OPEN);
   timeBagShooting();
   servos.setPWM(4, 0, CLOSE);
   delay(1000);
 } 
}

// Opens a flap
void openFlaps()
{
  if(bag_count < 0) {
    Serial.println("wtf got negative bags");
    bag_count = MAX_BAG_COUNT;
  }
  
  Serial.print("Opening servo ");
  Serial.println(closeOrder[MAX_BAG_COUNT - bag_count]);
  servos.setPWM(closeOrder[MAX_BAG_COUNT - bag_count--], 0, OPEN);
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

void shootBag() {     // for ~1300 msecs
  for(int i = 0; i < 130; i += 10){
    servos.setPWM(0, 0, map( i, 0, 1000, 0, 4096 ));
    delay(100);
  }
  servos.setPWM(0, 0, map( 0, 0, 1000, 0, 4096 ));
}

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
