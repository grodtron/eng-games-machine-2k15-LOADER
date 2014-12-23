void TestSetup() {
  homeMagnet();
  linearMotorToHomePosition();
}

void runAllTests() {
  TestPickDistance();
  TestLinearMotor();  
}

void TestPickDistance() { // for step calibration
  Serial.println("====Testing step distances====");

  homeMagnet();
  Serial.println("Testing from home to bag distance");
  motorMove(MID_STEPS + PICKUP_STEPS, directionDOWN, 255, NULL);
  delay(2000);
  homeMagnet();
  Serial.println("Testing from home to mid distance");
  motorMove(MID_STEPS, directionDOWN, 255, NULL);
  delay(2000);
  Serial.println("Testing from mid to bag distance");  
  motorMove(PICKUP_STEPS, directionDOWN, 255, NULL);  
  delay(2500);
  Serial.println("Testing small jab distance");  
  motorMove(JAB_STEPS, directionUP, 255, NULL);
  delay(2000);  
  motorMove(JAB_STEPS, directionDOWN, 255, NULL);
  delay(2000);
  motorMove(HOVER_STEPS, directionUP, 255, NULL);
  delay(2000);
  motorMove(HOVER_STEPS, directionDOWN, 255, NULL);
  delay(2000);
  motorMove(HOVER_STEPS, directionUP, 255, NULL);
  delay(2000);
  homeMagnet();
}

void TestLinearMotor() {
  Serial.println("====Testing rail motor====");
  linearMotorToHomePosition();  
  linearMotorToDroppingPosition(FIRST_POS);
  delay(2000);
  linearMotorToDroppingPosition(SECOND_POS);
  linearMotorToHomePosition();  
}

void TestServo() {
  Serial.println("====Testing loader servo====");

  for(servoPos = SERVO_MID; servoPos < SERVO_RIGHT; servoPos += 1) 
  {                                 
    myservo.write(servoPos);              
    delay(SERVO_ROTATE_DELAY);                      
  }
  delay(1000);

  for(servoPos = SERVO_RIGHT; servoPos >= SERVO_MID; servoPos -= 1) 
  {                                  
    myservo.write(servoPos);              
    delay(SERVO_ROTATE_DELAY);                      
  }
  delay(1000);

  for(servoPos = SERVO_MID; servoPos >= SERVO_LEFT; servoPos -= 1) 
  {                                 
    myservo.write(servoPos);              
    delay(SERVO_ROTATE_DELAY);                       
  }
  delay(1000);

  for(servoPos = SERVO_LEFT; servoPos < SERVO_MID; servoPos += 1)
  {                                
    myservo.write(servoPos);              
    delay(SERVO_ROTATE_DELAY);                    
  }
  delay(1000);  
}

void TestI2CArduino() {
  Serial.println("====Testing i2c communication with other arduino====");
  for(int i = 0; i < 8; ++i) {
    sendFlapToClose(0);
    delay(1000);   
  }
}

void TestWeightSensor() {
  Serial.println("====Testing weight sensor====");

  homeMagnet();
  linearMotorToHomePosition();
  motorMove(MID_STEPS, directionDOWN, 255, NULL);  
  for(; ; ) {
    int weight = analogRead(weightSensePin);
    Serial.print("weight pulled value: ");
    Serial.println(weight);
    delay(1250);
  }  
}

void TestDrop() {
  Serial.println("====Testing pickup and drop====");

  homeMagnet();
  linearMotorToHomePosition();
  motorMove(MID_STEPS+PICKUP_STEPS, directionDOWN, 255, NULL);  
  delay(1500);
  homeMagnet();
  delay(1000);
  dropBag();
  motorMove(200, directionDOWN, 255, NULL);  
}
