#include <WiFi.h>
#include <PubSubClient.h>

#include "MQTTConnector.h"
#include "Credentials.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

boolean mqttInitCompleted = false;
String clientId = "EspBrewIoT";
bool *isGreenButton;
bool *isRedButton;

void setMQTTGreenButton(bool *greenButton)
{
  isGreenButton = greenButton;
    
  if (isGreenButton != NULL)
    *isGreenButton = false;  
}

void setMQTTRedButton(bool *redButton)
{
  isRedButton = redButton;  

  if (isRedButton != NULL)
    *isRedButton = false;  
}

/* Incoming data callback. */
void dataCallback(char* topic, byte* payload, unsigned int length)
{
    // Topico
  String strTopic = String((char*)topic);
  char payloadStr[length + 1];
  memset(payloadStr, 0, length + 1);
  strncpy(payloadStr, (char*)payload, length);
  String strPayload = String((char*)payloadStr);
  
  Serial.println(strTopic);
  Serial.println(strPayload);

  if (strTopic == TOPIC_GREEN_BUTTON)
  {
    if (isGreenButton != NULL)
    {
      if (strPayload == "on")
        *isGreenButton = true;
      else
        *isGreenButton = false;

      MQTTPublish(TOPIC_GREEN_BUTTON_STATUS, "off");
    }
  }

  if (strTopic == TOPIC_RED_BUTTON)
  {    
    if (isRedButton != NULL)
    {
      if (strPayload == "on")
        *isRedButton = true;
      else
        *isRedButton = false;

      MQTTPublish(TOPIC_RED_BUTTON_STATUS, "off");
    }
  }
  
  Serial.printf("Data    : dataCallback. Topic : [%s]\n", topic);
  Serial.printf("Data    : dataCallback. Payload : %s\n", payloadStr);
}

void performConnect()
{
  uint16_t connectionDelay = 5000;
  while (!mqttClient.connected())
  {
    Serial.printf("Trace   : Attempting MQTT connection...\n");
    if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_KEY))
    {
      Serial.printf("Trace   : Connected to Broker.\n");

      /* Subscription to your topic after connection was succeeded.*/
      MQTTSubscribe(TOPIC_GREEN_BUTTON);
      MQTTSubscribe(TOPIC_RED_BUTTON);
    }
    else
    {
      Serial.printf("Error!  : MQTT Connect failed, rc = %d\n", mqttClient.state());
      Serial.printf("Trace   : Trying again in %d msec.\n", connectionDelay);
      delay(connectionDelay);
    }
  }
}

boolean MQTTPublish(const char* topic, char* payload)
{
  boolean retval = false;
  if (mqttClient.connected())
  {
    retval = mqttClient.publish(topic, payload);
  }
  return retval;
}

boolean MQTTSubscribe(const char* topicToSubscribe)
{
  boolean retval = false;
  if (mqttClient.connected())
  {
    retval = mqttClient.subscribe(topicToSubscribe);
    Serial.printf("Subscribe: [%s][%d]\n", topicToSubscribe, retval);
  }
  return retval;
}

boolean MQTTIsConnected()
{
  return mqttClient.connected();
}

void MQTTBegin()
{
  mqttClient.setServer(MQTT_BROKER, MQTT_BROKER_PORT);
  mqttClient.setCallback(dataCallback);
  mqttInitCompleted = true;
}

void MQTTLoop()
{
  if(mqttInitCompleted)
  {
    if (!MQTTIsConnected())
    {
      performConnect();
    }
    mqttClient.loop();
  }
}
