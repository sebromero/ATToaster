#ifndef LED_H
#define LED_H

#define MAX_BRIGHTNESS 255

class LED {	
 public: 	 
 	void update();
	void setCurrentBrightness(int brightness);
	void blink(int times, int blinkSpeed);
 	LED(int ledPin, int initialBrightness);

 private:
  	int _brightnessStepSize = 5;  
 	int _ledPin;
 	int _currentLEDBrightness = 0;
	int _pulsateDirection = 1;	
	void pulsateLED();
};

#endif
