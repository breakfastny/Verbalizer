#include <NewSoftSerial.h>

String idString = "Verbalizer 22";

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

NewSoftSerial btSerial(btRxPin, btTxPin);

void setup() {
  digitalWrite(vuLEDpin, LOW);
  digitalWrite(greenLEDpin, HIGH);
  digitalWrite(redLEDpin, HIGH);
  digitalWrite(d8Pin, HIGH);
  digitalWrite(d9Pin, HIGH);
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
}

void loop() {
  // HW serial - SW serial patch-thru
  if (btSerial.available()) {
    Serial.print((char)btSerial.read());
  }
  if (Serial.available()) {
    char c = (char)Serial.read();
    //    btSerial.print((char)Serial.read());
    if (c == '!') {
      Serial.print("beginning configuration of BTM511 as ");
      Serial.println(idString);
      btConfig(idString);
      Serial.println("completed configuration of BTM511");    
    } 
    else {
      btSerial.print(c);
    }
  }  
}

/* NOTE: configureBluetooth() should only be called once,
 * during initial assembly and testing.  All settings made here
 * are stored in non-volatile memory and do not need to be re-entered.
 * See other methods for userspace applications.
 * Most of these commands do not take effect until the next reset so, when
 * finished, we save all settings to non-volatile memory and restart the 
 * module.  A complete refernce for the commands and registers used can 
 * be found in the BTM511 datasheet available from Laird Technologies.
 */
void btConfig(String permanentName) {  
  // perform factory reset:  
  btSerial.println("at&f*");
  delay(1000);

  // enable handsfree HFP + serial (SPP) profiles:
  btSerial.println("ats102=$0011");
  delay(100);

  // configure the module to startup (and remain in) a  connectable state:
  btSerial.println("ats512=3");  // default to connectable (but not discoverable)
  delay(100);
  btSerial.println("ats554=0");  // never time out of this state
  delay(100);

  // specify that the device has no input/output capabilities (used to determine
  // how to handle Bluetooth pairing)
  btSerial.println("ats321=3");
  delay(100);  
  
  // when the host attempts to open the serial port, auto-answer after one ring
  btSerial.println("ats0=1");
  delay(100);  
  
  // enable the module's microphone pre-amp:
  btSerial.println("ats415=1");
  delay(100);

  // set the device's permanent friendly name
  btSerial.print("at+btn=\"");
  btSerial.print(permanentName);
  btSerial.println("\"");
  delay(100);

  // write the new settings to non-volatile memory:
  btSerial.println("at&w");
  delay(1000);

  // restart the module to have new settings take effect:
  btSerial.println("atz");
  delay(1000);
}

