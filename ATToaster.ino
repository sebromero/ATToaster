/*
This Arduino sketch implements the functionality for controlling a toaster using a relay, an LED, and a button. Here's a summary of the user functionality:

Relay Activation (Toasting Time):
- The relay controls the toaster's heating element. When activated, it powers the toaster for a specific duration (toasting time).
- The toasting time is stored in EEPROM memory, allowing it to persist even after the device is powered off. If no valid time is stored, a default duration is used.

Button Behavior:
- Short Press: Activates the relay, starting the toasting process. The relay remains on for the configured toasting time.
- Long Press: Enters an adjustment mode where the user can set a new toasting time. The duration is saved to EEPROM once adjustment is complete.

LED Indicators:
- Steady ON: Indicates that the relay is active, and the toaster is currently toasting.
- Pulsating: Indicates that the device is in adjustment mode, allowing the user to set the toasting time.

Relay and LED Logic:
- The relay automatically turns off after the configured toasting time has elapsed.
- The LED turns off when the relay is deactivated.
- If the device is in adjustment mode, the relay does not activate, and the LED pulsates to signal the mode.

This sketch ensures that the toaster operates safely and conveniently, with user-friendly feedback through the LED and persistent storage of the toasting duration.
*/

/**
 * On ATTiny85 it works best with ATTinyCore (no bootloader) and 8 MHz (internal)
 * http://drazzy.com/package_drazzy.com_index.json
 * Burn the bootloader to set the fuses correctly
 * When using USBTinyISP use SLOW mode
 */

#include <EEPROM.h>
#include "ZTimer.h"
#include "LED.h"

//PIN Configuration
#define LED_PIN 0
#define RELAY_PIN 3
#define BUTTON_PIN 1

//Logic Configuration
#define LONG_PRESS_THRESHOLD 2000 //Time after which a button press is considered long-press
#define DEBOUNCE_DELAY 50 // the debounce time; increase if the output flickers
#define LED_PULSE_TIME 6000 // Time in ms for the LED to complete a full pulse cycle
#define LED_PULSE_STEPS 255 // Number of steps in the LED pulse cycle
#define INITIAL_PULSE_BRIGHTNESS 255 // Initial brightness for the LED pulse
#define EEPROM_BASE_ADDRESS 0 // Base address for EEPROM storage
#define UNWRITTEN_EEPROM_LOCATION_VALUE 0xFFFF // Value indicating an unwritten EEPROM location
#define RELAY_DEFAULT_ACTIVATION_TIME 30 * 1000 //Default activation time in seconds
#define CPU_BREAK_DURATION 10 //CPU break duration in ms
#define BUTTON_PRESSED_STATE HIGH  //Default is HIGH with pull-up resistor, LOW with a pull down for normal push buttons (opposite for momentary push buttons) 
#define BLINK_COUNT 3 // How many times to blink the LED to confirm the new relay activation time
#define BLINK_DURATION 100 //Blink duration in ms

//Global variables
uint8_t debouncedButtonState; // The debounced state of the button. HIGH or LOW
uint8_t lastButtonState = BUTTON_PRESSED_STATE == HIGH ? LOW : HIGH; // Previous state of the button for debouncing
unsigned long lastDebounceTime = 0;  //The last time the output pin was toggled  
unsigned long lastRelayActivationTime = 0; // The last time the relay was activated
unsigned long relayActivationDuration = 0; // The duration for which the relay should be activated
unsigned long lastButtonPressedTime = 0; // The last time the button was pressed (for debouncing)
bool buttonIsPressed = false;
bool relayIsOn = false;
bool isInAdjustmentMode = false;

ZTimer ledTimer;
LED led = LED(LED_PIN, INITIAL_PULSE_BRIGHTNESS);

/**
 * Handles the LED timer event. If the device is in adjustment mode, it updates the LED state.
 * This applies the brightness changes to the LED in a pulsating manner.
 */
void handleLedTimerEvent() {
  if(isInAdjustmentMode){      
    led.update();
  }
}

void setup() {  
  ADCSRA = 0; // disable ADC as we don't need it and can save a bit of energy 
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  relayActivationDuration = getRelayActivationDurationFromMemory();
  
  ledTimer.setCallBack(handleLedTimerEvent);
  ledTimer.setWaitTime(LED_PULSE_TIME / LED_PULSE_STEPS / 2);
  ledTimer.resetTimer(true);
}

void loop() {
  ledTimer.checkTime();
  int currentButtonState = digitalRead(BUTTON_PIN);

  // If the switch changed, due to noise or pressing: reset the debouncing timer
  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
  }
  
  performButtonActions(currentButtonState);
  turnOffRelayIfNecessary();
  lastButtonState = currentButtonState;
  delay(CPU_BREAK_DURATION); //Let the CPU take a break for a bit
}

/**
 * Handles button actions based on the current button state.
 * The actions are as follows:
 * - If the button is pressed, toggle the relay state.
 * - If the button is pressed for a long time, enter adjustment mode.
 * - If the button is released and the device is in adjustment mode, exit adjustment mode and save the current relay activation duration to EEPROM.
 * @param currentButtonState The current state of the button (pressed or not pressed).
 */
void performButtonActions(int currentButtonState){
  bool signalIsStable = (millis() - lastDebounceTime) > DEBOUNCE_DELAY;
  if (!signalIsStable) return;
  
  bool buttonStateHasChanged = (currentButtonState != debouncedButtonState);
  debouncedButtonState = currentButtonState;
  buttonIsPressed = debouncedButtonState == BUTTON_PRESSED_STATE;
  unsigned long timeSinceButtonPressed = millis() - lastButtonPressedTime;
  
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
      relayActivationDuration =  timeSinceButtonPressed;
      saveRelayActivationDurationToMemory(relayActivationDuration);           
      led.blink(BLINK_COUNT, BLINK_DURATION);
      led.setCurrentBrightness(INITIAL_PULSE_BRIGHTNESS);
    }
    
  } else if(relayIsOn && buttonIsPressed && timeSinceButtonPressed > LONG_PRESS_THRESHOLD) {              
      isInAdjustmentMode = true;      
  }
}

/**
 * Retrieves the relay activation duration from EEPROM memory.
 * @return The duration in milliseconds. If no valid value is found, returns the default duration.
 */
unsigned long getRelayActivationDurationFromMemory(){
  int seconds = 0;
  EEPROM.get(EEPROM_BASE_ADDRESS, seconds);  
  unsigned long milliSeconds = seconds != UNWRITTEN_EEPROM_LOCATION_VALUE ? seconds * 1000 : RELAY_DEFAULT_ACTIVATION_TIME;
  return milliSeconds;
}

/**
 * Saves the relay activation duration to the EEPROM
 * @param milliseconds The duration in milliseconds to save
 */
void saveRelayActivationDurationToMemory(unsigned long milliseconds){
  int seconds = milliseconds / 1000;
  EEPROM.put(EEPROM_BASE_ADDRESS, seconds);
}

/**
 * Turns off the relay if it has been on for longer than the specified duration
 * and the device is not in adjustment mode.
 */
void turnOffRelayIfNecessary(){
  if(!isInAdjustmentMode && relayIsOn && (millis() - lastRelayActivationTime) > relayActivationDuration) {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);  
    relayIsOn = false;
  }
}
