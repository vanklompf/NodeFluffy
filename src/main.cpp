#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266mDNS.h>
#include <TFT_eSPI.h>
#include <SPI.h>

#include <Ticker.h>

#include "InetWeather.h"
#include "TempSensor.h"


static const int GPIO_DHT_DATA = 5;
static const int GPIO_DHT_3V = 16;
static const int GPIO_DHT_GND = 4;

static const int GPIO_LED_GREEN = 10;
static const int GPIO_LED_RED = 12;
static const int GPIO_LED_BLUE = 0;

Ticker myTimer;

static const int TEMP_ERROR = 0xFFFF;

static MDNSResponder mdns;

static TempSensor tempSensor(Serial, DHT11, GPIO_DHT_DATA, GPIO_DHT_3V, GPIO_DHT_GND);
static InetWeather inetWeather(Serial);
static Print& logger = Serial;

static TFT_eSPI tft;
static ESP8266WiFiMulti wifiMulti;


static void initWifi();
static void checkWifi();
static void initLcd();
static void initLed();

static void drawSplashscreen();
static void showLocalTemp();
static void showExternalTemp();
static void showTempScreen(const char* headline1, int temp1, const char* line3 = nullptr, const char* line4 = nullptr);


static void updateRect();
static void flipLed();

static bool isTempValid(float temp);
static bool isHumValid(float hum);

void setup(void)
{
  Serial.begin(115200);
  Serial.println("");

  initLcd();
  initLed();
  tempSensor.Init();
  initWifi();
  inetWeather.init();
  drawSplashscreen();
}

void loop(void)
{
  for (int i=0; i<100; i++)
  {
    checkWifi();

    showLocalTemp();
    updateRect();
    showExternalTemp();
    updateRect();
  }
}

void initLcd()
{
  ("initLcd()");
  tft.init();
  tft.fillScreen(ILI9163_LIGHTGREY);
  tft.setRotation(0);
  tft.setTextSize(1);

  logger.printf("TFT_WIDTH: %d\n", TFT_WIDTH);
  logger.printf("TFT_HEIGHT: %d\n", TFT_HEIGHT);
#ifdef ILI9163_DRIVER
  logger.println("Using ILI9163_DRIVER");
#endif
}

void initLed()
{
  logger.println(__PRETTY_FUNCTION__);
  pinMode(GPIO_LED_GREEN, OUTPUT);
  digitalWrite(GPIO_LED_GREEN, 1);

  pinMode(GPIO_LED_RED, OUTPUT);
  digitalWrite(GPIO_LED_RED, 1);

  pinMode(GPIO_LED_BLUE, OUTPUT);
  digitalWrite(GPIO_LED_BLUE, 1);
}

void drawSplashscreen()
{
  logger.println(__PRETTY_FUNCTION__);
  for(int32_t x=0; x<128; x++)
  {
      tft.drawFastVLine(x, 0, 128, ILI9163_BLUE);
      delay(10);
  }

  for(int32_t x=0; x<128; x++)
  {
      tft.drawFastHLine(0, x, 128, ILI9163_RED);
      delay(10);
  }
}

void initWifi()
{
  logger.println(__PRETTY_FUNCTION__);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("nowaki_Wifi", "knowak123");
  wifiMulti.addAP("gacoperek", "fedjp5cwcaZ6");
}

void checkWifi()
{
  logger.println(__PRETTY_FUNCTION__);
  static bool checked = false;

  if (!checked)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      checked = true;

      Serial.print("Connected to ");
      Serial.println(WiFi.SSID().c_str());
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      if (mdns.begin("esp8266", WiFi.localIP())) 
      {
        Serial.println("MDNS responder started");
      }        
    }
  }
}

void flipLed()
{
  static int greenState = 1;
  static int redState = 0;
  static int blueState = 1;

  greenState = 1 - greenState;
  redState = 1 - redState;
  blueState = 1 - blueState;
  digitalWrite(GPIO_LED_GREEN, greenState);
  digitalWrite(GPIO_LED_RED, redState);
  digitalWrite(GPIO_LED_BLUE, blueState);
}


void updateRect()
{
  for (uint32_t color : {ILI9163_RED, ILI9163_GREEN, ILI9163_BLUE, ILI9163_NAVY})
  {
    for(int32_t i = 0; i < 128; i++)
    {
      tft.fillRect(0, 75, i, 5, color);
      delay(10);
    }
    flipLed();
  }
}

static bool isTempValid(float temp)
{
  return ((temp > -50.0) && (temp < 50.0));
}

static bool isHumValid(float hum)
{
  return ((hum > 0.0) && (hum < 100.0));  
}

static void showTempScreen(uint32_t bgColor, const char* headline, int temp, const char* line3 = nullptr, const char* line4 = nullptr)
{
  tft.fillScreen(bgColor);
  tft.setTextColor(ILI9163_DARKGREY, bgColor);  
  tft.setCursor(0, 0, 4);
  tft.println(headline);

  tft.setTextColor(ILI9163_BLACK, bgColor);  
  tft.setTextFont(7);

  if (temp == TEMP_ERROR)
  {
    tft.print("...");
  }
  else
  {
    tft.print(temp);
  }

  tft.setTextFont(4);
  tft.print("  C");

  if (line3)
  {
    tft.setTextColor(ILI9163_DARKGREY, bgColor);
    tft.setCursor(0, 78, 4);
    tft.println(line3);
  }

  if (line4)
  {
    tft.setTextColor(ILI9163_BLACK, bgColor);
    tft.print(line4);
  }
}

static void showLocalTemp()
{
  //LOG("showLocalTemp()");

  static float oldTemp = TEMP_ERROR;
  static float oldHum;

  float temp = tempSensor.readTemp();
  float hum = tempSensor.readHum();

  //LOG("Temp:");
  //LOG(temp);
  //LOG("Hum:");
  //LOG(hum);

  temp = round(temp);
  hum = round(hum);

  if ((isTempValid(temp)) && (isHumValid(hum)))
  {
    oldTemp = temp;
    oldHum = hum;
  }
  char tbs[16];
  sprintf(tbs, "    %d %%", round(oldHum));
  showTempScreen(ILI9163_ORANGE, "Temp", (int)oldTemp, "Wilgotnosc", tbs);
}

static void showExternalTemp()
{
  //LOG("showExternalTemp()");
  showTempScreen(ILI9163_MAROON, "Temp zew.", inetWeather.getTemp(), "Sonina");  
}