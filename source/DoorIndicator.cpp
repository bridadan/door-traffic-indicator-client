#include "DoorIndicator.h"

const int DoorIndicator::maxSamplesAboveDangerThreshold = 60;
const int DoorIndicator::validSampleThreshold = 3;
const int DoorIndicator::waitBetweenSamplesMs = 50;
const int DoorIndicator::dangerDistanceThreshold = 150;

DoorIndicator::DoorIndicator(PinName pirPin, PinName ultrasonicPinName, PinName ledClkPin, PinName ledDataPin) 
: pirInterrupt(pirPin),
  ultrasonicPin(ultrasonicPinName, PIN_OUTPUT, PullUp, 0),
  ultrasonicPinInt(ultrasonicPinName),
  led(ledClkPin, ledDataPin, 1),
  samplesAboveDangerThreshold(0),
  validSampleCounter(0),
  sampling(false),
  curWarnStatus(SAFE),
  newWarnStatus(SAFE) {
}

void DoorIndicator::init() {
  pirInterrupt.rise(this, &DoorIndicator::pirTriggered);
  setWarnStatus(SAFE);
}

void DoorIndicator::onStateChange(mbed::util::FunctionPointer1<void, WarnStatus> fp) {
  stateChangeFp = fp;
}

void DoorIndicator::setWarnStatus(WarnStatus warnStatus) {
  newWarnStatus = warnStatus;
  curWarnStatus = warnStatus;
  
  switch (curWarnStatus) {
    case SAFE:
      printf("New Warn Status: SAFE\r\n");
      led.setColorRGB(0, 0, 0, 255);
    break;
    
    case CAUTION:
      printf("New Warn Status: CAUTION\r\n");
      led.setColorRGB(0, 255, 255, 0);
    break;
    
    case DANGER:
      printf("New Warn Status: DANGER\r\n");
      led.setColorRGB(0, 255, 0, 0);
    break;
  }
  
  stateChangeFp(warnStatus);
}

void DoorIndicator::distanceReady() {
  int curDistance = timerVal/29/2;
  printf("distance: %d\r\n", curDistance);
  
  if (curDistance > dangerDistanceThreshold) {
    samplesAboveDangerThreshold++;
    
    if (newWarnStatus != CAUTION) {
      newWarnStatus = CAUTION;
      validSampleCounter = -1;
    }
  } else {
    if (samplesAboveDangerThreshold > 0) {
      samplesAboveDangerThreshold--;
    }
    
    if (curDistance < dangerDistanceThreshold) {
      if (newWarnStatus != DANGER) {
        newWarnStatus = DANGER;
        validSampleCounter = -1;
      }
    }
  }
  
  if (validSampleCounter < validSampleThreshold) {
    validSampleCounter++;
  } else {
    if (curWarnStatus != newWarnStatus) {
      setWarnStatus(newWarnStatus);
    }
  }
  
  if (samplesAboveDangerThreshold >= maxSamplesAboveDangerThreshold) {
    minar::Scheduler::cancelCallback(distanceTaskHandle);
    distanceTaskHandle = NULL;
    
    if (newWarnStatus != SAFE) {
      setWarnStatus(SAFE);
    }
    
    printf("No one detected, going to sleep\r\n");
    sampling = false;
  }
}

void DoorIndicator::ultrasonicHigh() {
  timer.start();
}

void DoorIndicator::ultrasonicLow() {
  timer.stop();
  timerVal = timer.read_us();
  minar::Scheduler::postCallback(mbed::util::FunctionPointer0<void>(this, &DoorIndicator::distanceReady).bind());
}

void DoorIndicator::startGetDistance() {
  DigitalOut indicator(D5, 0);
  timer.reset();
  ultrasonicPinInt.rise(NULL);
  ultrasonicPinInt.fall(NULL);
  ultrasonicPin.output();
  ultrasonicPin = 0;
  wait_us(2);
  indicator = 1;
  ultrasonicPin = 1;
  wait_us(5);
  ultrasonicPin = 0;
  indicator = 0;
  ultrasonicPin.input();
  ultrasonicPinInt.rise(this, &DoorIndicator::ultrasonicHigh);
  ultrasonicPinInt.fall(this, &DoorIndicator::ultrasonicLow);
}

void DoorIndicator::pirTriggered() {
  samplesAboveDangerThreshold = 0;
  
  if (!sampling) {
    sampling = true;
    printf("pirTriggered!\r\n");  
    setWarnStatus(CAUTION);
    
    validSampleCounter = 0;
    
    distanceTaskHandle = minar::Scheduler::postCallback(mbed::util::FunctionPointer0<void>(this, &DoorIndicator::startGetDistance).bind())
      .tolerance(minar::milliseconds(10))
      .period(minar::milliseconds(waitBetweenSamplesMs))
      .getHandle();
  }
}