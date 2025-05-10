#include <unity.h>

// External test function declarations
extern void test_sineBreathe_zero_position(void);
extern void test_sineBreathe_quarter_position(void);
extern void test_sineBreathe_half_position(void);
extern void test_sineBreathe_three_quarter_position(void);
extern void test_sineBreathe_full_position(void);
extern void test_sineBreathe_scaled_output(void);

extern void test_breathing_step_time(void);
extern void test_breathing_steps_sufficient(void);
extern void test_breathing_cycle_total_time(void);
extern void test_cycle_position_calculation(void);

extern void test_debounce_filters_noise(void);
extern void test_debounce_accepts_stable_input(void);

void setUp(void) {
    // Set up test environment before each test case
}

void tearDown(void) {
    // Clean up after each test case
}

int main(void) {
    UNITY_BEGIN();

    // Run all tests for breathing functions
    RUN_TEST(test_sineBreathe_zero_position);
    RUN_TEST(test_sineBreathe_quarter_position);
    RUN_TEST(test_sineBreathe_half_position);
    RUN_TEST(test_sineBreathe_three_quarter_position);
    RUN_TEST(test_sineBreathe_full_position);
    RUN_TEST(test_sineBreathe_scaled_output);

    // Run all tests for timing
    RUN_TEST(test_breathing_step_time);
    RUN_TEST(test_breathing_steps_sufficient);
    RUN_TEST(test_breathing_cycle_total_time);
    RUN_TEST(test_cycle_position_calculation);

    // Run all tests for debounce functionality
    RUN_TEST(test_debounce_filters_noise);
    RUN_TEST(test_debounce_accepts_stable_input);

    return UNITY_END();
}