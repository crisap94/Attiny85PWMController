// ATTINY85 ADC to PWM Mapping with 20x4 I2C Display using TinyWireM and Brightness Levels

// Uncomment the following line to enable LCD debugging
// #define ENABLE_LCD

#ifdef ENABLE_LCD
#include <TinyWireM.h> // Use TinyWireM for ATTINY85 I2C communication
#include <TinyLiquidCrystal_I2C.h> // Library compatible with TinyWireM
// Initialize the 20x4 LCD with I2C address 0x27
TinyLiquidCrystal_I2C lcd(0x27, 20, 4);
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Define the ADC input pin and PWM output pin
#define ADC_INPUT_PIN 3   // ADC pin (PB3)
#define PWM_OUTPUT_PIN 1  // PWM pin (PB1)
#define MODE_INPUT_PIN 4  // Digital input for mode switch (PB4)

// Global variables for time tracking
volatile unsigned long millisCounter = 0;
volatile uint8_t overflowCount = 0;
unsigned long lastUpdate = 0;

// Sampling configuration
#define NUM_SAMPLES 10
#define THRESHOLD 5
uint8_t samples[NUM_SAMPLES];
uint8_t sampleIndex = 0;

// Breathing mode variables
bool isBreathingMode = false;
uint8_t breathingBrightness = 0;
bool breathingDirection = true; // true = increasing, false = decreasing
unsigned long breathingStartTime = 0;
unsigned long breathingDuration = 60000; // 1 minute in milliseconds
uint8_t userConfiguredBrightness = 0;

// Breathing cycle time constants
#define BREATHING_CYCLE_MS 4000 // 4 seconds per cycle (theoretical value)
// Number of steps per complete cycle (up and down)
#define BREATHING_STEPS 256
// Calculate time per step in microseconds for precise timing
#define BREATHING_STEP_US ((unsigned long)BREATHING_CYCLE_MS * 1000UL / BREATHING_STEPS)

unsigned long lastBreathingUpdate = 0;

void setupADC() {
    // Configure ADC
    ADMUX = (1 << ADLAR) | (ADC_INPUT_PIN & 0x07); // Left-adjust result, use ADC_INPUT_PIN
    ADCSRA = (1 << ADEN)  | // Enable ADC
             (1 << ADPS1) | (1 << ADPS0); // Set prescaler to 8 (1 MHz/8 = 125 kHz)
}

uint8_t readADC() {
    ADCSRA |= (1 << ADSC); // Start conversion
    while (ADCSRA & (1 << ADSC)); // Wait for conversion to complete
    return ADCH; // Return the high byte (8-bit resolution)
}

void setupPWM() {
    // Configure PWM on PB1 (OC0B)
    DDRB |= (1 << PWM_OUTPUT_PIN); // Set PB1 as output

    // Set Timer0 to Fast PWM mode with non-inverted output
    TCCR0A = (1 << WGM00) | (1 << WGM01) | // Fast PWM mode
             (1 << COM0B1); // Non-inverting mode for OC0B

    TCCR0B = (1 << CS00); // Set prescaler to 1 for ~31.25 kHz PWM frequency
}

void setupModeInput() {
    DDRB &= ~(1 << MODE_INPUT_PIN); // Set PB4 as input
    PORTB |= (1 << MODE_INPUT_PIN); // Enable pull-up resistor on PB4
}

#ifdef ENABLE_LCD
void setupDisplay() {
    lcd.init(); // Initialize the LCD for 20x4
    lcd.backlight();  // Turn on the backlight
}

void updateLCD(uint8_t filteredADC, uint8_t pwmPercentage, unsigned long elapsedBreathingTime) {
    lcd.setCursor(0, 0); // First line
    lcd.print("ADC:");
    lcd.print(filteredADC);
    lcd.print("     ");

    lcd.setCursor(0, 1); // Second line
    lcd.print("PWM:");
    lcd.print(pwmPercentage);
    lcd.print("%       ");

    lcd.setCursor(0, 2); // Third line
    lcd.print("Time:");
    lcd.print(elapsedBreathingTime / 1000);
    lcd.print("s/");
    lcd.print(breathingDuration / 1000);
    lcd.print("s      ");

    lcd.setCursor(0, 3); // Fourth line
    lcd.print("Mode:");
    lcd.print(isBreathingMode ? "Breathing" : "Normal   ");
}
#endif

void setupTimer() {
    // Configure Timer1 for more precise timing
    // Clear all registers
    TCCR1 = 0;
    TCNT1 = 0;

    // ATtiny85 is running at 8MHz
    // With prescaler 8, each tick is 1µs (8MHz/8 = 1MHz = 1µs per tick)
    TCCR1 = (1 << CS10) | (1 << CS11); // Set prescaler to 8 (CS10=1, CS11=1)

    // Enable Timer1 overflow interrupt
    TIMSK |= (1 << TOIE1);

    sei(); // Enable global interrupts
}

ISR(TIMER1_OVF_vect) {
    // Timer1 is 8-bit, so it overflows every 256 ticks
    // With 1µs per tick, that's every 256µs
    // Count overflows to track time more precisely
    overflowCount++;

    // Every ~4 overflows is ~1ms (256µs * 4 = 1024µs ≈ 1ms)
    if (overflowCount >= 4) {
        millisCounter++;
        overflowCount = 0;
    }
}

unsigned long getMillis() {
    // Get current milliseconds count with temporary interrupt disable for atomic read
    unsigned long millisCopy;
    uint8_t tcnt1;

    cli(); // Disable interrupts
    tcnt1 = TCNT1; // Read current timer value
    millisCopy = millisCounter;
    uint8_t ovfCopy = overflowCount;
    sei(); // Re-enable interrupts

    // Calculate partial milliseconds from current timer value
    // and add any pending overflow counts
    unsigned long extraMicros = (tcnt1 + (ovfCopy * 256UL)) / 4;

    // Combine for more accurate time
    return millisCopy + (extraMicros / 1000);
}

uint8_t computeFilteredAverage() {
    uint16_t sum = 0;
    uint8_t count = 0;
    uint8_t baseline = samples[0]; // Start with the first sample as a baseline

    for (uint8_t i = 0; i < NUM_SAMPLES; i++) {
        if (abs(samples[i] - baseline) <= THRESHOLD) {
            sum += samples[i];
            count++;
        }
    }

    return (count > 0) ? (sum / count) : baseline; // Avoid division by zero
}

// Use a sine wave curve instead of linear transitions for more natural breathing effect
uint8_t sineBreathe(uint8_t maxValue, uint8_t position) {
    // Convert position (0-255) to a sine wave value (0-maxValue)
    // Using a faster approximation method suitable for 8-bit microcontrollers

    // Fast sine approximation for 0-255 input range
    uint8_t pos = position;
    if (pos > 127) pos = 255 - pos; // Mirror for second half

    // Calculate sine wave value (approximation using integer math)
    uint16_t output = pos * (255 - pos) / 32; // Parabolic approximation
    if (output > 127) output = 127; // Clamp to range

    // Scale to maxValue
    return (output * maxValue) / 127;
}

void handleBreathingMode() {
    unsigned long currentMicros = getMillis() * 1000UL; // Convert to microseconds
    unsigned long elapsedBreathingTime = currentMicros - breathingStartTime * 1000UL;

    // Exit breathing mode if the trigger is deactivated
    if (!(PINB & (1 << MODE_INPUT_PIN))) { // Check if PB4 is LOW
        isBreathingMode = false;
        breathingStartTime = 0;
        lastBreathingUpdate = 0;
        breathingBrightness = 0;
        OCR0B = userConfiguredBrightness; // Restore user-configured brightness
#ifdef ENABLE_LCD
        updateLCD(samples[sampleIndex == 0 ? NUM_SAMPLES - 1 : sampleIndex - 1], (userConfiguredBrightness * 100) / 255, 0);
#endif
        return;
    }

    // Continue breathing mode if trigger is active
    if (elapsedBreathingTime < breathingDuration * 1000UL) {
        // Calculate the position in the breathing cycle (0-255)
        uint32_t cyclePositionUs = elapsedBreathingTime % (BREATHING_CYCLE_MS * 1000UL);
        uint8_t position = (cyclePositionUs * 255) / (BREATHING_CYCLE_MS * 1000UL);

        // Apply sine wave modulation for natural breathing effect
        breathingBrightness = sineBreathe(userConfiguredBrightness, position);

        // Update PWM with calculated breathing brightness
        OCR0B = breathingBrightness;
    } else {
        // Smoothly return to user-configured brightness
        if (OCR0B < userConfiguredBrightness) {
            OCR0B++;
        } else if (OCR0B > userConfiguredBrightness) {
            OCR0B--;
        }
        isBreathingMode = false;
    }

#ifdef ENABLE_LCD
    updateLCD(samples[sampleIndex == 0 ? NUM_SAMPLES - 1 : sampleIndex - 1], (userConfiguredBrightness * 100) / 255, elapsedBreathingTime);
#endif
}

void setup() {
#ifdef ENABLE_LCD
    TinyWireM.begin(); // Initialize TinyWireM for I2C communication
#endif
    setupADC();
    setupPWM();
    setupModeInput();
#ifdef ENABLE_LCD
    setupDisplay();
#endif
    setupTimer();

    // Initialize samples array
    for (uint8_t i = 0; i < NUM_SAMPLES; i++) {
        samples[i] = 0;
    }
}

void loop() {
    unsigned long currentMillis = getMillis();
    static unsigned long lastADCUpdate = 0;

    // Only update ADC readings every 10ms to reduce processing overhead
    if (currentMillis - lastADCUpdate >= 10) {
        lastADCUpdate = currentMillis;

        // Read ADC input for user-configured brightness
        samples[sampleIndex] = readADC();
        sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;
        uint8_t filteredADC = computeFilteredAverage();
        userConfiguredBrightness = (filteredADC / 10) * 10; // Apply threshold
    }

    // Implement simple debouncing for mode switch
    static uint8_t lastPinState = 0;
    static unsigned long lastDebounceTime = 0;
    uint8_t currentPinState = PINB & (1 << MODE_INPUT_PIN); // Read current pin state

    // If pin state changed, reset debounce timer
    if (currentPinState != lastPinState) {
        lastDebounceTime = currentMillis;
    }

    // Check if debounce period has elapsed (20ms)
    if ((currentMillis - lastDebounceTime) > 20) {
        // If pin is HIGH and mode is not active, activate breathing mode
        if (currentPinState && !isBreathingMode) {
            isBreathingMode = true;
            breathingStartTime = currentMillis;
        }
    }

    lastPinState = currentPinState; // Save current state for next comparison

    // Handle modes
    if (isBreathingMode) {
        handleBreathingMode();
    } else {
        OCR0B = userConfiguredBrightness; // Directly set brightness if not breathing
#ifdef ENABLE_LCD
        updateLCD(samples[sampleIndex == 0 ? NUM_SAMPLES - 1 : sampleIndex - 1], (userConfiguredBrightness * 100) / 255, 0);
#endif
    }
}
