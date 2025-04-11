#include <Arduino.h>
#include "ZTimer.h"

ZTimer::ZTimer() {}

bool ZTimer::checkTime() { //Delay time expired function "CheckTime()"
  unsigned long currentTime = getNow();
  unsigned long remainingTime = currentTime - _lastCheckedTime;
  bool shouldFire = _enabled && remainingTime >= _waitTime;
  
  if (!shouldFire) {
    return false; // Still Waiting
  }
  
  if (_shouldRestart) _lastCheckedTime = _fireEveryTime ? (_lastCheckedTime + _waitTime) : currentTime; //get ready for the next iteration Every or (After) Has Different affects
  
  if (_callback) {
    _callback();
  }
  return true; // Timer Finished  
}

ZTimer& ZTimer::setCallBack(TimerCallback callback){
  _callback = callback;
  return *this;
}

ZTimer& ZTimer::stopTimer() {
  _remainingTime = getNow() - _lastCheckedTime;
  _enabled = false;
  return *this;
}

ZTimer& ZTimer::startTimer() {
  _lastCheckedTime = getNow() + _remainingTime;
  _enabled = true;
  return *this;
}

ZTimer& ZTimer::resetTimer(bool restart ) {
  _lastCheckedTime = getNow();
  _shouldRestart = restart;
  _enabled = true;
  return *this;
}
ZTimer& ZTimer::setWaitTime(unsigned long Time){
  _waitTime = Time;     // Change the timers delay time
  return *this; 
}

ZTimer& ZTimer::setLastTime(unsigned long Time){
  _lastCheckedTime  = Time;    // Change the timers last time 
  return *this; 
}

ZTimer& ZTimer::pause() {
  _shouldRestart = false ;    // Pauses Timer at end of timing WARNING CheckTimer() returns true at end of timing event and wont reset to false
  return *this; 
}
ZTimer& ZTimer::enableFireEverytime()  {
  _fireEveryTime = true ;   // By adding waitMillis to lastMillis timing will always shift by (waitMillis) 
  return *this; 
}
ZTimer& ZTimer::enableFireAfter()  {
  _fireEveryTime = false;   // By shifting to millis() the next time interval will always be (waitMillis) long
  return *this; 
}
ZTimer& ZTimer::useMicros() {
  _useMicroSeconds = true ;    // Pauses Timer at end of timing WARNING CheckTimer() returns true at end of timing event and wont reset to false
  _lastCheckedTime = getNow();
  return *this; 
}
ZTimer& ZTimer::useMillis() {
  _useMicroSeconds = false ;    // Pauses Timer at end of timing WARNING CheckTimer() returns true at end of timing event and wont reset to false
  _lastCheckedTime = getNow();
  return *this; 
}

unsigned long ZTimer::getNow() {
  return _useMicroSeconds ? micros() : millis();
}
unsigned long ZTimer::getWaitTime(){ 
  return _waitTime;            // Access the timers delay time
}
unsigned long ZTimer::getLastTime(){
  return _lastCheckedTime;           // Access the timers last time 
}
