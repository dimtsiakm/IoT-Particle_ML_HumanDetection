/* 
 * Project IoT_HUMAN_DETECTION
 * Author: Dimitrios Tsiakmakis
 * Date: 
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// This #include statement was automatically added manually
#include "Adafruit_DHT_Particle.h"
#include <MQTT.h>
#include "Particle.h"
#include <Grove_ChainableLED.h>

const char* broker =  "192.168.10.194";
int port = 1883;

void mqttCallback(char* topic, byte* payload, unsigned int length);
void publishMessage(char* topic, char* message);
void checkMQTTConnection(void);

char* StringToChar(String Message) {
    int LEN = Message.length() + 1;
    char* payload = new char[LEN];
    Message.toCharArray(payload, LEN);
    return payload;
}

MQTT client(broker, port, mqttCallback);
bool mqttConnected = false;

#define DHTPIN D2       // Connection pin
#define DHTTYPE DHT11   // DHT 11

#define NUM_LEDS 1
ChainableLED leds(D4, D5, NUM_LEDS);

DHT dht(DHTPIN, DHTTYPE);

double Humidity = 0;
double Temperature = 0;
double DewPoint = 0;
double HeatIndex = 0;

void async_ReadHumidityTemp();
void async_sendData();

Thread thread_readhumtemp("readhumtemp", async_ReadHumidityTemp);
Thread thread_senddata("senddata", async_sendData);

int Fire_LED(String State) {
    // Check if string is empty
    String VAR = (State.c_str() ? State : "NULL");
    // Set the Status of the LED
    if (State == "0") {
        leds.setColorRGB(0, 0, 0, 0);
        return 1;
    } else if (State == "1") {
        leds.setColorRGB(0, 255, 255, 255);
        return 1;
    } 
    else if (State == "NULL"){
      leds.setColorRGB(0, 255, 0, 255);
      return 1;
    }else {
        return -1;
    }
}

void SerialPrintData() {
    Serial.printf("Humidity: %3.0f %% -- ", Humidity);
    Serial.printf("Temperature: %4.0f °C -- ", Temperature);
    Serial.printf("Dew Point: %6.2f °C -- ", DewPoint);
    Serial.printf("Heat Index: %6.2f °C -- ", HeatIndex);
    Serial.printf("%s\n", Time.timeStr().c_str());
}

void setup() {
    Serial.begin(9600);
    dht.begin();
    leds.init();
    leds.setColorRGB(0, 0, 255, 0);
    delay(400);
    checkMQTTConnection();
}

void loop() {
    // Publish Data to MQTT Broker
    if (client.isConnected()) {
        // Manage the async function calling
        client.loop();
    } else {
        // Try to reconnect in case of failure
        checkMQTTConnection();
    }
    delay(10);
}

void checkMQTTConnection() {
  if (!client.isConnected() && !mqttConnected) {
      String Name = "Sparkclient_" + String(Time.now());
      mqttConnected = client.connect(Name);
      if (mqttConnected) {
          Serial.println("Connected to MQTT broker!");
          client.subscribe("isHuman");
      }
  } else if (!client.isConnected() && mqttConnected) {
      Serial.println("Connection lost to MQTT broker!");
      mqttConnected = false;
  } 
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Reading and sending message over serial port
    Serial.printf("Message from PYTHON received in topic %s: ", payload);
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.printf("Message received in topic %s: ", topic);
    Serial.println(message);

    Fire_LED(message);
}

void publishMessage(char* topic, char* message) {
  // Function for publishing the message in MQTT
    if (client.isConnected()) {client.publish(topic, message);};
}

void async_ReadHumidityTemp() {
    while (true) {
      double hum = dht.getHumidity();
      double temp = dht.getTempCelcius();

      if (isnan(hum) || isnan(temp)) {
        Serial.println("Failed to read from DHT sensor!");
        continue;
      }

      Humidity = hum;
      Temperature = temp;
      DewPoint = dht.getDewPoint();
      HeatIndex = dht.getHeatIndex();
      delay(1000);
    }
}

void async_sendData() {
  while (true) {
    if (client.isConnected()) {
        publishMessage("Humidity", StringToChar(String(Humidity)));
        publishMessage("Temperature", StringToChar(String(Temperature)));
        publishMessage("DewPoint", StringToChar(String(DewPoint)));
        publishMessage("HeatIndex", StringToChar(String(HeatIndex)));
    }
    Serial.printf("message sent to MQTT broker: %s\n", Time.timeStr().c_str());
    SerialPrintData();
    delay(500);
  }
}

// particle compile argon DHT11.cpp Adafruit_DHT_Particle.cpp Adafruit_DHT_Particle.h
// particle flash DEV1 argon_firmware_XXXXXXXXXXXXX.bin
// particle flash DEV1 DHT11.cpp Adafruit_DHT_Particle.cpp Adafruit_DHT_Particle.h
// particle serial monitor --follow