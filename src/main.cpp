#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER //necessary for FastLED on ESPs
#include <FastLED.h>
#include <Wire.h>
#include <Time.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

boolean ledValues[110];
boolean ledStates[110];
boolean leo = false;
boolean kai = false;
int oldminute = 0;

//LED settings
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

//ntp - time synchronization
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
void setLED() {

  //reset old status
  for (int i = 0; i < 110; i++) {
    ledStates[i] = false;
  }

  //-----structural elements-----//

  //UHR
  if ((timeMin < 5)){
    ledStates[107] = true;
    ledStates[108] = true;
    ledStates[109] = true;
  }

  //VOR
  if ((timeMin > 24) && (timeMin < 30) || (timeMin > 49)){
    ledStates[33] = true;
    ledStates[34] = true;
    ledStates[35] = true;
  }

  //NACH
  if ((timeMin > 4) && (timeMin < 25) || (timeMin > 34) && (timeMin < 45)){
    ledStates[40] = true;
    ledStates[41] = true;
    ledStates[42] = true;
    ledStates[43] = true;
  }

  //-----numbers-----//

  //EINS
  if (((timeMin < 25) && (timeHour == 1) || (timeMin > 24) && (timeHour == 0) || (timeMin > 24) && (timeHour == 12)) && !ledStates[107]){
    ledStates[55] = true;
    ledStates[56] = true;
    ledStates[57] = true;
    ledStates[58] = true;
  }

  //EIN
  if ((timeHour == 1) && ledStates[107]){
    ledStates[55] = true;
    ledStates[56] = true;
    ledStates[57] = true;
  }

  //ZWEI
  if ((timeMin < 25) && (timeHour == 2) || (timeMin > 24) && (timeHour == 1)){
    ledStates[62] = true;
    ledStates[63] = true;
    ledStates[64] = true;
    ledStates[65] = true;
  }

  //DREI
  if ((timeMin < 25) && (timeHour == 3) || (timeMin > 24) && (timeHour == 2)){
    ledStates[73] = true;
    ledStates[74] = true;
    ledStates[75] = true;
    ledStates[76] = true;
  }

  //VIER
  if ((timeMin < 25) && (timeHour == 4) || (timeMin > 24) && (timeHour == 3)){
    ledStates[66] = true;
    ledStates[67] = true;
    ledStates[68] = true;
    ledStates[69] = true;
  }

  //FÜNF
  if ((timeMin < 25) && (timeHour == 5) || (timeMin > 24) && (timeHour == 4)){
    ledStates[44] = true;
    ledStates[45] = true;
    ledStates[46] = true;
    ledStates[47] = true;
  }

  //SECHS
  if ((timeMin < 25) && (timeHour == 6) || (timeMin > 24) && (timeHour == 5)){
    ledStates[77] = true;
    ledStates[78] = true;
    ledStates[79] = true;
    ledStates[80] = true;
    ledStates[81] = true;
  }

  //SIEBEN
  if ((timeMin < 25) && (timeHour == 7) || (timeMin > 24) && (timeHour == 6)){
    ledStates[93] = true;
    ledStates[94] = true;
    ledStates[95] = true;
    ledStates[96] = true;
    ledStates[97] = true;
    ledStates[98] = true;
  }

  //ACHT
  if ((timeMin < 25) && (timeHour == 8) || (timeMin > 24) && (timeHour == 7)){
    ledStates[84] = true;
    ledStates[85] = true;
    ledStates[86] = true;
    ledStates[87] = true;
  }

  //NEUN
  if ((timeMin < 25) && (timeHour == 9) || (timeMin > 24) && (timeHour == 8)){
    ledStates[99] = true;
    ledStates[100] = true;
    ledStates[101] = true;
    ledStates[102] = true;
  }

  //ZEHN
  if ((timeMin < 25) && (timeHour == 10) || (timeMin > 24) && (timeHour == 9)){
    ledStates[103] = true;
    ledStates[104] = true;
    ledStates[105] = true;
    ledStates[106] = true;
  }

  //ELF
  if ((timeMin < 25) && (timeHour == 11) || (timeMin > 24) && (timeHour == 10)){
    ledStates[47] = true;
    ledStates[48] = true;
    ledStates[49] = true;
  }

  //ZWÖLF
  if ((timeMin < 25) && (timeHour == 12) || (timeMin > 24) && (timeHour == 11)){
    ledStates[88] = true;
    ledStates[89] = true;
    ledStates[90] = true;
    ledStates[91] = true;
    ledStates[92] = true;
  }

  //-----Zeitunterteilung-----//

  //FÜNF
  if ((timeMin > 4) && (timeMin < 10) || (timeMin > 24) && (timeMin < 30) || (timeMin > 34) && (timeMin < 40) || (timeMin > 54)){
    ledStates[0] = true;
    ledStates[1] = true;
    ledStates[2] = true;
    ledStates[3] = true;
  }

  //ZEHN
  if ((timeMin > 9) && (timeMin < 15) || (timeMin > 39) && (timeMin < 45) || (timeMin > 49) && (timeMin < 55)){
    ledStates[11] = true;
    ledStates[12] = true;
    ledStates[13] = true;
    ledStates[14] = true;
  }

  //VIERTEL - Intervall für Dreiviertel ebenfalls eingeschlossen
  if ((timeMin > 14) && (timeMin < 20) || (timeMin > 44) && (timeMin < 50)){
    ledStates[22] = true;
    ledStates[23] = true;
    ledStates[24] = true;
    ledStates[25] = true;
    ledStates[26] = true;
    ledStates[27] = true;
    ledStates[28] = true;
  }

  //ZWANZIG
  if ((timeMin > 19) && (timeMin < 25)){
    ledStates[15] = true;
    ledStates[16] = true;
    ledStates[17] = true;
    ledStates[18] = true;
    ledStates[19] = true;
    ledStates[20] = true;
    ledStates[21] = true;
  }

  //HALB
  if ((timeMin > 24) && (timeMin < 45)){
    ledStates[51] = true;
    ledStates[52] = true;
    ledStates[53] = true;
    ledStates[54] = true;
  }

  //DREI(VIERTEL)
  if ((timeMin > 44) && (timeMin < 50)){
    for (int j = 22; j < 33; j++) {
      ledStates[j] = true;
    }
  }

  //Geburttage
  if ((timeDay == 26) && (timeMonth == 05)) {
    ledStates[36] = true;
    ledStates[37] = true;
    ledStates[38] = true;
    ledStates[39] = true;
    leo = true;
  }
  else
  {
    leo = false;
  }

  if ((timeDay == 20) && (timeMonth == 07)) {
    ledStates[36] = true;
    ledStates[37] = true;
    ledStates[38] = true;
    ledStates[39] = true;
    kai = true;
  }
  else
  {
    kai = false;
  }

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

  //-----initialize IO-----//
  FastLED.addLeds<LED_TYPE, PIN_1, COLOR_ORDER>(led1, NUM_LEDS_PART_A).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, PIN_2, COLOR_ORDER>(led2, NUM_LEDS_PART_B).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, PIN_3, COLOR_ORDER>(led3, NUM_LEDS_PART_C).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, PIN_4, COLOR_ORDER>(led4, NUM_LEDS_PART_D).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE, PIN_WORDS, COLOR_ORDER>(leds, NUM_LEDS_PART_E).setCorrection(TypicalLEDStrip);

}


void loop() {

  //check mqtt connection
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

  //-----words-----//

  //get data for current time
  setLED();

  for (int i = 0; i < 36; i++) {
    if (ledStates[i] == true) {
      leds[i].setRGB( MQTT_brightness, MQTT_brightness, MQTT_brightness);
    }
    else
    {
      leds[i] = CRGB::Black;
    }
  }
  for (int i = 40; i < 110; i++) {
    if (ledStates[i] == true) {
      leds[i].setRGB( MQTT_brightness, MQTT_brightness, MQTT_brightness);
    }
    else
    {
      leds[i] = CRGB::Black;
    }
  }

  //---birthdays---
  if ((ledStates[36] == true) && (leo == true)) {
    leds[36] = CRGB::Red;
    leds[37] = CRGB::Red;
    leds[38] = CRGB::Red;
    leds[39] = CRGB::Red;
  }

  if ((ledStates[36] == true) && (kai == true)) {
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


