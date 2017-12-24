#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <initializer_list>

static const int GPIO_DHT_DATA = 5;
static const int GPIO_DHT_3V = 16;
static const int GPIO_DHT_GND = 4;

static const char* SSID = "Orange-8CC1";
static const char* PASSWORD = "Sonina473b";

static const int TEMP_ERROR = 0xFFFF;

static MDNSResponder mdns;
static DHT dht(GPIO_DHT_DATA, DHT11); 
static TFT_eSPI tft;

static int externalTemp = TEMP_ERROR;

static void initDht(void);
static void initWifi(void);
static void checkWifi(void);
static void initLcd(void);
static void drawSplashscreen(void);
static void showLocalTemp(void);
static void showExternalTemp(void);
static void showTempScreen(const char* headline1, int temp1, const char* line3 = nullptr, const char* line4 = nullptr);


static int getExternalTemp(void);


static void updateRect(void);

static bool isTempValid(float temp);
static bool isHumValid(float hum);

void setup(void)
{
  Serial.begin(115200);
  Serial.println("");

  initLcd();
  initDht();
  initWifi();
  drawSplashscreen();
}

void loop(void)
{
  for (int i=0; i<100; i++)
  {
    checkWifi();

    showLocalTemp();
    updateRect();

    if (externalTemp == TEMP_ERROR)
    {
      externalTemp = getExternalTemp();
    }
    showExternalTemp();
    updateRect();
  }
  externalTemp = TEMP_ERROR;
}

void initLcd(void)
{
  tft.init();
  tft.fillScreen(ILI9163_LIGHTGREY);
  tft.setRotation(2);
  tft.setTextSize(1);

  Serial.print("TFT_WIDTH: ");
  Serial.println(TFT_WIDTH);

  Serial.print("TFT_HEIGHT: ");
  Serial.println(TFT_HEIGHT);
#ifdef ILI9163_DRIVER
    Serial.println("Using ILI9163_DRIVER");
#endif
}

void initDht(void)
{
  pinMode(GPIO_DHT_3V, OUTPUT);
  digitalWrite(GPIO_DHT_3V, 1);
  pinMode(GPIO_DHT_GND, OUTPUT);
  digitalWrite(GPIO_DHT_GND, 0);

  dht.begin();
}

void drawSplashscreen(void)
{
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

void initWifi(void)
{
  WiFi.begin(SSID, PASSWORD);
}

void checkWifi(void)
{
  static bool checked = false;

  if (!checked)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      checked = true;

      Serial.print("Connected to ");
      Serial.println(SSID);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());

      if (mdns.begin("esp8266", WiFi.localIP())) 
      {
        Serial.println("MDNS responder started");
      }        
    }
  }
}

void updateRect(void)
{
  for (uint32_t color : {ILI9163_RED, ILI9163_GREEN, ILI9163_BLUE, ILI9163_NAVY})
  {
    for(int32_t i = 0; i < 128; i++)
    {
      tft.fillRect(0, 75, i, 5, color);
      delay(10);
    }
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
    tft.setCursor(0, 80, 4);
    tft.println(line3);
  }

  if (line4)
  {
    tft.setTextColor(ILI9163_BLACK, bgColor);
    tft.print(line4);
  }
}

static void showLocalTemp(void)
{
  static float oldTemp;
  static float oldHum;

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  Serial.print("Temp:");
  Serial.println(temp);
  Serial.print("Hum:");
  Serial.println(hum);

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

static void showExternalTemp(void)
{
  showTempScreen(ILI9163_CYAN, "Temp zew.", externalTemp, "Sonina");  
}

static HTTPClient http;
static DynamicJsonBuffer jsonBuffer(4*JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(4));
static const char* url = 
"http://query.yahooapis.com/v1/public/yql?q=select%20item.condition%20from%20weather.forecast%20where%20woeid%3D519340%20and%20u%3D'c'&format=json&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys";
static int getExternalTemp(void)
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("showExternalTemp");
    http.begin(url);
    //http.setTimeout(2000);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.GET();
    Serial.printf("Http Code: %d\n", httpCode);

    JsonObject& root = jsonBuffer.parseObject(http.getString());
    if (!root.success()) 
    {
      Serial.println("Parsing failed!");
      return TEMP_ERROR;
    }
    const char* temp = root["query"]["results"]["channel"]["item"]["condition"]["temp"];
    
    http.end();

    Serial.print("Json temp: ");  
    Serial.println(temp);

    return atoi(temp);
  }
  else
  {
    return TEMP_ERROR;
  }
}

