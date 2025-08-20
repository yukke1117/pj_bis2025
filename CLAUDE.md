# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an STM32U385RGTxQ microcontroller application for testing and monitoring various peripherals including internal DAC/ADC loopback testing and external ADS1299 ADC communication. Results are displayed on both LCD and UART serial output.

## Build and Development Commands

### Building the Project
```bash
# Build in Debug configuration (from Debug directory)
cd Debug && make clean && make all

# Build in Release configuration (if available)
cd Release && make clean && make all

# Build specific targets (from Debug/Release directory)
make pj-bis2025.elf
```

### Flashing to Target
Use STM32CubeIDE's built-in debugger or external tools like ST-LINK to flash the generated `.elf` file to the NUCLEO-U385RG-Q board.

### Serial Monitoring
Connect to UART5 with baud rate 115200 to monitor debug output:
```bash
# Example using screen on macOS/Linux
screen /dev/tty.usbserial-* 115200

# Alternative tools for different platforms
minicom -D /dev/ttyUSB0 -b 115200  # Linux
putty                              # Windows (GUI application)
picocom /dev/ttyUSB0 -b 115200    # Lightweight alternative
```

## Architecture Overview

### Core Components

1. **Main Application Loop** (`Core/Src/main.c`)
   - Initializes all peripherals (SPI1/2, DAC1, ADC1, OPAMP1/2, TIM3, UART5)
   - Performs DAC/ADC loopback testing with OPAMP1 as voltage follower
   - Communicates with external ADS1299 via SPI2
   - Updates LCD display and UART output with monitoring data

2. **LCD Display System** (`Core/Src/lcd.c`, `Core/Inc/lcd.h`)
   - SPI1-based communication with LCD module
   - Supports 4-bit grayscale rendering
   - Animation functions: scroll, bounce, blink text
   - Image display capability with logo shown at startup

3. **ADS1299 External ADC Interface**
   - SPI2 communication protocol implementation
   - Device ID reading and channel data acquisition
   - GPIO control pins: CS (Chip Select), RESET, DRDY (Data Ready)
   - Register-based configuration using defined command set

### Peripheral Configuration

- **SPI1**: LCD display communication
- **SPI2**: ADS1299 external ADC communication
- **DAC1**: Generates test signal (~0.5V fixed output)
- **ADC1**: Measures analog signals through OPAMP1
- **OPAMP1/2**: Configured as voltage followers
- **TIM3**: PWM generation for LCD backlight (EXTCOMIN signal)
- **UART5**: Debug message output at 115200 baud
- **GPIO**: Control pins for peripherals (CS, RESET, DISP, etc.)

### Pin Assignments

#### LCD Interface
- **LCD_CS**: PC9 (SPI1 Chip Select)
- **LCD_DISP**: PC10 (Display Enable)
- **LCD_EXTCOMIN**: PC8 (External COM Inversion - TIM3_CH3)

#### ADS1299 External ADC
- **ADC_CS**: PC7 (SPI2 Chip Select)
- **ADC_RESET**: PC6 (Hardware Reset)
- **ADC_DRDY**: PB15 (Data Ready Signal)

#### Debug Interface
- **DEBUG_JTMS_SWDIO**: PA13
- **DEBUG_JTCK_SWCLK**: PA14
- **DEBUG_JTDI**: PA15
- **DEBUG_JTDO_SWO**: PB3

### Key Data Flow

1. DAC1 outputs fixed voltage → OPAMP1 (voltage follower) → ADC1 measurement
2. ADS1299 data acquisition via SPI2 → Display on LCD/UART
3. All monitoring data updated in real-time loop with HAL_Delay intervals

## Project Structure

- `Core/`: Application code and HAL configuration
  - `Src/`: Source files including main.c, LCD drivers, image data
  - `Inc/`: Header files and configuration
- `Drivers/`: STM32 HAL drivers and CMSIS
- `Debug/`: Build output and makefiles
- `*.ioc`: STM32CubeMX configuration file
- `*.ld`: Linker scripts for FLASH and RAM configurations

## Important Implementation Details

- LCD uses 4-bit grayscale mode with line-by-line transmission
- ADS1299 register addresses defined in main.c (REG_ID through REG_GPIO)
- Fixed DAC output value of 625 (~0.5V at 3.3V reference)
- LCD displays startup logo for 3 seconds before entering monitoring mode
- All peripheral initializations handled by HAL MX_*_Init() functions

## Troubleshooting

### Common Issues and Solutions

#### Build Issues
- **"make: *** No targets specified" error**: Ensure you're in the Debug or Release directory before running make
- **Missing toolchain**: Install STM32CubeIDE or GNU Arm Embedded Toolchain
- **Header file not found**: Run STM32CubeMX code generation if HAL files are missing

#### Hardware Connection Issues
- **Serial port not found**: Check USB cable connection and driver installation
- **No UART output**: Verify baud rate is set to 115200 and UART5 pins are properly connected
- **Permission denied on serial port (Linux/macOS)**: Add user to dialout group or use `sudo`

#### ADS1299 Communication Issues
- **Device ID reads as 0x00 or 0xFF**: 
  - Check SPI2 connections (MOSI, MISO, SCLK)
  - Verify power supply to ADS1299
  - Ensure proper RESET sequence timing
- **DRDY signal not toggling**: Check ADC configuration registers and START command

#### LCD Display Issues
- **Blank display**: Check LCD_DISP pin is HIGH and power supply is stable
- **Corrupted display**: Verify SPI1 clock speed and timing parameters
- **No backlight**: Check TIM3 PWM output on LCD_EXTCOMIN pin

#### DAC/ADC Loopback Test Issues
- **ADC reading doesn't match DAC output**: 
  - Verify OPAMP1 is properly configured as voltage follower
  - Check physical connection between DAC1_OUT1 and OPAMP1_VINP
  - Ensure ADC1 calibration is performed