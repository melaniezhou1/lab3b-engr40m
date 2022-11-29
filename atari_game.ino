/* ATARI BREAKOUT (ADAPTED)
OBJECTIVE OF GAME: "Breakout" of the top row of LEDs by hitting the moving ball on the paddle that you control with left and right controls
HOW TO MOVE THE BALL BETWEEN COLUMNS: If the ball hits the left side of the paddle, the ball moves to the left by one column. 2. If the ball hits the right side of the paddle, 
the ball moves to the right by one column. 
ON THE LEFTMOST AND RIGHTMOST COLUMN: If the ball has already broken out of the LEDs on the side, the ball will reset to the middle of the game so that you can continue play and 
not get stuck in an infinite loop. 
*/

const int L_BUTTON = A5;
const int R_BUTTON = 0; 
const int S_BUTTON = A4;
const int DEBOUNCE_DELAY = 75; // in milliseconds
const byte ANODE_PINS[8] = {13, 12, 11, 10, 9, 8, 7, 6};
const byte CATHODE_PINS[8] = {A3, A2, A1, A0, 5, 4, 3, 2};
const int ON = 1; 
const int OFF = 0;

int WIN = 0; 
int BALL_X = 4;
int BALL_Y= 3;
byte left_paddle = 2;
byte middle_paddle = 3;
byte right_paddle = 4;
byte paddle_row = 7;

const int TIME_DELAY = 200;
unsigned long previous = 0;
unsigned long start = 1;
unsigned long down = 1;

const byte smile[8][8] = 
{{0, 0, 1, 0, 1, 0, 0, 0},
 {0, 0, 1, 0, 1, 0, 0, 0},
 {0, 0, 1, 0, 1, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0},
 {0, 0, 0, 0, 0, 0, 0, 0},
 {0, 1, 0, 0, 0, 1, 0, 0},
 {0, 0, 1, 0, 1, 0, 0, 0},
 {0, 0, 0, 1, 0, 0, 0, 0}};

void setup() {
  pinMode(L_BUTTON, INPUT_PULLUP); // LEFT BUTTON
  pinMode(R_BUTTON, INPUT_PULLUP); // RIGHT BUTTON
  pinMode(S_BUTTON, INPUT_PULLUP); // START BUTTON
  Serial.begin(115200);
  Serial.setTimeout(100);
  
  for (byte i = 0; i < 8; i++) {
    pinMode(ANODE_PINS[i], OUTPUT);
    pinMode(CATHODE_PINS[i], OUTPUT);
  }
  
  for (byte i = 0; i < 8; i++) {
    digitalWrite(ANODE_PINS[i], HIGH);
    digitalWrite(CATHODE_PINS[i], HIGH);
  }
}

// used to display on the LEDs in the matrix that are set to on
void display(byte pattern[8][8]) {
  int wait = 1000;
  for (byte i=0; i< 8; i++){
    for (byte j=0; j < 8; j++) {
      if (pattern[i][j] == ON) {
        digitalWrite(CATHODE_PINS[j], LOW);
      } else {
        digitalWrite(CATHODE_PINS[j], HIGH);
      }
    }
    digitalWrite(ANODE_PINS[i], LOW);
    delayMicroseconds(wait);
    digitalWrite(ANODE_PINS[i], HIGH);
  }
}

// turns off every LED in the matrix 
void turnOff(byte pattern[8][8]){
  for (byte i=0; i<8;i++){
    for (byte j=0; j<8; j++) {
      pattern[i][j] = OFF; 
    }    
  } 
}

// smile that is displayed when a user wins the game
void displaySmile(byte pattern[8][8]) {
  for (byte i=0; i<8;i++){
    for (byte j=0; j<8; j++) {
      if (smile[i][j] == ON) {
        pattern[i][j] = ON;
      }
    }    
  } 
}

// moves the ball down
void movePatternDown(byte pattern[8][8]) {
  for (int i = 6; i >= 1; i--) {
    for (int j = 0; j < 8; j++) {
        if (i > 1) {
          pattern[i][j] = pattern[i-1][j];
        } 
        else {
          pattern[i][j] = OFF;
        }
    }
  }
}

// moves the ball up 
void movePatternUp(byte pattern[8][8]) {
  for (int i = 1; i < 7; i++) {
    for (int j = 0; j < 8; j++) {
        if (i < 6) {
          pattern[i][j] = pattern[i+1][j];
        } else {
          pattern[i][j] = OFF;
        }
    }
  }  
}

// if the ball hits the paddle, we need the direction to switch from up to down
// if the ball hits the left side of the paddle, then the ball moves left one column 
// if the ball hits the right side of the paddle, then the ball moves right one column
// if the ball hits the middle of the paddle, then the ball stays in it's current column
void checkDirection(byte pattern[8][8]) {
  for (int i=0; i<8; i++){
    if (pattern[6][i] == ON) {
      down = 0;
      if (i == right_paddle && (i+1 < 8)) {
        pattern[6][i] = OFF;
        pattern[6][i+1] = ON;
        BALL_Y = i+1;
      } else if (i == left_paddle && (i-1 > -1)) {
        pattern[6][i] = OFF;
        pattern[6][i-1] = ON;
        BALL_Y = i-1;
      } else {
        pattern[6][i] = ON;
      }
      
    } else if (pattern[1][i] == 1) {
      down = 1;
    }
  }
}

// if the ball hits an LED, then the LED turns off 
void checkHit(byte pattern[8][8]) {
  if (pattern[1][BALL_Y] == ON) {
    pattern[0][BALL_Y] = OFF;
  }
}

// in cases where the left column is broken out of or the right column is broken out, we reset the ball to the middle to continue play
void checkReset(byte pattern[8][8]) {
  if (pattern[0][0] == OFF && pattern[6][0] == ON) {
    pattern[6][0] = OFF;
    pattern[4][3] = ON;
  }
  if (pattern[0][7] == OFF && pattern[6][7] == ON) {
    pattern[6][7] = OFF;
    pattern[4][3] = ON;
  }
}

// when all initial "breakout LEDs" are turned off, the game has been won 
void checkWin(byte pattern[8][8]) {
  for (int i = 0; i < 8; i++) {
    if (pattern[0][i] == ON && start != 1) {
      return;
    }
  }
  if (start != 1) {
    WIN = 8;
  }
}

void startBall(byte pattern[8][8]) {
  pattern[BALL_X][BALL_Y] = ON;
}


void loop(){
  static byte ledOn[8][8];

  unsigned long now = millis();
  byte S_button_state = HIGH;
  static byte S_last_button_state = HIGH;
  static long S_last_button_state_change = 0;
  static char message[50];             // buffer for sprintfs
  byte S_reading = digitalRead(S_BUTTON);

  byte L_button_state = HIGH;
  static byte L_last_button_state = HIGH;
  static long L_last_button_state_change = 0;
  byte L_reading = digitalRead(L_BUTTON);

  byte R_button_state = HIGH;
  static byte R_last_button_state = HIGH;
  static long R_last_button_state_change = 0;
  byte R_reading = digitalRead(R_BUTTON);

  // starts the game by initializing the breakout LEDs, the ball, and the paddle
  if (now - S_last_button_state_change > DEBOUNCE_DELAY) {
    if (S_reading == LOW && S_button_state == HIGH) {
      // turn paddle on 
      for (byte i=left_paddle; i < (right_paddle + 1); i++){
        ledOn[paddle_row][i] = !ledOn[paddle_row][i];
      }

      // turn game on 
    
      for (byte c=0; c<8; c++) {
          ledOn[0][c] = !ledOn[0][c];
      }

      if (start == 1) {
        startBall(ledOn);
        start = 0;
      }
    }
  }

  else if( S_reading == LOW && S_last_button_state == HIGH) {
    sprintf(message, "ignored, %ld ms since last\n", now - S_last_button_state_change);
    Serial.print(message);
  }  
  if (S_reading != S_last_button_state) {
    S_last_button_state_change = now;
  }
  S_last_button_state = S_button_state;

  // controls left button which moves the paddle to the left 
  if (now - L_last_button_state_change > DEBOUNCE_DELAY) {
    if (L_reading == LOW && L_button_state == HIGH) {
      for (byte i=left_paddle; i < (right_paddle + 1); i++){
        ledOn[paddle_row][i] = 0;
      }
      right_paddle -= 1;
      middle_paddle -= 1;
      left_paddle -= 1;
    
    if (left_paddle < 0 || right_paddle < 2) {
      left_paddle = 0;
      middle_paddle = 1;
      right_paddle = 2;
    }   
      for (byte i=left_paddle; i < (right_paddle + 1); i++){
        ledOn[paddle_row][i] = !ledOn[paddle_row][i];
      }
    }
  } 
   else if(L_reading == LOW && L_last_button_state == HIGH) {
    sprintf(message, "ignored, %ld ms since last\n", now - L_last_button_state_change);
    Serial.print(message);
  }  

  if (L_reading != L_last_button_state) {
    L_last_button_state_change = now;
  }
  L_last_button_state = L_button_state;

   // controls right button which moves the paddle to the right
  if (now - R_last_button_state_change > DEBOUNCE_DELAY) {
    if (R_reading== LOW && R_button_state == HIGH) {
      for (byte i=left_paddle; i < (right_paddle + 1); i++){
        ledOn[paddle_row][i] = 0;
      }
      right_paddle += 1;
      middle_paddle += 1;
      left_paddle += 1;
    
    if (left_paddle > 5 || right_paddle > 7) {
      left_paddle = 5;
      middle_paddle = 6;
      right_paddle = 7;
    }   
      for (byte i=left_paddle; i < (right_paddle + 1); i++){
        ledOn[paddle_row][i] = !ledOn[paddle_row][i];
      }
    }
  }
  else if(R_reading == LOW && R_last_button_state == HIGH) {
    sprintf(message, "ignored, %ld ms since last\n", now - R_last_button_state_change);
    Serial.print(message);
  } 
  if (R_reading != R_last_button_state) {
    R_last_button_state_change = now;
  }
  R_last_button_state = R_button_state;


  // the actual game
  long updateTime = millis();
  if (updateTime > TIME_DELAY + previous) {
    previous = millis();
    if (down == 1) {
      movePatternDown(ledOn);
    } else {
      movePatternUp(ledOn);
    }
  }
  checkHit(ledOn);
  checkDirection(ledOn);
  checkReset(ledOn);
  checkWin(ledOn);
  if (WIN == 8) {
    turnOff(ledOn);
    displaySmile(ledOn);
  }
  display(ledOn);
}

