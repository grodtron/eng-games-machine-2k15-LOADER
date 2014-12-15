/* Read Quadrature Encoder
 * Connect Encoder to Pins encoder0PinA, encoder0PinB, and +5V.
 *
 * Sketch by max wolf / www.meso.net
 * v. 0.1 - very basic functions - mw 20061220
 *
 */
int val;
int n = LOW;
int encoderPin = 5; // sensor is on T1 and is counted using the 16 bit timer/counter 1 
int DOWNPin = A0; // blue
int UPPin = 6; // white
const int DOWN = 1;
const int UP = 0;

const int linearMotorAwayPin = 7;
const int linearMotorHomePin = 2;

const int linearHomePin = 4;
const int linearAwayPin = 3;

const int weightSensePin = A3;
const int weightSenseThreshold = 512;

const int lowerHallSensePin = A1;
const int upperHallSensePin = A2;

volatile boolean counting;

#define MOTORS_OFF() do {     \
  digitalWrite(DOWNPin, LOW); \
  digitalWrite(UPPin, LOW);   \
  counting = false;           \
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

  Serial.begin (9600);
  
  configureCounter();
  
  Serial.print("Homing magnet... ");
  motorMove(3500, UP, 255, stopMotorsOnLowerHallSensor);
  Serial.println("done");

  Serial.print("Homing linear... ");
  digitalWrite(linearMotorHomePin, HIGH);
  digitalWrite(linearMotorAwayPin, LOW);
  while(! digitalRead(linearHomePin) );
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, LOW);
  Serial.println("done");
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

ISR(TIMER1_COMPA_vect) {
  MOTORS_OFF();
}

void stopMotorsOnLowerHallSensor(){
  while(! digitalRead(lowerHallSensePin) );
 
   MOTORS_OFF();
}

void stopMotorsOnUpperHallSensor(){
  while(! digitalRead(upperHallSensePin) );
 
   MOTORS_OFF();
}

void startCountingUpTo(int steps){
  counting = true;
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

int getCurrentMoveCount(){
  int val = TCNT1L;
  val |= TCNT1H << 8;
  return val;
}

void loop() {
  Serial.println("== loop ==");
  Serial.println("Picking");
  motorMove(3000, DOWN);
  delay(200);
  Serial.println("Homing");
  
  const int pickDistance = 800;
  motorMove(pickDistance, UP);
  delay(100);
  int count = 0;
  while(analogRead(weightSensePin) < weightSenseThreshold){
    if(++count % 3 == 0){
      // Go to dropping position
      digitalWrite(linearMotorHomePin, HIGH);
      digitalWrite(linearMotorAwayPin, LOW);
      while(! digitalRead(linearHomePin) );
      digitalWrite(linearMotorHomePin, LOW);
      digitalWrite(linearMotorAwayPin, LOW);      
    }else{
      // Go to dropping position
      digitalWrite(linearMotorHomePin, LOW);
      digitalWrite(linearMotorAwayPin, HIGH);
      delay(550);
      digitalWrite(linearMotorHomePin, LOW);
      digitalWrite(linearMotorAwayPin, LOW);
    }
    
    motorMove(pickDistance, DOWN);
    motorMove(pickDistance, UP);
    
    Serial.print("read value: ");
    Serial.print(analogRead(weightSensePin));
    Serial.print(" vs threshold " );
    Serial.println(weightSenseThreshold);
    
    delay(200);
  }
  
  motorMove(3500, UP, 255, stopMotorsOnLowerHallSensor);
  delay(500);
  
  
  int weight = analogRead(weightSensePin);

  Serial.println(weight);
  
  if(weight > weightSenseThreshold){
    
    if(weight > twoBagThreshold){
      Serial.println("Got two bags Dropping");
      motorMove(300, UP, 255, stopMotorsOnUpperHallSensor);      
    )else{
      Serial.println("Dropping");
      
      // Go to dropping position
      digitalWrite(linearMotorHomePin, LOW);
      digitalWrite(linearMotorAwayPin, HIGH);
      while(! digitalRead(linearAwayPin) );
      digitalWrite(linearMotorHomePin, LOW);
      digitalWrite(linearMotorAwayPin, LOW);
      
      motorMove(300, UP, 255, stopMotorsOnUpperHallSensor);
  
      // Go back to home position
      digitalWrite(linearMotorHomePin, HIGH);
      digitalWrite(linearMotorAwayPin, LOW);
      while(! digitalRead(linearHomePin) );
      digitalWrite(linearMotorHomePin, LOW);
      digitalWrite(linearMotorAwayPin, LOW);
    }
  }else{    
    Serial.println("no bag :(");
  }
  
  
} 


// Move n steps, then stop
bool motorMove(int steps, int direction, int speed, void(*loopFunc)()) {
  Serial.print("motorMove "); Serial.println(steps);
  
  int count = 0;
  
  speed = speed % 256;
  
  if(direction == DOWN) { // move DOWN
    digitalWrite(DOWNPin, HIGH);
    analogWrite(UPPin, 255 - speed);
  }
  else { // move UPs
    analogWrite(UPPin, speed);
    digitalWrite(DOWNPin, LOW);    
  }

  startCountingUpTo(steps);

  while(counting){
    if (loopFunc) loopFunc();
  }

  return true;
}
