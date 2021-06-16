#include <Arduino.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DHTTYPE DHT11

#define DINAMO_CW D2
#define DINAMO_CCW D3
#define ENABLE_DINAMO D4

#define RAIN_SENSOR_A D5
#define RAIN_SENSOR_D D6

DHT dht(D7, DHTTYPE);

const char *ssid = "IT";
const char *password = "dirumahaja";

// const char *mqtt_server = "ec2-100-26-40-148.compute-1.amazonaws.com";
const char *mqtt_server = "ec2-54-147-187-160.compute-1.amazonaws.com";

WiFiClient espClient;
PubSubClient client(espClient);

long now = millis();
long lastMeasure = 0;

void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(ENABLE_DINAMO,OUTPUT);
  pinMode(DINAMO_CW,OUTPUT);
  pinMode(DINAMO_CCW,OUTPUT);
  pinMode(RAIN_SENSOR_A,INPUT);
  pinMode(RAIN_SENSOR_D,INPUT);
  
  digitalWrite(DINAMO_CW,LOW);
  digitalWrite(DINAMO_CCW,LOW);
  dht.begin();
  Serial.println("Mqtt Node-RED");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected())
  {
    reconnect();
  }
  if (!client.loop())
  {
    client.connect("ESP8266Client");
  }
  now = millis();
  if (now - lastMeasure > 5000)
  {
    lastMeasure = now;

    int celcius = dht.readTemperature();
    int humidity = dht.readHumidity();

    if(digitalRead(RAIN_SENSOR_D) == LOW){
      Serial.println("Mendung/Hujan");
      analogWrite(ENABLE_DINAMO,255);
      digitalWrite(DINAMO_CW,HIGH);
      digitalWrite(DINAMO_CCW,LOW);
      Serial.println("Dinamo Nyala Searah Jarum Jam");
      delay(3000);
      digitalWrite(DINAMO_CW,LOW);
      digitalWrite(DINAMO_CCW,LOW);
      Serial.println("Dinamo Mati");
      Serial.println();
      delay(1000);
    }else{
      Serial.println("Cerah");
      analogWrite(ENABLE_DINAMO,255);
      digitalWrite(DINAMO_CW,LOW);
      digitalWrite(DINAMO_CCW,HIGH);
      Serial.println("Dinamo Nyala Berlawanan Jarum Jam");
      delay(3000);
      digitalWrite(DINAMO_CW,LOW);
      digitalWrite(DINAMO_CCW,LOW);
      Serial.println("Dinamo Mati");
      Serial.println();
      delay(1000);
    }

    static char temperatureTemp[7];
    dtostrf(celcius, 4, 2, temperatureTemp);
    
    static char humidityHum[7];
    dtostrf(humidity, 4, 2, humidityHum);

    static char rainSensor[7];
    dtostrf(digitalRead(RAIN_SENSOR_D), 4, 2, rainSensor);

    static char rainAnalog[7];
    dtostrf(digitalRead(RAIN_SENSOR_A), 4, 2, rainAnalog);

    client.publish("room/temp", temperatureTemp);
    client.publish("room/hum", humidityHum);
    client.publish("room/rain", rainSensor);
    client.publish("room/rainAnalog", rainAnalog);

    Serial.print("Suhu : ");
    Serial.print(celcius);
    Serial.println(" °C");

    Serial.print("Kelembaban : ");
    Serial.print(humidity);
    Serial.println(" %");

  }
}