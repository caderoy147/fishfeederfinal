#include <ArduinoJson.h>
#include "time.h"
#include "sntp.h"

/*
  Rui Santos
  Complete project details at our blog.
    - ESP32: https://RandomNerdTutorials.com/esp32-firebase-realtime-database/
    - ESP8266: https://RandomNerdTutorials.com/esp8266-nodemcu-firebase-realtime-database/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Based in the RTDB Basic Example by Firebase-ESP-Client library by mobizt
  https://github.com/mobizt/Firebase-ESP-Client/blob/main/examples/RTDB/Basic/Basic.ino
*/

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <WiFiManager.h>


#include <Stepper.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// // Insert your network credentials
// #define WIFI_SSID "PLDTHOMEFIBR90e10" // your wifi SSID
// #define WIFI_PASSWORD "PLDTWIFIwuv5u" // your wifi PASSWORD

// Insert Firebase project API Key
#define API_KEY "AIzaSyAjZN7E2KzqD6eoNTBBNvXmxYmplxWOYZg"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://fishfeeder-8bf7c-default-rtdb.asia-southeast1.firebasedatabase.app/" 

#define LedPin 2                            // GPIO pin where the built-in LED is connected on ESP32
#define Led1Pin 12                            // GPIO pin where the built-in LED is connected on ESP32


///
const char* ntpServer1="asia.pool.ntp.org";
const char* ntpServer2="asia.pool.ntp.org";
const long gmtOffset_sec=28800;
const int daylightOffset_sec=28800;

const char* time_zone = "Asia/Manila";



//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

//steper
const int stepsPerRevolution = 2048;
Stepper myStepper(stepsPerRevolution,32, 25, 33, 26);  
WiFiServer server(80);




///////current time timer.h
int dayTest;
int hourTest;
int minuteTest;
String ampmCurentNow;


int hourPh;
int hourPhFinal;


String dayCurrentTime;

int resetWifiPin = 0;

WiFiManager wifiManager;


void setup(){


  pinMode(resetWifiPin,INPUT);

 Serial.begin(115200);

  // Initialize WiFiManager


  // Uncomment the line below if you want to reset the saved credentials
 //  wifiManager.resetSettings();

  // Set config portal timeout to 5 minutes
  wifiManager.setConfigPortalTimeout(300);

  // Custom parameters to be configured
  WiFiManagerParameter custom_ssid("ssid", "SSID", "yourAP", 40);
  WiFiManagerParameter custom_password("password", "Password", "yourPassword", 40);

  // Add parameters
  wifiManager.addParameter(&custom_ssid);
  wifiManager.addParameter(&custom_password);

  // Check if there are saved WiFi credentials
  if (!wifiManager.autoConnect("FishFeeder")) {
    
    delay(3000);
    ESP.restart();
    delay(5000);
  }


  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {

    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(LedPin, OUTPUT);
  pinMode(Led1Pin, OUTPUT);
  myStepper.setSpeed(15);




  sntp_servermode_dhcp(1);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  configTzTime(time_zone, ntpServer1, ntpServer2);
  

}

void loop(){

  int amountToFeedSpin;

   if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
      sendDataPrevMillis = millis();


        updateLedStatus();

        updateManualStatus();

        amountToFeedSpin = scheduler();

        spin(amountToFeedSpin); 
        
        primer();

        printLocalTime();
        
     }

 wifireset();


}

void spin(int amount){

  int amountSpins = amount;
  int delayNeeded = ((4000*amountSpins));
  
    // Implement your logic for moving the stepper motor based on the schedule
  // You can use the SchedulerSettings structure to get the schedule details
  
  if(amount!=0){
  myStepper.step(stepsPerRevolution*amountSpins); // move one step clockwise
  
  delay(delayNeeded);
  }else{
  moveClockwiseStop();}




}





int scheduler()
{

  //////////////////global declarations for mathching

  //////data types of user
  int amountToFeedValue;
  String ampmUser;
  String dayUser;
  JsonArray dayArray;
  int hourUser;
  int minuteUser;



/////

    String dayUser1;
    String dayUser2;
    String dayUser3;
    String dayUser4;
    String dayUser5;
    String dayUser6;
    String dayUser7;

  //////////////////////////////////////scheduler users chosen sched


   dayCurrentTime;


    //day checker
    if(dayTest==1){
      dayCurrentTime = "MON";
    }else if(dayTest==2){
      dayCurrentTime = "TUE";
    }else if(dayTest==3){
      dayCurrentTime = "WED";
    }else if(dayTest==4){
      dayCurrentTime = "THU";
    }else if(dayTest==5){
      dayCurrentTime = "FRI";
    }else if(dayTest==6){
      dayCurrentTime = "SAT";
    }else{
      dayCurrentTime = "SUN";
    }



  if (Firebase.RTDB.getJSON(&fbdo, "/schedulerSettings")) {
    String jsonString = fbdo.jsonString();

    // Now jsonString contains the JSON data as a String


    // Now you can parse jsonString using ArduinoJson library
    // Example: Parse JSON data with ArduinoJson
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, jsonString);

    // Extract the value of 'amountToFeed' field
    amountToFeedValue = doc["amountToFeed"].as<int>();
    dayArray = doc["days"].as<JsonArray>();

    // Now you can iterate over the elements in the array
    for (int i = 0; i < dayArray.size(); i++) {
        String day = dayArray[i].as<String>();
        // Do something with each day
    }

     dayUser1 = dayArray[0].as<String>();
     dayUser2 = dayArray[1].as<String>();
     dayUser3 = dayArray[2].as<String>();
     dayUser4 = dayArray[3].as<String>();
     dayUser5 = dayArray[4].as<String>();
     dayUser6 = dayArray[5].as<String>();
     dayUser7 = dayArray[6].as<String>();


    hourUser = doc["hour"].as<int>();
    minuteUser = doc["minute"].as<int>();
    ampmUser = doc["ampm"].as<String>();


  } else {

  }


  ////////////////////////////////////////////// whats the current time day



  if (Firebase.RTDB.getJSON(&fbdo, "/currentTime")) {
    String jsonString = fbdo.jsonString();

    // Now jsonString contains the JSON data as a String

    // Now you can parse jsonString using ArduinoJson library
    // Example: Parse JSON data with ArduinoJson
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, jsonString);

    


  } else {
  }


  // Assuming dayCurrent, hourUser, minuteUser, ampmUser, amountToFeedValue are already defined







      // Check if dayCurrent matches any day in the array
      if ((dayCurrentTime == dayUser1 || dayCurrentTime == dayUser2 || dayCurrentTime == dayUser3 || dayCurrentTime == dayUser4 || dayCurrentTime == dayUser5 || dayCurrentTime == dayUser6 || dayCurrentTime == dayUser7) && hourTest == hourUser && minuteTest == minuteUser && ampmCurentNow == ampmUser) {
          // Schedules match, return amountToFeedValue (dayCurrent == dayUser1 || dayCurrent == dayUser2 || dayCurrent == dayUser3 || dayCurrent == dayUser4 || dayCurrent == dayUser5 || dayCurrent == dayUser6 || dayCurrent == dayUser7) &&
          Serial.println("ITS FEEDING TIME!!!");


          return amountToFeedValue;
      } else {
          // Schedules do not match, return 0
          Serial.println("Not Yet feeding time");
          return 0;
      }
 
  /////////////////////////////////////////checker /in this par we match values


}









void updateManualStatus() {
  while (Firebase.RTDB.getString(&fbdo, "/ManualStatus")) {
    String manualstatus = fbdo.stringData();
    if (manualstatus.toInt() == 1) {
      myStepper.step(stepsPerRevolution); // move one step clockwise
    } else {
      moveClockwiseStop();
      Serial.println("stepper off");
      break;
    }
  }
}

void moveClockwiseStop(){
  myStepper.step(0);
  digitalWrite(32,LOW);
  digitalWrite(25,LOW);
  digitalWrite(33,LOW);
  digitalWrite(26,LOW);
  
}



void updateLedStatus()
{
  if (Firebase.RTDB.getString(&fbdo, "/Led1Status"))
  {
    String ledstatus = fbdo.stringData();
    if (ledstatus.toInt() == 1)
    {
      digitalWrite(LedPin, HIGH);
      digitalWrite(Led1Pin, HIGH);

    }
    else
    {
      digitalWrite(LedPin, LOW);
      digitalWrite(Led1Pin, LOW);
    }
  }
}



void printLocalTime(){
  delay(1000);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo,"%A, %B %d %Y %H:%M:%S");




  dayTest = timeinfo.tm_wday;
  hourTest = timeinfo.tm_hour;
  minuteTest = timeinfo.tm_min;

  ampmCurentNow;
      if (timeinfo.tm_hour < 12) {
        ampmCurentNow = "am";
    } else {
        ampmCurentNow = "pm";
    }

  
 
}


void primer(){
int delayNeeded = ((4000*6));
String primerOff = "0";

    
   if (Firebase.RTDB.getString(&fbdo, "/PrimerButton"))
  {
    String PrimerButton = fbdo.stringData();
    if (PrimerButton.toInt() == 1)
    {
      myStepper.step(stepsPerRevolution*6);
      delay(delayNeeded);
      moveClockwiseStop();
        // Write an Int number on the database path test/int
     if (Firebase.RTDB.setString(&fbdo, "/PrimerButton", primerOff)){
     }
     else {

     }

    }else{

    }
  }
}

void wifireset(){

  // Uncomment the line below if you want to reset the saved credentials
  if(digitalRead(resetWifiPin) == LOW){
      wifiManager.resetSettings();
  }else
  {
   
  }
 
}



