#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;

// Threshold definitions
#define LIGHT_THRESHOLD 10.0      // lux threshold to differentiate day vs. night
#define BATTERY_THRESHOLD 3.5     // minimum battery voltage for safe operation
#define BATTERY_DIVIDER 2.0       // divider ratio used in your battery measurement circuit

// Pin definitions
#define RELAY_PIN 10              // Relay control pin
#define BATTERY_PIN A0            // Analog pin to measure battery voltage

// Four status LEDs for the states:
//   - Daytime & Battery Sufficient
//   - Daytime & Battery Low
//   - Nighttime & Battery Sufficient
//   - Nighttime & Battery Low
#define LED_STATE_DAY_SUFFICIENT 2  
#define LED_STATE_DAY_LOW        3  
#define LED_STATE_NIGHT_SUFFICIENT 4  
#define LED_STATE_NIGHT_LOW      5  

// Two extra LEDs for sensor error indications:
//   - No light sensor info
//   - 0 light level reading
#define LED_NO_SENSOR 8  
#define LED_ZERO_LIGHT 9

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  // Initialize BH1750 sensor
  if (lightMeter.begin()) {
    Serial.println("BH1750 initialized successfully");
  } else {
    Serial.println("Error initializing BH1750");
  }
  
  // Set pin modes for relay and LEDs
  pinMode(RELAY_PIN, OUTPUT);
  
  pinMode(LED_STATE_DAY_SUFFICIENT, OUTPUT);
  pinMode(LED_STATE_DAY_LOW, OUTPUT);
  pinMode(LED_STATE_NIGHT_SUFFICIENT, OUTPUT);
  pinMode(LED_STATE_NIGHT_LOW, OUTPUT);
  
  pinMode(LED_NO_SENSOR, OUTPUT);
  pinMode(LED_ZERO_LIGHT, OUTPUT);
}

void loop() {
  // Read ambient light in lux from the BH1750 sensor
  float lux = lightMeter.readLightLevel();
  Serial.print("Lux: ");
  Serial.println(lux);
  
  // Read battery voltage from analog pin and convert it to volts.
  // (Assumes a voltage divider is used; adjust BATTERY_DIVIDER accordingly)
  int sensorValue = analogRead(BATTERY_PIN);
  float batteryVoltage = sensorValue * (5.0 / 1023.0) * BATTERY_DIVIDER;
  Serial.print("Battery Voltage: ");
  Serial.println(batteryVoltage);
  
  // Turn off all LEDs initially
  digitalWrite(LED_STATE_DAY_SUFFICIENT, LOW);
  digitalWrite(LED_STATE_DAY_LOW, LOW);
  digitalWrite(LED_STATE_NIGHT_SUFFICIENT, LOW);
  digitalWrite(LED_STATE_NIGHT_LOW, LOW);
  digitalWrite(LED_NO_SENSOR, LOW);
  digitalWrite(LED_ZERO_LIGHT, LOW);
  
  // Check for sensor error conditions:
  // Here we assume a negative lux value indicates no sensor info.
  bool sensorError = false;
  if(lux < 0) {  
    digitalWrite(LED_NO_SENSOR, HIGH);
    sensorError = true;
  }
  // Check if the sensor reading is exactly 0 lux.
  if(lux == 0) {
    digitalWrite(LED_ZERO_LIGHT, HIGH);
  }
  
  // Only proceed with normal logic if sensor data is valid.
  if(!sensorError) {
    // Determine day or night based on lux threshold.
    if(lux > LIGHT_THRESHOLD) {
      // It’s considered daytime.
      if(batteryVoltage >= BATTERY_THRESHOLD) {
        // State 1: Daytime & Battery Sufficient.
        digitalWrite(LED_STATE_DAY_SUFFICIENT, HIGH);
        digitalWrite(RELAY_PIN, LOW);  // Turn relay off (no light needed)
      } else {
        // State 2: Daytime & Battery Low.
        digitalWrite(LED_STATE_DAY_LOW, HIGH);
        digitalWrite(RELAY_PIN, LOW);  // Ensure relay remains off
      }
    } else {
      // It’s considered nighttime.
      if(batteryVoltage >= BATTERY_THRESHOLD) {
        // State 3: Nighttime & Battery Sufficient.
        digitalWrite(LED_STATE_NIGHT_SUFFICIENT, HIGH);
        digitalWrite(RELAY_PIN, HIGH); // Turn relay on (light on)
      } else {
        // State 4: Nighttime & Battery Low.
        digitalWrite(LED_STATE_NIGHT_LOW, HIGH);
        digitalWrite(RELAY_PIN, LOW);  // Do not power the relay to save battery
      }
    }
  } else {
    // If sensor error, turn off the relay.
    digitalWrite(RELAY_PIN, LOW);
  }
  
  delay(1000);  // Update every 1 second
}
