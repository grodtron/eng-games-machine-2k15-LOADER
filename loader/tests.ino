void TestSetup() {
  homeMagnet();
  linearMotorToHomePosition();
  TestPickDistance();
  TestLinearMotor();
}

void TestPickDistance() { // for step calibration
  homeMagnet();
  linearMotorToHomePosition();
  Serial.println("Testing from home to bag distance");
  motorMove(MID_STEPS + PICKUP_STEPS, directionDOWN, 255, NULL);
  homeMagnet();
  Serial.println("Testing from home to mid distance");
  motorMove(MID_STEPS, directionDOWN, 255, NULL);
  delay(2000);
  Serial.println("Testing from mid to bag distance");  
  motorMove(PICKUP_STEPS, directionDOWN, 255, NULL);  
  delay(2500);
  homeMagnet();
  dropBag();
  delay(2500);
  Serial.println("Testing from upper hall to mid distance");  
  motorMove(MID_STEPS+105, directionDOWN, 255, NULL);
  delay(1000);
  Serial.println("Testing from mid to bag distance");  
  motorMove(PICKUP_STEPS, directionDOWN, 255, NULL);  
  delay(2000);
  
  Serial.println("Testing small jab distance");  
  motorMove(200, directionUP, 255, NULL);
  delay(2000);  
  motorMove(200, directionDOWN, 255, NULL);
  delay(1000);
  motorMove(285, directionUP, 255, NULL);

}

void TestLinearMotor() {
  linearMotorToHomePosition();  
  linearMotorToDroppingPosition(FIRST_POS);
  delay(2000);
  linearMotorToDroppingPosition(SECOND_POS);
  linearMotorToHomePosition();  
}
