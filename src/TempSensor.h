#pragma once

#include <memory>
#include <stdint.h>
#include <DHT.h>

class TempSensor final
{
public:
  TempSensor(Print& logger, uint8_t type, uint8_t dataPin);
  TempSensor(Print& logger, uint8_t type, uint8_t dataPin, uint8_t vccPin, uint8_t gndPin);
  void Init();

  float readTemp();
  float readHum();

private:
  void readSensor();
  void checkValidity();

  const uint32_t MIN_READ_INTERVAL = 10000;
  const uint32_t MAX_VALIDITY = 100000;

  Print& m_logger;
  DHT m_dht;
  const bool m_isGpioPowered;
  const uint8_t m_vccPin;
  const uint8_t m_gndPin;

  uint32_t m_lastReadTime;
  float m_lastTemp;
  float m_lastHum;
};
