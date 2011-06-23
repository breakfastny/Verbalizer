

void btVolumeUp() {
  btSerial.println("at+gou");
  delay(100);
}

void btVolumeDown() {
  btSerial.println("at+god");
  delay(100);  
}

void btMicGainUp() {
  btSerial.println("at+giu");
  delay(100);  
}

void btMicGainDown() {
  btSerial.println("at+gid");
  delay(100);  
}

boolean btIsConnected() {
  // not yet implemented
  
  // TODO - Zach - implement 
}

void btSetName(String newName) {
  btSerial.print("at+btf=\"");
  btSerial.print(newName);
  btSerial.println("\"");
  delay(100);  
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
