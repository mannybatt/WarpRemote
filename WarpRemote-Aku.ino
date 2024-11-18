



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

  int choice = random(1,8);
  switch (choice) {
    case 1: irsend.sendNEC(16195807, 32); //red
    break;
    case 2: irsend.sendNEC(16228447, 32); //green
    break;
    case 3: irsend.sendNEC(16212127, 32); // blue
    break;
    case 4: irsend.sendNEC(16208047, 32); //indigo
    break;
    case 5: irsend.sendNEC(16199887, 32); //gold
    break;
    case 6: irsend.sendNEC(16232527, 32); //cyan
    break;
    case 7: irsend.sendNEC(16244767, 32); //white
    break;
    case 8: irsend.sendNEC(16216207, 32); //purple
    break;
  }
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

    //Power off - Aku
    if (value == 0) {
      irsend.sendNEC(16203967, 32);
    }
    //Power on - Aku
    if (value == 1) {
      irsend.sendNEC(16236607, 32);
    }
    //Red
    if (value == 2) {
      irsend.sendNEC(16195807, 32);
    }
    //Green
    if (value == 3) {
      irsend.sendNEC(16228447, 32);
    }
    //Blue
    if (value == 4) {
      irsend.sendNEC(16212127, 32);
    }
    //Indigo
    if (value == 5) {
      irsend.sendNEC(16208047, 32);
    }
    //Gold
    if (value == 6) {
      irsend.sendNEC(16199887, 32);
    }
    //Cyan
    if (value == 7) {
      irsend.sendNEC(16232527, 32);
    }
    //White
    if (value == 8) {
      irsend.sendNEC(16244767, 32);
    }
    //Flow
    if (value == 9) {
      irsend.sendNEC(16246807, 32);
    }
    //Purple
    if (value == 10) {
      irsend.sendNEC(16216207, 32);
    }
    //Brightness Up - Aku
    if (value == 11) {
      irsend.sendNEC(16187647, 32);
    }
    //Brightness Down - Aku
    if (value == 12) {
      irsend.sendNEC(16220287, 32);
    }
    if (value == 999) {
      //Do Nothing
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
  
  irsend.sendNEC(16236607, 32); //Aku
  delay(350);
}

void nox() {
  
  irsend.sendNEC(16769565, 32);
  delay(350);
}

void startLights() {
  
  lumos();
  delay(350);
  irsend.sendNEC(16756815, 32);
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
  ArduinoOTA.setHostname("WarpRemote-Aku");                                                          /** TODO **/
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
