//current version
/* Fridge controller for chest freezer
    Support the following
      Temperate control
      Humidity control

   Hardware
   Esp8266?
    Temp / humidity sensor
    two SSR's
      Fridge Power
      Dehumidifier Power
        Dehumidification reverse?
   Display
    Rely on web portal

*/

// Import required libraries
#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"
#include <secrets.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoOTA.h> //owa
#include "Webpage.h"
#include <WebSerial.h>
#include <ESPAsyncTCP.h>

// VERSION CONTROL
const String Version = "V1.3";

// Network credentials
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password

// Hardware Pinout definitions **************************************
uint8_t FridgePin = D8;
uint8_t HumidPin = D5;
uint8_t OneWirePin = D2;
uint8_t DHTPin = D1;

// Sensor Setup ****************************************************
#define DHTTYPE DHT11     // DHT 11
DHT dht(DHTPin, DHTTYPE);

OneWire oneWire(OneWirePin);
DallasTemperature sensors(&oneWire);
DeviceAddress Thermo;
//uint8_t sensor1[8] = { 0x28, 0xF5, 0xFE, 0x31, 0x04, 0x00, 0x00, 0xDA };//green-yellow-blue
uint8_t sensor1[8] = { 0x28, 0x40, 0x49, 0x32, 0x04, 0x00, 0x00, 0xA0 };//red-orage-brown

//START OF GLOBAL VARIABLES CONTROL BLOCK **************************
// Tempvariables
int TempProtectDelaySeconds = 240;
int TempMinRunSeconds = 60;
int TempTarget = 38;
int TempLowOffset = 4;
double TempCurrent = 0;
int TempStatus = 0;
unsigned long TempStartTime = 0;
unsigned long TempEndTime = 0;
String TempText = "";
int TempOffTime = 0;
int TempLastOnTime = 0;
int TempCycleCount = 0;



int HumidProtectDelaySeconds = 21600;  // Off time of 6 hours
int HumidMinRunSeconds = 5400;   // Min run time 1.5 hour
int HumidMaxRunSeconds = 7200;  // max run time 2 hours
int HumidTarget = 30;
int HumidLowOffset = 10;
double HumidCurrent = 0;
int HumidStatus = 0;
unsigned long HumidStartTime = 0;
unsigned long HumidEndTime = 0;
String HumidText = "";
int HumidOffTime = 0;
int HumidLastOnTime = 0;
int HumidCycleCount = 0;


unsigned long SlowWebserial=0;

// INPUT VALUES
double HumidDHT = 0;
double TempDHT = 0;
double SpareTemp = 0;

// Smoothing Varoables **********************************************
int positions = 5;
float SmoothingDHTTemp[6];  //0 = position, 1-5 values
float SmoothingDHTHumid[6];
float SmoothingOWSTemp[6];

//Periodic Timers ****************************************************
unsigned long LoopStartTime = 0;
unsigned long TimerDHT11 = 0;

AsyncWebServer server(80);  //port 80

//OTA CONTENT BELOW************************************************************************************
const char* host = "ChestFreezer";       //OTA SETUP INFORMATION

//OTA SETUP
void SetupOTA() {
  ArduinoOTA.setHostname(host);
  // ArduinoOTA.setPort(3232);  // Port defaults to 3232
  // ArduinoOTA.setPassword("Fridge");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  //ArduinoOTA.begin();
}
// OTA END ***********************************************************************************************

void SetupSmoothing(float A[]) {
  for (byte i = 0; i < (positions); i++) {
    A[i] = 0.0;
  }
}

void AddSmoothing(float A[], float newvalue) {
  for (byte i = 0; i < positions; i++) {
    A[i] = A[i + 1];
  }
  A[5] = newvalue;
}

float GetSmoothedValue(float A[]) {
  float Total = 0;
  for (byte i = 0; i < positions; i++) {
    Total = (Total + A[i]);
  }
  float output = 0;
  output = Total / positions;
  return output;
  //WebSerial.println(1+String(A[0])+"-"+String(A[1])+"-"+String(A[2])+"-"+String(A[3])+"-"+String(A[4])+"-"+String(A[5]) );
}

float AddAndGetSmoothedValue(float B[], float newvalue) {
  AddSmoothing(B, newvalue);
  float output = 0;
  output = GetSmoothedValue(B);
  return output;
}

String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float t = dht.readTemperature(true);
  if (isnan(t)) {
    //TempDHT = -99;
    TempDHT = TempDHT - 1;
  }
  else {
    TempDHT = t;
  }
}

void readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if (isnan(h)) {
    //HumidDHT = -99;
    HumidDHT = HumidDHT - 1;
  }
  else {
    HumidDHT = h;
  }
}
//*************************************************************************************
//**********************************************Unused here for one wire discovery ****
//*************************************************************************************
// void printAddress() {
// int deviceCount = sensors.getDeviceCount();
// Serial.println("1 wire Devices found ->" + String(deviceCount));
// Serial.println("Printing One Wire Addresses...");
// for (int i = 0;  i < deviceCount;  i++)
// {
// Serial.print("Sensor ");
// Serial.print(i + 1);
// Serial.print(" : ");
// sensors.getAddress(Thermo, i);
// for (uint8_t i = 0; i < 8; i++)
// {
// Serial.print("0x");
// if (Thermo[i] < 0x10) Serial.print("0");
// Serial.print(Thermo[i], HEX);
// if (i < 7) Serial.print(", ");
// }
// Serial.println("");
// }
// }

// Replaces placeholder with values
String processor(const String& var) {
  if (var == "TEMPERATURE") return String(TempCurrent);
  else if (var == "HUMIDITY") return String(HumidCurrent);
  else if (var == "SpareTemp") return String(SpareTemp);
  else if (var == "TempTarget")return String(TempTarget);
  else if (var == "HumidTarget") return String(HumidTarget);
  else if (var == "TempLowOffset") return String(TempLowOffset);
  else if (var == "HumidLowOffset") return String(HumidLowOffset);
  else if (var == "TempMinRunSeconds")  return String(TempMinRunSeconds);
  else if (var == "HumidMinRunSeconds")  return String(HumidMinRunSeconds);
  else if (var == "TempStatus") return String(TempStatus);
  else if (var == "HumidStatus")  return String(HumidStatus);
  else if (var == "TempProtectDelaySeconds") return String(TempProtectDelaySeconds);
  else if (var == "HumidProtectDelaySeconds") return String(HumidProtectDelaySeconds);
  else if (var == "TempLastOnTime") return String(TempLastOnTime);
  else if (var == "HumidLastOnTime")return String(HumidLastOnTime);
  else if (var == "TempOffTime") return String(TempOffTime);
  else if (var == "HumidOffTime") return String(HumidOffTime);
  else if (var == "HumidCycleCount") return String(HumidCycleCount);
  else if (var == "TempCycleCount") return String(TempCycleCount);
  else if (var == "Version") return String(Version);
  return String();
}

void setup() {
  pinMode(FridgePin, OUTPUT);
  pinMode(HumidPin, OUTPUT);
  digitalWrite(FridgePin, LOW);
  digitalWrite(HumidPin, LOW);

  Serial.begin(115200);
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  //Expecting Temperature|Humidity|sparetemp|TempStatus|Humidstatus|TemplastonTime|HumidlastOnTime|TempOffTime|HumidOffTime|TempcycleCount|HumidCycleCount
  server.on("/UpdateGroup", HTTP_GET, [](AsyncWebServerRequest * request) {
    double SecondsTempLastontime = TempLastOnTime;// / 60.0;
    double SecondsHumidlastontime = HumidLastOnTime / 60.0;
    double secondsTempOfftime = TempOffTime;// / 60.0;
    double SecondsHumidoffTime = HumidOffTime / 60.0;

    String TempString = String(TempCurrent) + "|" + String(HumidCurrent) + "|" + String(SpareTemp) + "|" + String(TempStatus) + "|" + String(HumidStatus) + "|" + String(SecondsTempLastontime) + "|" + String(SecondsHumidlastontime) + "|" + String(secondsTempOfftime) + "|" + String(SecondsHumidoffTime) + "|" + String(TempCycleCount) + "|" + String(HumidCycleCount);
    request->send_P(200, "text/plain", TempString.c_str());
  });

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    float inputFloat;
    if (request->hasParam("inputFloat")) {
      inputMessage = request->getParam("inputFloat")->value();
      Serial.println("Float input ->" + inputMessage);
    }
  });

  // Start server
  WebSerial.begin(&server);
  //WebSerial.msgCallback(recvMsg);

  server.begin();
  dht.begin();
  sensors.begin();
  SetupOTA();
  SetupSmoothing(SmoothingDHTTemp);
  SetupSmoothing(SmoothingDHTHumid);
  SetupSmoothing(SmoothingOWSTemp);
}

void SerialUpdate() {
  Serial.write(12);
  Serial.println("");
  Serial.println("Fridge Status   = " + String(TempStatus));
  Serial.println("Fridge Current Temp  " + String(TempCurrent));
  //  Serial.printf("Fridge Target Temp = %d\r\n", TempTarget);
  //  Serial.printf("Fridge Off Temp = %d\r\n", TempTarget - TempLowOffset);
  //  Serial.printf("Fridge Start Time  = %d\r\n", TempStartTime / 1000);
  //  Serial.printf("Fridge End Time  = %d\r\n", TempEndTime / 1000);
  //  Serial.printf("Fridge Min Run Time  = %d\r\n", TempMinRunSeconds);
  //  Serial.printf("Fridge Protect Time  = %d\r\n", TempProtectDelaySeconds);
  //  Serial.println("     Message ->" + TempText);
  Serial.println("");
  Serial.printf("Humid Status   = %d\r\n", HumidStatus);
  Serial.printf("Humid Current Humid  = %d\r\n", HumidCurrent);
  //  Serial.printf("Humid Target Humid = %d\r\n", HumidTarget);
  //  Serial.printf("Humid Off Humid = %d\r\n", HumidTarget - HumidLowOffset);
  //  Serial.printf("Humid Start Time  = %d\r\n", HumidStartTime / 1000);
  //  Serial.printf("Humid End Time  = %d\r\n", HumidEndTime / 1000);
  //  Serial.printf("Humid Min Run Time  = %d\r\n", HumidMinRunSeconds);
  //  Serial.printf("Humid Protect Time  = %d\r\n", HumidProtectDelaySeconds);
  //  Serial.println("     Message ->" + HumidText);
  //  Serial.println("");
  //  Serial.println("Spare Temperature: " + String(SpareTemp));
  //  Serial.println("");


  //printAddress(); //Look up one wire network devices
  //  Serial.printf("Left Status     = %d,%d,%d\r\n",LLightS,LLightB,LdirU);

}

void WebSerialUpdate() {
  WebSerial.println("");
  WebSerial.println("Fridge Status   = " + String(TempStatus));
  WebSerial.println("Fridge Current Temp  " + String(TempCurrent));
  WebSerial.println("Fridge Target Temp = " + String(TempTarget));
  WebSerial.println("Fridge Off Temp = " + String(TempTarget - TempLowOffset));
  WebSerial.println("Fridge Start Time  = " + String(TempStartTime / 1000));
  WebSerial.println("Fridge End Time  = " + String(TempEndTime / 1000));
  WebSerial.println("Fridge Min Run Time  = " + String(TempMinRunSeconds));
  WebSerial.println("Fridge Protect Time  = " + String(TempProtectDelaySeconds));
  WebSerial.println("     Message ->" + TempText);
  WebSerial.println("");
  WebSerial.println("Humid Status   = " + String(HumidStatus));
  WebSerial.println("Humid Current Humid  = " + String(HumidCurrent));
  WebSerial.println("Humid Target Humid = " + String(HumidTarget));
  WebSerial.println("Humid Off Humid = " + String(HumidTarget - HumidLowOffset));
  WebSerial.println("Humid Start Time  = " + String(HumidStartTime / 1000));
  WebSerial.println("Humid End Time  = " + String(HumidEndTime / 1000));
  WebSerial.println("Humid Min Run Time  = " + String(HumidMinRunSeconds));
  WebSerial.println("Humid Protect Time  = " + String(HumidProtectDelaySeconds));
  WebSerial.println("     Message ->" + HumidText);
  WebSerial.println("");
  WebSerial.println("Spare Temperature: " + String(SpareTemp));
  WebSerial.println("");
  //printAddress(); //Look up one wire network devices
  //  Serial.printf("Left Status     = %d,%d,%d\r\n",LLightS,LLightB,LdirU);

}

void Simulate() {
  if (TempStatus == 0) {
    TempCurrent = TempCurrent + .5;
  }
  else {
    TempCurrent = TempCurrent - .5;
  }
  if (HumidStatus == 0) {
    HumidCurrent = HumidCurrent + .54;
  }
  else {
    HumidCurrent = HumidCurrent - .5;
  }

}

void loop() {
  //Simulate();
  ArduinoOTA.handle();
  //WebSerial.println("READSENSORS");

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {

    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 10) {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected.\n");
      Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
      ArduinoOTA.begin();
    }
    else {
      Serial.print("Connection Failure. Delaying before next attempt");
      delay(30000);
    }
  }

  // DHT LOOP TIME
  if ((millis() - TimerDHT11) >= (7 * 1000)) {
    //WebSerialUpdate();
    SerialUpdate();
    TimerDHT11 = millis();
    readDHTHumidity();
    readDHTTemperature();

    SpareTemp = AddAndGetSmoothedValue(SmoothingDHTTemp, TempDHT);
    HumidCurrent = AddAndGetSmoothedValue(SmoothingDHTHumid, HumidDHT);

    //HumidCycleCount=HumidCycleCount+1;
  }

  if ((millis() - SlowWebserial) >= (30 * 1000)) {
    SlowWebserial = millis();
    WebSerial.println(String((millis()/1000)/60)); //Heartbeat to webterminal 30 seconds
    WebSerial.println(String(Version)); //Heartbeat to webterminal 30 seconds
  }

  //Read values in slower than full loop
  if ((millis() - LoopStartTime) >= (2 * 1000)) {
    float liveValue = 0;
    LoopStartTime = millis();
    sensors.requestTemperatures();
    liveValue = sensors.getTempF(sensor1); // Gets the values of the temperature
    TempCurrent = AddAndGetSmoothedValue(SmoothingOWSTemp, liveValue);
  }


  // Handle Temperature Loop First
  //*******************************************************************************************************************
  if (TempStatus == 0) {
    TempText = (String((millis() - TempEndTime)) + "  " + String(TempProtectDelaySeconds * 1000));
    int SecondsOff = (millis() - TempEndTime) / 1000;
    TempOffTime = SecondsOff;
    if ((millis() - TempEndTime) >= (TempProtectDelaySeconds * 1000)) {
      TempText = ("  OK To Turn on Fridge") ;  //+ String(TempCurrent) + " > " + String(TempTarget) + "  OK To Turn on Fridge");
      if (TempCurrent > TempTarget) {
        TempStatus = 1;
        TempStartTime = millis();
        digitalWrite(FridgePin, HIGH);
      }
    }
  }
  else { //Fridge is running here
    if ((millis() - TempStartTime) >= (TempMinRunSeconds * 1000)) {
      TempText = ("Fridge is On Long Enough");
      if (TempCurrent < TempTarget - TempLowOffset) {
        TempStatus = 0;
        digitalWrite(FridgePin, LOW);
        Serial.println("TURN OFF");
        TempLastOnTime = (millis() - TempStartTime) / 1000;
        TempCycleCount = TempCycleCount + 1;
      }
    }
    else {
      TempText = ("Fridge is on but Not On Long Enough ");// + String((millis() - TempStartTime)) + "  " + String(TempMinRunSeconds * 1000));
    }
    TempEndTime = millis();
  }





  //Handle Humidity Loop
  //*********************************************************************************************************************
  if (HumidStatus == 0) {
    HumidText = (String((millis() - HumidEndTime)) + "  " + String(HumidProtectDelaySeconds * 1000));
    if ((millis() - HumidEndTime) >= (HumidProtectDelaySeconds * 1000)) {
      HumidText = ("OK To Turn on Humid:"); //+ String(HumidCurrent) + " > " + String(HumidTarget));
      if (HumidCurrent > HumidTarget) {
        HumidStatus = 1;
        HumidStartTime = millis();
        digitalWrite(HumidPin, HIGH);
      }
    }
  }
  else { //Humid is running here
    if ((millis() - HumidStartTime) >= (HumidMinRunSeconds * 1000)) {
      HumidText = ("Fridge is On Long Enough");
      if (HumidCurrent < HumidTarget - HumidLowOffset) {
        HumidStatus = 0;
        digitalWrite(HumidPin, LOW);
        Serial.println("TURN OFF");
        HumidCycleCount = HumidCycleCount + 1;
      }
    }
    else {
      HumidText = ("Humid is on but Not On Long Enough "); // + String((millis() - TempStartTime)) + "  " + String(TempMinRunSeconds * 1000));
    }
    HumidEndTime = millis();
  }

  //delay(250);
}
