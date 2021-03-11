#ifndef ARDUINO_CREDENTIALS_H
#define ARDUINO_CREDENTIALS_H

/* WiFi Credentials to connect Internet */
#define STA_SSID "Juju"
#define STA_PASS "bellatrix"

/* Provide MQTT broker credentials as denoted in maqiatto.com. */
#define MQTT_BROKER       "maqiatto.com"
#define MQTT_BROKER_PORT  1883
#define MQTT_USERNAME     "voidmmn@gmailll.com"
#define MQTT_KEY          ":x@!vi02"


/* Provide topic as it is denoted in your topic list. 
 * For example mine is : cadominna@gmail.com/topic1
 * To add topics, see https://www.maqiatto.com/configure
 */
#define TOPIC_MESSAGE "voidmmn@gmail.com/espbrew/message"
#define TOPIC_PUMP_SWITCH "voidmmn@gmail.com/espbrew/pumpSwitch"
#define TOPIC_HEATER_SWITCH "voidmmn@gmail.com/espbrew/heaterSwitch"
#define TOPIC_TEMPERATURE_STATUS "voidmmn@gmail.com/espbrew/temperature"
#define TOPIC_TASK_TEMPERATURE_STATUS "voidmmn@gmail.com/espbrew/taskTemperature"
#define TOPIC_TASK_DURATION_STATUS "voidmmn@gmail.com/espbrew/taskDuration"
#define TOPIC_GREEN_BUTTON "voidmmn@gmail.com/espbrew/greenButton"
#define TOPIC_GREEN_BUTTON_STATUS "voidmmn@gmail.com/espbrew/greenButtonStatus"
#define TOPIC_RED_BUTTON "voidmmn@gmail.com/espbrew/redButton"
#define TOPIC_RED_BUTTON_STATUS "voidmmn@gmail.com/espbrew/redButtonStatus"
#define TOPIC_SUBSCRIBE "voidmmn@gmail.com/espbrew/#"

#endif /* ARDUINO_CREDENTIALS_H */
