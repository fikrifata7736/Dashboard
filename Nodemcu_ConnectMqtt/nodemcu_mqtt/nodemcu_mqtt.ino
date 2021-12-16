
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#define ONE_WIRE_BUS D1
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire); 

int fan = D2;
int lamp = D4;

const int ph_Pin  = A0;
float Po = 0;
float PH_step;
int nilai_analog_PH;
double TeganganPh;
float PH4 = 3.226;
float PH7 = 2.691;

// Update these with values suitable for your network.

const char* ssid = "Hanif Phones ";
const char* password = "sesemele";
const char* mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(fan,OUTPUT);
  pinMode(lamp,OUTPUT);
  pinMode (ph_Pin, INPUT);
  sensors.begin();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
  sensors.requestTemperatures();                
  nilai_analog_PH = analogRead(ph_Pin);
  TeganganPh = 3.3 / 1024.0 * nilai_analog_PH;
  PH_step = (PH4 - PH7) / 3;
  Po = 7.00 + ((PH7 - TeganganPh) / PH_step);
  
  Serial.print("Nilai ADC Ph: ");
  Serial.println(nilai_analog_PH);
  Serial.print("TeganganPh: ");
  Serial.println(TeganganPh, 3);
  Serial.print("Temperature: ");
  Serial.print(sensors.getTempCByIndex(0));
  Serial.println("°C");
  Serial.print("Nilai PH cairan: ");
  Serial.println(Po, 2);

  if(sensors.getTempCByIndex(0)>28){
     digitalWrite(fan,HIGH);
     digitalWrite(lamp,LOW);
     Serial.print("Fan nyala");
  }else{
    digitalWrite(fan,LOW);
    digitalWrite(lamp,HIGH);
    Serial.println("lampu nyala");
    }
 
  delay(3000);
}
