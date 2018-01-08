#include "TempSensor.h"
#include <DHT.h>

TempSensor::TempSensor(Print& logger, uint8_t type, uint8_t dataPin) 
: m_logger(logger),
  m_dht(dataPin, type),
  m_isGpioPowered(false),
  m_vccPin(0xFF),
  m_gndPin(0xFF)
{
}

TempSensor::TempSensor(Print& logger, uint8_t type, uint8_t dataPin, uint8_t vccPin, uint8_t gndPin)
: m_logger(logger),
  m_dht(dataPin, type),
  m_isGpioPowered(true),
  m_vccPin(vccPin),
  m_gndPin(gndPin)
{
}

void TempSensor::Init()
{
  m_logger.println("TempSensor::Init()");
  if (m_isGpioPowered)
  {
    pinMode(m_vccPin, OUTPUT);
    digitalWrite(m_vccPin, 1);
    pinMode(m_gndPin, OUTPUT);
    digitalWrite(m_gndPin, 0);
  }
  m_dht.begin();
}

void TempSensor::readSensor()
{
    m_logger.println(__PRETTY_FUNCTION__);
    float currentTemp = m_dht.readTemperature();
    float currentHum = m_dht.readHumidity();
    uint32_t now = millis();

    if (!isnan(currentTemp) && !isnan(currentHum))
    {
      m_lastTemp = currentTemp;
      m_lastHum = currentHum;
      m_lastReadTime = now;
    }
    else if (now - m_lastReadTime > MAX_VALIDITY)
    {
      m_lastTemp = currentTemp;
      m_lastHum = currentHum;
    }
}

void TempSensor::checkValidity()
{
  if (millis() - m_lastReadTime > MIN_READ_INTERVAL)
  {
    readSensor();
  }
}

float TempSensor::readTemp()
{
  checkValidity();
  return m_lastTemp;
}

float TempSensor::readHum()
{
  checkValidity();
  return m_lastHum;
}