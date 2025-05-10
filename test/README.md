# Testing the ATtiny85 PWM Controller

This directory contains tests for the ATtiny85 PWM Controller project. The tests are set up using PlatformIO's unit testing framework, which is completely free and open-source.

## Test Structure

The tests are divided into two categories:

1. **Native Tests** (`test_native/`) - These tests run on your computer without requiring any hardware. They test platform-independent code like algorithmic functions.

2. **Embedded Tests** (`test_embedded/`) - These tests run on the actual ATtiny85 hardware and verify hardware-dependent functionality like PWM output.

## Running the Tests

### Prerequisites

1. Install PlatformIO Core (CLI) or PlatformIO IDE (VSCode extension)
2. For embedded tests: Connect an ATtiny85 device to your computer with a programmer

### Running Native Tests

```bash
# Navigate to the project directory
cd "/home/crisap94/Documents/Embeded Projects/Attiny85PWMController"

# Run native tests
pio test -e native
```

### Running Embedded Tests

```bash
# Navigate to the project directory
cd "/home/crisap94/Documents/Embeded Projects/Attiny85PWMController"

# Run embedded tests (requires hardware)
pio test -e attiny85
```

## Test Descriptions

### Native Tests

- `test_breathing_functions.cpp` - Tests the sine wave breathing effect calculation function

### Embedded Tests

- `test_pwm.cpp` - Tests PWM configuration and output on the actual hardware

## Adding New Tests

1. For native tests, add new `.cpp` files to the `test_native/` directory
2. For embedded tests, add new `.cpp` files to the `test_embedded/` directory
3. Follow the Unity test framework syntax for writing test cases

## Troubleshooting

- If you encounter issues with native tests, make sure your development environment has the necessary compilers installed
- For embedded tests, verify that your programmer is correctly connected and recognized
- Check the PlatformIO documentation for more information: https://docs.platformio.org/en/latest/advanced/unit-testing/