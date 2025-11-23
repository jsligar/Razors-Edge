#include "PowerManager.h"

PowerManager::PowerManager() :
    lastUpdateTime(0)
{
}

bool PowerManager::init() {
    Serial.println("Initializing Power Manager (Tractor Version - No Voltage Sensing)...");
    lastUpdateTime = millis();
    Serial.println("✓ Power Manager initialized (No monitoring)");
    return true;
}

void PowerManager::update() {
    // No monitoring in tractor version
    lastUpdateTime = millis();
}
