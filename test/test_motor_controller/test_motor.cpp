#include <unity.h>
#include "../../src/MotorController.h"
#include "../../src/config.h"

MotorController* motors;

void setUp(void) {
    motors = new MotorController();
    motors->init();
}

void tearDown(void) {
    delete motors;
}

// Test initialization
void test_motor_controller_init(void) {
    TEST_ASSERT_EQUAL_FLOAT(0.0f, motors->getCurrentSpeedLeft());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, motors->getCurrentSpeedRight());
    TEST_ASSERT_EQUAL(GEAR_PARK, motors->getCurrentGear());
}

// Test gear changes
void test_gear_change_to_first(void) {
    motors->setGear(GEAR_1ST);
    TEST_ASSERT_EQUAL(GEAR_1ST, motors->getCurrentGear());
}

void test_gear_change_sequence(void) {
    motors->setGear(GEAR_1ST);
    TEST_ASSERT_EQUAL(GEAR_1ST, motors->getCurrentGear());

    motors->setGear(GEAR_2ND);
    TEST_ASSERT_EQUAL(GEAR_2ND, motors->getCurrentGear());

    motors->setGear(GEAR_3RD);
    TEST_ASSERT_EQUAL(GEAR_3RD, motors->getCurrentGear());
}

// Test speed control
void test_set_speed_basic(void) {
    motors->setGear(GEAR_1ST);
    motors->setSpeed(50.0f);
    motors->update(); // Process ramping

    // Speed should start ramping up
    float currentSpeed = motors->getCurrentSpeedLeft();
    TEST_ASSERT_GREATER_OR_EQUAL(0.0f, currentSpeed);
    TEST_ASSERT_LESS_OR_EQUAL(50.0f, currentSpeed);
}

void test_emergency_stop(void) {
    motors->setGear(GEAR_2ND);
    motors->setSpeed(50.0f);
    motors->update();

    motors->emergencyStop();

    TEST_ASSERT_EQUAL_FLOAT(0.0f, motors->getTargetSpeedLeft());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, motors->getTargetSpeedRight());
}

void test_stop_function(void) {
    motors->setGear(GEAR_2ND);
    motors->setSpeed(50.0f);
    motors->update();

    motors->stop();

    TEST_ASSERT_EQUAL_FLOAT(0.0f, motors->getTargetSpeedLeft());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, motors->getTargetSpeedRight());
}

// Test direction control
void test_direction_forward(void) {
    motors->setDirection(true);
    TEST_ASSERT_TRUE(motors->isForward());
}

void test_direction_reverse(void) {
    motors->setDirection(false);
    TEST_ASSERT_FALSE(motors->isForward());
}

// Test park gear prevents movement
void test_park_gear_prevents_movement(void) {
    motors->setGear(GEAR_PARK);
    motors->setSpeed(100.0f); // Try to set full speed
    motors->update();

    // Speed should remain zero in park
    TEST_ASSERT_EQUAL_FLOAT(0.0f, motors->getCurrentSpeedLeft());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, motors->getCurrentSpeedRight());
}

// Test individual motor control
void test_individual_motor_speeds(void) {
    motors->setGear(GEAR_2ND);
    motors->setSpeedLeft(30.0f);
    motors->setSpeedRight(40.0f);

    TEST_ASSERT_EQUAL_FLOAT(30.0f, motors->getTargetSpeedLeft());
    TEST_ASSERT_EQUAL_FLOAT(40.0f, motors->getTargetSpeedRight());
}

// Test control modes
void test_control_mode_synchronized(void) {
    motors->setControlMode(MOTOR_SYNCHRONIZED);
    TEST_ASSERT_EQUAL(MOTOR_SYNCHRONIZED, motors->getControlMode());
}

void test_control_mode_differential(void) {
    motors->setControlMode(MOTOR_DIFFERENTIAL);
    TEST_ASSERT_EQUAL(MOTOR_DIFFERENTIAL, motors->getControlMode());
}

void test_control_mode_balanced(void) {
    motors->setControlMode(MOTOR_BALANCED);
    TEST_ASSERT_EQUAL(MOTOR_BALANCED, motors->getControlMode());
}

void setup() {
    delay(2000);
    UNITY_BEGIN();

    RUN_TEST(test_motor_controller_init);
    RUN_TEST(test_gear_change_to_first);
    RUN_TEST(test_gear_change_sequence);
    RUN_TEST(test_set_speed_basic);
    RUN_TEST(test_emergency_stop);
    RUN_TEST(test_stop_function);
    RUN_TEST(test_direction_forward);
    RUN_TEST(test_direction_reverse);
    RUN_TEST(test_park_gear_prevents_movement);
    RUN_TEST(test_individual_motor_speeds);
    RUN_TEST(test_control_mode_synchronized);
    RUN_TEST(test_control_mode_differential);
    RUN_TEST(test_control_mode_balanced);

    UNITY_END();
}

void loop() {
    // Nothing
}
