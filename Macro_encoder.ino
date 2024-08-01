#include "PluggableUSBHID.h"
#include "USBKeyboard.h"

// RP2040 zero connection
#define A_ENC 12  // Pin signal A
#define B_ENC 13  // Pin signal B
#define S_ENC 11  // Pin switch signal
#define ENABLE 3

#define RED 8
#define GREEN 9
#define BLUE 10

// Value for analogWrite
#define FULLON 512
#define HALFON 100

// Time value
#define TAB_INTERVAL 1000
#define DEBOUNCEDELAY 500
#define DOUBLECLICK 400

// Key definition
#define KEY_TAB 0x09
#define KEY_ENTER 0x28
#define KEY_ESC 0x29

// Encoder direction
#define CCW -1
#define CW 1

// Number of mode implemented
#define MAX_MODE 2

USBKeyboard keyboard;

// Encoder state memory
bool previousStateA = 0;
bool previousStateS = 0;

// Transition variable
bool transition = false;
bool first = true;
bool flag = true;
uint8_t mode = 0;

// Timer variable
uint32_t timer = 0;
uint32_t transition_timer = 0;
uint32_t timer1 = 0;
uint32_t first_timer = 0;


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
  
  // Do not remove delay 
  delay(1); 
  
  switch (mode) {
    
    case 0:
      Tab_RAlt();
      break;
    
    case 1:
      Cursor_control();
      break;

    default:    // Red blink -> error in the execution

      digitalWrite(BLUE, LOW);
      digitalWrite(GREEN, LOW);
      digitalWrite(RED,HIGH);
      delay(300);
      digitalWrite(RED,LOW);
      delay(300);
      break;
  }
}

int8_t encoder_rotation() {

  bool n = digitalRead(A_ENC);
  int8_t ans = 0;

  // Check for rising condition
  if ((previousStateA == HIGH) && (n == LOW)) {
    if (digitalRead(B_ENC) == HIGH) {
      Serial.println("CCW");
      ans = CCW;
    } else {
      Serial.println("CW");
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
    analogWrite(RED, FULLON);
    analogWrite(GREEN, LOW);
    analogWrite(BLUE, LOW);
    first = false;
    first_timer = millis();
    flag = true;
  }
  int rot = encoder_rotation();
  // flag used to press AltGr + Tab the first time
  if (flag) {
    if (rot != 0) {

      timer = millis();  // reset timer
      keyboard.key_code(KEY_TAB, KEY_RALT); // Open App selection

      flag = false;
    }
  }
  else /*(if flag == flase)*/ {
    // Use encoder as arrow if AltGr + Tab already pressed
    if (rot == CCW) {
      timer = millis();
      keyboard.key_code(LEFT_ARROW);
    }
    if (rot == CW) {
      timer = millis();  // reset timer
      keyboard.key_code(RIGHT_ARROW);
    } 
  }

  // Reset flag frequently, because the laptop sometimes don't recive the first input correctly
  if (millis() - timer > TAB_INTERVAL) {
    flag = true;
    timer = millis();
  }

  // Press encoder => press enter and reset flag
  if (digitalRead(S_ENC) && (millis() - timer1 >= DEBOUNCEDELAY ) && (millis() - first_timer >= DOUBLECLICK)) {
    flag = true;
    keyboard.key_code_raw(KEY_ENTER);
    timer1 = millis();
  }

  state_transition();
}

void Cursor_control(){
  /* Control cursor left/right or up/down by rotating, press the encoder to change mod*/

  // Code executed only the first time in the state
  if (first) {
    analogWrite(RED, LOW);
    analogWrite(GREEN, FULLON);
    analogWrite(BLUE, LOW);
    first = false;
    flag = true;
  }

  // Change flag variable once in DEBOUNCEDELAY ms
  if((millis() - timer >= DEBOUNCEDELAY) && digitalRead(S_ENC)){
    flag = !flag;
    timer = millis();
  } 

  int8_t rot = encoder_rotation();

  if(flag){
    if(rot == CW) keyboard.key_code(RIGHT_ARROW);
    if(rot == CCW) keyboard.key_code(LEFT_ARROW);
    analogWrite(RED, LOW);
  }
  else{   
    if(rot == CW) keyboard.key_code(DOWN_ARROW);
    if(rot == CCW) keyboard.key_code(UP_ARROW);
    analogWrite(RED, HALFON);
  }

  // Function that handle state transition
  state_transition();

}

void state_transition(){
  /* Function that detect a doubleclick to change the variable "mode" */

  bool stateS = digitalRead(S_ENC);

  // Check if there is a double click between now and the previous loop
  if(transition && stateS && (millis() - transition_timer <= DOUBLECLICK && !previousStateS)){
    
    transition = false;
    first = true;

    // Update timer to avoid moultiple mode change
    transition_timer = millis() + DOUBLECLICK;
    
    mode ++; if(mode > MAX_MODE-1) mode = 0;   // Switch mode
  }

  // Save time when clicked first time
  if((!previousStateS) && stateS && !transition) {
    transition = true;
    transition_timer = millis();
  }

  // Reset flag if too much time pass
  if(transition && (millis()- transition_timer > DOUBLECLICK)) transition = false;

  // Save Switch state
  previousStateS = stateS;
}