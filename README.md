# ATTINY85 ADC to PWM Controller with Breathing Effect

This repository contains a firmware for the ATTINY85 microcontroller that implements brightness control for a monochrome LED strip using an ADC input. It includes a breathing effect, user-configurable brightness, and optional debugging via a 20x4 I2C LCD display.

## Features

- **ADC to PWM Mapping**: Reads analog input from a potentiometer or sensor and maps it to PWM output for LED brightness control. For example, if the ADC reads a value of 512 (midpoint), it translates to a 50% duty cycle on the PWM output, ensuring the LED brightness is proportionally adjusted.
- **Breathing Effect**: Provides a smooth sine-wave brightness transition in a breathing pattern lasting exactly 4 seconds per cycle with improved timing accuracy.
- **User-Configurable Brightness**: Dynamically adjusts brightness based on the ADC input.
- **LCD Debugging (Optional)**: Displays real-time information such as ADC values, PWM percentages, elapsed breathing time, and operational mode.
- **Consistent Timing**: Maintains consistent breathing cycle durations regardless of brightness settings.

## Hardware Requirements

- **Frequency**: Recommended PWM frequency for COB LED strips is 5 kHz or higher to prevent flickering.

- **ATTINY85 Microcontroller**: Configured for an internal 8 MHz clock.
- **Monochrome LED Strip**: Controlled via the PWM output (PB1) with a suitable transistor driver.
- **Potentiometer**: Connected to the ADC input (PB3) for brightness adjustment.
- **20x4 I2C LCD Display (Optional)**: Used for debugging, connected via TinyWireM to PB0 and PB2.
- **Mode Switch**: Connected to PB4 to toggle the breathing effect.
  - Pull-down resistor: 6.8 kΩ to ensure a stable LOW signal when the switch is not active.
  - Series resistor: 6.8 kΩ to protect the pin when reading the signal.

## Pin Configuration

| Pin     | Function           |
|---------|--------------------|
| PB0/PB2 | I2C (for LCD)      |
| PB1     | PWM Output         |
| PB3     | ADC Input          |
| PB4     | Mode Switch Input  |

## Software Setup

1. **Install Dependencies**:
   - Use the `TinyWireM` and `TinyLiquidCrystal_I2C` libraries for I2C communication (if using the LCD):
     - `TinyWireM`: [GitHub Link](https://github.com/adafruit/TinyWireM)
     - `TinyLiquidCrystal_I2C`: [GitHub Link](https://github.com/lucas-inacio/TinyLiquidCrystal_I2C)
   - Add these libraries to your Arduino IDE libraries folder.

2. **Configuration**:
   - Uncomment `#define ENABLE_LCD` in the code to enable LCD functionality.
   - Adjust hardware connections based on the pin configuration.

3. **Upload Firmware**:
   - Use an ISP programmer to upload the firmware to the ATTINY85 via the Arduino IDE.
   - Ensure that the ATTINY85 is configured to use its 8 MHz internal clock.

## How It Works

### Note on PWM Frequency and Timing Accuracy
The improved timing algorithm uses a more precise timer configuration to ensure consistent 4-second breathing cycles. A higher PWM frequency (~31 kHz) reduces visible flickering in COB LED strips, while the sine-wave brightness modulation creates more natural transitions than linear fading. The careful timer calculation and microsecond-level timing provide significantly better accuracy compared to older versions.

### Smoothing Algorithm
- The ADC readings are processed using a smoothing algorithm:
  - Samples are stored in a circular buffer (`NUM_SAMPLES` = 10).
  - Only values within a `THRESHOLD` difference from the baseline are averaged to reduce noise.
  - This filtered value determines the LED brightness.

### Sampling
- ADC samples are taken every main loop iteration.
- The filtered value ensures consistent brightness adjustments without sudden changes.

1. **ADC to PWM**:
   - Reads the ADC input (PB3) and maps the value to a PWM duty cycle (PB1).
   - Brightness is smoothed with a threshold and averaging technique to reduce noise.

2. **Breathing Effect**:
   - Triggered by setting PB4 to HIGH (with proper debouncing for reliability).
   - Smoothly transitions brightness using a sine-wave pattern for more natural breathing effect.
   - Accurate 4-second cycle length thanks to improved microsecond-level timing.
   - Automatically reverts to user-configured brightness after 60 seconds.

3. **LCD Debugging**:
   - Displays:
     - Current ADC value.
     - PWM percentage.
     - Elapsed breathing time.
     - Mode (Breathing or Normal).

## Troubleshooting

- **LCD Not Displaying**:
  - Verify the I2C connections to PB0 and PB2.
  - Check that the I2C address matches your LCD module (default: 0x27).

- **ADC Noise Issues**:
  - Ensure proper grounding and consider adding a capacitor (e.g., 0.1uF) near the ADC input.

- **Breathing Mode Not Activating**:
  - Confirm that PB4 is set HIGH for triggering the mode.
  - Check for pull-up resistor configuration on PB4.

## Customization

- Adjust `BREATHING_CYCLE_MS` and `breathingDuration` in the code for different breathing timings. Default values: `BREATHING_CYCLE_MS` is set to 4000 ms for a precise 4-second breathing cycle, and `breathingDuration` is set to 60000 ms for a maximum breathing time of 60 seconds.
- Modify the ADC threshold and smoothing algorithm for varying levels of noise filtering.

## Tested Configurations

- **Microcontroller**: ATTINY85 at 8 MHz internal clock.
- **LCD Display**: 20x4 I2C with address 0x27.
- **LED Strip**: Monochrome strip driven via a transistor connected to PB1.

## Continuous Integration

This project uses GitHub Actions for continuous integration:

- **Test Workflow**: Automatically runs unit tests when pull requests are opened or code is pushed to master.
- **Build Workflow**: Builds the firmware and verifies compilation.

You can see the status of these workflows on the GitHub repository's Actions tab.

## Contributions

Contributions are welcome! Please feel free to open issues or submit pull requests to enhance the project. When submitting pull requests, please ensure that:

1. All tests pass successfully
2. New features include appropriate tests
3. Code follows the existing style and conventions

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

