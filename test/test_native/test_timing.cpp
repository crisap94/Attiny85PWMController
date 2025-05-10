#include <unity.h>
#include <stdint.h>

// Simple timing function test
// These tests verify the timing calculations used in our code

// Constants from the main code
#define BREATHING_CYCLE_MS 4000
#define BREATHING_STEPS 256
#define BREATHING_STEP_US ((unsigned long)BREATHING_CYCLE_MS * 1000UL / BREATHING_STEPS)

// Test that the step time calculation matches the expected value
void test_breathing_step_time() {
    // Expected step time: 4000ms * 1000 / 256 = 15,625 microseconds
    TEST_ASSERT_EQUAL_UINT32(15625UL, BREATHING_STEP_US);
}

// Test that we have enough steps for smooth animation
void test_breathing_steps_sufficient() {
    // For a smooth animation, we need at least 30 steps per second
    // For a 4-second cycle, that's at least 120 steps
    TEST_ASSERT_GREATER_THAN_UINT32(120, BREATHING_STEPS);
}

// Test that the total cycle time calculation is correct
void test_breathing_cycle_total_time() {
    // Total time should be step time * number of steps
    unsigned long total_us = BREATHING_STEP_US * BREATHING_STEPS;
    TEST_ASSERT_EQUAL_UINT32(BREATHING_CYCLE_MS * 1000UL, total_us);
}

// Calculate cycle position from elapsed time
uint8_t calculate_position(unsigned long elapsed_us) {
    uint32_t cycle_position_us = elapsed_us % (BREATHING_CYCLE_MS * 1000UL);
    return (cycle_position_us * 255) / (BREATHING_CYCLE_MS * 1000UL);
}

// Test the cycle position calculation
void test_cycle_position_calculation() {
    // At 0ms, position should be 0
    TEST_ASSERT_EQUAL_UINT8(0, calculate_position(0));

    // At 2000ms (half cycle), position should be ~127
    TEST_ASSERT_EQUAL_UINT8(127, calculate_position(2000000));

    // At 4000ms (full cycle), position should be 0 again (wraparound)
    TEST_ASSERT_EQUAL_UINT8(0, calculate_position(4000000));

    // At 6000ms (1.5 cycles), position should be ~127 again
    TEST_ASSERT_EQUAL_UINT8(127, calculate_position(6000000));
}