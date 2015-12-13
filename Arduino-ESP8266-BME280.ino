#include <ESP8266WiFi.h>
#include <Wire.h>
#include "BME280.h"
#include "ambient.h"

extern "C" {
#include "user_interface.h"
}

#define LED 4
#define SDA 14
#define SCL 13

#define PERIOD 300

//#define VERBOSE

const char* ssid = "・・・SSID・・・";
const char* password = "・・・パスワード・・・";

const char* key = "・・・Ambientのチャネル・キー・・・";

WiFiClient client;
AMBIENT ambient(key, &client);

BME280 bme280;

void setup()
{
    int t = millis();
    wifi_set_sleep_type(LIGHT_SLEEP_T);

#ifdef VERBOSE
    Serial.begin(115200);
    delay(10);
    Serial.println("Start");
    pinMode(LED, OUTPUT);
#endif

    WiFi.begin(ssid, password);

    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
#ifdef VERBOSE
        digitalWrite(LED, i++ % 2);
        Serial.print(".");
#endif
    }
#ifdef VERBOSE
    digitalWrite(LED, LOW);
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
#endif

    bme280.begin(SDA, SCL);

    // 電源投入直後の値は不安定なので、読み捨てる
    bme280.readTemperature();
    bme280.readHumidity();
    bme280.readPressure();
    double temp = 0.0, pressure = 0.0, humid=0.0;
    char tempbuf[8];
    char humidbuf[8];
    char pressurebuf[8];

    temp = bme280.readTemperature();
    humid = bme280.readHumidity();
    pressure = bme280.readPressure();

#ifdef VERBOSE
    Serial.print("temp: ");
    Serial.print(temp);
    Serial.print(" DegC,  humid: ");
    Serial.print(humid);
    Serial.print(" %, pressure: ");
    Serial.print(pressure);
    Serial.println(" hPa");
#endif

    ftoa(temp, tempbuf);
    ftoa(humid, humidbuf);
    ftoa(pressure, pressurebuf);

#ifdef VERBOSE
    digitalWrite(LED, HIGH);
#endif
    ambient.post("temp", tempbuf, "humid", humidbuf, "pressure", pressurebuf);
#ifdef VERBOSE
    digitalWrite(LED, LOW);
    Serial.print("dt: ");
#endif
    t = millis() - t;
#ifdef VERBOSE
    Serial.println(t);
#endif
    t = (t < PERIOD * 1000) ? (PERIOD * 1000 - t) : 1;
    ESP.deepSleep(t * 1000, RF_DISABLED);
    delay(1000);
}


void loop()
{
    while (true) {
        yield();
    }
}

void ftoa(double val, char *buf) {
    itoa((int)val, buf, 10);
    int i = strlen(buf);
    buf[i++] = '.';
    val = (int)(val * 10) % 10;
    itoa(val, &buf[i], 10);
}
