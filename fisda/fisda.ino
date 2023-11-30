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
   wifiManager.resetSettings();

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




    if (Firebase.RTDB.getString(&fbdo, "/Led1Status")) {
      String ledstatus = fbdo.stringData();
      if (ledstatus.toInt() == 1) {
        digitalWrite(LedPin, HIGH);
        digitalWrite(Led1Pin, HIGH);
        Serial.println("LED on");
      } else {
        digitalWrite(LedPin, LOW);
        digitalWrite(Led1Pin, LOW);
        Serial.println("LED off");
      }
    }



    if (Firebase.RTDB.getString(&fbdo, "/ManualStatus")) {
      String manualstatus = fbdo.stringData();
      if (manualstatus.toInt() == 1) {
        moveClockwise();
        Serial.println("steppper on");
      } else {
        moveClockwiseStop();
        Serial.println("stepper off");
      }
    }

    
  }
}



void moveClockwise() { // set initial speed
  myStepper.step(stepsPerRevolution); // move 5 revolutions clockwise
}
void moveClockwiseStop(){
  myStepper.step(0);
}