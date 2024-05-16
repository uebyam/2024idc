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
void robotJunction();
void robotReverse();
void robotStop();


void (*func[])() = {robotForward, robotRight, robotLeft, robotJunction};
char *dbgstr[] = {"Forward", "Right", "Left", "Junction"};

int hitJunction = 0;

void setup() {
  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);

  Serial.begin(9600);
}


void loop() {
  int irleft = analogRead(IR_LEFT);
  int irright = analogRead(IR_RIGHT);

  int bits = (irright > 400) | ((irleft > 400) << 1);
  if (bits != 3 && hitJunction > 0) hitJunction = 2;

  func[bits]();
}


void motors(int l, int r) {
  motorL.setSpeed(l);
  motorR.setSpeed(r);
}


void robotStop() { motors(0, 0); }
void robotReverse() { motors(SPEED, SPEED); }
void robotForward() { motors(-SPEED, -SPEED); }
void robotRight() { motors(-TURN_SPEED, TURN_SPEED); }
void robotLeft() { motors(TURN_SPEED, -TURN_SPEED); }


void robotJunction() {
  if (hitJunction == 0) {
    hitJunction++;
    robotForward();
  }

  if (hitJunction < 2) return;

  robotStop();

  Serial.print("H");

  for(;;) delay(1000);
}