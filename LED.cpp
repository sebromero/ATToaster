#include "LED.h"
#include <Arduino.h>

LED::LED(int ledPin, int initialBrightness) : _ledPin(ledPin) {
	pinMode(_ledPin, OUTPUT);	
  setCurrentBrightness(initialBrightness);
}

void LED::update() {
	pulsateLED();  
}

void LED::setCurrentBrightness(int brightness) {
  _currentLEDBrightness = brightness;
  _pulsateDirection = _currentLEDBrightness == MAX_BRIGHTNESS ? -1 : 1;
}

void LED::blink(int times, int blinkSpeed){  
  for(int i = 0; i < times; ++i){
    digitalWrite(_ledPin, HIGH);
    delay(blinkSpeed);
    digitalWrite(_ledPin, LOW);
    delay(blinkSpeed);    
  }
}

void LED::pulsateLED(){	  
  _currentLEDBrightness = _currentLEDBrightness + (BrightnessStepSize * _pulsateDirection);  
  analogWrite(_ledPin, _currentLEDBrightness);
  if(_currentLEDBrightness + BrightnessStepSize > MAX_BRIGHTNESS || _currentLEDBrightness - BrightnessStepSize < 0) _pulsateDirection *= -1;
}
