void linearMotorToNewPickingPosition (int delayTime) {
  Serial.print("Trying new picking position... ");
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, HIGH);
  delay(delayTime);
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, LOW);
  Serial.println("done");
}

void linearMotorToHomePosition (boolean lowerMagnetWhileMoving) {
  Serial.print("Homing linear... ");
  digitalWrite(linearMotorHomePin, HIGH);
  digitalWrite(linearMotorAwayPin, LOW);
  
  if(lowerMagnetWhileMoving) {
    delay(650); // we could wait till middle linear pin is hit to start dropping magnet, needs to be tested
    motorMove(MID_STEPS, directionDOWN, 255, NULL);    
  }
  
  while(! digitalRead(linearHomePin) );
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, LOW);
  Serial.println("done");
}

void linearMotorToDroppingPosition (int pos, boolean homeMagnetWhileMoving) {
  Serial.print("to dropping position linear... ");
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, HIGH);
  
  if(homeMagnetWhileMoving) {
//    delay(250); // wait a bit to reduce the shakiness
    homeMagnet();  
  }
  
  if (pos == FIRST_POS) {
    while(! digitalRead(linearMidPin) && !digitalRead(linearAwayPin));
  } else if (pos == SECOND_POS) {
    while(! digitalRead(linearAwayPin) );
  }
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, LOW);
  Serial.println("done");
}
