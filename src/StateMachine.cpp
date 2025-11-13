#include "StateMachine.h"
#include "PowerManager.h"
#include "MotorController.h"
#include "LightController.h"
#include "UserInterface.h"
#include "InputHandler.h"
#include "Navigation.h"
#include "SafetySystem.h"

StateMachine::StateMachine() :
    currentState(STATE_INIT),
    previousState(STATE_INIT),
    stateEntryTime(0),
    currentGear(GEAR_PARK),
    previousGear(GEAR_PARK),
    gearChangeTime(0),
    sportPlusStartTime(0),
    powerManager(nullptr),
    motorController(nullptr),
    lightController(nullptr),
    userInterface(nullptr),
    inputHandler(nullptr),
    navigation(nullptr),
    safetySystem(nullptr),
    transitionHistoryCount(0),
    transitionHistoryIndex(0)
{
}

bool StateMachine::init() {
    Serial.println("Initializing State Machine...");
    
    currentState = STATE_INIT;
    stateEntryTime = millis();
    
    Serial.println("✓ State Machine initialized");
    return true;
}

void StateMachine::update() {
    // Handle current state
    switch (currentState) {
        case STATE_INIT:
            handleInitState();
            break;
        case STATE_STANDBY:
            handleStandbyState();
            break;
        case STATE_READY:
            handleReadyState();
            break;
        case STATE_FORWARD:
            handleForwardState();
            break;
        case STATE_NEUTRAL:
            handleNeutralState();
            break;
        case STATE_REVERSE:
            handleReverseState();
            break;
        case STATE_FAULT:
            handleFaultState();
            break;
    }
    
    // Check for state transitions
    checkStateTransitions();
}

SystemState StateMachine::getCurrentState() {
    return currentState;
}

const char* StateMachine::getStateString() {
    return getStateString(currentState);
}

const char* StateMachine::getStateString(SystemState state) {
    switch (state) {
        case STATE_INIT: return "INIT";
        case STATE_STANDBY: return "STANDBY";
        case STATE_READY: return "READY";
        case STATE_FORWARD: return "FORWARD";
        case STATE_NEUTRAL: return "NEUTRAL";
        case STATE_REVERSE: return "REVERSE";
        case STATE_FAULT: return "FAULT";
        default: return "UNKNOWN";
    }
}

bool StateMachine::canTransitionTo(SystemState newState) {
    // Define valid state transitions
    switch (currentState) {
        case STATE_INIT:
            return (newState == STATE_STANDBY);
            
        case STATE_STANDBY:
            return (newState == STATE_READY || newState == STATE_FAULT);
            
        case STATE_READY:
            return (newState == STATE_FORWARD || newState == STATE_NEUTRAL || 
                   newState == STATE_STANDBY || newState == STATE_FAULT);
            
        case STATE_FORWARD:
            return (newState == STATE_READY || newState == STATE_NEUTRAL || 
                   newState == STATE_FAULT);
            
        case STATE_NEUTRAL:
            return (newState == STATE_READY || newState == STATE_STANDBY || 
                   newState == STATE_FAULT);
            
        case STATE_REVERSE:
            return (newState == STATE_READY || newState == STATE_NEUTRAL || 
                   newState == STATE_FAULT);
            
        case STATE_FAULT:
            return (newState == STATE_STANDBY); // Only after fault cleared
            
        default:
            return false;
    }
}

void StateMachine::forceState(SystemState state) {
    transitionToState(state);
}

void StateMachine::forceFaultState(uint8_t faultCode) {
    transitionToState(STATE_FAULT);
}

GearMode StateMachine::getCurrentGear() {
    return currentGear;
}

void StateMachine::handleShiftUp() {
    GearMode newGear = currentGear;

    switch (currentGear) {
        case GEAR_PARK:
            newGear = GEAR_1ST;
            break;
        case GEAR_REVERSE:
            newGear = GEAR_PARK;
            break;
        case GEAR_1ST:
            newGear = GEAR_2ND;
            break;
        case GEAR_2ND:
            newGear = GEAR_3RD;
            break;
        case GEAR_3RD:
            newGear = GEAR_SPORT_PLUS;
            break;
        case GEAR_ECO:
            newGear = GEAR_1ST;
            break;
        case GEAR_SPORT_PLUS:
            // Already at highest gear
            return;
    }

    if (isGearChangeAllowed(newGear)) {
        executeGearChange(newGear);
    }
}

void StateMachine::handleShiftDown() {
    GearMode newGear = currentGear;

    switch (currentGear) {
        case GEAR_PARK:
            newGear = GEAR_REVERSE;
            break;
        case GEAR_REVERSE:
            // Already at lowest gear
            return;
        case GEAR_1ST:
            newGear = GEAR_ECO;
            break;
        case GEAR_2ND:
            newGear = GEAR_1ST;
            break;
        case GEAR_3RD:
            newGear = GEAR_2ND;
            break;
        case GEAR_ECO:
            newGear = GEAR_PARK;
            break;
        case GEAR_SPORT_PLUS:
            newGear = GEAR_3RD;
            break;
    }

    if (isGearChangeAllowed(newGear)) {
        executeGearChange(newGear);
    }
}

void StateMachine::setGear(GearMode gear) {
    if (isGearChangeAllowed(gear)) {
        executeGearChange(gear);
    }
}

void StateMachine::checkSportPlusTimeout() {
    if (currentGear == GEAR_SPORT_PLUS) {
        unsigned long elapsed = millis() - sportPlusStartTime;
        
        if (elapsed > SPORT_PLUS_TIMEOUT_MS) {
            Serial.println("Sport+ timeout - downshifting to 3rd gear");
            executeGearChange(GEAR_3RD);
        }
    }
}

unsigned long StateMachine::getSportPlusRemainingTime() {
    if (currentGear != GEAR_SPORT_PLUS) return 0;
    
    unsigned long elapsed = millis() - sportPlusStartTime;
    if (elapsed >= SPORT_PLUS_TIMEOUT_MS) return 0;
    
    return SPORT_PLUS_TIMEOUT_MS - elapsed;
}

void StateMachine::setModuleReferences(PowerManager* power, MotorController* motors,
                                     LightController* lights, UserInterface* ui,
                                     InputHandler* inputs, Navigation* gps,
                                     SafetySystem* safety) {
    powerManager = power;
    motorController = motors;
    lightController = lights;
    userInterface = ui;
    inputHandler = inputs;
    navigation = gps;
    safetySystem = safety;
}

uint8_t StateMachine::getTransitionHistoryCount() {
    return transitionHistoryCount;
}

StateTransition StateMachine::getTransitionHistory(uint8_t index) {
    if (index < transitionHistoryCount) {
        return transitionHistory[index];
    }
    return {STATE_INIT, STATE_INIT, "Invalid", 0};
}

void StateMachine::clearTransitionHistory() {
    transitionHistoryCount = 0;
    transitionHistoryIndex = 0;
}

void StateMachine::checkStateTransitions() {
    SystemState newState = currentState;
    
    // Check for safety faults (highest priority)
    if (safetySystem && !safetySystem->isSystemSafe()) {
        newState = STATE_FAULT;
    }
    // Check other transition conditions based on current state
    else {
        switch (currentState) {
            case STATE_INIT:
                // Auto-transition to standby after initialization
                if (millis() - stateEntryTime > 2000) {
                    newState = STATE_STANDBY;
                }
                break;
                
            case STATE_STANDBY:
                // Transition to ready if key is on and system is safe
                if (checkKeySwitch() && checkSafetyConditions()) {
                    newState = STATE_READY;
                }
                break;
                
            case STATE_READY:
                // Transition based on pedal input
                if (!checkKeySwitch()) {
                    newState = STATE_STANDBY;
                } else if (checkPedalPosition()) {
                    newState = STATE_FORWARD;
                }
                break;
                
            case STATE_FORWARD:
                // Stay in forward while pedal is pressed and key is on
                if (!checkKeySwitch()) {
                    newState = STATE_STANDBY;
                } else if (!checkPedalPosition()) {
                    newState = STATE_READY;
                }
                break;
                
            case STATE_NEUTRAL:
                // Similar to ready state
                if (!checkKeySwitch()) {
                    newState = STATE_STANDBY;
                } else if (checkPedalPosition()) {
                    newState = STATE_FORWARD;
                }
                break;
                
            case STATE_FAULT:
                // Can only exit fault state manually after clearing faults
                if (safetySystem && safetySystem->isSystemSafe() && !checkKeySwitch()) {
                    newState = STATE_STANDBY;
                }
                break;
                
            case STATE_REVERSE:
                // Future implementation
                break;
        }
    }
    
    // Execute transition if state should change
    if (newState != currentState && canTransitionTo(newState)) {
        transitionToState(newState);
    }
}

void StateMachine::transitionToState(SystemState newState) {
    SystemState oldState = currentState;
    
    // Exit current state
    onExitState(currentState);
    
    // Record transition
    recordTransition(currentState, newState, "State Machine");
    
    // Update state
    previousState = currentState;
    currentState = newState;
    stateEntryTime = millis();
    
    // Enter new state
    onEnterState(newState);
    
    Serial.printf("State: %s → %s\n", getStateString(oldState), getStateString(newState));
}

void StateMachine::recordTransition(SystemState fromState, SystemState toState, const char* condition) {
    transitionHistory[transitionHistoryIndex].fromState = fromState;
    transitionHistory[transitionHistoryIndex].toState = toState;
    transitionHistory[transitionHistoryIndex].condition = condition;
    transitionHistory[transitionHistoryIndex].timestamp = millis();
    
    transitionHistoryIndex = (transitionHistoryIndex + 1) % MAX_TRANSITION_HISTORY;
    if (transitionHistoryCount < MAX_TRANSITION_HISTORY) {
        transitionHistoryCount++;
    }
}

void StateMachine::onEnterState(SystemState state) {
    // Actions to take when entering a state
    switch (state) {
        case STATE_INIT:
            // Initialization actions
            break;
            
        case STATE_STANDBY:
            // Ensure motors are disabled
            if (motorController) {
                motorController->emergencyStop();
            }
            break;
            
        case STATE_READY:
            // System ready, motors can be enabled
            break;
            
        case STATE_FORWARD:
            // Active driving mode
            break;
            
        case STATE_NEUTRAL:
            // Motors disabled but key on
            if (motorController) {
                motorController->stop();
            }
            break;
            
        case STATE_FAULT:
            // Emergency stop everything
            if (motorController) {
                motorController->emergencyStop();
            }
            if (lightController) {
                lightController->startWarningFlash();
            }
            break;
            
        case STATE_REVERSE:
            // Future implementation
            break;
    }
}

void StateMachine::onExitState(SystemState state) {
    // Actions to take when exiting a state
    switch (state) {
        case STATE_FAULT:
            // Stop warning lights when exiting fault state
            if (lightController) {
                lightController->stopAllAnimations();
            }
            break;
            
        default:
            // No special exit actions for other states
            break;
    }
}

// Individual state handlers
void StateMachine::handleInitState() {
    // Initialization state - just wait for timeout
}

void StateMachine::handleStandbyState() {
    // Standby state - motors disabled, waiting for key
}

void StateMachine::handleReadyState() {
    // Ready state - system armed, waiting for pedal input
}

void StateMachine::handleForwardState() {
    // Forward driving state - active motor control
}

void StateMachine::handleNeutralState() {
    // Neutral state - key on but motors disabled
}

void StateMachine::handleReverseState() {
    // Reverse state - future implementation
}

void StateMachine::handleFaultState() {
    // Fault state - everything disabled, show warnings
}

void StateMachine::executeGearChange(GearMode newGear) {
    previousGear = currentGear;
    currentGear = newGear;
    gearChangeTime = millis();
    
    // Handle special gear logic
    if (newGear == GEAR_SPORT_PLUS) {
        handleSportPlusEntry();
    }
    
    // Update motor controller
    if (motorController) {
        motorController->setGear(newGear);
    }
    
    // Update user interface
    if (userInterface) {
        userInterface->setGear(newGear);  // Update the displayed gear
        userInterface->showGearChange(newGear);  // Trigger animation
    }
    
    // Update lights
    if (lightController) {
        lightController->startGearShiftAnimation(newGear);
    }
    
    Serial.printf("Gear: %s → %s\n", 
                 GEAR_CONFIGS[previousGear].name, 
                 GEAR_CONFIGS[currentGear].name);
}

bool StateMachine::isGearChangeAllowed(GearMode newGear) {
    // Prevent rapid gear changes (cooldown period)
    unsigned long timeSinceLastChange = millis() - gearChangeTime;
    if (timeSinceLastChange < 500) {  // 500ms cooldown
        // Don't spam serial - just silently reject
        return false;
    }

    // Check if gear change is safe
    if (newGear == GEAR_SPORT_PLUS) {
        // Special checks for Sport+ mode
        if (powerManager) {
            float voltage = powerManager->getBatteryVoltage();
            float current = powerManager->getBatteryCurrent();

            if (voltage < 58.0f) {
                Serial.println("Sport+ denied: Battery voltage too low");
                return false;
            }

            if (current > 18.0f) {
                Serial.println("Sport+ denied: System under high load");
                return false;
            }
        }
    }

    return true;
}

void StateMachine::handleSportPlusEntry() {
    sportPlusStartTime = millis();
    Serial.println("Sport+ mode activated - 10 second timeout started");
}

// Condition checking methods
bool StateMachine::checkSafetyConditions() {
    return safetySystem ? safetySystem->isSystemSafe() : true;
}

bool StateMachine::checkKeySwitch() {
    return inputHandler ? inputHandler->isKeyOn() : false;
}

bool StateMachine::checkBatteryVoltage() {
    if (!powerManager) return true;
    return powerManager->getBatteryVoltage() > BATTERY_VOLTAGE_MIN;
}

bool StateMachine::checkPedalPosition() {
    if (!inputHandler) return false;
    return inputHandler->getPedalPosition() > 5.0f; // 5% threshold
}

bool StateMachine::isVehicleMoving() {
    if (!navigation) return false;
    return navigation->getSpeed() > 1.0f; // 1 MPH threshold
}

bool StateMachine::canEnableMotors() {
    return checkKeySwitch() && checkSafetyConditions() && checkBatteryVoltage();
}

bool StateMachine::shouldDisableMotors() {
    return !checkKeySwitch() || !checkSafetyConditions();
}