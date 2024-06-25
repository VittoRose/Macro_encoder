#include "PluggableUSBHID.h"
#include "USBKeyboard.h"

#define A_ENC 0  // Pin signal A
#define B_ENC 1  // Pin signal B
#define S_ENC 2  // Pin switch signal
#define ENABLE 3
#define SIZE 100


#define KEY_TAB 0x09
#define KEY_ENTER 0x28

USBKeyboard keyboard;

int counter = 0;
int currentStateA;
int previousStateA;
int old_counter = 0;
uint8_t mode = 0;
bool flag = true;
bool first = true;

unsigned long timer = 0;


void setup() {

  pinMode(A_ENC, INPUT_PULLUP);
  pinMode(B_ENC, INPUT_PULLUP);
  pinMode(S_ENC, INPUT_PULLDOWN);

  previousStateA = digitalRead(A_ENC);

  Serial.begin(115200);
}

void loop() {
  /*
  if (digitalRead(S_ENC)) {
    Serial.println("Button");
    keyboard.key_code(KEY_TAB, KEY_RALT);
    delay(300);
  }
*/

  switch (mode) {
    case 0:
      Tab_RAlt();
      break;
    default:
      // do nothing
      break;
  }
}


void Tab_RAlt() {
  /* Function to switch between windows using AltGr + Tab on windows */

  // Code executed only the first time in the state
  if (first) {

    first = false;
  }

  Serial.print("Flag: "); Serial.println(flag);

  int counter = EncCounter();
  // flag used to press AltGr + Tab the first time
  if (counter != old_counter && flag) {
    if (counter % 2 == 1) {
      // Do nothing
    } else {
      if (EncDir() == 1) {
        keyboard.key_code(KEY_TAB, KEY_RALT);
        timer = millis();
      } else if (EncDir() == 2) {
        keyboard.key_code(KEY_TAB, KEY_RALT);
        timer = millis();
      }
      flag = false;
    }
  }

  // Use encoder as arrow if AltGr + Tab already pressed
  if (counter != old_counter && !flag) {
    if (counter % 2 == 1) {
      // Do nothing
    } else {
      if (EncDir() == 1) {
        keyboard.key_code(RIGHT_ARROW);
        timer = millis();                 // reset timer
        Serial.println("Right arrow, timer reset");
      } else if (EncDir() == 2) {
        keyboard.key_code(LEFT_ARROW);
        timer = millis();                 // reset timer
        Serial.println("left arrow, timer reset");
      }
    }
  }

  if (digitalRead(S_ENC) && !flag) {
    flag = true;
    keyboard.key_code_raw(KEY_ENTER);
    Serial.println("ENTER + FLAG = TRUE");
  }
  
  // Reset flag if timer runout
  if(!flag && (millis() - timer >= 1000)) flag = true;

  old_counter = counter;
}

bool EncoderWasSleep() {
}

int EncCounter() {
  // Function that return a modified counter variable after the motion of the encoder
  currentStateA = digitalRead(A_ENC);

  if (currentStateA != previousStateA) {
    if (digitalRead(B_ENC) != currentStateA) counter++;  // If CW increase counter
    else counter--;
  }

  // Update encoder state
  previousStateA = currentStateA;

  return counter;
}

int EncDir() {
  // Function that return 1 if encoder moved CW, 2 if CCW
  if (EncCounter() - old_counter > 0) {
    return 1;
  } else if (EncCounter() - old_counter < 0) {
    return 2;
  }
  return 0;
}
