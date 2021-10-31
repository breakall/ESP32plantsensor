#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <DebugUtils.h>
// Wifi

#define WLAN_SSID "reallyslowwifi"
#define WLAN_PASS "88888888"

// MQTT Broker
const char *mqtt_broker = "192.168.1.2";
const char *temp_topic = "plant1/temperature";
const char *humidity_topic = "plant1/humidity";
const char *soil_moisture_topic = "plant1/soil_moisture";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);


// DHT22 temperature sensor 

#define DHTPIN 0     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);


// soil moisture sensor

const int AirValue = 3620;   //you need to replace this value with Value_1
const int WaterValue = 1680;  //you need to replace this value with Value_2
const int SensorPin = 34;
int soilMoistureValue = 1;
int soilmoisturepercent=0;

// for receving messages via MQTT
void callback(char *topic, byte *payload, unsigned int length) {
 dbg.print("Message arrived in topic: ");
 dbg.println(topic);
 dbg.print("Message:");
 for (int i = 0; i < length; i++) {
     dbg.print((char) payload[i]);
 }
 dbg.println();
 dbg.println("-----------------------");
}

#pragma region OTA_stuff
// keep track of whether an ota update has started so we can skip other blocking logic while it's in progress
bool is_ota_updating = false;

// configure the various events and stuff needed for arduino OTA (wifi) flashing
void setupOTA() {
  // port it'll listen for OTA update invitations on
  ArduinoOTA.setPort(3232);

  // reboot after a successful update, otherwise you'll have to manually reset it!
  ArduinoOTA.setRebootOnSuccess(true);

  // bunch of event handlers for the various start/stop/end/progress/etc. events of an OTA update
  ArduinoOTA
    .onStart([]() {
      dbg.println("Start updating");
      is_ota_updating = true;
    })
    .onEnd([]() {
      dbg.println("Update End");
      is_ota_updating = false;
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      dbg.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      is_ota_updating = false;
      dbg.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) dbg.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) dbg.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) dbg.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) dbg.println("Receive Failed");
      else if (error == OTA_END_ERROR) dbg.println("End Failed");
    });
}

#pragma endregion OTA_stuff

void setup() {
  Serial.begin(9600);
  setupOTA();
  dht.begin();

  // connecting to a WiFi network
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     dbg.println(".");
  }
  // call the debug helper to tell it wifi is ready and it can set up the UDP sending stuff
  dbg.wifiIsReady();
  dbg.println("Connected to the WiFi network");
  dbg.printf("IP address is: %s\n", WiFi.localIP().toString().c_str());

  // now that wifi is connected, spin up the OTA wifi flashing stuff
  ArduinoOTA.begin();

  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  
  while (!client.connected()) { 
    if (!is_ota_updating) {
      String client_id = "esp32-client-";
      client_id += String(WiFi.macAddress());
      dbg.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
      if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
          dbg.println("Connect to MQTT broker");
      } else {
          dbg.printf("failed with state %d\n", client.state());
          delay(2000);
      }
    }
  }
  // initialize moisture sensor pin maybe?
  soilMoistureValue = analogRead(SensorPin);
}

// Keeps track of the last time (# of milliseconds returned by millis()) a reading was taken so we know when to read again
unsigned long lastRead = millis();

void loop() {
  // handle events for arduino OTA wifi flashing
  ArduinoOTA.handle();

  // If the current millis() timestamp minus the last time we read is a difference of > 5000 (5 sec), do another read
  // this is in place of delay(5000) since the ArduinoOTA.handle() needs to be called very frequently
  if ((millis() - lastRead) > 5000 && !is_ota_updating) {

    // ############# TEMP/HUMIDITY SENSOR ############# 

    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      dbg.println(F("Failed to read from DHT sensor!"));
    } else {

      // Compute heat index in Fahrenheit (the default)
      float hif = dht.computeHeatIndex(f, h);
      // Compute heat index in Celsius (isFahreheit = false)
      float hic = dht.computeHeatIndex(t, h, false);

      // pub to mqtt
      char msgBuffer[20];
      client.publish(temp_topic, dtostrf(f, 5, 2, msgBuffer));
      client.publish(humidity_topic, dtostrf(h, 5, 2, msgBuffer));

      // console logging
      dbg.print(F("Humidity: "));
      dbg.print(h);
      dbg.print(F("%  Temperature: "));
      dbg.print(t);
      dbg.print(F("°C "));
      dbg.print(f);
      dbg.print(F("°F  Heat index: "));
      dbg.print(hic);
      dbg.print(F("°C "));
      dbg.print(hif);
      dbg.println(F("°F"));
    }


    // ######## soil moisture sensor ########

    soilMoistureValue = analogRead(SensorPin);  //put Sensor insert into soil
    soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);

    // print reading to console
    dbg.print("Raw moisture value: ");
    dbg.print(soilMoistureValue);
    dbg.println("%");

    dbg.print("Mapped moisture value: ");
    dbg.print(soilmoisturepercent);
    dbg.println("%");


    char cstr[16];
    itoa(soilmoisturepercent, cstr, 10);


    client.publish(soil_moisture_topic, cstr);


    // Update the lastRead millisecond stamp 
    lastRead = millis();
  }


}





