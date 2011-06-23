#include <NewSoftSerial.h>
#include <CapSense.h>
#include "pitches.h"
#include "VerbalizerEnums.h"

// Relace XX with your ID number  here
// Or give your board a custom name
String idString = "Verbalizer XX";

/* digital pins */
int pairingSwPin = 2;
int chgStatDrvPin = 3;
int chgStatSnsPin = 4;
int btRxPin = 5;
int btTxPin = 6;
int d8Pin = 8;
int d9Pin = 9;
int spkrPin = 10;
int vuLEDpin = 11;
int greenLEDpin = 12;
int redLEDpin = 13;

/* analog pins */
int a0Pin = 0;
int a1Pin = 1;
int battLvlPin = 2;
int micLvlPin = 7;

NewSoftSerial btSerial(btRxPin, btTxPin);
CapSense capSnsLeft = CapSense(18, 17);
CapSense capSnsRight = CapSense(18, 19);

// COMMANDS
const char CMD_ACTIVATE		= 0x80;
const char CMD_FINISHED		= 0x81;
const char STATUS_ACTIVATED	= 0x82;
const char STATUS_READY		= 0x83;
const char STATUS_DONE		= 0x84;
const char STATUS_CONNECTED	= 0x85;
const char ACK_OK		= 0x86;
const char ACK_BAD		= 0x87;

void setup() {
  digitalWrite(vuLEDpin, LOW);
  digitalWrite(greenLEDpin, HIGH);
  digitalWrite(redLEDpin, HIGH);
  digitalWrite(d8Pin, HIGH); // pull ups
  digitalWrite(d9Pin, HIGH); // pull ups
  digitalWrite(pairingSwPin, HIGH);
  digitalWrite(spkrPin, HIGH);
  digitalWrite(chgStatSnsPin, LOW);
  digitalWrite(chgStatDrvPin, LOW);

  pinMode(vuLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);
  pinMode(redLEDpin, OUTPUT);
  pinMode(d8Pin, INPUT);
  pinMode(d9Pin, INPUT);
  pinMode(pairingSwPin, INPUT);
  pinMode(spkrPin, OUTPUT);
  pinMode(chgStatSnsPin, INPUT);
  pinMode(chgStatDrvPin, OUTPUT);

  Serial.begin(9600);  
  btSerial.begin(9600);
  
  Serial.println("ready");
  
  // ==== SET CUSTOM NAME
  //btSetName(idString);
}

long lastReading = 0;
boolean buttonPressed = false;
char lastCommand = 0;
boolean waitingForAck = false;
boolean voiceSearchActive = false;

long lastBatteryTestTime = 0;
long lastMicLvlTime = 0;
ChargingState chgState = notCharging;
int micLvls[] = {
  0, 0, 0, 0, 0};

// notes for first trigger
int melody[] = {NOTE_D7, NOTE_D7, NOTE_E7, NOTE_A7};
// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {12, 12, 12, 8};


void loop() {
  
  // ======== BLUETOOTH PAIRING
  // Make the Bluetooth device pairable if
  // the back button is pressed.
  if (digitalRead(pairingSwPin) == LOW) {
    Serial.println("making discoverable");
    btSerial.println("at+btp");
    delay(50);
  }

  // ======== CAPACITIVE TOUCH EXAMPLE
  // Get the current value from capacitive sensors
  long leftSns = capSnsLeft.capSense(30);
  long rightSns = capSnsRight.capSense(30);
  // Activate voice search
  // When user starts touching the sensor
  if ((leftSns > 210 || rightSns > 210) && buttonPressed == false) {
    analogWrite(vuLEDpin, 255); // turn on light
    btSerial.print(CMD_ACTIVATE);
    buttonPressed = true;
    waitingForAck = true;
    Serial.println("activate");
    playNotes(melody, noteDurations, sizeof(melody) / sizeof(int));
  }
  // Finish the activation when the user releases the touch sensor
  else if (leftSns < 100 && rightSns < 100 && buttonPressed == true) {
    btSerial.print(CMD_FINISHED);
    buttonPressed = false;
    waitingForAck = true;
    Serial.println("finished");
    // take down light
    analogWrite(vuLEDpin, 0);
  }

  
  /*
  // ======== FLEX SENSOR EXAMPLE
  // Read the A0 sensor
  int flex = analogRead(A0);
  //Serial.println(flex);
  // When user starts bending
  if (flex > 850 && buttonPressed == false) {
    analogWrite(vuLEDpin, 255); // turn on light
    btSerial.print(CMD_ACTIVATE);
    buttonPressed = true;
    waitingForAck = true;
  }
  // Finish the activation when the user bends back flex
  else if (flex < 800 && buttonPressed == true) {
    btSerial.print(CMD_FINISHED);
    buttonPressed = false;
    waitingForAck = true;
    // take down light
    analogWrite(vuLEDpin, 0);
  }
  */
  
  /*  
  // ======== IR EXAMPLE
  // Read the A0 sensor
  // we get values from around 900 down to 20. 
  // map re-maps these values to a range from 0 to 100.
  int ir = map(analogRead(A0), 900, 20, 0, 100);
  //Serial.println(ir);
  // When user puts finger on sensor
  if (ir > 80 && buttonPressed == false) {
    analogWrite(vuLEDpin, 255); // turn on light
    btSerial.print(CMD_ACTIVATE);
    buttonPressed = true;
    waitingForAck = true;
    Serial.println("activated");
  }
  // Finish the activation when the user removes finger
  else if (ir < 70 && buttonPressed == true) {
    btSerial.print(CMD_FINISHED);
    buttonPressed = false;
    waitingForAck = true;
    // take down light
    analogWrite(vuLEDpin, 0);
    Serial.println("finished activation");
  }
  */  
  
  /*
  // ======== PUSH BUTTON EXAMPLE
  // We use a NC button
  // Read the D8 sensor
  // Attach it between D8 and GND
  // it's either HIGH or LOW
  int val = digitalRead(d8Pin);
   // Finish the activation when the user removes finger
  if (val == LOW && buttonPressed == true) {
    btSerial.print(CMD_ACTIVATE);
    buttonPressed = false;
    waitingForAck = true;
    Serial.println("activated");
  }
  else if (val == HIGH && buttonPressed == false) {
    btSerial.print(CMD_FINISHED);
    buttonPressed = true;
    waitingForAck = true;
    Serial.println("finished activation");
  }
  */
  
  
  // ======== INCOMING SERIAL DATA
  // Look for serial data from Bluetooth chip
  if (btSerial.available() > 0) {
    
    // Read the first char
    char c = (char)btSerial.read();
    
    // Print the incoming data from the Bluetooth module.
    Serial.print(c);

    // Getting acknowledgement from desktop app
    // We're not really doing anything here other than
    // resetting the ack flag.
    if (waitingForAck == true) {
      if (c == ACK_OK) {
        waitingForAck = false;
      }
      else if (c == ACK_BAD) {
        waitingForAck = false;
      }
    }
    
    // Execute commands depending on what 
    // command we're getting from the
    // desktop app.
    switch (c) {
    
      case STATUS_READY:
        btSerial.print(ACK_OK); // let app know we got the command
        
        // Blink the VU meter on
        analogWrite(vuLEDpin, 255);
        delay(200);
        analogWrite(vuLEDpin, 0);
        voiceSearchActive = true;
        break;
      
      case STATUS_DONE:
        btSerial.print(ACK_OK);
        
        // Turn the VU meter off
        analogWrite(vuLEDpin, 0);
        voiceSearchActive = false;
        break;
       
        case STATUS_CONNECTED:
          btSerial.print(ACK_OK);
        break;
        
        case 0x57:
          Serial.print("GOT w");
        break;
                
      default:
          
      break;
    }
  }


  
  // ====== BATTERY LEVEL
  // monitor battery charge progress
  if (millis() - lastBatteryTestTime > 1000) {
    chgState = battGetChargingState();
    int chgLevel = battGetChargeLevel();

    if (chgState == charging || chgState == charged) {
      digitalWrite(greenLEDpin, LOW);
    } 
    else {
      digitalWrite(greenLEDpin, HIGH);
    }
    lastBatteryTestTime = millis();
  }
  if (chgState == charging && millis() - lastBatteryTestTime > 100) {
    digitalWrite(greenLEDpin, HIGH);
  }


  /*
  // ========= MIC LEVELS
  if (millis() - lastMicLvlTime > 10) {
    int micMax = 0;
    int micMin = 1023;
    for (int i=sizeof(micLvls)-1; i; i--) {
      micLvls[i] = micLvls[i-1];
      micMax = max(micMax, micLvls[i]);
      micMin = min(micMin, micLvls[i]);
    }
    micLvls[0] = analogRead(micLvlPin);    
    micMax = max(micMax, micLvls[0]);
    micMin = min(micMin, micLvls[0]);
    int diff = 434 - micMin;    
    analogWrite(vuLEDpin, map(micMax, 512, 1024, 0, 255));    
  }
  */
  
}



// ===== PLAY NOTES THROUGH SPEAKER
void playNotes(int melody[], int noteDurations[], int numNotes) {
  
    // Inspired by Tom Igoe's Tone tutorial 
    // http://arduino.cc/en/Tutorial/Tone
   
   // iterate over the notes of the melody:
   for (int thisNote = 0; thisNote < numNotes; thisNote++) {
    
     // to calculate the note duration, take one second 
     // divided by the note type.
     //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
     int noteDuration = 1000/noteDurations[thisNote];
     tone(spkrPin, melody[thisNote],noteDuration);
     
     // to distinguish the notes, set a minimum time between them.
     // the note's duration + 30% seems to work well:
     int pauseBetweenNotes = noteDuration * 1.30;
     delay(pauseBetweenNotes >= 0 ? pauseBetweenNotes : 1);
  
     // stop the tone playing:
     noTone(spkrPin);

   }   
}

