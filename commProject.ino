
#include <SoftwareSerial.h>
#include <Adafruit_Soundboard.h>

/////////////////??////////GENERATED CODE///////////////////////////
// Choose any two pins that can be used with SoftwareSerial to RX & TX
#define SFX_TX 11
#define SFX_RX 10

// Connect to the RST pin on the Sound Board
#define SFX_RST 4

// We'll be using software serial
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);

// pass the software serial to Adafruit_soundboard, the second
// argument is the debug port (not used really) and the third 
// arg is the reset pin
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RST);
/////////////////////////END GENERATED CODE/////////////////////////

/* 
PINS USED:
2   - Breadboard Push Button
4   - SFX_RST
5   - PIR
6   - Push to talk button
8   - Dial
10  - SFX_RX  
11  - SFX_TX
12  - Remote pin
*/

//CONSTANTS DEFINED FOR PIN LOCATION
const int PUSH_BUTTON_PIN = 2;
const int PIR_PIN = 5;
const int PUSH_TO_TALK_PIN = 6;
const int DIAL_PIN = 8;
const int PHONE_HOOK_PIN = 12;
const int REMOTE_PIN = 12;

//OTHER CONSTANTS
const long REMOTE_DELAY = 7200000; //Remote delays all processing for 2 hours

  //MIN_PIR_DELAY to MAX_PIR_DELAY is the ranges of random time between PIR checks
const long MIN_PIR_DELAY = 1000;   //in testing, 5-10s 
const long MAX_PIR_DELAY = 2000;
/*
const int MIN_PIR_DELAY = 90000;  //live, 90-900s
const int MAX_PIR_DELAY = 900000;
*/

//Variables for the reading current status
int buttonState = 0;
int pushToTalkState = 0;
int pirState = LOW;
int dialState = 0;
int phoneHookState = 0;
int remoteState = 0;

//timerValue - creates a random amount of time between PIR sensor checks
long timerValue = 0;


void setup() {
  //Declare INPUT_PULLUP sensors
  pinMode(PUSH_BUTTON_PIN, INPUT_PULLUP);  
  pinMode(PUSH_TO_TALK_PIN, INPUT_PULLUP);
  pinMode(DIAL_PIN, INPUT_PULLUP);
  pinMode(PHONE_HOOK_PIN, INPUT_PULLUP);  
  pinMode(REMOTE_PIN, INPUT_PULLUP);
  
  //Declare INPUT sensors
  pinMode(PIR_PIN, INPUT);

/////////////This is all from the original sound FX code////////////////////////
  Serial.begin(115200);
  Serial.println("Adafruit Sound Board!");
// softwareserial at 9600 baud
  ss.begin(9600);
// can also do Serial1.begin(9600)
  if (!sfx.reset()) {
    Serial.println("Not found");
    while (1);
  }
  Serial.println("SFX board found");
}

/////////////STATES///////////////
//State null = forever loop
//State 0 = phone on hook
//State 1 = phone lifted
//State 2 = button triggered
//State 3 = dial spun

//BEGIN LOOPING
void loop() {

  //generate random timer    
    timerValue = random(MIN_PIR_DELAY, MAX_PIR_DELAY);
       
    long startTime = millis();

  do {
    //read the state of the varius input values
    phoneHookState = digitalRead(PHONE_HOOK_PIN);
    remoteState = digitalRead(REMOTE_PIN);
    
    if(phoneHookState == HIGH){
      
      //if phone is lifted
      state1();

      /*
    } else if (remoteState == HIGH){     
      
      //If remote is used, delay all processing for 2 hours
      delay(REMOTE_DELAY);
      */
    }
      
    } while ((millis() - startTime) < timerValue);


    //if random timer reached, go to state0
    state0;
}

//triggers either a ringtone or soundclip after a specified delay
void triggerSound(int delayLength, bool ringtone){

  //delay a specified amount of time
  delay(delayLength);

  //if ringtone sound
  if (ringtone){

    //Get a random ring tone
    long randRingTone = random(5,9);

    //Play random ringtone
    sfx.playTrack((uint8_t)randRingTone); //THIS LINE PLAYS from the "randRingTone"

  } else {
    digitalWrite(13,HIGH);
    
    //Get a random soundclip
    //long randVader = random(5);

    //Play random soundclip
    //sfx.playTrack((uint8_t)randVader); //THIS LINE PLAYS from the "randVader" file selection
  }
  digitalWrite(13,LOW);
}

bool checkPIR(){

    //Check the pir's current state
    pirState = digitalRead(PIR_PIN);

    //return PIR status of high or not
    return (pirState == HIGH);      

}

//Phone on receiver
void state0(){

  //grab the current time value
  unsigned long startTime = millis();
  
  do {

    //wait 0.5s between detection checks
    delay(500);

    if (checkPIR){
      //if detection, play ringtone
      triggerSound(0,true);
    
    //End the cycle of checking since something was detected
    startTime = millis() + 10000;
    }   
    
  //check if PIR to detects something for 10s
  } while ((millis()-startTime) < 10000);  

  //if no detection, drop to state null (forever loop)
}

//Phone lifted, nothing pressed
void state1(){

  //trigger sound w/ delay
  triggerSound(2000, false);  

  //continue will cause a loop until the phone is placed back on the receiver
  bool continueLoop = true;

  //loop forever for next phone input
  while(continueLoop){    

    //read the state of the various input values
    pushToTalkState = digitalRead(PUSH_TO_TALK_PIN);
    phoneHookState = digitalRead(PHONE_HOOK_PIN);
    dialState = digitalRead(DIAL_PIN);
        
     if (phoneHookState == HIGH){
     //on phone placed back on holder, reset to null state
        continueLoop = false;
    
    } else if (pushToTalkState == HIGH){
    //on push to talk button use, go to state 2
    state2();
    
    } else if (dialState == HIGH){
    //on dial used, go to state 3
    state3();
  }
  }
}

//Push to talk button triggered
void state2(){
   
  //trigger sound w/o delay
  triggerSound(250, false);
  
  //drop back to state 1
}

//Dial spun
void state3(){
  
  //Wait until dial no longer active
  while (dialState == HIGH){
  //Check dial status while it is not moving every .1s
  delay(100);
  dialState = digitalRead(DIAL_PIN);
  }  
  
  //trigger sound w/ delay
  triggerSound(250, false);
  
  //drop back to state 1
}

//Remote used
void state4(){

  //Prep an infinite loop
  bool continueLoop = true;
  
  //Delay a second to allow user to lift hand off remote button
  delay(1000);
  
  //Loop until remote is used again
  while (continueLoop){
    
    //Check remote state every .1s
    delay(100);
    remoteState = digitalRead(REMOTE_PIN);
    
    //If remote was used, exit loop
    if (remoteState == HIGH){
      continueLoop = false;
    }
  }
  
  //Drop back to null state
}


