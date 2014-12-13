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
int DOWNPin = 10; // blue
int UPPin = 9; // white
const int DOWN = 1;
const int UP = 0;

const int weightSensePin = A0;
const int weightSenseThreshold = 450;

const int lowerHallSensePin = A1;
const int upperHallSensePin = A2;

volatile boolean counting;

#define MOTORS_OFF() do {     \
  digitalWrite(DOWNPin, LOW); \
  digitalWrite(UPPin, LOW);   \
  counting = false;           \
} while(0)

bool motorMove(int steps, int direction, void(*loopFunc)() = NULL);

void setup() { 
  pinMode (encoderPin,INPUT);
  pinMode (lowerHallSensePin,INPUT);
  pinMode (upperHallSensePin,INPUT);
  pinMode (DOWNPin,OUTPUT);
  pinMode (UPPin,OUTPUT);

  Serial.begin (9600);
  
  configureCounter();
  
  Serial.println("Homing");
  motorMove(3500, UP, stopMotorsOnLowerHallSensor);
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
  motorMove(2600, DOWN);
  delay(200);
  Serial.println("Homing");
  
  const int pickDistance = 1400;
  motorMove(pickDistance, UP);
  delay(100);
  while(analogRead(weightSensePin) < weightSenseThreshold){
    motorMove(pickDistance, DOWN);
    motorMove(pickDistance, UP);
    delay(100);
  }
  
  motorMove(3500, UP, stopMotorsOnLowerHallSensor);
  delay(500);
  if(analogRead(weightSensePin) > weightSenseThreshold){
    Serial.println("Dropping");
    motorMove(300, UP, stopMotorsOnUpperHallSensor);
    delay(500);
  }else{
    Serial.println("no bag :(");
  }
  
  
} 


// Move n steps, then stop
bool motorMove(int steps, int direction, void(*loopFunc)()) {
  Serial.print("motorMove "); Serial.println(steps);
  
  int count = 0;
  
  if(direction == DOWN) { // move DOWN
    digitalWrite(DOWNPin, HIGH);
    digitalWrite(UPPin, LOW);
  }
  else { // move UPs
    digitalWrite(UPPin, HIGH);
    digitalWrite(DOWNPin, LOW);    
  }

  startCountingUpTo(steps);

  while(counting){
    if (loopFunc) loopFunc();
  }

  return true;
}
