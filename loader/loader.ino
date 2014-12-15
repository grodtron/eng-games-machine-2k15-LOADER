/* Read Quadrature Encoder
 * Connect Encoder to Pins encoder0PinA, encoder0PinB, and +5V.
 *
 * Sketch by max wolf / www.meso.net
 * v. 0.1 - very basic functions - mw 20061220
 *
 */
#include <Wire.h>

#define BADGER_ADDRESS 0x9
#define LOADER_ADDRESS 0x8 
#define directionDOWN 1
#define directionUP   0

int val;
int encoderPin = 5; // sensor is on T1 and is counted using the 16 bit timer/counter 1 
int DOWNPin = A0;   // blue
int UPPin = 6;      // white

const int linearMotorAwayPin = 7;
const int linearMotorHomePin = 2;

const int linearHomePin = 4;
const int linearAwayPin = 3;

const int weightSensePin = A3;
const int weightSenseThreshold = 512; // 480 !
const int twoBagThreshold = 590;

const int lowerHallSensePin = A1;
const int upperHallSensePin = A2;

volatile boolean motorIsON;

//distance the magnet goes up/down by to check/get bags
const int pickDistance = 800;

#define MOTORS_OFF() do {     \
  digitalWrite(DOWNPin, LOW); \
  digitalWrite(UPPin, LOW);   \
  motorIsON = false;           \
} while(0)

bool motorMove(int steps, int direction, int speed=255, void(*loopFunc)() = NULL);

void setup() { 
  pinMode (encoderPin,INPUT);
  pinMode (lowerHallSensePin,INPUT);
  pinMode (upperHallSensePin,INPUT);
  pinMode (DOWNPin,OUTPUT);
  pinMode (UPPin,OUTPUT);

  pinMode(linearMotorAwayPin, OUTPUT);
  pinMode(linearMotorHomePin, OUTPUT);
  pinMode(linearAwayPin, INPUT);
  pinMode(linearHomePin, INPUT);
  /*
  Wire.begin(LOADER_ADDRESS);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  */
  Serial.begin (9600);

  configureCounter();

  Serial.print("Homing magnet... ");
  motorMove(3500, directionUP, 255, stopMotorsOnLowerHallSensor);
  Serial.println("done");

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

ISR(TIMER1_COMPA_vect) { MOTORS_OFF(); }

void stopMotorsOnLowerHallSensor(){
  while(! digitalRead(lowerHallSensePin) );

   MOTORS_OFF();
}

void stopMotorsOnUpperHallSensor(){
  while(! digitalRead(upperHallSensePin) );

   MOTORS_OFF();
}

void startCountingUpTo(int steps){
  motorIsON = true;
  // count up to `steps`. NB that OCR1AH must be written *before* OCR1AL
  OCR1AH = 0xff & (steps >> 8);
  OCR1AL = 0xff &  steps;
  
  // Zero out the counter register
  TCNT1H = 0;
  TCNT1L = 0;
  
  // enable output compare interrupt
  TIMSK1 = (1 << OCIE1A);
  sei();
}

int getCurrentMoveCount(){ //UNUSED
  int val = TCNT1L;
  val |= TCNT1H << 8;
  return val;
}

void loop() {

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
      /* stops based on a delay
         makes exact position of drop a bit random */
      linearMotorToDroppingPosition(0);
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

  if(weight > weightSenseThreshold){
    /*if(weight > twoBagThreshold){
      Serial.println("Got two bags, Dropping!");
      motorMove(300, directionUP, 255, stopMotorsOnUpperHallSensor);      
    }else{*/
      Serial.println("Dropping the one bag");
      // stops based on LinearAwaySensor reading
      linearMotorToDroppingPosition(1);
      motorMove(300, directionUP, 255, stopMotorsOnUpperHallSensor);
      linearMotorToHomePosition();
    //}
  }
  else
  {    
    Serial.println("no bag :(");
  }
}

void linearMotorToDroppingPosition (int check) {
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, HIGH);
  if (check == 0) {
    delay(550);
  }else{
    while(! digitalRead(linearAwayPin) );
  }
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
  while(motorIsON){
    if (loopFunc) loopFunc();
  }

  return true;
}

/*
void sendFlapToClose(int servonum, int state)
{
  Wire.beginTransmission(BADGER_ADDRESS); // Master writer
  Wire.write(servonum);  
  Wire.write(state);
  Wire.endTransmission();
  Serial.print(servonum);
  Serial.println(state);
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

// function that executes when data is requested by badger/master
void requestEvent()
{
  Wire.write("Responding to badger request"); // 28 bytes
}*/
