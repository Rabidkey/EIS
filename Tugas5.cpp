#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "secrets.h"        // WiFi Configuration (WiFi name and Password)
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1       // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char *ssid = "Wokwi-GUEST";
const char *password = "";

// Powered by CoinDesk - https://www.coindesk.com/price/bitcoin
const String url = "http://api.coindesk.com/v1/bpi/currentprice/BTC.json";
const String historyURL = "http://api.coindesk.com/v1/bpi/historical/close.json";
const String cryptoCode = "BTC";
String lastMinutePrice;

// 'icons8-bitcoin-24', 24x24px
const unsigned char bitcoinIcon[] PROGMEM = {
    0x00, 0x7e, 0x00, 0x03, 0xff, 0xc0, 0x07, 0x81, 0xe0, 0x0e, 0x00, 0x70, 0x18, 0x28, 0x18, 0x30,
    0x28, 0x0c, 0x70, 0xfc, 0x0e, 0x60, 0xfe, 0x06, 0x60, 0xc7, 0x06, 0xc0, 0xc3, 0x03, 0xc0, 0xc7,
    0x03, 0xc0, 0xfe, 0x03, 0xc0, 0xff, 0x03, 0xc0, 0xc3, 0x83, 0xc0, 0xc1, 0x83, 0x60, 0xc3, 0x86,
    0x60, 0xff, 0x06, 0x70, 0xfe, 0x0e, 0x30, 0x28, 0x0c, 0x18, 0x28, 0x18, 0x0e, 0x00, 0x70, 0x07,
    0x81, 0xe0, 0x03, 0xff, 0xc0, 0x00, 0x7e, 0x00};

HTTPClient http;
String lastPrice;

void setup()
{
    Serial.begin(115200);

    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Connecting to WiFi...");
    display.display();
    http.end();

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
    }

    Serial.print("CONNECTED to SSID: ");
    Serial.println(ssid);

    display.print("Connected to ");
    display.println(ssid);
    display.display();
    delay(5000);
}

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Getting current data...");

        http.begin(url);
        int httpCode = http.GET();
        Serial.print("HTTP Code: ");
        Serial.println(httpCode);
        if (httpCode > 0)
        {
            StaticJsonDocument<768> doc;
            DeserializationError error = deserializeJson(doc, http.getString());

            if (error)
            {
                Serial.print(F("deserializeJson failed: "));
                Serial.println(error.f_str());
                delay(2500);
                return;
            }

            Serial.print("HTTP Status Code: ");
            Serial.println(httpCode);

            String BTCUSDPrice = doc["bpi"]["USD"]["rate_float"].as<String>();

            // Store last minute price before update
            lastMinutePrice = lastPrice;
            lastPrice = BTCUSDPrice.toDouble();

            if (atof(BTCUSDPrice.c_str()) == lastPrice.toDouble())
            {
                Serial.print("Price hasn't changed (Current/Last): ");
                Serial.print(BTCUSDPrice);
                Serial.print(" : ");
                Serial.println(lastPrice);
                delay(60000);
                return;
            }
            else
            {
                lastPrice = atof(BTCUSDPrice.c_str());
            }
            String lastUpdated = doc["time"]["updated"].as<String>();

            // Display Header
            display.clearDisplay();
            display.setTextSize(1);
            printCenter("BTC/USD", 0, 0);
            display.clearDisplay();
            display.drawBitmap((128 / 2) - (24 / 2), 0, bitcoinIcon, 24, 24, WHITE);

            display.display();
            http.end();

            // Display BTC Price
            display.setTextSize(1);
            printCenter("$" + BTCUSDPrice, 0, 32);

            // Display 24hr. Percent Change
            bool isUp = BTCUSDPrice.toDouble() > lastMinutePrice.toDouble();
            double percentChange;
            String minChangeString = "1Min. Change: ";
            if (isUp)
            {
                percentChange = ((BTCUSDPrice.toDouble() - lastMinutePrice.toDouble()) / lastMinutePrice.toDouble()) * 100;
            }
            else
            {
                percentChange = ((lastMinutePrice.toDouble() - BTCUSDPrice.toDouble()) / lastMinutePrice.toDouble()) * 100;
                minChangeString = minChangeString + "-";
            }
            display.setTextSize(1);
            minChangeString = minChangeString + percentChange + "%";
            printCenter(minChangeString, 0, 55);

            display.display();
            http.end();
        }
        delay(60000);
    }
}

void printCenter(const String buf, int x, int y)
{
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); // calc width of new string
    display.setCursor((x - w / 2) + (128 / 2), y);
    display.print(buf);
}
