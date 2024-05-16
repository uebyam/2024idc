#include <AM2302-Sensor.h>
#include "CytronMotorDriver.h"

#define IR_LEFT    A0
#define IR_RIGHT   A1
#define SPEED      150
#define TURN_SPEED 120
#define TEMP       7
#define FAN        6

CytronMD motorL(PWM_PWM, 3, 9);
CytronMD motorR(PWM_PWM, 10, 11);
AM2302::AM2302_Sensor am2302{TEMP};


void robotForward();
void robotRight();
void robotLeft();
void robotTask();
void robotReverse();
void robotStop();


void (*func[])() = {robotForward, robotRight, robotLeft, robotTask};
char *dbgstr[] = {"Forward", "Right", "Left", "Task"};

unsigned long am2302InitTime = 0;

void setup() {
  pinMode(IR_LEFT, INPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(FAN, OUTPUT);

  digitalWrite(FAN, 0);

  if (!am2302.begin()) {
    for (int i = 1;; i = !i) {
      digitalWrite(12, i);
      delay(200);
    }
  } else am2302InitTime = millis();

  Serial.begin(9600);
}


void loop() {
  int irleft = analogRead(IR_LEFT);
  int irright = analogRead(IR_RIGHT);

  int bits = (irright > 400) | ((irleft > 400) << 1);

  func[bits]();
}


void motors(int l, int r) { motorL.setSpeed(l); motorR.setSpeed(r); }
void robotStop() { motors(0, 0); }
void robotReverse() { motors(SPEED, SPEED); }
void robotForward() { motors(-SPEED, -SPEED); }
void robotRight() { motors(-TURN_SPEED, TURN_SPEED); }
void robotLeft() { motors(TURN_SPEED, -TURN_SPEED); }


void robotTask() {
  static int fanStatus = 0;
  
  robotStop();

  Serial.print("D");
  Serial.flush();

  int die = 1;
  for (int i = 0; i < 500; i++) {
    if (Serial.read() == 'Y') {
      die = 0;
      break;
    }
    delay(20);
  }
  if (die) {
    Serial.print("0");
    for (;;) delay(1000);
  }
  
  while (1) {
    int chr = Serial.read();
    if (chr == 'T') {  // Temperature
      while (millis() < am2302InitTime + 3000) delay(1000);
      am2302.read();
      String sTemp = String(am2302.get_Temperature());
      Serial.write(sTemp.length());  // assumes length < 256. it usually is
      Serial.write(sTemp.c_str(), sTemp.length());
    } else if (chr == 'F') {  // Fan
      fanStatus = !fanStatus;
      digitalWrite(FAN, fanStatus);
    } else if (chr == 'B') break;
    else delay(20);
  }

