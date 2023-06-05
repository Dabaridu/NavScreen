//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1

//libraries are here 
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "PinChangeInterrupt.h"
#include "A4988.h"
//#include "EEPROM.h"

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

int DrawType;

int menusize = -1; // starts counting from 0-X

// bool Value;
// bool Label;

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
  
  public:
    float Value;
    float increment;
    Screen(String Label, float Value, float increment){
      this -> Label = Label;
      this -> Value = Value;
      this -> increment = increment;
      menusize ++;
    }
  
  void drawLabel(){
    lcd.setCursor(1,0);
    lcd.print(Label);
  }
  void drawValue(){
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
    new Screen("Pomik  ", 10, 1), 
    new Screen("Vrtl   ", 90, 1), //90 = 360°
    new Screen("Premik ",0,10),
    new Screen("Zavrti ",0,10),
  };
//int menusize = 6; //0<->4


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

void selectpointer(){
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

void exenavijanje(){ //program za navijanje tuljave (ne spreminjaj po nepotrebi)
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

void screendraw(bool Lab, bool Val){ //draw all basic stuff for menu GUI

  if (millis() - simpleinterval > 1500){// vedno na prvem mestu ker logični razlogi
    lcd.clear();
    simpleinterval = millis();
  }

  if (Val == true){
    screens[counter]->drawValue();
  }
  if (Lab == true){
    screens[counter]->drawLabel();
  }

  selectpointer();
}

void manualcontrollPomikanje(){
  int premik;
  int prejpemik;
  while(Clicked == false){
    screendraw(true, true);
    premik = screens[counter]->Value;
    if (premik > prejpemik){
      precni.rotate(screens[counter]->increment);
    }
    else if(premik < prejpemik){
      precni.rotate(-(screens[counter]->increment));
    }
    prejpemik = premik;
  }
}

void manualcontrollVrtenje(){
  int premik;
  int prejpemik;
  while(Clicked == false){
    screendraw(true, true);
    premik = screens[counter]->Value;
    if (premik > prejpemik){
      rotacijski.rotate(screens[counter]->increment);
    }
    else if(premik < prejpemik){
      rotacijski.rotate(-(screens[counter]->increment));
    }
    prejpemik = premik;
  }
}

void start(){
  int premik;
  int prejpemik;
  bool Alfred; 

  int rem = screens[counter]->Value; //zapomni si kje je vstopil

  screendraw(true, false);

  while(Clicked == false){ //clicked je "interupt"... to zna mogoče delat
        premik = screens[counter]->Value;
    if (premik > prejpemik){
      //pogoj če zavrtimo v desno
      Alfred = true;
      lcd.setCursor(1,1);
      lcd.print("TRUE  ");
    }
    else if(premik < prejpemik){
      //pogoj če zavrtimo v levo
      Alfred = false; 
      lcd.setCursor(1,1);
      lcd.print("FALSE ");
    }
    prejpemik = premik;
  }
  if (Alfred == true){
    lcd.setCursor(1,1);
    lcd.print("Navijam");
    exenavijanje();
  }

  screens[counter]->Value = rem; //iztopi na mestu kjer je vstopil
}

void loop(){
//    screens[counter].draw();
  screendraw(true, true);

//special conditions for screens 
  if (Clicked == false){
    switch(counter){//preveri v katerem meniju se trenutno nahajamo
      case 0: 
      start();
      break;
      case 2:
      break;
      case 5:
      manualcontrollPomikanje();
      break;
      case 6:
      manualcontrollVrtenje();
      break;
    }
  }

  // //--------------------------Enter manual move mode------------------------------
  // if((counter == 5)&&(Clicked == false)){
  //     manualcontrollPomikanje();
  // }
  // if((counter == 6)&&(Clicked == false)){
  //     manualcontrollVrtenje();
  // }
}