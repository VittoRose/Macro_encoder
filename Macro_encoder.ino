#include "PluggableUSBHID.h"
#include "USBKeyboard.h"

#define A_ENC 12  // Pin signal A
#define B_ENC 13  // Pin signal B
#define S_ENC 11  // Pin switch signal
#define ENABLE 3

#define RED 8
#define GREEN 9
#define BLUE 10

#define TAB_INTERVAL 5000

#define KEY_TAB 0x09
#define KEY_ENTER 0x28
#define KEY_ESC 0x29

#define CCW -1
#define CW 1

USBKeyboard keyboard;

bool previousStateA = 0;
bool first = true;
bool flag = true;
uint8_t mode = 0;
unsigned long timer = 0;

void setup() {
  pinMode(A_ENC, INPUT_PULLUP);
  pinMode(B_ENC, INPUT_PULLUP);
  pinMode(S_ENC, INPUT_PULLDOWN);

  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  previousStateA = digitalRead(A_ENC);
  Serial.begin(9600);
}

void loop() {

  switch (mode) {
    case 0:
      Tab_RAlt();
      break;
    case 1:
      Serial.println("Mode 1");
      break;
    default:
      Serial.println("Mode default");
      break;
  }
}

int8_t encoder_rotation() {

  int n = digitalRead(A_ENC);
  int8_t ans = 0;

  // Check for rising condition
  if ((previousStateA == HIGH) && (n == LOW)) {
    if (digitalRead(B_ENC) == HIGH) {
      //Serial.println("CCW");
      previousStateA = n;
      ans = CCW;
    } else {
      //Serial.println("CW");
      previousStateA = n;
      ans = CW;
    }
  }
  previousStateA = n;
  return ans;
}

void Tab_RAlt() {
  /* Function to switch between windows using AltGr + Tab on windows */

  // Code executed only the first time in the state
  if (first) {
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
    digitalWrite(BLUE, LOW);
    first = false;
  }

  // flag used to press AltGr + Tab the first time
  if (flag) {
    if (encoder_rotation() != 0) {

      Serial.println("Encoder_motion");

      timer = millis();  // reset timer
      keyboard.key_code(KEY_TAB, KEY_RALT); // Open App selection

      flag = false;
    }
  }
  else /*(if flag == flase)*/ {
    // Use encoder as arrow if AltGr + Tab already pressed
    if (encoder_rotation() == CW) {
      timer = millis();  // reset timer
      Serial.println("Right arrow");
      keyboard.key_code(RIGHT_ARROW);
    } else if (encoder_rotation() == CCW) {
      timer = millis();
      Serial.println("Left arrow");
      keyboard.key_code(LEFT_ARROW);
    }
  }

  // Reset flag frequently, because the laptop sometimes don't recive the first input correctly
  if (millis() - timer > TAB_INTERVAL) {
    flag = true;
    timer = millis();
  }

  // Press encoder => press enter and reset flag
  if (digitalRead(S_ENC)) {
    flag = true;
    keyboard.key_code_raw(KEY_ENTER);
    delay(300);
  }
}