#ifndef LED_H
#define LED_H

#define MAX_BRIGHTNESS 255

class LED {	
 public:
 	static const int BrightnessStepSize = 5; 	
 	static const int steps = MAX_BRIGHTNESS / BrightnessStepSize; 	
 	void update();
  void setCurrentBrightness(int brightness);
  void blink(int times, int blinkSpeed);
 	LED(int ledPin, int initialBrightness);

 private:
 	int _ledPin;
 	int _currentLEDBrightness = 0;
	int _pulsateDirection = 1;	
	void pulsateLED();
};

#endif
