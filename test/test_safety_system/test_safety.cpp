#include <unity.h>
#include "../../src/SafetySystem.h"
#include "../../src/config.h"

SafetySystem* safety;

void setUp(void) {
    safety = new SafetySystem();
    safety->init();
}

void tearDown(void) {
    delete safety;
}

// Test initialization
void test_safety_system_init(void) {
    TEST_ASSERT_TRUE(safety->isSystemSafe());
    TEST_ASSERT_EQUAL(NO_FAULT, safety->getFaultCode());
}

// Test battery voltage fault detection
void test_low_battery_detection(void) {
    safety->setBatteryData(50.0f, 0.0f); // Below 54V minimum
    safety->update();
    TEST_ASSERT_FALSE(safety->isSystemSafe());
    TEST_ASSERT_EQUAL(LOW_BATTERY, safety->getFaultCode());
}

void test_battery_voltage_normal(void) {
    safety->setBatteryData(60.0f, 0.0f); // Normal voltage
    safety->update();
    TEST_ASSERT_TRUE(safety->isSystemSafe());
}

// Test overcurrent detection
void test_battery_overcurrent_detection(void) {
    safety->setBatteryData(60.0f, 60.0f); // Exceeds 50A limit
    safety->update();
    delay(1100); // Wait for overcurrent timeout
    safety->update();
    TEST_ASSERT_FALSE(safety->isSystemSafe());
    TEST_ASSERT_EQUAL(BATTERY_OVERCURRENT, safety->getFaultCode());
}

void test_motor_left_overcurrent(void) {
    safety->setMotorData(25.0f, 10.0f); // Left motor exceeds 20A
    safety->update();
    TEST_ASSERT_FALSE(safety->isSystemSafe());
    TEST_ASSERT_EQUAL(MOTOR_LEFT_OVERCURRENT, safety->getFaultCode());
}

void test_motor_right_overcurrent(void) {
    safety->setMotorData(10.0f, 25.0f); // Right motor exceeds 20A
    safety->update();
    TEST_ASSERT_FALSE(safety->isSystemSafe());
    TEST_ASSERT_EQUAL(MOTOR_RIGHT_OVERCURRENT, safety->getFaultCode());
}

// Test motor imbalance detection
void test_motor_imbalance_fault(void) {
    safety->setMotorData(20.0f, 5.0f); // 60% imbalance - should fault
    safety->update();
    TEST_ASSERT_FALSE(safety->isSystemSafe());
    TEST_ASSERT_EQUAL(MOTOR_IMBALANCE, safety->getFaultCode());
}

void test_motor_balance_ok(void) {
    safety->setMotorData(10.0f, 11.0f); // ~10% imbalance - OK
    safety->update();
    TEST_ASSERT_TRUE(safety->isSystemSafe());
}

// Test key switch
void test_key_switch_off_fault(void) {
    safety->setKeyState(false);
    safety->update();
    TEST_ASSERT_FALSE(safety->isSystemSafe());
    TEST_ASSERT_EQUAL(KEY_SWITCH_OFF, safety->getFaultCode());
}

void test_key_switch_on(void) {
    safety->setKeyState(true);
    safety->update();
    TEST_ASSERT_TRUE(safety->isSystemSafe());
}

// Test fault clearing
void test_clear_specific_fault(void) {
    safety->setBatteryData(50.0f, 0.0f); // Create low battery fault
    safety->update();
    TEST_ASSERT_FALSE(safety->isSystemSafe());

    safety->clearFault(LOW_BATTERY);
    TEST_ASSERT_FALSE(safety->hasFault(LOW_BATTERY));
}

void test_clear_all_faults(void) {
    safety->setBatteryData(50.0f, 0.0f); // Low battery
    safety->setKeyState(false); // Key off
    safety->update();
    TEST_ASSERT_FALSE(safety->isSystemSafe());

    safety->clearFaults();
    TEST_ASSERT_TRUE(safety->isSystemSafe());
}

// Test fault history
void test_fault_history_recording(void) {
    safety->setBatteryData(50.0f, 0.0f); // Create fault
    safety->update();

    TEST_ASSERT_GREATER_THAN(0, safety->getFaultHistoryCount());
    FaultHistory history = safety->getFaultHistory(0);
    TEST_ASSERT_EQUAL(LOW_BATTERY, history.faultCode);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, history.batteryVoltage);
}

// Test emergency shutdown
void test_emergency_shutdown(void) {
    safety->emergencyShutdown();
    TEST_ASSERT_TRUE(safety->isEmergencyShutdown());
    TEST_ASSERT_FALSE(safety->isSystemSafe());
}

// Test safety limits configuration
void test_safety_limits_update(void) {
    SafetyLimits newLimits;
    newLimits.batteryVoltageMin = 50.0f;
    newLimits.batteryVoltageMax = 65.0f;
    newLimits.batteryCurrentMax = 40.0f;
    newLimits.motorCurrentMax = 15.0f;
    newLimits.motorCurrentStall = 12.0f;
    newLimits.motorImbalanceWarn = 10.0f;
    newLimits.motorImbalanceFault = 25.0f;
    newLimits.speedMax = 20.0f;

    safety->setSafetyLimits(newLimits);
    SafetyLimits retrieved = safety->getSafetyLimits();

    TEST_ASSERT_EQUAL_FLOAT(50.0f, retrieved.batteryVoltageMin);
    TEST_ASSERT_EQUAL_FLOAT(40.0f, retrieved.batteryCurrentMax);
}

void setup() {
    delay(2000); // Wait for serial
    UNITY_BEGIN();

    RUN_TEST(test_safety_system_init);
    RUN_TEST(test_low_battery_detection);
    RUN_TEST(test_battery_voltage_normal);
    RUN_TEST(test_battery_overcurrent_detection);
    RUN_TEST(test_motor_left_overcurrent);
    RUN_TEST(test_motor_right_overcurrent);
    RUN_TEST(test_motor_imbalance_fault);
    RUN_TEST(test_motor_balance_ok);
    RUN_TEST(test_key_switch_off_fault);
    RUN_TEST(test_key_switch_on);
    RUN_TEST(test_clear_specific_fault);
    RUN_TEST(test_clear_all_faults);
    RUN_TEST(test_fault_history_recording);
    RUN_TEST(test_emergency_shutdown);
    RUN_TEST(test_safety_limits_update);

    UNITY_END();
}

void loop() {
    // Nothing to do here
}
