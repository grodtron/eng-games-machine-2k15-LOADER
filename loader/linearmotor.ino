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
    homeMagnet();  
  }
  
  if (pos == FIRST_POS) {
    while( digitalRead(linearMidPin) );
  } else if (pos == SECOND_POS) {
    while(! digitalRead(linearAwayPin) );
  }
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, LOW);
  Serial.println("done");
}
