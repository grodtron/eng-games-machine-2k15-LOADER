void runTests() {
//while(1) TestSwitches();
//  TestPickDistance();
  TestLinearMotor();
//  TestWeightSensor();
//  TestServo();
//  TestHallSensor();
//  TestDrop();
//  TestI2CArduino();
}

void TestPickDistance() { // for step calibration
  Serial.println("====Testing step distances====");

  homeMagnet();
  linearMotorToHomePosition();

  Serial.println("Testing from home to bag distance");
  motorMove(MID_STEPS + PICKUP_STEPS, directionDOWN, 255, NULL);
  while(!Serial.available()) ; Serial.read(); 
  homeMagnet();
  Serial.println("Testing from home to mid distance");
  motorMove(MID_STEPS, directionDOWN, 255, NULL);
  while(!Serial.available()) ; Serial.read(); 
  Serial.println("Testing from mid to bag distance");  
  motorMove(PICKUP_STEPS, directionDOWN, 255, NULL);  
  while(!Serial.available()) ; Serial.read(); 

  for(int i = 0; i < 3; ++i) {
    linearMotorToNewPickingPosition(RANDOM_DELAY); // Move back to a new position if we tried all 3 spots
//    rotateLoader();
    while(!Serial.available()) ; Serial.read();
  }

  linearMotorToHomePosition();

  Serial.println("Testing small jab distance");  
  motorMove(JAB_STEPS, directionUP, 255, NULL);
  while(!Serial.available()) ; Serial.read(); 
  motorMove(JAB_STEPS, directionDOWN, 255, NULL);
  while(!Serial.available()) ; Serial.read(); 
  motorMove(HOVER_STEPS, directionUP, 255, NULL);
  while(!Serial.available()) ; Serial.read(); 
  motorMove(HOVER_STEPS, directionDOWN, 255, NULL);
  while(!Serial.available()) ; Serial.read(); 
  motorMove(PICKUP_STEPS, directionUP, 255, NULL);
  while(!Serial.available()) ; Serial.read(); 
  homeMagnet();
}

void TestLinearMotor() {
  Serial.println("====Testing rail motor====");
  homeMagnet();
  linearMotorToHomePosition(false);  
  while(!Serial.available()) ; Serial.read(); 
  linearMotorToDroppingPosition(FIRST_POS, false);
  servoTurnLeft(30);
  while(!Serial.available()) ; Serial.read(); 
  linearMotorToDroppingPosition(SECOND_POS);
  while(!Serial.available()) ; Serial.read(); 
  linearMotorToHomePosition(false);  
}

void TestServo() {
  Serial.println("====Testing loader servo====");
  homeMagnet();
  linearMotorToHomePosition();
  for(int i = 0; i < 3; ++i) {
  motorMove(MID_STEPS + PICKUP_STEPS, directionDOWN, 255, NULL);
  servoTurnRight(30);
  while(!Serial.available()) ; Serial.read(); 
  centerServo(30);
  while(!Serial.available()) ; Serial.read(); 
  servoTurnLeft(30);
  while(!Serial.available()) ; Serial.read(); 
  centerServo(30);
  while(!Serial.available()) ; Serial.read(); 
  }
}

void TestI2CArduino() {
  Serial.println("====Testing i2c communication with other arduino====");
  static int count = 0;
  for(int i = 0; i < 8; ++i) {
    while(!Serial.available()) ; Serial.read(); 
    sendFlapToClose(count);
    count = (count + 1) % 2;  
  }
}

void TestWeightSensor() {
  Serial.println("====Testing weight sensor====");

  homeMagnet();
  linearMotorToHomePosition();
  motorMove(MID_STEPS, directionDOWN, 255, NULL);  
  for(int i = 0; i < 10; ++i) {
    int weight = analogRead(weightSensePin);
    Serial.print("weight pulled value: ");
    Serial.println(weight);
    while(!Serial.available()) ; Serial.read(); 
//    delay(1250);
  }  
}

void TestDrop() {
  Serial.println("====Testing pickup and drop====");
  static int tower_count = 0;
  linearMotorToHomePosition();
  homeMagnet();
  motorMove(MID_STEPS+PICKUP_STEPS, directionDOWN, 255, NULL);  
  while(!Serial.available()) ; Serial.read(); 
  linearMotorToDroppingPosition(tower_count++ % 2);
  homeMagnet();
  while(!Serial.available()) ; Serial.read(); 
  dropBag();
  motorMove(200, directionDOWN, 255, NULL);  
}

void TestPullupAndMove() {
  linearMotorToDroppingPosition (SECOND_POS, true); 
  while(!Serial.available()) ; Serial.read(); 
  linearMotorToHomePosition (true);
}

void TestHallSensor() {
 Serial.print("Lower hall sensor: "); Serial.print(digitalRead(lowerHallSensePin)); 
 Serial.print(" Upper hall sensor: "); Serial.print(digitalRead(upperHallSensePin)); 
 Serial.println("===");
 delay(250);
}

void TestSwitches() {
 Serial.print("home switch(");
 Serial.print(linearHomePin);
 Serial.print(")=");
 Serial.println(digitalRead(linearHomePin)); 
 
 Serial.print("away switch(");
 Serial.print(linearAwayPin);
 Serial.print(")="); 
 Serial.println(digitalRead(linearAwayPin));   

 Serial.print("mid switch(");
 Serial.print(linearMidPin);
 Serial.print(")=");
 Serial.println(digitalRead(linearMidPin));

 Serial.println("===");
 delay(1000); 
}
