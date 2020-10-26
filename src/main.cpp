#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#include <Wire.h>
#include <Time.h>
#include <WiFiUdp.h>
#include <NTPClient.h>


boolean ledValues[110];
boolean leo = false;
boolean kai = false;
int oldminute = 0;

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define PIN_1 14
#define PIN_2 12
#define PIN_3 13
#define PIN_4 15
#define PIN_WORDS 5

//set LED arrays
#define NUM_LEDS_PART_A 1     //1
#define NUM_LEDS_PART_B 1     //2
#define NUM_LEDS_PART_C 1     //3
#define NUM_LEDS_PART_D 1     //4
#define NUM_LEDS_PART_E 110   //words

CRGB led1[NUM_LEDS_PART_A];
CRGB led2[NUM_LEDS_PART_B];
CRGB led3[NUM_LEDS_PART_C];
CRGB led4[NUM_LEDS_PART_D];
CRGB leds[NUM_LEDS_PART_E];

//include passwords
#include "credentials.h"
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_SERVER_IP;
const char* user= MQTT_USER;
const char* passw= MQTT_PASSWORD;

//Wifi
WiFiClient wordclock;
PubSubClient client(wordclock);
String receivedString;
int MQTT_brightness = 255;
int old_brightness = 255;

//ntp 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String formattedDate;
String dayStamp;
String timeStamp;

int timeSec = 0;
int timeMin = 0;
int timeHour = 0;
int timeDay = 0;
int timeMonth = 0;

//-------------------------------------functions----------------------------------//

//callback for receiving MQTT
void callback(char* topic, byte* payload, unsigned int length) {

 if (strcmp(topic,"home/livingroom/wordclock/brightness")==0){
 
 for (int i=0;i<length;i++) {
  receivedString += (char)payload[i];
 }
  
  MQTT_brightness = receivedString.toInt();
  receivedString = "";
 }
}

//reconnect for mqtt-broker
void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 if (client.connect("wordclock", user, passw)) {
  Serial.println("connected");

  //subscribe
  client.subscribe("home/livingroom/wordclock/brightness");
  
 } else {
  Serial.print("failed, rc=");
  Serial.print(client.state());
  Serial.println(" try again in 5 seconds");
  // Wait 5 seconds before retrying
  delay(5000);
  }
 }
}


//---set time---//
void setTime(){

  oldminute = timeMin;

  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }

  timeSec = timeClient.getSeconds();
  timeMin = timeClient.getMinutes();
  timeHour = timeClient.getHours();

  time_t rawtime = timeClient.getEpochTime();
  struct tm * ti;
  ti = localtime (&rawtime);

  timeMonth = (ti->tm_mon + 1) < 10 ? 0 + (ti->tm_mon + 1) : (ti->tm_mon + 1);
  timeDay = (ti->tm_mday) < 10 ? 0 + (ti->tm_mday) : (ti->tm_mday);
  
  //change to 12h-hourFormat
  if (timeHour > 12){
    timeHour = timeHour - 12;
  }

}


//---determine LED status-----//
boolean setLED(int i) {

  //reset old status
  for (int i = 0; i < 110; i++) {
    ledValues[i] = false;
  }


  //-----structural elements-----//

  //UHR
  if ((timeMin < 5))
  {
    ledValues[107] = true;
    ledValues[108] = true;
    ledValues[109] = true;
  }

  //VOR
  if ((timeMin > 24) && (timeMin < 30) || (timeMin > 49))
  {
    ledValues[33] = true;
    ledValues[34] = true;
    ledValues[35] = true;
  }

  //NACH
  if ((timeMin > 4) && (timeMin < 25) || (timeMin > 34) && (timeMin < 45))
  {
    ledValues[40] = true;
    ledValues[41] = true;
    ledValues[42] = true;
    ledValues[43] = true;
  }

  //-----numbers-----//

  //EINS
  if (((timeMin < 25) && (timeHour == 1) || (timeMin > 24) && (timeHour == 0) || (timeMin > 24) && (timeHour == 12)) && !ledValues[107])
  {
    ledValues[55] = true;
    ledValues[56] = true;
    ledValues[57] = true;
    ledValues[58] = true;
  }

  //EIN
  if ((timeHour == 1) && ledValues[107]) {
    ledValues[55] = true;
    ledValues[56] = true;
    ledValues[57] = true;
  }

  //ZWEI
  if ((timeMin < 25) && (timeHour == 2) || (timeMin > 24) && (timeHour == 1))
  {
    ledValues[62] = true;
    ledValues[63] = true;
    ledValues[64] = true;
    ledValues[65] = true;
  }

  //DREI
  if ((timeMin < 25) && (timeHour == 3) || (timeMin > 24) && (timeHour == 2))
  {
    ledValues[73] = true;
    ledValues[74] = true;
    ledValues[75] = true;
    ledValues[76] = true;
  }

  //VIER
  if ((timeMin < 25) && (timeHour == 4) || (timeMin > 24) && (timeHour == 3))
  {
    ledValues[66] = true;
    ledValues[67] = true;
    ledValues[68] = true;
    ledValues[69] = true;
  }

  //FÜNF
  if ((timeMin < 25) && (timeHour == 5) || (timeMin > 24) && (timeHour == 4))
  {
    ledValues[44] = true;
    ledValues[45] = true;
    ledValues[46] = true;
    ledValues[47] = true;
  }

  //SECHS
  if ((timeMin < 25) && (timeHour == 6) || (timeMin > 24) && (timeHour == 5))
  {
    ledValues[77] = true;
    ledValues[78] = true;
    ledValues[79] = true;
    ledValues[80] = true;
    ledValues[81] = true;
  }

  //SIEBEN
  if ((timeMin < 25) && (timeHour == 7) || (timeMin > 24) && (timeHour == 6))
  {
    ledValues[93] = true;
    ledValues[94] = true;
    ledValues[95] = true;
    ledValues[96] = true;
    ledValues[97] = true;
    ledValues[98] = true;
  }

  //ACHT
  if ((timeMin < 25) && (timeHour == 8) || (timeMin > 24) && (timeHour == 7))
  {
    ledValues[84] = true;
    ledValues[85] = true;
    ledValues[86] = true;
    ledValues[87] = true;
  }

  //NEUN
  if ((timeMin < 25) && (timeHour == 9) || (timeMin > 24) && (timeHour == 8))
  {
    ledValues[99] = true;
    ledValues[100] = true;
    ledValues[101] = true;
    ledValues[102] = true;
  }

  //ZEHN
  if ((timeMin < 25) && (timeHour == 10) || (timeMin > 24) && (timeHour == 9))
  {
    ledValues[103] = true;
    ledValues[104] = true;
    ledValues[105] = true;
    ledValues[106] = true;
  }

  //ELF
  if ((timeMin < 25) && (timeHour == 11) || (timeMin > 24) && (timeHour == 10))
  {
    ledValues[47] = true;
    ledValues[48] = true;
    ledValues[49] = true;
  }

  //ZWÖLF
  if ((timeMin < 25) && (timeHour == 12) || (timeMin > 24) && (timeHour == 11))
  {
    ledValues[88] = true;
    ledValues[89] = true;
    ledValues[90] = true;
    ledValues[91] = true;
    ledValues[92] = true;
  }

  //-----Zeitunterteilung-----//

  //FÜNF
  if ((timeMin > 4) && (timeMin < 10) || (timeMin > 24) && (timeMin < 30) || (timeMin > 34) && (timeMin < 40) || (timeMin > 54))
  {
    ledValues[0] = true;
    ledValues[1] = true;
    ledValues[2] = true;
    ledValues[3] = true;
  }

  //ZEHN
  if ((timeMin > 9) && (timeMin < 15) || (timeMin > 39) && (timeMin < 45) || (timeMin > 49) && (timeMin < 55))
  {
    ledValues[11] = true;
    ledValues[12] = true;
    ledValues[13] = true;
    ledValues[14] = true;
  }

  //VIERTEL
  if ((timeMin > 14) && (timeMin < 20) || (timeMin > 44) && (timeMin < 50)) //Intervall für Dreiviertel ebenfalls eingeschlossen
  {
    ledValues[22] = true;
    ledValues[23] = true;
    ledValues[24] = true;
    ledValues[25] = true;
    ledValues[26] = true;
    ledValues[27] = true;
    ledValues[28] = true;
  }

  //ZWANZIG
  if ((timeMin > 19) && (timeMin < 25))
  {
    ledValues[15] = true;
    ledValues[16] = true;
    ledValues[17] = true;
    ledValues[18] = true;
    ledValues[19] = true;
    ledValues[20] = true;
    ledValues[21] = true;
  }

  //HALB
  if ((timeMin > 24) && (timeMin < 45))
  {
    ledValues[51] = true;
    ledValues[52] = true;
    ledValues[53] = true;
    ledValues[54] = true;
  }

  //DREI(VIERTEL)
  if ((timeMin > 44) && (timeMin < 50))
  {
    for (int j = 22; j < 33; j++) {
      ledValues[j] = true;
    }
  }

  //Geburttage

  if ((timeDay == 26) && (timeMonth == 05)) {
    ledValues[36] = true;
    ledValues[37] = true;
    ledValues[38] = true;
    ledValues[39] = true;
    leo = true;
  }
  else
  {
    leo = false;
  }

  if ((timeDay == 20) && (timeMonth == 07)) {
    ledValues[36] = true;
    ledValues[37] = true;
    ledValues[38] = true;
    ledValues[39] = true;
    kai = true;
  }
  else
  {
    kai = false;
  }

  return ledValues[i];

}


//-------------------------------MAIN------------------------------------------//

void setup() {

  Serial.begin(9600); 

  //Wifi
  WiFi.hostname("wordclock");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  //mqtt
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  //ntp 
  timeClient.begin();
  timeClient.setTimeOffset(3600); //GMT+1

  //-----Pinout initialisieren-----//
  FastLED.addLeds<LED_TYPE, PIN_1, COLOR_ORDER>(led1, NUM_LEDS_PART_A).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, PIN_2, COLOR_ORDER>(led2, NUM_LEDS_PART_B).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, PIN_3, COLOR_ORDER>(led3, NUM_LEDS_PART_C).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, PIN_4, COLOR_ORDER>(led4, NUM_LEDS_PART_D).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, PIN_WORDS, COLOR_ORDER>(leds, NUM_LEDS_PART_E).setCorrection(TypicalLEDStrip);

}


void loop() {

  //check connection
  if (!client.connected()){
    reconnect();
  }
  client.loop();

  //update time
  setTime();

  //-----set LEDs-----//

  //timeMins

  //LED1
  if ((timeMin%5) == 0)
  {
    led1[0] = CRGB::Black;
  }
  else
  {
    led1[0].setRGB( MQTT_brightness, MQTT_brightness, MQTT_brightness);
  }

  //LED2
  if ( ((timeMin%5) == 0) || ((timeMin%5) == 1) )
  {
    led2[0] = CRGB::Black;
  }
  else
  {
    led2[0].setRGB( MQTT_brightness, MQTT_brightness, MQTT_brightness);
  }

  //LED3
  if ( ((timeMin%5) == 3) || ((timeMin%5) == 4) )
  {
    led3[0].setRGB( MQTT_brightness, MQTT_brightness, MQTT_brightness);
  }
  else
  {
    led3[0] = CRGB::Black;
  }

  //LED4
  if ((timeMin%5) == 4)
  {
    led4[0].setRGB( MQTT_brightness, MQTT_brightness, MQTT_brightness);
  }
  else
  {
    led4[0] = CRGB::Black;
  }

  //-----words-----}
  for (int i = 0; i < 36; i++) {
    if (setLED(i) == true) {
      leds[i].setRGB( MQTT_brightness, MQTT_brightness, MQTT_brightness);
    }
    else
    {
      leds[i] = CRGB::Black;
    }
  }
  for (int i = 40; i < 110; i++) {
    if (setLED(i) == true) {
      leds[i].setRGB( MQTT_brightness, MQTT_brightness, MQTT_brightness);
    }
    else
    {
      leds[i] = CRGB::Black;
    }
  }

  //---birthdays---
  if ((ledValues[36] == true) && (leo == true)) {
    leds[36] = CRGB::Red;
    leds[37] = CRGB::Red;
    leds[38] = CRGB::Red;
    leds[39] = CRGB::Red;
  }

  if ((ledValues[36] == true) && (kai == true)) {
    leds[36] = CRGB::Yellow;
    leds[37] = CRGB::Yellow;
    leds[38] = CRGB::Yellow;
    leds[39] = CRGB::Yellow;
  }

  //--activate LEDs---//
  if ((oldminute != timeMin) || (old_brightness != MQTT_brightness)){
    FastLED.show();
    old_brightness = MQTT_brightness;
  }

}


