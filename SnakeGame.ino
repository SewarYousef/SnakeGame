#include <LedControl.h>
#include <LiquidCrystal.h>

// Initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 6, 5, 4, 3);

// const int userInput = 0;
// const int selectedGame = 1;

#define userSetup 0
#define startGame 1
#define userInput 2


// Menu options
const char* options[] = { "Bounded", "Unbounded" };
int optionIndex = 0;
const int numOptions = sizeof(options) / sizeof(options[0]);


//Define The Snake as a Struct
typedef struct Snake Snake;
struct Snake {
  int head[2];      // the (row, column) of the snake head
  int body[40][2];  //An array that contains the (row, column) coordinates
  int len;          //The length of the snake
  int dir[2];       //A direction to move the snake along
};

//Define The Apple as a Struct
typedef struct Apple Apple;
struct Apple {
  int rPos;  //The row index of the apple
  int cPos;  //The column index of the apple
};


//MAX72XX led Matrix
const int DIN = 12;
const int CS = 11;
const int CLK = 10;
LedControl lc = LedControl(DIN, CLK, CS, 1);


// Joystick
const int varXPin = A3;  //X Value  from Joystick
const int varYPin = A4;  //Y Value from Joystick
const int SW = 2;        //Switch Pushbutton from Joystick

// Variables to store joystick values
int xValue = 0;
int yValue = 0;
int buttonState = 1;

byte pic[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };  //The 8 rows of the LED 8x8 Matrix

Snake snake = { { 1, 5 }, { { 0, 5 }, { 1, 5 } }, 2, { 1, 0 } };  //Initialize a snake object
Apple apple = { (int)random(0, 8), (int)random(0, 8) };           //Initialize an apple object in a random place in the game

//Variables To Handle The Game Time
float oldTime = 0;
float timer = 0;
float updateRate = 3;

// Buzzer
const int buzz = 9;

int i, j;  //Counters
void setup() {
  /*
  The MAX72XX is in power-saving mode on startup,
  we have to do a wakeup call
  */
  Serial.begin(9600);
  lc.shutdown(0, false);
  // Set the brightness to a medium values
  lc.setIntensity(0, 8);
  // Clear the display
  lc.clearDisplay(0);

  //Set Joystick Pins as INPUTs
  pinMode(varXPin, INPUT);
  pinMode(varYPin, INPUT);

  lcd.begin(16, 2);           // LCD's columns and rows
  pinMode(SW, INPUT_PULLUP);  // Use internal pull-up resistor

  pinMode(buzz, OUTPUT);      // Buzzer as output
}

int state = 0;
bool isBounded = false;

void loop() {
  // put your main code here, to run repeatedly:

  xValue = analogRead(varXPin);
  yValue = analogRead(varYPin);
  buttonState = digitalRead(SW);

  switch (state) {
    case userSetup:  //User Setup

      lcd.setCursor(0, 0);
      lcd.print("Select an option:");
      updateDisplay();
      state = userInput;
      break;
    case userInput:
      if (yValue < 400) {  // Joystick moved up
        optionIndex = 0;   // Select "Bounded"
        updateDisplay();
        delay(200);  // Debounce delay
      }
      if (yValue > 600) {  // Joystick moved down
        optionIndex = 1;   // Select "Unbounded"
        updateDisplay();
        delay(200);  // Debounce delay
      }
      if (buttonState == LOW) {  // Joystick button pressed
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Selected:");
        lcd.setCursor(0, 1);
        lcd.print(options[optionIndex]);
        if (options[optionIndex] == "Bounded") {
          isBounded = true;
        } else if (options[optionIndex] == "Unbounded") {
          isBounded = false;
        }
        state = startGame;
        break;

        case startGame:  //Game started

          if (isBounded) {
            bounded();
          } else {
            unbounded();
          }
          break;
      }
  }
}

void bounded() {
  float deltaTime = calculateDeltaTime();
  timer += deltaTime;

  //Check For Inputs
  int xVal = analogRead(varXPin);
  int yVal = analogRead(varYPin);


  if (xVal < 100 && snake.dir[1] == 0) {
    snake.dir[0] = 0;
    snake.dir[1] = -1;
    //Left(Movement)
  } else if (xVal > 920 && snake.dir[1] == 0) {
    snake.dir[0] = 0;
    snake.dir[1] = 1;
    //Right Direction
  } else if (yVal < 100 && snake.dir[0] == 0) {
    snake.dir[0] = -1;
    snake.dir[1] = 0;
  }
  //Up Direction
  else if (yVal < 100 && snake.dir[0] == 0) {
    snake.dir[0] = -1;
    snake.dir[1] = 0;
    //Downard Direction

  } else if (yVal > 920 && snake.dir[0] == 0) {
    snake.dir[0] = 1;
    snake.dir[1] = 0;
  }

  //Update
  if (timer > 1000 / updateRate) {
    timer = 0;
    boundedUpdate();
  }

  //Render
  Render();
}

void unbounded() {
  float deltaTime = calculateDeltaTime();
  timer += deltaTime;

  //Check For Inputs
  int xVal = analogRead(varXPin);
  int yVal = analogRead(varYPin);

  if (xVal < 100 && snake.dir[1] == 0) {
    snake.dir[0] = 0;
    snake.dir[1] = -1;
    //Left(Movement)
  } else if (xVal > 920 && snake.dir[1] == 0) {
    snake.dir[0] = 0;
    snake.dir[1] = 1;
    //Right Direction
  } else if (yVal < 100 && snake.dir[0] == 0) {
    snake.dir[0] = -1;
    snake.dir[1] = 0;
  }
  //Up Direction
  else if (yVal < 100 && snake.dir[0] == 0) {
    snake.dir[0] = -1;
    snake.dir[1] = 0;
    //Downard Direction

  } else if (yVal > 920 && snake.dir[0] == 0) {
    snake.dir[0] = 1;
    snake.dir[1] = 0;
  }

  //Update
  if (timer > 1000 / updateRate) {
    timer = 0;
    unBoundedUpdate();
  }

  //Render
  Render();
}

float calculateDeltaTime() {
  float currentTime = millis();
  float dt = currentTime - oldTime;
  oldTime = currentTime;
  return dt;
}

void reset() {
  for (int j = 0; j < 8; j++) {
    pic[j] = 0;
  }
}

void displayX() {
  //Displayx X sign if the snake clashes
  byte xPattern[8] = {
    0b10000001,
    0b01000010,
    0b00100100,
    0b00011000,
    0b00100100,
    0b01000010,
    0b10000001
  };

  for (int i = 0; i < 8; i++) {
    lc.setRow(0, i, xPattern[i]);
  }
  lcd.clear();
  lcd.print("Game Over!");
  buzzerr();  
  delay(2000);  // Show selection for a second
  lcd.clear();
  state = 0;
  
}



void Render() {

  for (i = 0; i < 8; i++) {
    lc.setRow(0, i, pic[i]);
  }
}

void removeFirst() {
  for (j = 1; j < snake.len; j++) {
    snake.body[j - 1][0] = snake.body[j][0];
    snake.body[j - 1][1] = snake.body[j][1];
  }
}

// This is for the 16x2 LCD to update the screen
void updateDisplay() {
  lcd.setCursor(0, 1);
  lcd.print("                ");  // Clear previous option
  lcd.setCursor(0, 1);
  lcd.print(options[optionIndex]);
}

void unBoundedUpdate() {
  reset();  //Reset (Clear) the 8x8 LED matrix

  int newHead[2] = { snake.head[0] + snake.dir[0], snake.head[1] + snake.dir[1] };

  //Handle Borders
  if (newHead[0] == 8) {
    newHead[0] = 0;
  } else if (newHead[0] == -1) {
    newHead[0] = 7;
  }
  if (newHead[1] == 8) {
    newHead[1] = 0;
  } else if (newHead[1] == -1) {
    newHead[1] = 7;
  }

  //Check If The Snake hits itself
  for (j = 0; j < snake.len; j++) {
    if (snake.body[j][0] == newHead[0] && snake.body[j][1] == newHead[1]) {
      //Pause the game for 1 sec then Reset it
      displayX();
      // buzzerr();
      snake = { { 1, 5 }, { { 0, 5 }, { 1, 5 } }, 2, { 1, 0 } };  //Reinitialize the snake object
      apple = { (int)random(0, 8), (int)random(0, 8) };           //Reinitialize an apple object
      return;
    }
  }

  //Check if The snake ate the apple
  if (newHead[0] == apple.rPos && newHead[1] == apple.cPos) {
    snake.len = snake.len + 1;
    apple.rPos = (int)random(0, 8);
    apple.cPos = (int)random(0, 8);
  } else {
    removeFirst();  //Shifting the array to the left
  }

  snake.body[snake.len - 1][0] = newHead[0];
  snake.body[snake.len - 1][1] = newHead[1];

  snake.head[0] = newHead[0];
  snake.head[1] = newHead[1];

  //Update the pic Array to Display(snake and apple)
  for (j = 0; j < snake.len; j++) {
    pic[snake.body[j][0]] |= 128 >> snake.body[j][1];
  }
  pic[apple.rPos] |= 128 >> apple.cPos;
}

void boundedUpdate() {
  reset();  //Reset (Clear) the 8x8 LED matrix

  int newHead[2] = { snake.head[0] + snake.dir[0], snake.head[1] + snake.dir[1] };

  // Handle Borders if Bounded
  if (newHead[0] == 8 || newHead[0] == -1 || newHead[1] == 8 || newHead[1] == -1) {
    //Pause the game for 1 sec then Reset it
    displayX();
    // buzzerr();
    snake = { { 1, 5 }, { { 0, 5 }, { 1, 5 } }, 2, { 1, 0 } };  //Reinitialize the snake object
    apple = { (int)random(0, 8), (int)random(0, 8) };           //Reinitialize an apple object
    return;
  }

  //Check If The Snake hits itself
  for (j = 0; j < snake.len; j++) {
    if (snake.body[j][0] == newHead[0] && snake.body[j][1] == newHead[1]) {
      //Pause the game for 1 sec then Reset it
      displayX();
      // buzzerr();
      snake = { { 1, 5 }, { { 0, 5 }, { 1, 5 } }, 2, { 1, 0 } };  //Reinitialize the snake object
      apple = { (int)random(0, 8), (int)random(0, 8) };           //Reinitialize an apple object
      return;
    }
  }

  //Check if The snake ate the apple
  if (newHead[0] == apple.rPos && newHead[1] == apple.cPos) {
    snake.len = snake.len + 1;
    apple.rPos = (int)random(0, 8);
    apple.cPos = (int)random(0, 8);
  } else {
    removeFirst();  //Shifting the array to the left
  }

  snake.body[snake.len - 1][0] = newHead[0];
  snake.body[snake.len - 1][1] = newHead[1];

  snake.head[0] = newHead[0];
  snake.head[1] = newHead[1];

  //Update the pic Array to Display(snake and apple)
  for (j = 0; j < snake.len; j++) {
    pic[snake.body[j][0]] |= 128 >> snake.body[j][1];
  }
  pic[apple.rPos] |= 128 >> apple.cPos;
}

void buzzerr() {
  digitalWrite(buzz, HIGH);
  delay(500);
  digitalWrite(buzz, LOW);
}