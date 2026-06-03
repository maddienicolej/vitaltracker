#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include <Wire.h>
#include "time.h"
#include <Adafruit_MLX90614.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <math.h>


// Network and Firebase credentials
#define WIFI_SSID "ENTER_SSID_HERE"
#define WIFI_PASSWORD "ENTER_SSID_PW_HERE"

#define Web_API_KEY "AIzaSyCiTjrAgUaS7PY2DSuLZctDyGkvop839PQ"
#define DATABASE_URL "https://vitaltracker-a9d1c-default-rtdb.firebaseio.com/"
#define USER_EMAIL "ENTER_USER_ID"
#define USER_PASS "ENTER_USER_PW"

//////////module vars below
bool eventReady;
//body temp global vars and definitions
Adafruit_MLX90614 mlx;
const int tempSamples = 10;
float tempReadings[tempSamples];
int tempIndex = 0;

float feverThreshold = 100.4;
float spikeThreshold = 1.0;
bool tempSpike = false;

float lastTemp = 0;
float temperature;

unsigned long lastTempRead = 0;
const int tempInterval = 1000;

//mpu global vars and definitions
Adafruit_MPU6050 mpu;
#define FREE_FALL_THRESHOLD 2.5
#define IMPACT_THRESHOLD 18.0
#define STILLNESS_THRESHOLD 11
#define IMPACT_WINDOW 700

bool possibleFall = false;
bool fallDetected = false;
unsigned long freeFallTime = 0;


//config for EKG mod, vars, and sampling settings
#define EKG_PIN 34
#define LO_PLUS 25
#define LO_MINUS 26

const int SAMPLE_RATE_HZ = 250;
const int SAMPLE_INTERVAL_US = 1000000 / SAMPLE_RATE_HZ;

unsigned long lastSampleTime = 0;

const int BUFFER_SIZE = 2500;
int buffer_1[BUFFER_SIZE];
int buffer_2[BUFFER_SIZE];
int* current_buffer = buffer_1;
int* upload_buffer = buffer_2;
bool buffer_ready = false;
int count = 0;

/////BPM vars
int threshold = 2400;
const unsigned long refractoryPeriod = 300;
int prevValue = 0;

bool peakActive = false;
unsigned long lastBeatTime = 0;
float bpm = -1;

//////////initialize mods
void initEKG(){
  pinMode(EKG_PIN, INPUT);
  pinMode(LO_PLUS, INPUT);
  pinMode(LO_MINUS, INPUT);

  analogSetAttenuation(ADC_11db);
  Serial.println("EKG Monitor Ready");
} 

void initMLX(){
  if (!mlx.begin())
  {
    Serial.println("MLX90614 not detected");
    while (1);
  }
}

void initMPU(){
  if (!mpu.begin())
  {
    Serial.println("MPU6050 not detected");
    while (1);
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);

  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

//////////module functions below
// averaging filter for ekg
int readEKGFiltered() {
  int sum = 0;
  const int N = 8; // number of samples to average

  for (int i = 0; i < N; i++) {
    sum += analogRead(EKG_PIN);
  }
  return sum / N;
}

// temp function
float averageTemp()
{
  float sum = 0;
  for (int i = 0; i < tempSamples; i++)
    sum += tempReadings[i];
  return sum / tempSamples;
}

//read ekg (+ process BPM)
void getEKG() {
  unsigned long now = micros();

  // Enforce precise sampling rate (250 Hz)
  if (now - lastSampleTime >= SAMPLE_INTERVAL_US) {
    lastSampleTime = now;

    // Check electrode connection
    if (digitalRead(LO_PLUS) || digitalRead(LO_MINUS)) {
      Serial.println(0); // leads off
      peakActive = false;
      return;
    }

    int ekgValue = readEKGFiltered();
    //Serial.println(ekgValue);
    
    processBPM(ekgValue); ////add this line to larger code
    current_buffer[count++] = ekgValue;
      
    if (count == BUFFER_SIZE){
      upload_buffer = current_buffer;
      buffer_ready = true;

      current_buffer = (current_buffer == buffer_1) ? buffer_2 : buffer_1;

      count = 0;
    }
  }
}

// process BPM
void processBPM(int ekgValue) {
  unsigned long currentTime = millis();

  // Detect rising edge (R-peak)
  if (ekgValue > threshold && ekgValue > prevValue && !peakActive) {
    if (currentTime - lastBeatTime > refractoryPeriod) {
      peakActive = true;
      if (lastBeatTime != 0) {
        unsigned long rrInterval = currentTime - lastBeatTime;

        if (rrInterval > 0) {
          float newBpm = 60000.0 / rrInterval;

          if (bpm < 0) {
            bpm = newBpm;
          } else {
            bpm = 0.8 * bpm + 0.2 * newBpm;
          }

          if (bpm >= 40 && bpm <= 200) {
            Serial.print("BPM: ");
            Serial.println(bpm);
            //return; // ignore unrealistic values
          }
        }
      }

      lastBeatTime = currentTime;
    }
  }
  if (peakActive && ekgValue < prevValue) {
    peakActive = false;
  }
  prevValue = ekgValue;
}

//read temp
void readTemperature()
{
  if (millis() - lastTempRead < tempInterval)
    return;

  lastTempRead = millis();

  float tempC = mlx.readObjectTempC();

  tempReadings[tempIndex] = tempC;

  tempIndex = (tempIndex + 1) % tempSamples;

  float avgTempF = averageTemp() * 9.0 / 5.0 + 32.0;

  Serial.print("Temperature: ");
  Serial.print(avgTempF);
  Serial.println(" F");

  float change = avgTempF - lastTemp;

  if (avgTempF >= feverThreshold && change >= spikeThreshold) {
    Serial.println("TEMPERATURE SPIKE DETECTED");
    tempSpike = true;
    eventReady = true;
  }
  lastTemp = avgTempF;
  temperature = avgTempF;
}

//detect fall
void detectFall()
{
  sensors_event_t a, g, t;

  mpu.getEvent(&a, &g, &t);

  float magnitude =
    sqrt(
      a.acceleration.x * a.acceleration.x +
      a.acceleration.y * a.acceleration.y +
      a.acceleration.z * a.acceleration.z
    );

  if (!possibleFall && magnitude < FREE_FALL_THRESHOLD) {
    possibleFall = true;
    freeFallTime = millis();
  }

  // Cancel if impact does not happen soon enough
  if (possibleFall && (millis() - freeFallTime > IMPACT_WINDOW)) {
    possibleFall = false;
  }

  // Step 2: impact after free fall
  if (possibleFall && magnitude > IMPACT_THRESHOLD) {
    Serial.println("FALL DETECTED");
    fallDetected = true;
    eventReady = true;
    possibleFall = false;
  }
}
///////////////////end of functions for mods

// User functions
void processData(AsyncResult &aResult);

// Authentication
UserAuth user_auth(Web_API_KEY, USER_EMAIL, USER_PASS);

// Firebase components
FirebaseApp app;
WiFiClientSecure ssl_client;
using AsyncClient = AsyncClientClass;
AsyncClient aClient(ssl_client);
RealtimeDatabase Database;


String databasePath; // Database main path
// Database child nodes
String tempPath = "/temperature";
String spo2Path = "/spo2";
String bpmPath = "/bpm";
//String mpuPath = "/mpuBool";
//String tempBoolPath = "/tempBool";
String ekgPath = "/ekg";
//String timePath = "/lastTriggered";

// Parent Node (to be updated in every loop)
String parentPath;

int timestamp;

const char* ntpServer = "pool.ntp.org";

//float temperature;

// Create JSON objects for storing data
object_t jsonData, obj1, obj2, obj3;
object_t statusJson, obj4, obj5, obj6, obj7;
JsonWriter writer;


// Initialize WiFi
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
}

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return(0);
  }
  time(&now);
  return now;
}

void setup(){
  Serial.begin(115200);

  Wire.begin(21, 22); //(sda: 21, scl: 22)
  initEKG();
  initMPU();
  initMLX();

  initWiFi();
  configTime(0, 0, ntpServer);

  // Configure SSL client
  ssl_client.setInsecure();
  ssl_client.setConnectionTimeout(1000);
  ssl_client.setHandshakeTimeout(5);

  // Initialize Firebase
  initializeApp(aClient, app, getAuth(user_auth), processData, "🔐 authTask");
  app.getApp<RealtimeDatabase>(Database);
  Database.url(DATABASE_URL);

  Serial.println("System Ready");
}

void uploadReadings(){
  databasePath = "/readings/device001";
      
      timestamp = getTime();
      Serial.print ("time: ");
      Serial.println (timestamp);

      parentPath = databasePath + "/" + String(timestamp);
      

    if (buffer_ready) {
    jsonData.clear();

    writer.create(obj1, bpmPath, bpm);
    writer.create(obj2, tempPath, temperature);

    String ekgString = "";
    for (int i = 0; i < BUFFER_SIZE; i++) {
        ekgString += String(upload_buffer[i]);
        if (i < BUFFER_SIZE - 1) ekgString += ",";
    }

    writer.create(obj3, ekgPath, ekgString);
    //writer.create(obj4, tempBoolPath, tempSpike);
    //writer.create(obj5, mpuPath, fallDetected);
    writer.join(jsonData, 3, obj1, obj2, obj3);
    

    //buffer_ready = false;
    //tempSpike = false;
    //fallDetected = false;

    // Send JSON string to Firebase
    //append ekg>obj5 to path without timestamp(databasePath or readings/) as chart data
    Database.set<object_t>(aClient, parentPath, jsonData, processData, "RTDB_Send_Data");

    Serial.println("Data sent to Firebase!");
  }
}
void updateEventFlags(){
  statusJson.clear();
  String statusPath = "/status/device001/";
  int timestamp = getTime();
  bool eventTriggered = false;

  if (tempSpike && fallDetected) {
    writer.create(obj4, "/tempBool", true);
    writer.create(obj5, "/tempTime", timestamp);
    writer.create(obj6, "/mpuBool", true);
    writer.create(obj7, "/fallTime", timestamp);
    writer.join(statusJson, 4, obj4, obj5, obj6, obj7);
  }
  else if (tempSpike) {
    writer.create(obj4, "/tempBool", true);
    writer.create(obj5, "/tempTime", timestamp);
    writer.join(statusJson, 2, obj4, obj5);
  }
  else if (fallDetected) {
    writer.create(obj6, "/mpuBool", true);
    writer.create(obj7, "/fallTime", timestamp);
    writer.join(statusJson, 2, obj6, obj7);
  }
  else {
    return;
  }
  Database.set<object_t>(aClient, statusPath, statusJson, processData, "EVENT_TRIGGERED_UPLOAD");
  Serial.println("Data sent to Firebase!");
  
  
  tempSpike = false;
  fallDetected = false;
  
}

void loop(){
  // Maintain authentication and async tasks
  app.loop();
  getEKG();
  readTemperature();
  detectFall();
  // Check if authentication is ready
    // Periodic data sending every 10 seconds
    unsigned long currentTime = millis();
    if (app.ready() && eventReady){
        updateEventFlags();
        eventReady = false;
    }

    if (app.ready() && buffer_ready){
      uploadReadings();
      buffer_ready = false;
  }
}



void processData(AsyncResult &aResult){
  if (!aResult.isResult())
    return;

  if (aResult.isEvent())
    Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

  if (aResult.isDebug())
    Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

  if (aResult.isError())
    Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

  if (aResult.available())
    Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
}
