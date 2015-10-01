#include "../config.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const int buttonPin = 2;           // number of the button pin
const int ledPin = 0;              // number of the LED pin
int buttonState = 0;               // button state starts at 0
int brightness = 0;                // brightness of LED starts at 0
int fadeAmount = 25.50;            // fadeAmount is equal to 10% of 255(max LED brightness)
int brighness = 0;            // fadeAmount is equal to 10% of 255(max LED brightness)
int buttonPressedCounter = 0;    // timer set to 0 whe depressing button
int buttonCounter = 0;             // button counter set to 0
int buttonLastState = 0;           // button last state set to 0


MDNSResponder mdns;
ESP8266WebServer server(80);


String webString="";     // String to display
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;             // interval at which to read sensor

bool ICACHE_FLASH_ATTR gettemperature()
{
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval)
  {
    // save the last time you read the sensor 
    previousMillis = currentMillis;
	}
}


void ICACHE_FLASH_ATTR handleNotFound()
{
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET)?"GET":"POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";
	for (uint8_t i=0; i<server.args(); i++)
  {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}
	server.send(404, "text/plain", message);
}

void ICACHE_FLASH_ATTR setup(void){
	Serial.begin(9600);
	WiFi.begin(ssid, password);
	Serial.println("");
	Serial.println("Wifi temperature sensor v0.1");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED)
  {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	if (mdns.begin(hostname, WiFi.localIP()))
  {
		Serial.println("MDNS responder started");
	}

	server.on("/", []()
  {
    if (gettemperature())
    {       // read sensor
      webString = String(hostname) + " reports:\n";
      webString+="Brightness: "+String(0)+" \n";
      server.send(200, "text/plain", webString);            // send to someones browser when asked
    }
    else 
    {
      webString="{\"error\": \"Cannot read data from sensor.\"";
      server.send(503, "text/plain", webString);            // send to someones browser when asked
    }
  });

	server.onNotFound(handleNotFound);

	server.begin();
	ESP.wdtEnable(5000);

  // set button as input
  pinMode(buttonPin, INPUT);
  // set LED as output
  pinMode(ledPin, OUTPUT);
}

void ICACHE_FLASH_ATTR loop(void){
	server.handleClient();
	ESP.wdtFeed();

  buttonState = digitalRead(buttonPin);
  
  // activate when button is being held down
  if (buttonState == HIGH)
  {
    buttonPressedCounter ++; // start counter
    Serial.print("Pressed Timer: ");
    Serial.println(buttonPressedCounter);
    // if counter equals 100 turn off LED and reset counter to 0
    if (buttonPressedCounter == 50)
    {
      buttonCounter = 0;
      analogWrite(ledPin, 0); // reset LED 
    }
  } 
  else 
  {
    // on button depress, set counter to 0
    buttonPressedCounter = 0; 
  }
  
  // activate once to increase button counter, which adjusts the LED brightness
  if (buttonState != buttonLastState)
  {
    if (buttonState == HIGH)
    {
      // increase counter only if less than 10
      if (buttonCounter < 10)
      {
        buttonCounter ++;
      }
      Serial.print("LED Level: ");
      Serial.println(buttonCounter);
      brightness = buttonCounter * fadeAmount;
      analogWrite(ledPin, brightness); // adjust LED brightness to buttoncounter * fadeamount
    }   
    buttonLastState = buttonState;
  }
}

