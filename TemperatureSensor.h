#ifndef ESPBREW_TEMPERATURE_SENSORS_H
#define ESPBREW_TEMPERATURE_SENSORS_H

#include "OneWire.h"
#include "DallasTemperature.h"

/* temperature measurement unit */
enum temperatureUnit
{
  Celsius = 0,
  Fahrenheit = 1
};

/* Determines the time interval at each temperature sensor reading */
const int TEMPERATURE_TIME_INTERVAL = 2000;

const int I2C_INDEX_TEMPERATURE_SENSOR = 0;

const int TEMPERATURE_SENSOR_PIN = 22;

// Sensor de temperatura
OneWire oneWire(TEMPERATURE_SENSOR_PIN);
DallasTemperature tempSensor(&oneWire);

void TemperatureSensorSetup()
{
  tempSensor.begin();  
}

float GetTemperature(temperatureUnit _unit)
{
  tempSensor.requestTemperaturesByIndex(I2C_INDEX_TEMPERATURE_SENSOR);

  if (_unit == Celsius)
    return(tempSensor.getTempCByIndex(I2C_INDEX_TEMPERATURE_SENSOR));
  else
    return(tempSensor.getTempFByIndex(I2C_INDEX_TEMPERATURE_SENSOR));
}

float GetTemperatureC()
{
  tempSensor.requestTemperaturesByIndex(I2C_INDEX_TEMPERATURE_SENSOR);
  return(tempSensor.getTempCByIndex(I2C_INDEX_TEMPERATURE_SENSOR));
}

float GetTemperatureF()
{
  tempSensor.requestTemperaturesByIndex(I2C_INDEX_TEMPERATURE_SENSOR);
  return(tempSensor.getTempCByIndex(I2C_INDEX_TEMPERATURE_SENSOR));
}

#endif /* ESPBREW_TEMPERATURE_SENSORS_H */
