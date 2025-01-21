#include <WiFi.h>
#include <MQTT.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <ArduinoJson.h>

const char *ssid = "Wokwi-GUEST";
const char *password = "";
const byte channel = 6; // WiFi channel (specific to Wokwi)
const char *mqtt_server = "host.wokwi.internal";

const int LED_red = 32, LED_green = 4;
#define DHTPIN 5 // DHT sensor pin
#define DHTTYPE DHT22

WiFiClient net;
MQTTClient client;
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMillis = 0, lastTempPublish = 0;
unsigned long lastWaterPublish = 0, lastElectricityPublish = 0;
bool state;
char msg[50];

const char *home_id = "home_1";
const char *device_id = "device_2";

void connect()
{
  Serial.print("Searching for WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.print("\nConnecting to MQTT...");
  while (!client.connect("arduino-spi", "try", "try"))
  {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("\nConnected!");

  // Construct the topic dynamically
  char topic[50];
  snprintf(topic, sizeof(topic), "/%s/%s/#", home_id, device_id);

  // Subscribe to the dynamically constructed topic
  client.subscribe(topic);

  Serial.print("Subscribed to topic: ");
  Serial.println(topic);
}

void messageReceived(String &topic, String &payload)
{
  Serial.println("Incoming: " + topic + " - " + payload);

  if (topic == String("/") + home_id + "/" + device_id + "/pump")
  {
    Serial.println("Pump command received: " + payload);
    // Handle pump commands (on/off)
    if ((char)payload[0] == '1')
    {
      digitalWrite(LED_green, LOW); // Turn the LED on (active low)
    }
    else if ((char)payload[0] == '0')
    {
      digitalWrite(LED_green, HIGH); // Turn the LED off
    }
  }

  if (topic == String("/") + home_id + "/" + device_id + "/heater")
  {
    Serial.println("Heater command received: " + payload);
    // Handle heater commands (on/off)
    if ((char)payload[0] == '1')
    {
      digitalWrite(LED_red, LOW); // Turn the LED on (active low)
    }
    else if ((char)payload[0] == '0')
    {
      digitalWrite(LED_red, HIGH); // Turn the LED off
    }
  }
}

float simulateTemperature()
{
  // Generate a random temperature between 30 and 60 degrees Celsius
  float temperature = random(3000, 6001) / 100.0;

  return temperature;
}

float simulateOutTemperature()
{
  // Generate a random temperature between 10 and 30 degrees Celsius
  float temperature = random(1000, 3001) / 100.0;

  return temperature;
}

float simulateWaterLevel()
{
  // Generate a random water level percentage between 0 and 100
  float waterLevel = random(0, 101);
  return waterLevel;
}

float simulateElectricityConsumption()
{
  // Generate a random electricity consumption value between 0 and 10 kWh
  float electricity = random(0, 1001) / 100.0;
  return electricity;
}

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, password, channel);
  pinMode(LED_red, OUTPUT);
  pinMode(LED_green, OUTPUT);

  randomSeed(digitalRead(4)); // Initialize random number generator

  state = true;

  client.begin(mqtt_server, net);
  client.onMessage(messageReceived);

  dht.begin();

  connect();

  delay(1000);
}

void loop()
{
  client.loop();

  if (!client.connected())
  {
    connect();
  }

  // Publish temperature, water level, and electricity consumption every 1 second
  if (millis() - lastTempPublish > 1000)
  {
    lastTempPublish = millis();

    /*
      float temperature = dht.readTemperature();
      snprintf(msg, 50, "%.2f", temperature);
      client.publish(String("/") + home_id + "/" + device_id + "/water_temperature", msg);
      Serial.printf("Water Temperature: %.2f Celsius\n", temperature);*/

    // Simulate water temperature
    float temperature = simulateTemperature();
    StaticJsonDocument<128> inTempDoc; // Use StaticJsonDocument with a defined size
    inTempDoc["water_temperature"] = temperature;
    char inTempPayload[128];
    serializeJson(inTempDoc, inTempPayload);
    client.publish(String("/") + home_id + "/" + device_id + "/water_temperature", inTempPayload);
    Serial.println(inTempPayload);

    // Simulate outside temperature
    float outsideTemperature = simulateOutTemperature();
    StaticJsonDocument<128> outTempDoc; // Use StaticJsonDocument with a defined size
    outTempDoc["outside_temperature"] = outsideTemperature;
    char outTempPayload[128];
    serializeJson(outTempDoc, outTempPayload);
    client.publish(String("/") + home_id + "/" + device_id + "/outside_temperature", outTempPayload);
    Serial.println(outTempPayload);

    // Simulate water level
    float waterLevel = simulateWaterLevel();
    StaticJsonDocument<128> waterLevelDoc; // Use StaticJsonDocument
    waterLevelDoc["water_level"] = waterLevel;
    char waterLevelPayload[128];
    serializeJson(waterLevelDoc, waterLevelPayload);
    client.publish(String("/") + home_id + "/" + device_id + "/water_level", waterLevelPayload);
    Serial.println(waterLevelPayload);

    // Simulate electricity consumption
    float electricity = simulateElectricityConsumption();
    StaticJsonDocument<128> electricityDoc; // Use StaticJsonDocument
    electricityDoc["electricity_consumption"] = electricity;
    char electricityPayload[128];
    serializeJson(electricityDoc, electricityPayload);
    client.publish(String("/") + home_id + "/" + device_id + "/electricity_consumption", electricityPayload);
    Serial.println(electricityPayload);
  }
}