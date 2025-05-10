#include <Arduino.h>
#include <unity.h>

// This test runs on the actual hardware
// It requires a ATtiny85 connected to your computer

// Define pins for test validation
#define TEST_PWM_PIN 1  // PB1, same as in main code

void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Clean up after each test
}

// Test PWM setup
void test_pwm_setup(void) {
    // Configure PWM on PB1 (OC0B)
    DDRB |= (1 << TEST_PWM_PIN); // Set PB1 as output
    
    // Set Timer0 to Fast PWM mode with non-inverted output
    TCCR0A = (1 << WGM00) | (1 << WGM01) | // Fast PWM mode
             (1 << COM0B1); // Non-inverting mode for OC0B
    
    TCCR0B = (1 << CS00); // Set prescaler to 1 for ~31.25 kHz PWM frequency
    
    // Check if the TCCR0A and TCCR0B registers are set correctly
    TEST_ASSERT_BITS(0xFF, 0b00100011, TCCR0A);
    TEST_ASSERT_BITS(0xFF, 0b00000001, TCCR0B);
    
    // Check if PB1 is set as output
    TEST_ASSERT_BIT_HIGH(TEST_PWM_PIN, DDRB);
}

// Test PWM Output
void test_pwm_output(void) {
    // Set PWM output to 50%
    OCR0B = 127;
    delay(10);
    // Without specialized equipment, we can only verify the register values
    TEST_ASSERT_EQUAL_INT(127, OCR0B);
    
    // Set PWM output to 25%
    OCR0B = 64;
    delay(10);
    TEST_ASSERT_EQUAL_INT(64, OCR0B);
    
    // Set PWM output to 75%
    OCR0B = 191;
    delay(10);
    TEST_ASSERT_EQUAL_INT(191, OCR0B);
}

void setup() {
    delay(2000); // Allow the MCU to power up fully
    
    UNITY_BEGIN();
    RUN_TEST(test_pwm_setup);
    RUN_TEST(test_pwm_output);
    UNITY_END();
}

void loop() {
    // Nothing to do here
}