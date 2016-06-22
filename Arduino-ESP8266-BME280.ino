#include <ESP8266WiFi.h>
#include <Wire.h>
#include "BME280.h"
#include "Ambient.h"

extern "C" {
#include "user_interface.h"
}

#define _DEBUG 1
#if _DEBUG
#define DBG(...) { Serial.print(__VA_ARGS__); }
#define DBGLED(...) { digitalWrite(__VA_ARGS__); }
#else
#define DBG(...)
#define DBGLED(...)
#endif /* _DBG */

#define LED 4
#define SDA 14
#define SCL 13

#define PERIOD 300

const char* ssid = "ssid";
const char* password = "password";
WiFiClient client;

unsigned int channelId = 100;
const char* writeKey = "...writeKey...";
Ambient ambient;

BME280 bme280;

void ftoa(double val, char *buf) {
    itoa((int)val, buf, 10);
    int i = strlen(buf);
    buf[i++] = '.';
    val = (int)(val * 10) % 10;
    itoa(val, &buf[i], 10);
}

void setup()
{
    int t = millis();
    wifi_set_sleep_type(LIGHT_SLEEP_T);    

#ifdef _DEBUG
    Serial.begin(115200);
    delay(10);
#endif
    DBG("Start");
    pinMode(LED, OUTPUT);

    WiFi.begin(ssid, password);

    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);

        DBGLED(LED, i++ % 2);
        DBG(".");
    }

    DBGLED(LED, LOW);
    DBG("WiFi connected");
    DBG("IP address: ");
    DBG(WiFi.localIP());
    DBG("\r\n");
    
    ambient.begin(channelId, writeKey, &client);
    bme280.begin(SDA, SCL);

    // 電源投入直後の値は不安定なので、読み捨てる
    bme280.readTemperature();
    bme280.readHumidity();
    bme280.readPressure();
    double temp = 0.0, pressure = 0.0, humid=0.0;
    char tempbuf[12];
    char humidbuf[12];
    char pressurebuf[12];

    temp = bme280.readTemperature();
    humid = bme280.readHumidity();
    pressure = bme280.readPressure();

    DBG("temp: ");
    DBG(temp);
    DBG(" DegC,  humid: ");
    DBG(humid);
    DBG(" %, pressure: ");
    DBG(pressure);
    DBG(" hPa\r\n");

    ftoa(temp, tempbuf);
    ambient.set(1, tempbuf);
    ftoa(humid, humidbuf);
    ambient.set(2, humidbuf);
    ftoa(pressure, pressurebuf);
    ambient.set(3, pressurebuf);

    DBGLED(LED, HIGH);

    ambient.send();

    DBGLED(LED, LOW);

    t = millis() - t;
    t = (t < PERIOD * 1000) ? (PERIOD * 1000 - t) : 1;
    ESP.deepSleep(t * 1000, RF_DEFAULT);
    delay(1000);
}


void loop()
{
    while (true) {
        yield();
    }
}

