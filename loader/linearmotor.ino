void linearMotorToNewPickingPosition (int delayTime) {
  Serial.print("Trying new picking position... ");
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, HIGH);
  delay(delayTime);
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, LOW);
  Serial.println("done");
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

void linearMotorToDroppingPosition (int pos) {
  Serial.print("to dropping position linear... ");
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, HIGH);
  if (pos == FIRST_POS) {
    while( digitalRead(linearMidPin) );
  } else if (pos == SECOND_POS) {
    while(! digitalRead(linearAwayPin) );
  }
  digitalWrite(linearMotorHomePin, LOW);
  digitalWrite(linearMotorAwayPin, LOW);
  Serial.println("done");
}
