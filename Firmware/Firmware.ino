
#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// ---------------- GPIO ----------------
constexpr uint8_t LED_PIN  = 2;
constexpr uint8_t PMS_RX   = 16;  
constexpr uint8_t PMS_TX   = 17;  
constexpr uint8_t I2C_SDA  = 21;
constexpr uint8_t I2C_SCL  = 22;
constexpr uint8_t BTN1_PIN = 25;
constexpr uint8_t BTN2_PIN = 26;
constexpr uint8_t BTN3_PIN = 32;

// wifi
const char* AP_SSID = "WeatherStation";
const char* AP_PASS = "weather123";

WebServer server(80);
Adafruit_BME280 bme;
HardwareSerial pmsSerial(2);

//sensor data
float temperatureC = NAN;
float humidityPct  = NAN;
float pressureHpa  = NAN;

uint16_t pm1_0 = 0;
uint16_t pm2_5 = 0;
uint16_t pm10  = 0;

int pm25AQI = -1;
int pm10AQI = -1;
int pmAQI   = -1;

bool bmeOK = false;
bool pmsOK = false;

unsigned long lastSensorRead = 0;
unsigned long lastButtonScan = 0;
unsigned long lastPmsPacket  = 0;



struct AQIBreakpoint {
  float cLow;
  float cHigh;
  int iLow;
  int iHigh;
};

const AQIBreakpoint PM25_BP[] = {
  {0,   30,   0,  50},
  {31,  60,  51, 100},
  {61,  90, 101, 250},
  {91, 120, 251, 350},
  {121, 250, 351, 430},
  {250, 1000, 430, 500}
};

const AQIBreakpoint PM10_BP[] = {
  {0,   50,   0,  50},
  {51, 100,  51, 100},
  {101,250, 101, 250},
  {251,350, 251, 350},
  {351,430, 351, 430},
  {430,1000,430, 500}
};

int calculateAQI(float concentration,
                 const AQIBreakpoint* bp,
                 size_t count) {
  if (isnan(concentration) || concentration < 0) return -1;

  
  float c = floor(concentration);

  for (size_t i = 0; i < count; i++) {
    if (c >= bp[i].cLow && c <= bp[i].cHigh) {
      float aqi =
        ((float)(bp[i].iHigh - bp[i].iLow) /
         (bp[i].cHigh - bp[i].cLow)) *
        (c - bp[i].cLow) + bp[i].iLow;

      return constrain((int)lround(aqi), 0, 500);
    }
  }

  return 500;
}

String aqiCategory(int aqi) {
  if (aqi < 0)   return "N/A";
  if (aqi <= 50) return "Good";
  if (aqi <= 100) return "Satisfactory";
  if (aqi <= 250) return "Moderate";
  if (aqi <= 350) return "Poor";
  if (aqi <= 430) return "Very Poor";
  return "Severe";
}

//BME280
void readBME280() {
  if (!bmeOK) return;

  temperatureC = bme.readTemperature();
  humidityPct  = bme.readHumidity();
  pressureHpa  = bme.readPressure() / 100.0F;

  if (isnan(temperatureC) ||
      isnan(humidityPct) ||
      isnan(pressureHpa)) {
    bmeOK = false;
  }
}

//PMS5003
bool readPMS5003() {
  
  while (pmsSerial.available() >= 32) {

    if (pmsSerial.peek() != 0x42) {
      pmsSerial.read();
      continue;
    }

    uint8_t data[32];
    size_t n = pmsSerial.readBytes(data, 32);

    if (n != 32) return false;

    if (data[0] != 0x42 || data[1] != 0x4D)
      continue;

    uint16_t frameLength =
      ((uint16_t)data[2] << 8) | data[3];

    if (frameLength != 28)
      continue;

    uint16_t receivedChecksum =
      ((uint16_t)data[30] << 8) | data[31];

    uint16_t calculatedChecksum = 0;

    for (int i = 0; i < 30; i++)
      calculatedChecksum += data[i];

    if (calculatedChecksum != receivedChecksum)
      continue;

   
    pm1_0 = ((uint16_t)data[10] << 8) | data[11];
    pm2_5 = ((uint16_t)data[12] << 8) | data[13];
    pm10  = ((uint16_t)data[14] << 8) | data[15];

    pm25AQI = calculateAQI(
      pm2_5, PM25_BP,
      sizeof(PM25_BP) / sizeof(PM25_BP[0])
    );

    pm10AQI = calculateAQI(
      pm10, PM10_BP,
      sizeof(PM10_BP) / sizeof(PM10_BP[0])
    );

    
    pmAQI = max(pm25AQI, pm10AQI);

    lastPmsPacket = millis();
    pmsOK = true;
    return true;
  }

  if (millis() - lastPmsPacket > 5000)
    pmsOK = false;

  return false;
}

// stastus led
void updateLED() {
  static unsigned long lastBlink = 0;
  static bool ledState = false;

  
  if (!bmeOK || !pmsOK) {
    digitalWrite(LED_PIN, HIGH);
    return;
  }

  
  if (millis() - lastBlink >= 1000) {
    lastBlink = millis();
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}

//buttons
bool buttonPressed(uint8_t pin) {
  static uint8_t last1 = HIGH;
  static uint8_t last2 = HIGH;
  static uint8_t last3 = HIGH;

  uint8_t current = digitalRead(pin);
  uint8_t* last = nullptr;

  if (pin == BTN1_PIN) last = &last1;
  if (pin == BTN2_PIN) last = &last2;
  if (pin == BTN3_PIN) last = &last3;

  if (last != nullptr &&
      current == LOW &&
      *last == HIGH) {
    *last = current;
    return true;
  }

  if (last != nullptr)
    *last = current;

  return false;
}

void handleButtons() {
  if (millis() - lastButtonScan < 30)
    return;

  lastButtonScan = millis();

  
  if (buttonPressed(BTN1_PIN)) {
    readBME280();
    readPMS5003();
  }

 
  if (buttonPressed(BTN2_PIN)) {
    WiFi.softAPdisconnect(true);
    delay(100);
    WiFi.softAP(AP_SSID, AP_PASS);
  }

  // SW3: restart the ESP32.
  if (buttonPressed(BTN3_PIN)) {
    ESP.restart();
  }
}

// web dashboard
String htmlPage() {
  String html;
  html.reserve(5000);

  html += F(
    "<!DOCTYPE html><html><head>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<meta http-equiv='refresh' content='5'>"
    "<title>Weather Station</title>"
    "<style>"
    "body{font-family:Arial,sans-serif;background:#f2f2f2;"
    "margin:0;padding:20px;color:#222}"
    ".box{max-width:600px;margin:auto;background:white;padding:22px;"
    "border-radius:16px;box-shadow:0 2px 10px rgba(0,0,0,.12)}"
    "h1{text-align:center;margin-top:0}"
    ".grid{display:grid;grid-template-columns:1fr 1fr;gap:12px}"
    ".card{padding:15px;background:#f7f7f7;border-radius:10px}"
    ".label{font-size:13px;color:#666}"
    ".value{font-size:24px;font-weight:bold;margin-top:4px}"
    ".aqi{grid-column:1/-1;text-align:center}"
    ".small{text-align:center;color:#666;margin-top:18px;font-size:13px}"
    "</style></head><body><div class='box'>"
    "<h1>Weather Station</h1><div class='grid'>"
  );

  html += "<div class='card'><div class='label'>Temperature</div><div class='value'>";
  html += isnan(temperatureC) ? "N/A" : String(temperatureC, 1) + " &deg;C";
  html += "</div></div>";

  html += "<div class='card'><div class='label'>Humidity</div><div class='value'>";
  html += isnan(humidityPct) ? "N/A" : String(humidityPct, 1) + " %";
  html += "</div></div>";

  html += "<div class='card'><div class='label'>Pressure</div><div class='value'>";
  html += isnan(pressureHpa) ? "N/A" : String(pressureHpa, 1) + " hPa";
  html += "</div></div>";

  html += "<div class='card'><div class='label'>PM1.0</div><div class='value'>";
  html += pmsOK ? String(pm1_0) + " &micro;g/m&sup3;" : "N/A";
  html += "</div></div>";

  html += "<div class='card'><div class='label'>PM2.5</div><div class='value'>";
  html += pmsOK ? String(pm2_5) + " &micro;g/m&sup3;" : "N/A";
  html += "</div></div>";

  html += "<div class='card'><div class='label'>PM10</div><div class='value'>";
  html += pmsOK ? String(pm10) + " &micro;g/m&sup3;" : "N/A";
  html += "</div></div>";

  html += "<div class='card aqi'><div class='label'>PM-based AQI estimate</div><div class='value'>";
  html += (pmAQI >= 0) ? String(pmAQI) : "N/A";
  html += "</div><div>";
  html += aqiCategory(pmAQI);
  html += "</div></div>";

  html += F("</div><div class='small'>BME280: ");
  html += bmeOK ? "OK" : "ERROR";
  html += " &nbsp; | &nbsp; PMS5003: ";
  html += pmsOK ? "OK" : "ERROR";
  html += "<br>Connect to Wi-Fi network <b>";
  html += AP_SSID;
  html += "</b> and open <b>192.168.4.1</b>";
  html += F("</div></div></body></html>");

  return html;
}

void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleData() {
  String json = "{";

  json += "\"temperature\":";
  json += isnan(temperatureC) ? "null" : String(temperatureC, 2);
  json += ",";

  json += "\"humidity\":";
  json += isnan(humidityPct) ? "null" : String(humidityPct, 2);
  json += ",";

  json += "\"pressure\":";
  json += isnan(pressureHpa) ? "null" : String(pressureHpa, 2);
  json += ",";

  json += "\"pm1_0\":" + String(pm1_0) + ",";
  json += "\"pm2_5\":" + String(pm2_5) + ",";
  json += "\"pm10\":" + String(pm10) + ",";
  json += "\"aqi\":" + String(pmAQI) + ",";
  json += "\"aqi_category\":\"" + aqiCategory(pmAQI) + "\",";
  json += "\"bme_ok\":" + String(bmeOK ? "true" : "false") + ",";
  json += "\"pms_ok\":" + String(pmsOK ? "true" : "false");

  json += "}";

  server.send(200, "application/json", json);
}


void setup() {
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);

  
  Wire.begin(I2C_SDA, I2C_SCL);

  
  bmeOK = bme.begin(0x76, &Wire);

  if (!bmeOK)
    bmeOK = bme.begin(0x77, &Wire);

 
  pmsSerial.begin(
    9600,
    SERIAL_8N1,
    PMS_RX,
    PMS_TX
  );

  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);

  Serial.println();
  Serial.println("=== ESP32 WEATHER STATION ===");

  Serial.print("Wi-Fi SSID: ");
  Serial.println(AP_SSID);

  Serial.print("Password: ");
  Serial.println(AP_PASS);

  Serial.print("Dashboard: http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/api/data", handleData);
  server.begin();

 
  readBME280();

  
  unsigned long start = millis();

  while (millis() - start < 3000) {
    readPMS5003();
    delay(10);
  }
}


void loop() {
  server.handleClient();

  if (millis() - lastSensorRead >= 1000) {
    lastSensorRead = millis();

    readBME280();
    readPMS5003();
  }

  handleButtons();
  updateLED();
}
