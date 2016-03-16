/*
 * PackageLicenseDeclared: Apache-2.0
 * Copyright (c) 2016 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DOOR_INDICATOR_H
#define DOOR_INDICATOR_H

#include "mbed-drivers/mbed.h"
#include "p9813/p9813.h"
#include "minar-events/interrupt-in.h"

class DoorIndicator {
public:
  enum WarnStatus {
    SAFE,
    CAUTION,
    DANGER
  };
  
  DoorIndicator(PinName pirPin, PinName ultrasonicPin, PinName ledClkPin, PinName ledData);
  void init();
  void onStateChange(mbed::util::FunctionPointer1<void, WarnStatus> fp);
  
private:
  minar::events::InterruptIn pirInterrupt;
  DigitalInOut ultrasonicPin;
  InterruptIn ultrasonicPinInt;
  P9813 led;
  Timer timer;
  
  int timerVal;
  int samplesAboveDangerThreshold;
  int validSampleCounter;
  bool sampling;
  
  mbed::util::FunctionPointer1<void, WarnStatus> stateChangeFp;
  
  minar::callback_handle_t distanceTaskHandle;
  
  static int const maxSamplesAboveDangerThreshold;  
  static int const validSampleThreshold;
  static int const waitBetweenSamplesMs;
  static int const dangerDistanceThreshold;
  
  WarnStatus curWarnStatus;
  WarnStatus newWarnStatus;
  
  void setWarnStatus(WarnStatus warnStatus);
  void distanceReady();
  void ultrasonicHigh();
  void ultrasonicLow();
  void startGetDistance();
  void pirTriggered();
};

#endif