void triplePickUp() { // 3 jabs then test pick up
  motorMove(PICKUP_STEPS, directionDOWN, 255, NULL);
  for(int i = 0; i < 2; ++i) {
    motorMove(JAB_STEPS, directionUP, 255, NULL);
    delay(250);  
    motorMove(JAB_STEPS, directionDOWN, 255, NULL);
    delay(250);
  }
  
  motorMove(PICKUP_STEPS, directionUP, 255, NULL);
}

void swirlPickUp() { // jab, turn, jab, return, then test pick up
  motorMove(PICKUP_STEPS, directionDOWN, 255, NULL);
  motorMove(285, directionUP, 255, NULL);
  static int rotate_count = 0;
  if(rotate_count == 0) {
    for(servoPos = SERVO_MID; servoPos < SERVO_RIGHT; servoPos += 1) 
    {                            
      myservo.write(servoPos);              
      delay(SERVO_ROTATE_DELAY);                       
    }
    motorMove(HOVER_STEPS, directionDOWN, 255, NULL);
    motorMove(HOVER_STEPS, directionUP, 255, NULL);    

    for(servoPos = SERVO_RIGHT; servoPos >= SERVO_MID; servoPos -= 1) 
    {                                  
      myservo.write(servoPos);             
      delay(SERVO_ROTATE_DELAY);                       
    }  
  } else {
    for(servoPos = SERVO_MID; servoPos >= SERVO_LEFT; servoPos -= 1)  
    {                                 
      myservo.write(servoPos);              
      delay(20);                       
    }
    motorMove(HOVER_STEPS, directionDOWN, 255, NULL);
    motorMove(HOVER_STEPS, directionUP, 255, NULL);      

    for(servoPos = SERVO_LEFT; servoPos < SERVO_MID; servoPos += 1)  
    {                                
      myservo.write(servoPos);           
      delay(SERVO_ROTATE_DELAY);    
    } 
       
  }
  rotate_count = (rotate_count + 1 ) % 2;
  motorMove(HOVER_STEPS, directionDOWN, 255, NULL);
  delay(250);
  motorMove(PICKUP_STEPS, directionUP, 255, NULL);
}

void simplePickUp() { // simple up down
  motorMove(PICKUP_STEPS, directionDOWN, 255, NULL);
  motorMove(PICKUP_STEPS, directionUP, 255, NULL);
}

void rotateLoader() {
  static int rotate_count = 0;
  switch(rotate_count) {
    case 0:
      for(servoPos = SERVO_MID; servoPos < SERVO_RIGHT; servoPos += 1) 
      {                                 
        myservo.write(servoPos);              
        delay(SERVO_ROTATE_DELAY);                      
      }
      break;
    case 1:
      for(servoPos = SERVO_RIGHT; servoPos >= SERVO_MID; servoPos -= 1) 
      {                                  
        myservo.write(servoPos);              
        delay(SERVO_ROTATE_DELAY);                      
      }
      break;   
    case 2:
      for(servoPos = SERVO_MID; servoPos >= SERVO_LEFT; servoPos -= 1) 
      {                                 
        myservo.write(servoPos);              
        delay(SERVO_ROTATE_DELAY);                       
      }    
      break; 
    case 3:
    for(servoPos = SERVO_LEFT; servoPos < SERVO_MID; servoPos += 1)
    {                                
      myservo.write(servoPos);              
      delay(SERVO_ROTATE_DELAY);                    
    }
    linearMotorToNewPickingPosition(RANDOM_DELAY);
    homeMagnet();
    motorMove(MID_STEPS, directionDOWN, 255, NULL);  
    break;
  }
  rotate_count = (rotate_count + 1) % 4;
  delay(500);
}

// Below are testing functions, not used
void simpleMethod() { // simple linear pick
  for (int i = 0; i < 3; ++i) {
    rotationalLoaderMove(simplePickUp);    
    linearMotorToNewPickingPosition(RANDOM_DELAY);
    homeMagnet();
    motorMove(MID_STEPS, directionDOWN, 255, NULL);  
  }
  linearMotorToHomePosition();  
}
 
void jabMethod() { // triple jab
  for (int i = 0; i < 3; ++i) {
    rotationalLoaderMove(triplePickUp);
    linearMotorToNewPickingPosition(RANDOM_DELAY);
    homeMagnet();
    motorMove(MID_STEPS, directionDOWN, 255, NULL);  
  }
  linearMotorToHomePosition(); 
}

void swirlMethod() { // swirl pickups
  for (int i = 0; i < 7; ++i) {
    swirlPickUp();
    linearMotorToNewPickingPosition(RANDOM_DELAY);
    if (i == 3) linearMotorToHomePosition();   
  }
  homeMagnet();
  motorMove(MID_STEPS, directionDOWN, 255, NULL);  
  linearMotorToHomePosition();   
}

void rotationalLoaderMove(void(*pickUpFunc)()) {
  delay(500);
  pickUpFunc();  
  for(servoPos = SERVO_MID; servoPos < SERVO_RIGHT; servoPos += 1) 
  {                                 
    myservo.write(servoPos);              
    delay(SERVO_ROTATE_DELAY);                      
  }
  delay(500);
  pickUpFunc();
  for(servoPos = SERVO_RIGHT; servoPos >= SERVO_MID; servoPos -= 1) 
  {                                  
    myservo.write(servoPos);              
    delay(SERVO_ROTATE_DELAY);                      
  }
  delay(500);
  pickUpFunc();

  for(servoPos = SERVO_MID; servoPos >= SERVO_LEFT; servoPos -= 1) 
  {                                 
    myservo.write(servoPos);              
    delay(SERVO_ROTATE_DELAY);                       
  }
  delay(500);
  pickUpFunc();

  for(servoPos = SERVO_LEFT; servoPos < SERVO_MID; servoPos += 1)
  {                                
    myservo.write(servoPos);              
    delay(SERVO_ROTATE_DELAY);                    
  }
  delay(500);
  pickUpFunc();
}


