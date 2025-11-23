#include "Navigation.h"

Navigation::Navigation() {
}

bool Navigation::init() {
    Serial.println("Initializing Navigation (Tractor Version - No GPS)...");
    Serial.println("✓ Navigation initialized (GPS disabled)");
    return true;
}

void Navigation::update() {
    // No GPS to update in tractor version
}
