#include "VerbalizerEnums.h"


ChargingState battGetChargingState() {
  int state1, state2;
  digitalWrite(chgStatDrvPin, LOW);
  delay(1);
  state1 = digitalRead(chgStatSnsPin);
  digitalWrite(chgStatDrvPin, HIGH);
  delay(1);
  state2 = digitalRead(chgStatSnsPin);
  if (state1 == 1 && state2 == 1) {      // charging finished
    return charged;    
    // TODO - typedef this and other return values
  } 
  else if (state1 == 0 && state2 == 0) {  // charging
    return charging;
  } 
  else {                                  // not charging - battery or USB not conneted
    return notCharging;
  }  
}

int battGetChargeLevel() {
  return analogRead(battLvlPin);
}
