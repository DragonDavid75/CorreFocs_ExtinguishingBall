/*
  Rui Santos
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp32-http-get-post-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

const char* ssid = "WaycoPass";
const char* password = "3nj0y-d4y";

//Your Domain name with URL path or IP address with path
const char* serverName = "http://192.168.10.84:5000/webhook";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 2.5 seconds (2500)
unsigned long timerDelay = 2500;

// Variables sensor de CO2
const int smokePin = 34;
int adc_MQ;
float voltaje;
float Rs;
double CO2;


// Variables sensor de humedad y temperatura
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
float humidity;
float temperature;
float dtemp;

const String id = "BME281";
String s = "Full";

// Variables de activacion
const unsigned int activationPin = 19;
const unsigned int buzzerPin = 21;

void setup() {
  pinMode(activationPin, OUTPUT);
  digitalWrite(activationPin, LOW);  
  
  pinMode(buzzerPin, OUTPUT);
  
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");

  dht.begin(); 
  temperature = dht.readTemperature();
}

void loop() {
  //Obtener data del sensor y calcular valor
  delay(500);
  adc_MQ = analogRead(smokePin); //Lemos la salida analógica  del MQ
  voltaje = adc_MQ * (5.0 / 1023.0); //Convertimos la lectura en un valor de voltaje
  Rs=1000*((5-voltaje)/voltaje);  //Calculamos Rs con un RL de 1k
  CO2=0.4091*pow(Rs/5463, -1.497); // calculamos la concentración  de alcohol con la ecuación obtenida.

  
  humidity = dht.readHumidity();
  dtemp = dht.readTemperature() - temperature; 
  temperature = dht.readTemperature();
  
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  if (isnan(CO2)) {
    Serial.println(F("Failed to read from CO2 sensor!"));
    return;
  }
  Serial.print(F("CO2: "));
  Serial.print(CO2);
  Serial.print(F(" Humedad: "));
  Serial.print(humidity);
  Serial.print(F("% Temperatura: "));
  Serial.print(temperature);
  Serial.println(F("°C "));

  if(CO2 > 10000 || dtemp > 0.3){
    activate();
  }
  
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    httpPost();
    lastTime = millis();
  }
}

void httpPost(){
  //Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;
    
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
            
      // If you need an HTTP request with a content type: application/json, use the following:
      http.addHeader("Content-Type", "application/json");
      int httpResponseCode = http.POST("{\"api_key\":\"tPmAT5Ab3j7F9\",\"id\":\"BME281\",\"latitude\":\" 39.4549137\",\"longitude\":\"-0.3277921\",\"temperature\":\""+String(temperature)+"\",\"humidity\":\""+String(humidity)+"\",\"CO2\":\""+String(CO2)+"\",\"status\":\""+s+"\"}");

      // If you need an HTTP request with a content type: text/plain
      //http.addHeader("Content-Type", "text/plain");
      //int httpResponseCode = http.POST("Hello, World!");
     
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);

      switch(httpResponseCode){
          case 104:
              activate();
              break;
          case 105 :
              makeSound();
              break;
          default:
              noAction();
      }        
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}

void activate(){
  s = "Empty";
  digitalWrite(activationPin, HIGH);  
}
void makeSound(){
  digitalWrite(buzzerPin, HIGH);    
}

void noAction(){
  digitalWrite(buzzerPin, LOW);  
}
