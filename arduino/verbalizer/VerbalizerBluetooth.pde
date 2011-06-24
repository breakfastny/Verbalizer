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
 *
 */

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
