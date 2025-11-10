/*
 * Simple GT-U7 GPS Test for ESP32
 *
 * This sketch tests the GT-U7 GPS module connected via 5-pin header
 * to ESP32 GPIO 16 (RX) and GPIO 17 (TX).
 *
 * Upload this INSTEAD of the main Razors Edge firmware to test GPS.
 *
 * Wiring:
 *   GT-U7 VCC → ESP32 3.3V
 *   GT-U7 RX  → ESP32 GPIO 17
 *   GT-U7 TX  → ESP32 GPIO 16
 *   GT-U7 GND → ESP32 GND
 *   GT-U7 PPS → (not connected)
 *
 * How to use:
 * 1. Disconnect GT-U7 USB (if connected)
 * 2. Wire GT-U7 to ESP32 using 5-pin header
 * 3. Upload this sketch to ESP32
 * 4. Open Serial Monitor at 115200 baud
 * 5. Take GPS module OUTSIDE
 * 6. Wait 30-90 seconds for satellite fix
 *
 * You should see:
 * - Raw NMEA sentences scrolling
 * - Satellite count increasing
 * - Latitude/Longitude values appearing
 */

#include <Arduino.h>

// GPS pins
#define GPS_RX_PIN  16  // ESP32 RX - connects to GPS TX
#define GPS_TX_PIN  17  // ESP32 TX - connects to GPS RX
#define GPS_BAUD    9600

unsigned long lastPrint = 0;
unsigned long bytesReceived = 0;
unsigned long linesReceived = 0;

void setup() {
  // Serial to PC for viewing data
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║   GT-U7 GPS Test for ESP32             ║");
  Serial.println("║   GPIO 16 (RX) ← GPS TX                ║");
  Serial.println("║   GPIO 17 (TX) → GPS RX                ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println();

  // GPS on Serial2 (GPIO 16 RX, GPIO 17 TX)
  Serial2.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  Serial.println("GPS Serial initialized on GPIO 16/17");
  Serial.printf("Baud rate: %d\n", GPS_BAUD);
  Serial.println();
  Serial.println("🛰️  Take GPS module OUTSIDE for best results!");
  Serial.println("⏱️  Wait 30-90 seconds for satellite fix");
  Serial.println();
  Serial.println("Expected output:");
  Serial.println("  $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47");
  Serial.println("  $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A");
  Serial.println();
  Serial.println("════════════════════════════════════════");
  Serial.println("GPS DATA (live stream):");
  Serial.println("════════════════════════════════════════");
  Serial.println();
}

void loop() {
  // Read from GPS and print to PC
  while (Serial2.available()) {
    char c = Serial2.read();
    Serial.write(c);  // Print raw GPS data

    bytesReceived++;
    if (c == '\n') {
      linesReceived++;
    }
  }

  // Print statistics every 10 seconds
  if (millis() - lastPrint > 10000) {
    Serial.println();
    Serial.println("════════════════════════════════════════");
    Serial.printf("📊 Statistics (after %lu seconds):\n", millis() / 1000);
    Serial.printf("   Bytes received: %lu\n", bytesReceived);
    Serial.printf("   Lines received: %lu\n", linesReceived);

    if (bytesReceived == 0) {
      Serial.println();
      Serial.println("⚠️  WARNING: No GPS data received!");
      Serial.println("   Check:");
      Serial.println("   • GT-U7 VCC connected to ESP32 3.3V");
      Serial.println("   • GT-U7 TX connected to ESP32 GPIO 16");
      Serial.println("   • GT-U7 RX connected to ESP32 GPIO 17");
      Serial.println("   • GT-U7 GND connected to ESP32 GND");
      Serial.println("   • GT-U7 USB is DISCONNECTED");
      Serial.println("   • LED on GT-U7 is blinking");
    } else {
      Serial.println("   ✅ GPS is transmitting data!");
      Serial.println();
      Serial.println("   To decode GPS data, look for:");
      Serial.println("   • $GPGGA - Position and fix data");
      Serial.println("   • $GPRMC - Recommended minimum data");
      Serial.println("   • Number after 'E,1,' is satellite count");
    }

    Serial.println("════════════════════════════════════════");
    Serial.println();

    lastPrint = millis();
  }

  delay(1);
}
