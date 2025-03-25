#include <ACS712.h>

// ACS712 சென்சார் பின் அமைப்பு
const int CURRENT_SENSOR_PIN = A0;  // அனலாக் பின் A0
const int VOLTAGE_SENSOR_PIN = A1;  // அனலாக் பின் A1

// ACS712 30A மாடல் பயன்படுத்துகிறோம்
ACS712 currentSensor(ACS712_30A, CURRENT_SENSOR_PIN);

// வோல்டேஜ் சென்சார் மாறிகள்
const float VOLTAGE_REFERENCE = 5.0;  // Arduino வோல்டேஜ் reference
const float VOLTAGE_DIVIDER_RATIO = 0.2;  // வோல்டேஜ் divider ratio

void setup() {
  Serial.begin(9600);  // சீரியல் கம்யூனிகேஷன் தொடங்குதல்
  
  // கரண்ட் சென்சார் கேலிப்ரேஷன்
  Serial.println("Calibrating current sensor...");
  currentSensor.calibrate();
  Serial.println("Calibration completed!");
}

void loop() {
  // கரண்ட் அளவீடு
  float current = currentSensor.getCurrentAC();
  
  // வோல்டேஜ் அளவீடு
  int rawVoltage = analogRead(VOLTAGE_SENSOR_PIN);
  float voltage = (rawVoltage * VOLTAGE_REFERENCE / 1024.0) / VOLTAGE_DIVIDER_RATIO;
  
  // பவர் கணக்கீடு
  float power = voltage * current;
  
  // JSON வடிவில் தரவு அனுப்புதல்
  Serial.print("{");
  Serial.print("\"current\":");
  Serial.print(current);
  Serial.print(",\"voltage\":");
  Serial.print(voltage);
  Serial.print(",\"power\":");
  Serial.print(power);
  Serial.println("}");
  
  delay(1000);  // 1 வினாடி இடைவெளி
} 