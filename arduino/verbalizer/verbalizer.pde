/*
 *
 *  Copyright (c) 2011, BREAKFAST LLC
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of the BREAKFAST LLC nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL BREAKFAST LLC BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *  Author: Zachary Eveland, Blacklabel Development 
 *   and Mattias Gunneras, BREAKFAST
 *
 */


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
const char HEARTBEAT            = 0x88;
const char STATUS_DISCONNECT    = 0x89;

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
  
  Serial.println("\n === BOOT ===\n");
  
  // ==== SET CUSTOM NAME
  //btSetName(idString);
}

long lastReading = 0;
boolean buttonPressed = false;
char lastCommand = 0;
boolean waitingForAck = false;
boolean voiceSearchActive = false;
long lastHeartbeat = 0;
boolean isConnected = false;
boolean wasConnected = false;

long lastBatteryTestTime = 0;
long lastMicLvlTime = 0;
ChargingState chgState = notCharging;
int micLvls[] = {
  0, 0, 0, 0, 0};


// ======= SOUNDS AND MELODIES
// connected
int melConnect[] = {NOTE_D6, NOTE_D7};
int durConnect[] = {8, 8};
// activate
int melActivate[] = {NOTE_D7};
int durActivate[] = {12};
// not connected
int melNotConnected[] = {NOTE_C6, NOTE_D5};
int durNotConnected[] = {12, 12};
// ready to speak
int melReady[] = {NOTE_D7, NOTE_D7, NOTE_E7, NOTE_A7};
int durReady[] = {16, 16, 16, 10};
// Disconnect
int melDisconnect[] = {NOTE_D5, NOTE_D4};
int durDisconnect[] = {8, 8};


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
    buttonPressed = true;
    startActivation();
  }
  // Finish the activation when the user releases the touch sensor
  else if (leftSns < 100 && rightSns < 100 && buttonPressed == true) {
    buttonPressed = false;
    finishActivation();
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
    // command we're getting from the desktop app.
    switch (c) {
      
      case STATUS_READY:
        btSerial.print(ACK_OK); // let app know we got the command
        voiceSearchActive = true;
        // Blink the VU meter on
        analogWrite(vuLEDpin, 255);
        delay(200);
        analogWrite(vuLEDpin, 0);
        playNotes(melReady, durReady, sizeof(melReady) / sizeof(int));
        break;
      
      case STATUS_DONE:
        btSerial.print(ACK_OK);
        voiceSearchActive = false;
        // Turn the VU meter off
        analogWrite(vuLEDpin, 0);
        break;
      
      case STATUS_CONNECTED:
        btSerial.print(ACK_OK);
        isConnected = true;
        break;
      
      case STATUS_DISCONNECT:
        isConnected = false;
        break;
      
      case HEARTBEAT:
        lastHeartbeat = millis();
        isConnected = true;
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
  
  
  // ========= HEARTBEAT / CONNECTION ALIVE
  // Check that we've been getting heartbeats
  if (millis() - lastHeartbeat > 5000) {
    isConnected = false;
    voiceSearchActive = false;
    digitalWrite(vuLEDpin, LOW);
  }
  
  // ========= MIC LEVELS
  if (millis() - lastMicLvlTime > 120 && voiceSearchActive) {
    int micMax = 0;
    //int micMin = 1023;
    for (int i=sizeof(micLvls)-1; i; i--) {
      micLvls[i] = micLvls[i-1];
      micMax = max(micMax, micLvls[i]);
      //micMin = min(micMin, micLvls[i]);
    }
    micLvls[0] = analogRead(micLvlPin);    
    micMax = max(micMax, micLvls[0]);
    //micMin = min(micMin, micLvls[0]);
    //int diff = 434 - micMin;
    //Serial.println(micLvlPin);
    analogWrite(vuLEDpin, map(micMax, 512, 1024, 0, 200));
    lastMicLvlTime = millis();
  }
  
  
  // ========= CHECK IF CONNECTION STATUS HAS CHANGED
  if (wasConnected != isConnected) {
    if(isConnected) {
      playNotes(melConnect, durConnect, sizeof(melConnect) / sizeof(int));
    }else{
      playNotes(melDisconnect, durDisconnect, sizeof(melDisconnect) / sizeof(int));
    }
    wasConnected = isConnected;
  }
  
  
}


void startActivation () {
  if (isConnected) {
    Serial.println("activate");
    btSerial.print(CMD_ACTIVATE);
    waitingForAck = true;
    playNotes(melActivate, durActivate, sizeof(melActivate) / sizeof(int));
    delay(100);
  }else{
    Serial.println("not connected");
    analogWrite(vuLEDpin, 0);
    playNotes(melNotConnected, durNotConnected, sizeof(melNotConnected) / sizeof(int));
    delay(100);
  }
}

void finishActivation () {
  if (isConnected) {
    Serial.println("finish");
    btSerial.print(CMD_FINISHED);
    analogWrite(vuLEDpin, 0);
  }else{
    //
  }
}



// ===== PLAY NOTES THROUGH SPEAKER
void playNotes(int melody[], int noteDurations[], int numNotes) {
    //return;
    // Inspired by Tom Igoe's Tone tutorial 
    // http://arduino.cc/en/Tutorial/Tone
    
   // All LEDs should be brought down while playing sounds since 
   // together they use too much current.
   digitalWrite(vuLEDpin, LOW); 
   digitalWrite(greenLEDpin, HIGH);
   digitalWrite(redLEDpin, HIGH);
   
   // iterate over the notes of the melody:
   for (int thisNote = 0; thisNote < numNotes; thisNote++) {
    
     // to calculate the note duration, take one second 
     // divided by the note type.
     //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
     int noteDuration = 1000/noteDurations[thisNote];
     tone(spkrPin, melody[thisNote], noteDuration);
     
     // to distinguish the notes, set a minimum time between them.
     // the note's duration + 30% seems to work well:
     int pauseBetweenNotes = noteDuration * 1.30;
     delay(pauseBetweenNotes >= 0 ? pauseBetweenNotes : 1);
  
     // stop the tone playing:
     noTone(spkrPin);
   }
   // The speaker transistor needs to be brought high
   // in order to avoid damage to the speaker.
   digitalWrite(spkrPin, HIGH); 
   delay(100);
}

