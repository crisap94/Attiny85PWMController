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
unsigned int breathingCycleTime = 80000; // 4 seconds per cycle
unsigned int breathingStepTime = (breathingCycleTime * 1000) / 512; // Adjusted for accurate step timing with millisCounter
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
    // Configure Timer1 for millisecond counting
    TCCR1 = (1 << CS11) | (1 << CS10); // Prescaler 64
    TIMSK = (1 << TOIE1); // Enable overflow interrupt
    sei(); // Enable global interrupts
}

ISR(TIMER1_OVF_vect) {
    millisCounter += 2; // Increment by 2ms per overflow for correct timing with an 8 MHz clock and prescaler 64
}

unsigned long getMillis() {
    unsigned long millisCopy;
    cli();
    millisCopy = millisCounter;
    sei();
    return millisCopy;
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

void handleBreathingMode() {
    unsigned long currentMillis = getMillis();
    unsigned long elapsedBreathingTime = currentMillis - breathingStartTime;

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
    if (elapsedBreathingTime < breathingDuration) {
        if (currentMillis - lastBreathingUpdate >= breathingStepTime) {
            lastBreathingUpdate = currentMillis;
            int stepSize = max(1, userConfiguredBrightness / 256); // Proportional increment
            if (breathingDirection) {
                breathingBrightness += stepSize;
                if (breathingBrightness >= userConfiguredBrightness) {
                    breathingDirection = false;
                }
            } else {
                breathingBrightness -= stepSize;
                if (breathingBrightness <= 0) {
                    breathingDirection = true;
                }
            }
        }
        // Update PWM with breathing brightness
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

    // Read ADC input for user-configured brightness
    samples[sampleIndex] = readADC();
    sampleIndex = (sampleIndex + 1) % NUM_SAMPLES;
    uint8_t filteredADC = computeFilteredAverage();
    userConfiguredBrightness = (filteredADC / 10) * 10; // Apply threshold

    // Check if mode switch is activated
    if (PINB & (1 << MODE_INPUT_PIN)) { // Check if PB4 is HIGH
        if (!isBreathingMode) {
            isBreathingMode = true;
            breathingStartTime = currentMillis;
            lastBreathingUpdate = currentMillis;
        }
    }

    if (isBreathingMode) {
        handleBreathingMode();
    } else {
        OCR0B = userConfiguredBrightness; // Directly set brightness if not breathing
#ifdef ENABLE_LCD
        updateLCD(filteredADC, (userConfiguredBrightness * 100) / 255, 0);
#endif
    }
}
