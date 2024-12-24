# ATTINY85 ADC to PWM Controller with Breathing Effect

This repository contains a firmware for the ATTINY85 microcontroller that implements brightness control for a monochrome LED strip using an ADC input. It includes a breathing effect, user-configurable brightness, and optional debugging via a 20x4 I2C LCD display.

## Features

- **ADC to PWM Mapping**: Reads analog input from a potentiometer or sensor and maps it to PWM output for LED brightness control.
- **Breathing Effect**: Provides a smooth brightness transition in a breathing pattern lasting 4 seconds per cycle.
- **User-Configurable Brightness**: Dynamically adjusts brightness based on the ADC input.
- **LCD Debugging (Optional)**: Displays real-time information such as ADC values, PWM percentages, elapsed breathing time, and operational mode.
- **Consistent Timing**: Maintains consistent breathing cycle durations regardless of brightness settings.

## Hardware Requirements

- **ATTINY85 Microcontroller**: Configured for an internal 8 MHz clock.
- **Monochrome LED Strip**: Controlled via the PWM output (PB1) with a suitable transistor driver.
- **Potentiometer**: Connected to the ADC input (PB3) for brightness adjustment.
- **20x4 I2C LCD Display (Optional)**: Used for debugging, connected via TinyWireM to PB0 and PB2.
- **Mode Switch**: Connected to PB4 to toggle the breathing effect.

## Pin Configuration

| Pin     | Function           |
|---------|--------------------|
| PB0/PB2 | I2C (for LCD)      |
| PB1     | PWM Output         |
| PB3     | ADC Input          |
| PB4     | Mode Switch Input  |

## Software Setup

1. **Install Dependencies**:
   - Use the `TinyWireM` and `TinyLiquidCrystal_I2C` libraries for I2C communication (if using the LCD).
2. **Configuration**:
   - Uncomment `#define ENABLE_LCD` in the code to enable LCD functionality.
   - Adjust hardware connections based on the pin configuration.
3. **Upload Firmware**:
   - Use an ISP programmer to upload the firmware to the ATTINY85 via the Arduino IDE.

## How It Works

1. **ADC to PWM**:
   - Reads the ADC input (PB3) and maps the value to a PWM duty cycle (PB1).
   - Brightness is smoothed with a threshold and averaging technique to reduce noise.

2. **Breathing Effect**:
   - Triggered by setting PB4 to HIGH.
   - Smoothly transitions brightness from 0 to maximum and back over a 4-second cycle.
   - Automatically reverts to user-configured brightness after 60 seconds.

3. **LCD Debugging**:
   - Displays:
     - Current ADC value.
     - PWM percentage.
     - Elapsed breathing time.
     - Mode (Breathing or Normal).

## Example Connections

### Circuit Diagram
```
+---------+       +----------+       +-----------------+
| Potentiometer |  --->  | ATTINY85  | --->  | LED Strip Driver |
+---------+       +----------+       +-----------------+
       PB3 (ADC Input)       PB1 (PWM Output)
```

### Optional LCD Display
```
+------------+       +----------+
| 20x4 I2C LCD | ---> | ATTINY85 |
+------------+       +----------+
      PB0, PB2 (I2C Lines)
```

## Known Issues

- Ensure the ATTINY85 is configured for an 8 MHz internal clock.
- LCD functionality requires additional libraries (`TinyWireM` and `TinyLiquidCrystal_I2C`).

## Contributions

Contributions are welcome! Please feel free to open issues or submit pull requests to enhance the project.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
