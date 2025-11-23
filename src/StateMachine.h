#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <Arduino.h>
#include "config.h"

// Forward declarations
class PowerManager;
class MotorController;
class LightController;
class UserInterface;
class InputHandler;
class Navigation;
class SafetySystem;

struct StateTransition {
    SystemState fromState;
    SystemState toState;
    const char* condition;
    unsigned long timestamp;
};

class StateMachine {
public:
    StateMachine();
    
    // Initialization and main update
    bool init();
    void update();
    
    // State management
    SystemState getCurrentState();
    const char* getStateString();
    const char* getStateString(SystemState state);
    bool canTransitionTo(SystemState newState);
    void forceState(SystemState state);
    void forceFaultState(uint8_t faultCode);
    
    // Direction management (tractor version)
    DirectionMode getCurrentDirection();
    void setDirection(DirectionMode dir);
    
    // System references (set by main.cpp)
    void setModuleReferences(PowerManager* power, MotorController* motors,
                           LightController* lights, UserInterface* ui,
                           InputHandler* inputs, Navigation* gps,
                           SafetySystem* safety);
    
    // State transition history
    uint8_t getTransitionHistoryCount();
    StateTransition getTransitionHistory(uint8_t index);
    void clearTransitionHistory();
    
private:
    // Current state
    SystemState currentState;
    SystemState previousState;
    unsigned long stateEntryTime;
    
    // Direction state (tractor version)
    DirectionMode currentDirection;
    DirectionMode previousDirection;
    unsigned long directionChangeTime;
    
    // Module references
    PowerManager* powerManager;
    MotorController* motorController;
    LightController* lightController;
    UserInterface* userInterface;
    InputHandler* inputHandler;
    Navigation* navigation;
    SafetySystem* safetySystem;
    
    // State transition history
    static const uint8_t MAX_TRANSITION_HISTORY = 20;
    StateTransition transitionHistory[MAX_TRANSITION_HISTORY];
    uint8_t transitionHistoryCount;
    uint8_t transitionHistoryIndex;
    
    // State machine logic
    void checkStateTransitions();
    void transitionToState(SystemState newState);
    void recordTransition(SystemState fromState, SystemState toState, const char* condition);
    
    // State entry/exit handlers
    void onEnterState(SystemState state);
    void onExitState(SystemState state);
    
    // Individual state handlers
    void handleInitState();
    void handleStandbyState();
    void handleReadyState();
    void handleForwardState();
    void handleNeutralState();
    void handleReverseState();
    void handleFaultState();
    
    // Direction logic
    void executeDirectionChange(DirectionMode newDir);
    bool isDirectionChangeAllowed(DirectionMode newDir);
    
    // Condition checking
    bool checkSafetyConditions();
    bool checkKeySwitch();
    bool checkBatteryVoltage();
    bool checkPedalPosition();
    bool isVehicleMoving();
    
    // Safety interlocks
    bool canEnableMotors();
    bool shouldDisableMotors();
};

#endif // STATE_MACHINE_H