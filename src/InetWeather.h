#pragma once

#include <memory>
#include <stdint.h>
#include <cstring>

class Print;
class HTTPClient;
struct JsonContainer;

class InetWeather final
{
public:
  InetWeather(Print& logger);
  ~InetWeather();
  void init();
  int getTemp();
  bool isValid() const {return m_isValid;}

  void operator=(const InetWeather&) = delete;

private:
  const int MIN_READ_INTERVAL = 300000;
  const uint32_t MAX_VALIDITY = MIN_READ_INTERVAL * 6;
  static constexpr const char* YAHOO_WEATHER_HOST = "query.yahooapis.com";
  static constexpr const char* YAHOO_WEATHER_URI = 
    "select wind,atmosphere,astronomy,item.condition,item.forecast"
    " from weather.forecast"
    " where woeid in (select woeid from geo.places(1) where text=\"krakow,pl\")"
    " and u='c'"
    " limit 1"
    "&format=json"
    "&env=store://datatables.org/alltableswithkeys";


  Print& m_logger;
  std::unique_ptr<HTTPClient> m_httpClient;
  std::unique_ptr<JsonContainer> m_jsonContainer;

  int m_lastTemp;
  uint32_t m_lastReadTime;
  bool m_isValid;
  char m_uriBuffer[std::strlen(YAHOO_WEATHER_URI) + 300];

  void fetchWeather();
  void checkValidity();
  void prepareUri();
};
