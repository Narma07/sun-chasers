// Arduino Code for Solar-Powered Smart Mobile Charger with BMS Integration

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Sensor Pins
#define VOLTAGE_SENSOR A0   // Voltage sensor connected to A0
#define CURRENT_SENSOR A1   // ACS712 current sensor connected to A1
#define TEMPERATURE_SENSOR A2 // LM35 temperature sensor (if used)

// BMS Parameters
#define BATTERY_FULL_VOLTAGE 4.2  // Maximum battery voltage
#define BATTERY_LOW_VOLTAGE 3.0   // Minimum battery voltage

float readVoltage();
float readCurrent();
float readTemperature();
float readBatteryLevel();

void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
}

void loop() {
  float voltage = readVoltage();
  float current = readCurrent();
  float temperature = readTemperature();
  float batteryLevel = readBatteryLevel();

  // Display data on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,0);
  display.print("Voltage: "); display.print(voltage); display.println(" V");

  display.setCursor(0,10);
  display.print("Current: "); display.print(current); display.println(" A");

  display.setCursor(0,20);
  display.print("Temp: "); display.print(temperature); display.println(" C");

  display.setCursor(0,30);
  display.print("Battery: "); display.print(batteryLevel); display.println(" %");

  display.display();

  // Send data to Flask via Serial
  Serial.print(voltage); Serial.print(",");
  Serial.print(current); Serial.print(",");
  Serial.print(temperature); Serial.print(",");
  Serial.print(batteryLevel); Serial.println();

  delay(1000);
}

// Read voltage from sensor
float readVoltage() {
  int sensorValue = analogRead(VOLTAGE_SENSOR);
  float voltage = (sensorValue / 1024.0) * 5.0 * 5; // Adjust scaling factor as needed
  return voltage;
}

// Read current from ACS712 sensor
float readCurrent() {
  int sensorValue = analogRead(CURRENT_SENSOR);
  float voltage = (sensorValue / 1024.0) * 5.0;  // Convert to voltage
  float current = (voltage - 2.5) / 0.185;      // Sensitivity of ACS712 (e.g., 185mV/A)
  return current;
}

// Read temperature from LM35 sensor
float readTemperature() {
  int sensorValue = analogRead(TEMPERATURE_SENSOR);
  float temperature = (sensorValue / 1024.0) * 5.0 * 100.0;
  return temperature;
}

// Read battery level percentage
float readBatteryLevel() {
  float voltage = readVoltage();
  float percentage = ((voltage - BATTERY_LOW_VOLTAGE) / (BATTERY_FULL_VOLTAGE - BATTERY_LOW_VOLTAGE)) * 100.0;
  if (percentage > 100) percentage = 100;
  if (percentage < 0) percentage = 0;
  return percentage;
}
