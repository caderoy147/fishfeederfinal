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
const char* ntpServer1="ph.pool.ntp.org";
const char* ntpServer2="time.nist.gov";
const long gmtOffset_sec=28800;
const int daylightOffset_sec=28800;

const char* time_zone = "CET-1CSET,M3.5.0,M10.5.0/3";


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


void setup(){
 Serial.begin(115200);

  // Initialize WiFiManager
  WiFiManager wifiManager;

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
  if (!wifiManager.autoConnect("AutoConnectAP")) {
    Serial.println("Failed to connect, we should reset and see if it connects");
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  Serial.println("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
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
}

void loop(){

  int amountToFeedSpin;

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "test/int", count)){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    count++;
    
    // Write an Float number on the database path test/float
    if (Firebase.RTDB.setFloat(&fbdo, "test/float", 0.01 + random(0,100))){
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }



    updateLedStatus();

    updateManualStatus();

    amountToFeedSpin = scheduler();

    spin(amountToFeedSpin);

    Serial.println("LOCAL TIME::");
    printLocalTime();
  }
}

void spin(int amount){

  int amountSpins = amount;
  
    // Implement your logic for moving the stepper motor based on the schedule
  // You can use the SchedulerSettings structure to get the schedule details
  
  if(amount==0){
  myStepper.step(stepsPerRevolution*amountSpins); // move one step clockwise
  delay(60000);}else{
  moveClockwiseStop();}

}


void checkScheduler1() {
  if (Firebase.RTDB.getJSON(&fbdo, "/schedulerSettings")) {
    String jsonString = fbdo.jsonString();

    // Now jsonString contains the JSON data as a String
    Serial.println("JSON Data: " + jsonString);

    // Now you can parse jsonString using ArduinoJson library
    // Example: Parse JSON data with ArduinoJson
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, jsonString);

    // Extract the value of 'amountToFeed' field
    String amountToFeedValue = doc["amountToFeed"].as<String>();

    Serial.println("Amount to feed: " + amountToFeedValue);
  } else {
    Serial.println("Failed to get JSON data");
  }
}





void checkCurrentTime() {
  if (Firebase.RTDB.getJSON(&fbdo, "/currentTime")) {
    String jsonString = fbdo.jsonString();

    // Now jsonString contains the JSON data as a String
    Serial.println("JSON Data: " + jsonString);

    // Now you can parse jsonString using ArduinoJson library
    // Example: Parse JSON data with ArduinoJson
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, jsonString);

    // Extract the value of 'amountToFeed' field
    String currentDayToday = doc["day"].as<String>();

    Serial.println("today is: " + currentDayToday);
  } else {
    Serial.println("Failed to get JSON data");
  }
}








int scheduler()
{

  //////////////////global declarations for mathching

  //////data types of user
  int amountToFeedValue;
  String ampmUser;
  String dayUser;
  JsonArray dayArray;
  String hourUser;
  String minuteUser;


  //////data types for current time
  String ampmCurrent;
  String dayCurrent;
  String hourCurrent;
  String minuteCurrent;



/////

String dayUser1;
String dayUser2;
String dayUser3;
String dayUser4;
String dayUser5;
String dayUser6;
String dayUser7;

  //////////////////////////////////////scheduler users chosen sched

  if (Firebase.RTDB.getJSON(&fbdo, "/schedulerSettings")) {
    String jsonString = fbdo.jsonString();

    // Now jsonString contains the JSON data as a String
    Serial.println("JSON Data: " + jsonString);

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
        Serial.println(day);
    }

     dayUser1 = dayArray[0].as<String>();
     dayUser2 = dayArray[1].as<String>();
     dayUser3 = dayArray[2].as<String>();
     dayUser4 = dayArray[3].as<String>();
     dayUser5 = dayArray[4].as<String>();
     dayUser6 = dayArray[5].as<String>();
     dayUser7 = dayArray[6].as<String>();


    hourUser = doc["hour"].as<String>();
    minuteUser = doc["minute"].as<String>();
    ampmUser = doc["ampm"].as<String>();


  } else {
    Serial.println("Failed to get JSON data");
  }


  ////////////////////////////////////////////// whats the current time day



  if (Firebase.RTDB.getJSON(&fbdo, "/currentTime")) {
    String jsonString = fbdo.jsonString();

    // Now jsonString contains the JSON data as a String
    Serial.println("JSON Data: " + jsonString);

    // Now you can parse jsonString using ArduinoJson library
    // Example: Parse JSON data with ArduinoJson
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, jsonString);

    // Extract the value of 'amountToFeed' field
    dayCurrent = doc["day"].as<String>();
    hourCurrent = doc["hour"].as<String>();
    minuteCurrent = doc["minute"].as<String>();
    ampmCurrent = doc["ampm"].as<String>();


  } else {
    Serial.println("Failed to get JSON data");
  }


  // Assuming dayCurrent, hourUser, minuteUser, ampmUser, amountToFeedValue are already defined





    Serial.println("Amount to feed: " + String(amountToFeedValue));
    Serial.println("SHED IS DAY: " + dayUser1);
    Serial.println("SHED IS DAY: " + dayUser2);
    Serial.println("SHED IS DAY: " + dayUser3);
    Serial.println("SHED IS DAY: " + dayUser4);
    Serial.println("SHED IS DAY: " + dayUser5);
    Serial.println("SHED IS DAY: " + dayUser6);
    Serial.println("SHED IS DAY: " + dayUser7);
    Serial.println("SHED IS hour is: " + hourUser);
    Serial.println("SHED IS minute is: " + minuteUser);
    Serial.println("SHED IS am/pm is: " + ampmUser);

    Serial.println("today is: " + dayCurrent);
    Serial.println("current hour is: " + hourCurrent);
    Serial.println("current minute is: " + minuteCurrent);
    Serial.println("current am/pm is: " + ampmCurrent);
	

      // Check if dayCurrent matches any day in the array
      if ((dayCurrent == dayUser1 || dayCurrent == dayUser2 || dayCurrent == dayUser3 || dayCurrent == dayUser4 || dayCurrent == dayUser5 || dayCurrent == dayUser6 || dayCurrent == dayUser7) && hourCurrent == hourUser && minuteCurrent == minuteUser && ampmCurrent == ampmUser) {
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
      Serial.println("LED on");
    }
    else
    {
      digitalWrite(LedPin, LOW);
      digitalWrite(Led1Pin, LOW);
      Serial.println("LED off");
    }
  }
}



void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    return;
  }
  Serial.println(&timeinfo,"%A, %B %d %Y %H:%M:%S");
}


