# Google Meet HID Telephony Device

A hardware mute button controller for Google Meet using RP2040-Zero that implements true HID Telephony protocol (Usage Page 0x0B) with bidirectional communication and visual feedback.

## Overview

This project implements a dedicated hardware mute button for Google Meet that goes beyond simple keyboard emulation. Using the HID Telephony specification, it creates a proper telephony device that can both send commands to and receive status updates from Google Meet via WebHID API.

## Features

- **True HID Telephony Implementation** - Uses Usage Page 0x0B instead of keyboard emulation
- **Bidirectional Communication** - Sends mute commands and receives status updates
- **Visual Feedback** - NeoPixel LED indicates current mute status
- **Composite USB Device** - Combines Telephony and Consumer Control interfaces
- **WebHID API Integration** - Direct communication with Google Meet web application
- **3D Printable Enclosure** - Custom designed case for the hardware

## Hardware Requirements

- **Microcontroller**: RP2040-Zero (Waveshare)
- **Components**:
  - Built-in BOOT button (used as mute button)
  - Built-in NeoPixel LED (WS2812)
  - USB-C cable for connection
- **Optional**: 3D printed enclosure (STL files included)

## Software Dependencies

### Firmware
- Arduino IDE (1.8.x or 2.x)
- Raspberry Pi Pico/RP2040 board package (Earle Philhower version)
- Adafruit TinyUSB Library
- Adafruit NeoPixel Library
- Bounce2 Library (for button debouncing)

### Host Application
- Google Chrome (version 89+ with WebHID support)
- Google Meet web application

## Installation

### 1. Arduino IDE Setup

1. Install Arduino IDE from [arduino.cc](https://www.arduino.cc/)
2. Add RP2040 board support:
   - Open Preferences → Additional Board Manager URLs
   - Add: `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
3. Install board package:
   - Tools → Board → Boards Manager
   - Search and install "Raspberry Pi Pico/RP2040"
4. Install required libraries:
   - Adafruit TinyUSB Library
   - Adafruit NeoPixel
   - Bounce2

### 2. Firmware Upload

1. Connect RP2040-Zero while holding BOOT button
2. Select board: Tools → Board → Raspberry Pi Pico
3. Select USB Stack: Tools → USB Stack → Adafruit TinyUSB
4. Upload the sketch

### 3. Google Meet Setup

1. Open Google Meet in Chrome
2. Allow WebHID device permission when prompted
3. The device will automatically connect when joining a meeting

## Usage

- **Press Button**: Toggle mute/unmute in Google Meet
- **LED Indicators**:
  - Red: Muted
  - Green: Unmuted
  - Blue: Connecting/Initializing

## Technical Architecture

### USB Descriptor Structure
```
Device
├── Configuration
│   ├── Interface 0: HID Telephony
│   │   └── Endpoint IN (Interrupt)
│   │   └── Endpoint OUT (Interrupt)
│   └── Interface 1: Consumer Control
│       └── Endpoint IN (Interrupt)
```

### HID Report Format
- **Input Report** (1 byte): Hook Switch state (bit 5)
- **Output Report** (1 byte): LED state from host

### Key Design Decisions

1. **RP2040 over ATmega32U4/SAMD21**: Native USB support, dual-core processing, cost-effectiveness
2. **TinyUSB over HID-Project**: Better composite device support, more flexible descriptor management
3. **HID Telephony over Keyboard Emulation**: Proper semantic meaning, no keystroke conflicts, bidirectional communication
4. **WebHID over Native Application**: Cross-platform compatibility, no driver installation required


## Future Enhancements

- **Extended Controls**: Volume adjustment, video toggle, screen share
- **Multiple Usage IDs**: Support for different conferencing platforms
- **Configuration Interface**: WebUSB-based settings management
- **ESP32-S3 Migration**: Wi-Fi integration for remote control
- **Feature Reports**: Advanced configuration via HID Feature Reports
- **PIO Optimization**: Using RP2040's PIO for advanced LED effects

## Troubleshooting

### Device Not Recognized
- Ensure Chrome has WebHID support enabled
- Check USB cable supports data transfer
- Try different USB port

### LED Not Updating
- Verify Google Meet has device permissions
- Check if mute status sync is enabled in Meet settings

### Button Not Working
- Confirm device appears in Chrome's HID device list
- Check serial monitor for debug output
- Ensure proper USB descriptor enumeration


## References

- [USB HID Usage Tables 1.12](https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf)
- [WebHID API Specification](https://wicg.github.io/webhid/)
- [RP2040 Datasheet](https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf)
- [TinyUSB Documentation](https://docs.tinyusb.org/)
- [Bounce2 Library](https://github.com/thomasfredericks/Bounce2)

## License

MIT License

Copyright (c) 2025 TOKIDA

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


## Acknowledgments

- Adafruit for TinyUSB library and excellent documentation
- Raspberry Pi Foundation for RP2040 development tools
- Google Chrome team for WebHID API implementation
- Thomas O Fredericks for Bounce2 library
- Open source community for various code examples and inspirations
