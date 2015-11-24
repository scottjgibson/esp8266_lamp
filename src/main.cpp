#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <Button.h> 

//needed for library
#include <EEPROM.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#define BUTTON_PIN 0
#define LED_PIN 2

#define PULLUP true
#define INVERT true
#define DEBOUNCE_MS 150
Button myBtn(BUTTON_PIN, PULLUP, INVERT, DEBOUNCE_MS);    //Declare the button

enum {OFF, FADE_UP, HOLD, PULSE};       
uint8_t STATE;                   //The current state machine state
int brightness = 0;
int lastHoldLevel = 0;

void setup() {
  Serial.begin(9600);
  WiFiManager wifi;
  wifi.autoConnect("Light");
  brightness = 0;
  STATE = OFF;
  Serial.println("State: OFF");
  pinMode(LED_PIN, OUTPUT);
}

#define FADE_UP_RATE 200


const uint8_t gammaTbl[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };


void loop()
{
  uint32_t timePressed;

  myBtn.read();
  analogWrite(LED_PIN, gammaTbl[brightness]);

  switch (STATE)
  {
    case OFF:                
      brightness = 0;
      lastHoldLevel = 0;
      if (myBtn.pressedFor(100))
      {
        STATE = FADE_UP;
      }
      break;
    case FADE_UP:                
      if (myBtn.isPressed())
      {
        timePressed = millis() - myBtn.lastChange();
        brightness = map(timePressed, 0, 3000, lastHoldLevel, 255);
        brightness = constrain(brightness, 0, 255);
        if (brightness == 255)
        {
          analogWrite(LED_PIN, gammaTbl[100]);
          delay(100);
          analogWrite(LED_PIN, gammaTbl[brightness]);
          STATE = HOLD;
        }
      }
      else if (myBtn.wasReleased())
      {
        STATE = HOLD;
      }
      break;
    case HOLD:                
      lastHoldLevel = brightness;
      if (myBtn.wasPressed())
      {
        for (int i = brightness; i > 0; i--)
        {
          analogWrite(LED_PIN, gammaTbl[i]);
          delay(3);
        }
        STATE = OFF;
      }
      break;
    default:
      STATE = OFF;
      break;

  }
}

