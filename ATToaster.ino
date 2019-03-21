#include <EEPROM.h>
#include "ZTimer.h"
#include "LED.h"

#define LED_PIN 0
#define RELAY_PIN 3
#define BUTTON_PIN 1

#define LONG_PRESS_THRESHOLD 2000
#define DEBOUNCE_DELAY 50 // the debounce time; increase if the output flickers
#define LED_PULSE_TIME 6000
#define LED_PULSE_STEPS 255
#define INITIAL_PULSE_BRIGHTNESS 255
#define EEPROM_BASE_ADDRESS 0
#define UNWRITTEN_EEPROM_LOCATION_VALUE 0xFFFF
#define RELAY_DEFAULT_ACTIVATION_TIME 30 * 1000

uint8_t debouncedButtonState;
uint8_t lastButtonState = HIGH; //Default is HIGH with pull-up resistor
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled  
unsigned long lastRelayActivationTime = 0;
unsigned long relayActivationDuration = 0;
unsigned long lastButtonPressedTime = 0;
bool buttonIsPressed = false;
bool relayIsOn = false;
bool isInAdjustmentMode = false;

ZTimer ledTimer;
LED led = LED(LED_PIN, INITIAL_PULSE_BRIGHTNESS);

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  relayActivationDuration = getRelayActivationDurationFromMemory();
  
  ledTimer.SetCallBack([&]() {
    if(isInAdjustmentMode){      
      led.update();
    }    
  });
  ledTimer.SetWaitTime(LED_PULSE_TIME / LED_PULSE_STEPS / 2);
  ledTimer.ResetTimer(true);
}

void loop() {
  ledTimer.CheckTime();
  int currentButtonState = digitalRead(BUTTON_PIN);

  // If the switch changed, due to noise or pressing: reset the debouncing timer
  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  performButtonActions(currentButtonState);
  turnOffRelayIfNecessary();
  lastButtonState = currentButtonState;
  delay(10); //Let the CPU take a break for a bit
}

void performButtonActions(int currentButtonState){
  //If the signal has been stable for 'debounceDelay'
  bool signalIsStable = (millis() - lastDebounceTime) > DEBOUNCE_DELAY;
  if (!signalIsStable) return;
  
  bool buttonStateHasChanged = (currentButtonState != debouncedButtonState);
  debouncedButtonState = currentButtonState;
  buttonIsPressed = debouncedButtonState == LOW;
  
  if (buttonStateHasChanged) {    
    if(buttonIsPressed){
      relayIsOn = !relayIsOn;
      lastButtonPressedTime = millis();        
      
      if(relayIsOn) { 
        lastRelayActivationTime = millis(); 
      }
  
      digitalWrite(LED_PIN, relayIsOn);
      digitalWrite(RELAY_PIN, relayIsOn);     
    } else if(isInAdjustmentMode) {
      isInAdjustmentMode = false;
      digitalWrite(RELAY_PIN, LOW);
      relayIsOn = false;      
      relayActivationDuration =  millis() - lastButtonPressedTime;
      saveRelayActivationDurationToMemory(relayActivationDuration);           
      led.blink(3, 100);
      led.setCurrentBrightness(INITIAL_PULSE_BRIGHTNESS);
    }
    
  } else if(relayIsOn && buttonIsPressed && (millis() - lastButtonPressedTime) > LONG_PRESS_THRESHOLD) {              
      isInAdjustmentMode = true;      
  }
}

unsigned long getRelayActivationDurationFromMemory(){
  int seconds = 0;
  EEPROM.get(EEPROM_BASE_ADDRESS, seconds);  
  unsigned long milliSeconds = seconds != UNWRITTEN_EEPROM_LOCATION_VALUE ? seconds * 1000 : RELAY_DEFAULT_ACTIVATION_TIME;
  return milliSeconds;
}

void saveRelayActivationDurationToMemory(unsigned long milliseconds){
  int seconds = milliseconds / 1000;
  EEPROM.put(EEPROM_BASE_ADDRESS, seconds);
}

void turnOffRelayIfNecessary(){
  //Turn off relay after 'relayActivationDuration' if it was active
  if(!isInAdjustmentMode && relayIsOn && (millis() - lastRelayActivationTime) > relayActivationDuration) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);  
    relayIsOn = false;
  }
}
