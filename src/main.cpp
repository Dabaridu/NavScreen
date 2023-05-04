//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1

//libraries are here 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "PinChangeInterrupt.h"
#include "A4988.h"

//constants are here 
#define LCDcolums 2
#define LCDrows 16

//rotary encoder
#define CLK 2 //2
#define DT 3 //3
#define SW 9

//Variables are here 
int counter = 0; //rotary encoder
int currentState; //rotary encoder
int initState; //rotary encoder

unsigned long debounceDelay = 10; //rotary encoder button
//defining menu size and parameters of the device 

int simpleinterval;

bool StartFlow; //variable to control the flow of the start of the program
int Navojidolzina;
int Navojisirina;
int precnipomikstopinj;
int vrtljaj;
int pomikdomov;

bool Clicked = false;

//Classes are here
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
//leave clear

// using a 200-step motor (most common)
#define MOTOR_STEPS 200
// configure the pins connected
#define DIR 4
#define STEP 7
#define DIR2 10
#define STEP2 5
// #define MS1 10
// #define MS2 11
// #define MS3 12
A4988 rotacijski(MOTOR_STEPS, DIR, STEP);
A4988 precni(MOTOR_STEPS, DIR2, STEP2);

class Screen {
  private:
    String Label;
    float increment;
  
  public:
    float Value;
    Screen(String Label, float Value, float increment){
      this -> Label = Label;
      this -> Value = Value;
      this -> increment = increment;
    }
  
  void draw(){
    lcd.setCursor(1,0);
    lcd.print(Label);
    lcd.setCursor(1,1);
    lcd.print(Value);
  }

  void add(){
    Value = Value + increment;
  }

  void sub(){
    Value = Value - increment;
  }
};

//making it public so that all functions can see it 
  Screen *screens[] = {
    new Screen("Start  ", 0, 1),
    new Screen("Dolzina", 15, 1),
    new Screen("Sirina ", 3, 1),
    new Screen("Pomik  ", 90, 1),
    new Screen("Vrtl   ", 90, 1),
  };
int menusize = 4; //0<->4


void button_press(){
  int buttonVal = digitalRead(SW);
  //If we detect LOW signal, button is pressed
  if (buttonVal == LOW) {
    if (millis() - debounceDelay > 200) {
      Clicked = !Clicked;
      Serial.print("Button pressed! ");
      Serial.println(Clicked);
    }
    debounceDelay = millis();
  }
}


void encoder_value() {
  // Read the current state of CLK
  currentState = digitalRead(CLK);
  // If last and current state of CLK are different, then we can be sure that the pulse occurred
  if (currentState != initState  && currentState == 1) {
    // Encoder is rotating counterclockwise so we decrement the counter
    if (digitalRead(DT) != currentState) {
      if (Clicked == true){
        if (counter > 0){
          counter --;
        }
      }
      else {
        screens[counter]->sub();
      }
    } else {
      if (Clicked == true){
        if (counter < menusize){
          counter ++;
        }
      }
      else{
        screens[counter]->add();
      }
    }
    // print the value in the serial monitor window
    Serial.print("Counter: ");
    Serial.println(counter);
  }
  // Remember last CLK state for next cycle
  initState = currentState;
}


void setup()
{
  lcd.init();  // initialize the lcd 
  lcd.backlight(); //set backlight value

  //rotary encoder setup
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(SW, INPUT_PULLUP);
  // Setup Serial Monitor
  Serial.begin(9600);
  // Read the initial state of CLK
  initState = digitalRead(CLK);
  // Call encoder_value() when any high/low changed seen
  // on interrupt 0 (pin 2), or interrupt 1 (pin 3)
  attachInterrupt(0, encoder_value, CHANGE);
  attachInterrupt(1, encoder_value, CHANGE);
  attachPCINT(digitalPinToPCINT(SW), button_press, CHANGE);

  // Set target motor RPM to 1RPM and microstepping to 1 (full step mode)
  rotacijski.begin(60, 4);
  precni.begin(60, 4);
}

void select(){
  if (Clicked == true){
    lcd.setCursor(0,0);
    lcd.print(">");
    lcd.setCursor(0,1);
    lcd.print(" ");
  } else {
    lcd.setCursor(0,0);
    lcd.print(" ");
    lcd.setCursor(0,1);
    lcd.print(">");
  }
}

void exenavijanje(){
  precnipomikstopinj = screens[3]->Value;
  vrtljaj = screens[4]->Value;
  //90° = cel obrat motorja 
  for (Navojisirina = screens[2]->Value; Navojisirina > 0; Navojisirina -= 1){
    for (Navojidolzina = screens[1]->Value; Navojidolzina > 0; Navojidolzina -= 1){
      //stepper.rotate(360); //rotacijski 
      precni.rotate(precnipomikstopinj); //prečni
      rotacijski.rotate(vrtljaj); 
      pomikdomov += precnipomikstopinj; 
    }
    precnipomikstopinj = ~(precnipomikstopinj-1); //eniški kompliment, -1 --> nasprotna vrednost (pomik prečne osi v drugo smer kot prej)
  }
  screens[0]->Value = 0;
  precni.rotate(~(pomikdomov-1));
}

void loop(){
//    screens[counter].draw();
  if (millis() - simpleinterval > 1500){
    lcd.clear();
  }
simpleinterval = millis();
screens[counter]->draw();
select();

  if((screens[0]->Value)>0){ //if start is more than 0
      exenavijanje();
    }
  

}

void manualcontroll(){
  
}