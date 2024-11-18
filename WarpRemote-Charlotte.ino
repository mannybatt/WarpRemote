



// ***************************************
// ********** Global Variables ***********
// ***************************************


//Globals for Wifi Setup and OTA
#include <credentials.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//WiFi Credentials
#ifndef STASSID
#define STASSID "your_ssid"
#endif
#ifndef STAPSK
#define STAPSK  "your_password"
#endif
const char* ssid = STASSID;
const char* password = STAPSK;

//MQTT
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#ifndef AIO_SERVER
#define AIO_SERVER      "your_MQTT_server_address"
#endif
#ifndef AIO_SERVERPORT
#define AIO_SERVERPORT  0000 //Your MQTT port
#endif
#ifndef AIO_USERNAME
#define AIO_USERNAME    "your_MQTT_username"
#endif
#ifndef AIO_KEY
#define AIO_KEY         "your_MQTT_key"
#endif
#define MQTT_KEEP_ALIVE 150
unsigned long previousTime;
float mqttConnectFlag = 0.0;

//IR
#include <IRremoteESP8266.h>
#include <IRsend.h>
const uint16_t kIrLed = D2; 
IRsend irsend(kIrLed);

//MQTT Startup
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe warpRemote = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/WarpRemote");
Adafruit_MQTT_Publish pingMaster = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/PingMaster");




// ***************************************
// *************** Setup *****************
// ***************************************


void setup() {

  //Initialize IR
  irsend.begin();

  //Initialize Serial
  Serial.begin(115200);
  Serial.println("Booting");

  //Wait and Turn on Lights
  delay(5000);
  lumos();
  delay(1500);
  irsend.sendNEC(16756815, 32);
  Serial.println("Let there be light!");

  //WiFi Initialization
  wifiSetup();

  //Initialize MQTT
  mqtt.subscribe(&warpRemote);
  MQTT_connect();
  delay(1000);
  pingMaster.publish(1);
  
}




// ***************************************
// ************* Da Loop *****************
// ***************************************


void loop() {

  //Network Housekeeping
  ArduinoOTA.handle();
  MQTT_connect();

  //IR State Manager
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(10))) {
    Serial.println("Subscription Recieved");
    uint16_t value = atoi((char *)warpRemote.lastread);
    Serial.println(value);
    delay(500);

    //Power on - Charlotte
    if (value == 101) {
      irsend.sendNEC(16753245, 32);
    }
    //1
    if (value == 102) {
      irsend.sendNEC(16720605, 32);
    }
    //2
    if (value == 103) {
      irsend.sendNEC(16761405, 32);
    }
    //3
    if (value == 104) {
      irsend.sendNEC(16769055, 32);
    }
    //4
    if (value == 105) {
      irsend.sendNEC(16748655, 32);
    }
    //5
    if (value == 106) {
      irsend.sendNEC(16738455, 32);
    }
    //6
    if (value == 107) {
      irsend.sendNEC(16756815, 32);
    }
    //7
    if (value == 108) {
      irsend.sendNEC(16724175, 32);
    }
    //8
    if (value == 109) {
      irsend.sendNEC(16743045, 32);
    }
    //Brightness Up - Charlotte
    if (value == 110) {
      irsend.sendNEC(16187647, 32);
    }
    //Brightness Down - Charlotte
    if (value == 111) {
      irsend.sendNEC(16220287, 32);
    }
    if (value == 999) {
      //Do Nothing
    }

    //Both Off
    if (value == 200) {
      irsend.sendNEC(16769565, 32);
      delay(500);
      irsend.sendNEC(16203967, 32);
    }
    //Both On
    if (value == 201) {
      irsend.sendNEC(16236607, 32);
      delay(500);
      irsend.sendNEC(16753245, 32);
    }
    delay(250);
  }
  delay(1);

  if((millis() - previousTime) > 120 * 1000) {
    previousTime = millis();
    pingMaster.publish(1);
  }
}




// ***************************************
// ********** Backbone Methods ***********
// ***************************************


void lumos() {

  delay(350);
  irsend.sendNEC(16753245, 32); //Charlotte
}

void nox() {

  delay(350);
  irsend.sendNEC(16203967, 32);
}

void startLights() {

  lumos();
  delay(350);
}

void MQTT_connect() {

  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    if (mqttConnectFlag == 0) {
      //Serial.println("Connected");
      mqttConnectFlag++;
    }
    return;
  }
  Serial.println("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      //while (1);
      Serial.println("Wait 5 secomds to reconnect");
      delay(5000);
    }
  }
}

void wifiSetup() {

  //Serial
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println();
  Serial.println("****************************************");
  Serial.println("Booting");

  //WiFi and OTA
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.setHostname("WarpRemote-Charlotte");                                                          /** TODO **/
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
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
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
