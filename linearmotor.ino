void linearMotorToNewPickingPosition (int delayTime) {
  Serial.print("Trying new picking position... ");

  /*analogWrite(linearMotorHomePin, 0);
  analogWrite(linearMotorAwayPin, 200);*/  
  
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, HIGH);
  delay(delayTime);
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, LOW);
  /*analogWrite(linearMotorHomePin, 0);
  analogWrite(linearMotorAwayPin, 0);*/
  Serial.println("done");
}

void linearMotorToHomePosition (boolean lowerMagnetWhileMoving) {
  Serial.print("Homing linear... ");

  /*analogWrite(linearMotorHomePin, 200);
  analogWrite(linearMotorAwayPin, 0);*/

  digitalWrite(linearMotorHomePin, HIGH);
  digitalWrite(linearMotorAwayPin, LOW);
  
  if(lowerMagnetWhileMoving) {
    delay(650); // we could wait till middle linear pin is hit to start dropping magnet, needs to be tested
    motorMove(MID_STEPS, directionDOWN, 255, NULL);    
  }
  
  while(! digitalRead(linearHomePin) );
  
  /*analogWrite(linearMotorHomePin, 0);
  analogWrite(linearMotorAwayPin, 0);*/
  
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, LOW);
  Serial.println("done");
}

void linearMotorToDroppingPosition (int pos, boolean homeMagnetWhileMoving) {
  Serial.print("to dropping position linear... ");

  /*analogWrite(linearMotorHomePin, 0);
  analogWrite(linearMotorAwayPin, 200);*/

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

  /*analogWrite(linearMotorHomePin, 0);
  analogWrite(linearMotorAwayPin, 0);*/

  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, LOW);
  Serial.println("done");
}
