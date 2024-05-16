#include "CytronMotorDriver.h"

#define IR_LEFT    A0
#define IR_RIGHT   A1
#define SPEED      150
#define TURN_SPEED 120

CytronMD motorL(PWM_PWM, 3, 9);
CytronMD motorR(PWM_PWM, 10, 11);


void robotForward();
void robotRight();
void robotLeft();
void robotUTurn();
void robotReverse();
void robotStop();


void (*func[])() = {robotForward, robotRight, robotLeft, robotUTurn};
char *dbgstr[] = {"Forward", "Right", "Left", "U-Turn"};


void setup() {
  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);
}


void loop() {
  int irleft = analogRead(IR_LEFT);
  int irright = analogRead(IR_RIGHT);

  int bits = (irright > 400) | ((irleft > 400) << 1);
  func[bits]();
}


void robotStop() {
  motorL.setSpeed(0);
  motorR.setSpeed(0);
}


void robotReverse() {
  motorL.setSpeed(SPEED);
  motorR.setSpeed(SPEED);
}


void robotForward() {
  motorL.setSpeed(-SPEED);
  motorR.setSpeed(-SPEED);
}


void robotRight() {
  motorL.setSpeed(-TURN_SPEED);
  motorR.setSpeed(TURN_SPEED);
}


void robotLeft() {
  motorL.setSpeed(TURN_SPEED);
  motorR.setSpeed(-TURN_SPEED);
}


void robotUTurn() {
  robotStop();
  delay(100);

  int irLeft = analogRead(IR_LEFT);
  int irRight = analogRead(IR_RIGHT);

  auto busywaitfunc = [&](unsigned long ms, void (*run)()) {
    run();
    delay(ms);
    irLeft = analogRead(IR_LEFT);
    irRight = analogRead(IR_RIGHT);
  };
  
  
  while (irRight <= 400) busywaitfunc(5, robotReverse);

  robotStop();
  delay(100);
  
  while (irRight > 400) busywaitfunc(5, robotRight);
  while (irRight <= 400) busywaitfunc(5, robotRight);
  while (irRight > 400) busywaitfunc(5, robotRight);

  robotStop();
  delay(100);
}