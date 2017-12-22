#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>

#include <DHT.h>
#include <TFT_eSPI.h>
#include <SPI.h>

const int GPIO_LED = 5;
const int GPIO_DHT = 4;
const int GPIO_BTN = 16;

const char* ssid = "gacoperek";
const char* password = "fedjp5cwcaZ6";

MDNSResponder mdns;
DHT dht(GPIO_DHT, DHT11);
TFT_eSPI tft = TFT_eSPI(128, 128);  

void setup(void)
{  
  //Init GPIO
  pinMode(GPIO_LED, OUTPUT);
  pinMode(GPIO_BTN, INPUT_PULLUP);

  //Init serial
  Serial.begin(115200);
  Serial.println("");

  //Init DHT
  dht.begin();

  //Init LCD
  tft.init();
  tft.setRotation(1);
  tft.setTextSize(1);

  Serial.print("TFT_WIDTH: ");
  Serial.println(TFT_WIDTH);

  Serial.print("TFT_HEIGHT: ");
  Serial.println(TFT_HEIGHT);
#ifdef ILI9163_DRIVER
    Serial.println("Using ILI9163_DRIVER");
#endif
  

/*
  //Init WiFi
  WiFi.begin(ssid, password);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
*/

  for(int32_t x=0; x<128; x++)
  {
      tft.drawFastVLine(x, 0, 128, 0xF81F);
      delay(5);
  }

  for(int32_t x=0; x<128; x++)
  {
      tft.drawFastHLine(0, x, 128, 0x001F);
      delay(5);
  }
}


void updateRect(void)
{
  static int32_t rect_x = 0;
  static int32_t rect_y = 0;
  //static uint8_t rotation = 0;

  //tft.setRotation(rotation);
  //Serial.print("rotation: ");
  //Serial.println(rotation);
  //rotation++;

  tft.setRotation(1);
  tft.fillScreen(0x07E0);
  for(int32_t i = 0; i < 10; i++)
  {
    tft.fillRect(rect_x % 128, rect_y % 128, 15, 15, 0x07E0);
    rect_x+=2;
    rect_y++;
    tft.fillRect(rect_x % 128, rect_y % 128, 15, 15, 0xFD20);
    delay(20);
  }

  tft.setTextColor(TFT_WHITE,TFT_BLACK);  
  tft.setCursor(0, 0, 1);
  tft.print("x: ");
  tft.println(rect_x); 
  tft.print("y: ");
  tft.println(rect_y); 
}


void updateLed(void)
{
  static uint8_t ledState = 0;

  digitalWrite(GPIO_LED, ledState);
  ledState = 1 - ledState;
}


void loop(void)
{
  updateRect();
  updateLed();

  Serial.print("Temp: ");
  Serial.print(dht.readTemperature());
  Serial.print("\tHum: ");
  Serial.println(dht.readHumidity());

  Serial.print("Btn: ");
  Serial.println(digitalRead(GPIO_BTN));

  delay(1800);
}