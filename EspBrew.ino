#include <WiFi.h>
#include <Arduino.h>

#define _DEBUG_

#include "MQTTConnector.h"
#include "Credentials.h"
#include "Scheduler.h"
#include "Recipes.h"

#define WIFI_TIMEOUT 1000

const int ENTER_BUTTON_PIN = 4;
const int ESC_BUTTON_PIN = 0;
const int BUZZ_PIN = 15;
const int PUMP_PIN = 26;
const int HEATER_PIN = 11;
Schedule newSchedule;


//////////////////////// W i F i ///////////////////////////
void WiFiBegin(const char* ssid, const char* pass)
{
  WiFi.begin(ssid, pass);
  Serial.printf("Waiting for AP connection ...\n");
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(WIFI_TIMEOUT);
    Serial.printf(".");
  }
  IPAddress ip = WiFi.localIP();
  Serial.printf("\nConnected to AP. IP : %d.%d.%d.%d\n", 
    ip[0],ip[1],ip[2],ip[3]);
}

void setup(void)
{
  // Iniciar a porta serial  
  Serial.begin(115200);

  // Iniciar o buzzer
  pinMode(BUZZ_PIN, OUTPUT);

  // Iniciar o buzzer
  pinMode(PUMP_PIN, OUTPUT);

/*
  // Conectar ao WiFi
  WiFi.begin(ssid, password);
  
  // Aguardando conexão do WiFi
  while (WiFi.status() != WL_CONNECTED)
  {
      Serial.print(".");
      delay(500);
  }
   
  Serial.println("");
  Serial.print("WiFi conectado. Endereço IP: ");
  Serial.println(WiFi.localIP());

  relogioNTPSetup();  
  sensorTemperaturaSetup();*/
  WiFiBegin(STA_SSID, STA_PASS);
  MQTTBegin();

  ScheduleSetup();
}

void loop(void)
{
  MQTTLoop();

  if (newSchedule.TotalTasks > 0)
  {
    newSchedule.Loop();    
  }

  newSchedule.DisplayStatus();
  
  //displayTemperaturaF();
  /*if (verificarBotaoResistivo(ENTER_BUTTON_PIN, true))
  {
    newSchedule.TogglePause();
  }*/

/*  if (verificarBotaoResistivo(ENTER_BUTTON_PIN, true))
  {
    digitalWrite(BUZZ_PIN, HIGH);
//    Serial.println(getLocalTime());  
//    Serial.println("Temperatura: " + getTemperatureC());  
  }
  else
  {
    digitalWrite(BUZZ_PIN, LOW);
  }*/

//  delay(2000);
}

void ScheduleSetup()
{
  MocRecipe(&newSchedule);
  newSchedule.BuzzerPin = BUZZ_PIN;
  newSchedule.PumpPin = PUMP_PIN;
  newSchedule.HeaterPin = HEATER_PIN;
  newSchedule.GreenButtonPin = ENTER_BUTTON_PIN;
  newSchedule.RedButtonPin = ESC_BUTTON_PIN;
  newSchedule.BlueLeftButtonPin = ENTER_BUTTON_PIN;
  newSchedule.BlueRightButtonPin = ENTER_BUTTON_PIN;
  setMQTTGreenButton(&newSchedule.GreenButtonPressed);
  setMQTTRedButton(&newSchedule.RedButtonPressed);
}
