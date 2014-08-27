/*************************************************** 
 * This is a sketch to interface a soil sensor & xively
 * using the Adafruit CC3000 breakout board (or WiFi shield)
 * 
 * Written by Marco Schwartz for Open Home Automation
 ****************************************************/

// Libraries
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include<stdlib.h>
#include <Wire.h>
#include "Adafruit_HTU21DF.h"

// Define CC3000 chip pins
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

// Create CC3000 instances
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
SPI_CLOCK_DIV2); // you can change this clock speed

// WLAN parameters
#define WLAN_SSID       "Drink it, eat it - guest"        // cannot be longer than 32 characters!
#define WLAN_PASS       "2066750668"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

// xively parameters
#define WEBSITE  "api.xively.com"
#define API_KEY "8hrnm4H4bkuMsGPgObA8HHYgJEhWORiQbI9SWJZO4ihrOG4V"
#define DEVICE  "1528823350"


Adafruit_HTU21DF htu = Adafruit_HTU21DF();
uint32_t ip;

void setup(void)
{
  // Initialize
  Serial.begin(115200);
  Serial.println(F("\nInitializing..."));
  htu.begin();
  
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
}

void loop(void)
{
  // Connect to WiFi network
  cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);
  Serial.println(F("Connected!"));

  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(50);
  }
  // Get the website IP & print it
  ip = 0;
  Serial.print(WEBSITE); Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000.getHostByName(WEBSITE, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }
    delay(50);
  }
//Print ip
  cc3000.printIPdotsRev(ip);
  Serial.println();
  
  // Get data & transform to integers
   float t = htu.readTemperature()*1.8+32;
   float h = htu.readHumidity();
   
  // Convert data to String
  char temp[10];
  char hum[10];
  dtostrf(t,4,2,temp);
  dtostrf(h,4,2,hum);
  String temperature = String(temp);
  String humidity = String(hum);
  Serial.println(temperature);
  Serial.println(humidity);
  
  // Prepare JSON for xively & get length
  int length = 0;
  
  String data = "{\"version\":\"1.0.0\",\"datastreams\": [ {\"id\":\"temperature\", \"current_value\":\""; 
  data += String(temperature);
  data += "\"}, {\"id\":\"humidity\",\"current_value\":\"";
  data += String(humidity);
  data += "\"} ] }" ;
   
  Serial.println();
  Serial.println(data);  
    
  length = data.length();
  Serial.print("Data length: ");
  Serial.println(length);
  Serial.println();

  // Print request for debug purposes
  
        // send the HTTP PUT request:
    Serial.print("PUT /v2/feeds/");
    Serial.print(DEVICE);
    Serial.println(" HTTP/1.1");
    Serial.println("Host: api.xively.com");
    Serial.print("X-ApiKey: ");
    Serial.println(API_KEY);
    Serial.print("User-Agent: ");
    Serial.println("Temp");
    Serial.print("Content-Length: ");
    Serial.println(length);
    Serial.println("Connection: close");
    Serial.println();
    Serial.println(data);
 
    
  // Send request
  Adafruit_CC3000_Client client = cc3000.connectTCP(ip, 80);
  if (client.connected()) {
    Serial.print("Sending headers");
    client.print("PUT /v2/feeds/");
    client.print(DEVICE);
    client.println(" HTTP/1.1");
    client.println("Host: api.xively.com");
    client.print("X-ApiKey: ");
    client.println(API_KEY);
    client.print("User-Agent: ");
    client.println("Temp");
    client.print("Content-Length: ");
    client.println(length);
    client.println("Connection: close");
    client.println();
    client.println(data);
  } 
  else {
    Serial.println(F("Connection failed"));    
    return;
  }

  Serial.println(F("-------------------------------------"));
  while (client.connected()) {
    while (client.available()) {
      char c = client.read();         
      Serial.print(c);
    }
  }
  client.close();
  Serial.println(F("-------------------------------------"));

  Serial.println(F("\n\nDisconnecting"));
  cc3000.disconnect();
    Serial.print("There!!");
  // Wait 10 seconds until next update
  delay(10000);

}

