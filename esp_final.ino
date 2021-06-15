#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <functional>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <StreamString.h>


#define BTN_VAL HIGH

#define PRINT_ALLOWED 0

#define PRINT(x) if(PRINT_ALLOWED) Serial.println((x));

HTTPClient http;
WiFiClient client;
StaticJsonDocument<1024> buffer;

bool startWPS()
{
    bool wpsSuccess = false;
    while(!wpsSuccess) {
        ao:
        // Blink 3 volte
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);
        delay(200);
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);
        delay(200);
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);
        delay(200);

        wpsSuccess = WiFi.beginWPSConfig() && WiFi.SSID().length() > 0;
        if(wpsSuccess)
        {
            unsigned long time = millis();
            while (WiFi.status() != WL_CONNECTED && millis() - time <= 60000)
            {
                unsigned long t = millis();
                while(digitalRead(0) == BTN_VAL && millis() - t < 1000){yield();};
                if(millis() - t >= 1000)
                {
                    PRINT("Reset WIFI")
                    WiFi.disconnect();
                    WiFi.mode(WIFI_STA);
                    goto ao;
                }
                yield();
            }
            wpsSuccess = WiFi.status() == WL_CONNECTED;
        }
    }
    return wpsSuccess;
}


void resetWifi()
{
    unsigned long t = millis();
    while(digitalRead(0) == BTN_VAL && millis() - t < 1000 ){yield();};

    if(millis() - t >= 1000)
    {
        // Blink 3 volte
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);
        delay(200);
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);
        delay(200);
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);
        delay(200);

        PRINT("reset wifi")
        WiFi.disconnect();
        WiFi.mode(WIFI_STA);
        startWPS();
    }
}

void isConnected(unsigned long timeout) {
    unsigned long time = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - time <= timeout)
    {
        resetWifi();
        yield();
    }
}

void automaticConnect()
{
    while(WiFi.status() != WL_CONNECTED)
    {
        digitalWrite(2, HIGH);
        WiFi.mode(WIFI_STA);
        WiFi.begin(WiFi.SSID(), WiFi.psk());
        //WiFi.begin("GIAMMY98", "giammy64");
        PRINT(String("ssid: ") + WiFi.SSID() + String("\npassword: ") + WiFi.psk())
        // isConnected via VECCHI SSID e PSK
        isConnected(20000);
    }
    digitalWrite(2, LOW);
}

bool getNextCommand() {
    if(!Serial.available()) return false;
    if(deserializeJson(buffer, Serial) != DeserializationError::Ok) return false;
    PRINT("COMMAND: " + buffer["command"].as<String>());
    return true;
}

void httpReq() {

    String method = buffer["method"].as<String>();
    String url    = buffer["url"].as<String>();
    String payload= method == "GET" ? "" : buffer["payload"].as<String>();

    PRINT(method)
    PRINT(url)
    PRINT(payload)

    StaticJsonDocument<1024> doc;
    doc["response"] = "";

    if(http.begin(client, url))
    {
        http.useHTTP10(true);
        http.addHeader("Content-Type", "application/json");
        http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

        int httpCode = method == "GET" ? http.GET() : http.PUT(payload);

        if(httpCode >= 200 && httpCode < 300) {
            StreamString serverStream;
            String serverResult = "";
            serverStream.begin();

            if(http.writeToStream(&serverStream) < 0) {
                PRINT("ERRSTREAM")
            }
            else {
                while(serverStream.available()) {
                    serverResult.concat(static_cast<char>(serverStream.read()));
                    yield();
                }
                doc["response"] = serverResult;
            }
            serverStream.end();
        }
        http.end();
    }
    else PRINT("UNREACHABLEHOST")

    serializeJson(doc, Serial);
}

String getAddress() {
    String mac = WiFi.macAddress();
    String r = "";
    for(int i = 0; i < mac.length(); i++) {
        if(mac[i] == ':') continue;
        r.concat(mac[i]);
    }
    return r;
}

void executeNextCommand() {
    if(!getNextCommand()) return;

    switch(buffer["command"].as<String>()[0]) {
        case '0': {
            StaticJsonDocument<128> doc;
            doc["response"] = WiFi.status() != WL_CONNECTED ? String("NOTCONNECTED") : String("CONNECTED");
            serializeJson(doc, Serial);
            doc.clear();
            break;
        }
        case '1': {
            httpReq();
            break;
        }
        case '2': {
            StaticJsonDocument<128> doc;
            doc["response"] = getAddress();
            serializeJson(doc, Serial);
            doc.clear();
            break;
        }
    }

    buffer.clear();
}

void setup() {

    Serial.setRxBufferSize(1024);
    Serial.begin(9600);
    while(!Serial);


    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);

    PRINT("ESP STARTED")

    // RESET PIN
    pinMode(0, INPUT);

    // LED PIN
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);

    // Inizio connessione
    automaticConnect();

}

void loop() {

    // Check connessione
    automaticConnect();
    resetWifi();

    // Legge comando
    // Esegue comando
    executeNextCommand();
}
