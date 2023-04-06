/**********************************************************************************
 *  TITLE: Water Level controller using ESP32 (V2) with float switch sensor
 *  Click on the following links to learn more. 
 *  YouTube Video: https://youtu.be/-FC8jpl5hDI
 *  Related Blog : https://iotcircuithub.com/esp32-projects/
 *  by Tech StudyCell
 *  
 *  This code is provided free for project purpose and fair use only.
 *  Please do mail us to techstudycell@gmail.com if you want to use it commercially.
 *  Copyrighted © by Tech StudyCell
 *  
 *  Preferences--> Aditional boards Manager URLs : 
 *  https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
 *  
 *  Download Board ESP32 (2.0.7) : https://github.com/espressif/arduino-esp32
 *
 *  Download the libraries 
 *  AceButton Library (1.9.2): https://github.com/bxparks/AceButton
 **********************************************************************************/


#include <AceButton.h>
using namespace ace_button;


// define the connected GPIO pins 
#define BuzzerPin             18   //D4
#define PumpRelay             19   //D5
#define IndicatrLED           23   //D6
#define ModeLED               5  //D13

#define PumpSwitchPin         32   //D7
#define ModeSwitchPin         33   //D8
#define RstButtonPin          4   //D8
#define UpTankLevelHigh       27   //D9
#define UpTankLevelLow        14  //D10
#define DownTankLevelHigh     15  //D11
#define DownTankLevelLow      13  //D12


bool toggleState_1 = false; //Define the toggle state for relay
bool prevMode = false;
bool errorFlag = false;

int upTankLow = 0;
int upTankHigh = 0;
int downTankLow = 0;
int downTankHigh = 0;

String val;

ButtonConfig config1;
AceButton button1(&config1);
ButtonConfig config2;
AceButton button2(&config2);

void handleEvent1(AceButton*, uint8_t, uint8_t);
void handleEvent2(AceButton*, uint8_t, uint8_t);

void controlBuzzer(int duration){
  digitalWrite(BuzzerPin, HIGH);
  delay(duration);
  digitalWrite(BuzzerPin, LOW);
}

void checkMode(){
  if (digitalRead(ModeSwitchPin)!= prevMode){
    controlBuzzer(500);
    if(digitalRead(ModeSwitchPin) == LOW){
      digitalWrite(PumpRelay, HIGH);
      toggleState_1 = false;
    }
  }
  if(digitalRead(ModeSwitchPin) == HIGH){
    prevMode = true;
  }
  else{
    prevMode = false;
  }
  digitalWrite(ModeLED, !prevMode);
}

void controlPump(){
  if(digitalRead(ModeSwitchPin) == HIGH){
    if((downTankHigh == 0) && (downTankLow == 1)){
      if((upTankLow == 0) && (upTankHigh == 1)){
        if(toggleState_1 == false){
          controlBuzzer(1000);
          digitalWrite(PumpRelay, LOW);  //turn on the pump
          toggleState_1 = true;
          digitalWrite(IndicatrLED, !toggleState_1);
          Serial.print(val);
          Serial.println(" inside if");
          delay(8000);          
        }
      }
      else if((upTankLow == 1) && (upTankHigh == 0)){
        digitalWrite(PumpRelay, HIGH);  //turn off the pump
        toggleState_1 = false;
        digitalWrite(IndicatrLED, !toggleState_1);
        Serial.print(val);
        Serial.println(" inside else");
        delay(8000);
      }
    }
    else if((downTankLow == 0) && (downTankHigh == 1)){
      digitalWrite(PumpRelay, HIGH);  //turn off the pump
      toggleState_1 = false;
      digitalWrite(IndicatrLED, !toggleState_1);
      Serial.print(val);
      Serial.println(" inside else 2");
      delay(8000);
    } 
  }
  else{
    button1.check();
    
    if((downTankHigh == 1) && (downTankLow == 0) ){
      toggleState_1 = false;
      digitalWrite(PumpRelay, HIGH);  //turn off the pump
      digitalWrite(IndicatrLED, !toggleState_1);
      Serial.println(" inside if 2");
    }
  }
}

void errorCheck(){
  if( (upTankLow + upTankHigh + downTankLow + downTankHigh) > 2 ){
    delay(2000);
    upTankLow = digitalRead(UpTankLevelLow);
    upTankHigh = digitalRead(UpTankLevelHigh);
    downTankLow = digitalRead(DownTankLevelLow);
    downTankHigh = digitalRead(DownTankLevelHigh);
    if( (upTankLow + upTankHigh + downTankLow + downTankHigh) > 2 ){
      errorFlag = true;
      if(toggleState_1 == true){
        toggleState_1 = false;
        digitalWrite(PumpRelay, HIGH);  //turn off the pump
        digitalWrite(IndicatrLED, !toggleState_1);      
      }
      controlBuzzer(2000);
      Serial.print(val);
      Serial.println(" Float Sensor Error. ");
    }     
  }
  else{
    if((downTankLow == 0) && (downTankHigh == 1)){
      if(toggleState_1 == true){
        digitalWrite(BuzzerPin, HIGH);
        Serial.print(val);
        Serial.println(" Pump ON while DN_TANK Empty");
      }
    }
    else if((upTankLow == 1) && (upTankHigh == 0)){
      if(toggleState_1 == true){
        Serial.print(val);
        Serial.println("Pump ON while UP_TANK Full");
        controlBuzzer(2000);
        delay(500);
      }    
    }
    else{
      digitalWrite(BuzzerPin, LOW);
      //Serial.println(val);
    }
  }    
}

void setup()
{
  Serial.begin(115200);

  pinMode(PumpRelay, OUTPUT);
  pinMode(IndicatrLED, OUTPUT);
  pinMode(BuzzerPin, OUTPUT);
  pinMode(ModeLED, OUTPUT);

  pinMode(PumpSwitchPin, INPUT_PULLUP);
  pinMode(ModeSwitchPin, INPUT_PULLUP);
  pinMode(RstButtonPin, INPUT_PULLUP);
  
  pinMode(UpTankLevelHigh, INPUT_PULLUP);
  pinMode(DownTankLevelHigh, INPUT_PULLUP);
  pinMode(UpTankLevelLow, INPUT_PULLUP);
  pinMode(DownTankLevelLow, INPUT_PULLUP);


  //During Starting Relay & LEDs should TURN OFF
  digitalWrite(PumpRelay, HIGH); // used active LOW relay
  digitalWrite(IndicatrLED, HIGH);
  digitalWrite(ModeLED, HIGH);
  digitalWrite(BuzzerPin, LOW);

  config1.setEventHandler(button1Handler);
  button1.init(PumpSwitchPin);

  config2.setEventHandler(button2Handler);
  button2.init(RstButtonPin);

  delay(2000);
  controlBuzzer(500);
  delay(500);
}

void loop()
{ 
  upTankLow = digitalRead(UpTankLevelLow);
  upTankHigh = digitalRead(UpTankLevelHigh);
  downTankLow = digitalRead(DownTankLevelLow);
  downTankHigh = digitalRead(DownTankLevelHigh);

  val = String(upTankLow) + String(upTankHigh) + String(downTankLow) + String(downTankHigh);

  checkMode();
  
  if(!errorFlag){
    controlPump();
  }
  else{
    button2.check();
  }
  
  errorCheck(); 
}

void button1Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT1");
  if(digitalRead(ModeSwitchPin) == LOW){
    switch (eventType) {
      case AceButton::kEventPressed:
        if(downTankHigh == 0 && downTankLow == 1 ){
          controlBuzzer(1000);
          toggleState_1 = true;
          digitalWrite(PumpRelay, LOW);  //turn on the pump
          delay(1000);
        }
        break;
      case AceButton::kEventReleased:
        toggleState_1 = false;
        digitalWrite(PumpRelay, HIGH);  //turn off the pump
        controlBuzzer(500);
        delay(1000);
        break;
    }
    digitalWrite(IndicatrLED, !toggleState_1);
  }  
}

void button2Handler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  Serial.println("EVENT2");
  switch (eventType) {
    case AceButton::kEventReleased:
      errorFlag = false;
      controlBuzzer(500);
      delay(100);
      break;
  }  
}
