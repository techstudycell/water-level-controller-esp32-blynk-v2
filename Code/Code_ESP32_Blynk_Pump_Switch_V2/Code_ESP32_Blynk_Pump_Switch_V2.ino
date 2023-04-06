/**********************************************************************************
 *  TITLE: Water Level controller using ESP32 Blynk IoT (V2) with float switch sensor
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
 *  Blynk Library (1.2.0):  https://github.com/blynkkk/blynk-library
 *  AceButton Library (1.9.2): https://github.com/bxparks/AceButton
 **********************************************************************************/

/* Fill-in your Template ID (only if using Blynk.Cloud) */
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "";   //WiFi Name
char pass[] = "";   //WiFi Password


//#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <AceButton.h>
using namespace ace_button;


// define the connected GPIO pins 
#define BuzzerPin             18   //D18
#define PumpRelay             19   //D19
#define WifiLED               23   //D23
#define ModeLED               5    //D5

#define PumpSwitchPin         32   //D32
#define ModeSwitchPin         33   //D33
#define RstButtonPin          4    //D4
#define UpTankLevelHigh       27   //D27
#define UpTankLevelLow        14   //D14
#define DownTankLevelHigh     15   //D15
#define DownTankLevelLow      13   //D13

//Change the virtual pins according the rooms
#define VPIN_BUTTON_1    V1 
#define VPIN_BUTTON_2    V2
#define VPIN_BUTTON_3    V3 
#define VPIN_BUTTON_4    V4

bool toggleState_1 = false; //Define the toggle state for relay
bool prevMode = false;
bool errorFlag = false;

int upTankLow = 0;
int upTankHigh = 0;
int downTankLow = 0;
int downTankHigh = 0;

String val;

char auth[] = BLYNK_AUTH_TOKEN;

ButtonConfig config1;
AceButton button1(&config1);
ButtonConfig config2;
AceButton button2(&config2);

void handleEvent1(AceButton*, uint8_t, uint8_t);
void handleEvent2(AceButton*, uint8_t, uint8_t);

BlynkTimer timer;

void checkBlynkStatus() { // called every 3 seconds by SimpleTimer

  bool isconnected = Blynk.connected();
  if (isconnected == false) {
    //Serial.println("Blynk Not Connected");
    digitalWrite(WifiLED, HIGH);
  }
  if (isconnected == true) {
    digitalWrite(WifiLED, LOW);
    //Serial.println("Blynk Connected");
  }
}

BLYNK_WRITE(VPIN_BUTTON_4) {
  if(!prevMode){
    toggleState_1 = param.asInt();
    if(toggleState_1){
      if(downTankHigh == 0 && downTankLow == 1 ){
        controlBuzzer(1000);
        digitalWrite(PumpRelay, LOW);  //turn on the pump
        delay(1000);
      }
    }
    else{
      digitalWrite(PumpRelay, HIGH);  //turn off the pump
      controlBuzzer(500);
      delay(500);
    }    
  }
  else{
    Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_1);
  }
}

BLYNK_CONNECTED() {
  Blynk.virtualWrite(VPIN_BUTTON_1, (upTankLow ? "UP Tank Full" : "UP Tank Empty"));
  Blynk.virtualWrite(VPIN_BUTTON_2, (downTankLow ? "Down Tank Full" : "Down Tank Empty"));
  Blynk.virtualWrite(VPIN_BUTTON_3, (prevMode ? "AUTO" : "MANUAL"));
  Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_1);
}

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
      Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_1);
    }
  }
  if(digitalRead(ModeSwitchPin) == HIGH){
    prevMode = true;
  }
  else{
    prevMode = false;
  }
  digitalWrite(ModeLED, !prevMode);
  Blynk.virtualWrite(VPIN_BUTTON_3, (prevMode ? "AUTO" : "MANUAL"));
}

void controlPump(){
  Blynk.virtualWrite(VPIN_BUTTON_1, (upTankLow ? "UP Tank Full" : "UP Tank Empty"));
  Blynk.virtualWrite(VPIN_BUTTON_2, (downTankLow ? "Down Tank Full" : "Down Tank Empty"));
  
  if(digitalRead(ModeSwitchPin) == HIGH){
    if((downTankHigh == 0) && (downTankLow == 1)){
      if((upTankLow == 0) && (upTankHigh == 1)){
        if(toggleState_1 == false){
          controlBuzzer(1000);
          digitalWrite(PumpRelay, LOW);  //turn on the pump
          toggleState_1 = true;
          Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_1);
          Serial.print(val);
          Serial.println(" Pump ON (1)");
          delay(2000);          
        }
      }
      else if((upTankLow == 1) && (upTankHigh == 0)){
        digitalWrite(PumpRelay, HIGH);  //turn off the pump
        toggleState_1 = false;
        Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_1);
        Serial.print(val);
        Serial.println(" Pump OFF (2)");
        delay(2000);
      }
    }
    else if((downTankLow == 0) && (downTankHigh == 1)){
      digitalWrite(PumpRelay, HIGH);  //turn off the pump
      toggleState_1 = false;
      Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_1);
      Serial.print(val);
      Serial.println(" Pump OFF (3)");
      delay(2000);
    } 
  }
  else{
    button1.check();
    
    if((downTankHigh == 1) && (downTankLow == 0) ){
      toggleState_1 = false;
      digitalWrite(PumpRelay, HIGH);  //turn off the pump
      Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_1);
      Serial.println(" Pump OFF (4)");
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
        Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_1);
      }
      Blynk.virtualWrite(VPIN_BUTTON_1, "Float Sensor Error");
      Blynk.virtualWrite(VPIN_BUTTON_2, "Check and Reset");
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
  pinMode(WifiLED, OUTPUT);
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
  digitalWrite(WifiLED, HIGH);
  digitalWrite(ModeLED, HIGH);
  digitalWrite(BuzzerPin, LOW);

  config1.setEventHandler(button1Handler);
  button1.init(PumpSwitchPin);

  config2.setEventHandler(button2Handler);
  button2.init(RstButtonPin);

  WiFi.begin(ssid, pass);
  timer.setInterval(2000L, checkBlynkStatus); // check if Blynk server is connected every 2 seconds
  Blynk.config(auth);
  delay(1000);

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
  Blynk.run();
  timer.run(); // Initiates SimpleTimer

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
          Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_1);
          delay(1000);
        }
        break;
      case AceButton::kEventReleased:
        toggleState_1 = false;
        digitalWrite(PumpRelay, HIGH);  //turn off the pump
        Blynk.virtualWrite(VPIN_BUTTON_4, toggleState_1);
        controlBuzzer(500);
        delay(1000);
        break;
    }
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
