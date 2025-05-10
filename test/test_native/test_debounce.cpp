#include <unity.h>
#include <stdint.h>

// A simple debounce function that simulates the one in our main code
// Returns true if a pin state change should be accepted
bool debounceInput(uint8_t currentState, uint8_t* lastState, unsigned long currentTime, unsigned long* lastDebounceTime, unsigned long debounceDelay) {
    bool stateChanged = false;
    
    // If the switch changed, due to noise or pressing
    if (currentState != *lastState) {
        // Reset the debouncing timer
        *lastDebounceTime = currentTime;
    }
    
    // Check if enough time has passed since the last state change
    if ((currentTime - *lastDebounceTime) > debounceDelay) {
        // If the state has changed, accept it
        stateChanged = true;
    }
    
    // Save the current state for next comparison
    *lastState = currentState;
    
    return stateChanged;
}

// Test that noise is effectively filtered
void test_debounce_filters_noise() {
    uint8_t lastState = 0;
    unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 20; // 20ms debounce time
    
    // Simulate initial state (LOW)
    TEST_ASSERT_FALSE(debounceInput(0, &lastState, 0, &lastDebounceTime, debounceDelay));
    
    // Simulate noise (brief HIGH, less than debounce delay)
    TEST_ASSERT_FALSE(debounceInput(1, &lastState, 5, &lastDebounceTime, debounceDelay));
    
    // Simulate back to LOW before debounce period ends
    TEST_ASSERT_FALSE(debounceInput(0, &lastState, 10, &lastDebounceTime, debounceDelay));
    
    // No state change should be detected since not enough time has passed
    TEST_ASSERT_EQUAL_UINT8(0, lastState);
}

// Test that legitimate button presses are detected
void test_debounce_accepts_stable_input() {
    uint8_t lastState = 0;
    unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 20; // 20ms debounce time
    
    // Simulate initial state (LOW)
    TEST_ASSERT_FALSE(debounceInput(0, &lastState, 0, &lastDebounceTime, debounceDelay));
    
    // Simulate button press (HIGH, longer than debounce delay)
    TEST_ASSERT_FALSE(debounceInput(1, &lastState, 5, &lastDebounceTime, debounceDelay));
    
    // After debounce period has passed, state change should be accepted
    TEST_ASSERT_TRUE(debounceInput(1, &lastState, 30, &lastDebounceTime, debounceDelay));
    
    // State should now be HIGH
    TEST_ASSERT_EQUAL_UINT8(1, lastState);
}