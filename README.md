# Gesture Recognition Server/Service

This application acts as a service for gesture input API calls from client code such as [this](https://github.com/bronozoj/GestureAPISample). It connects to a gesture input device and runs recognition to provide real-time translated input that corresponds to movements.

## Compatibility

Currently, this works with the [BTW-61 Interial Measurement Unit](https://www.amazon.com/Accelerometer-Triple-Axis-Acceleration-Transducer-BWT61/dp/B018NL1R0Y) using the input DLL provided with the binaries. However, replacing the dll with your own interfacing dll can also allow any inertial measurement units to act as replacement input. Data accuracy, however, is not guaranteed.

The current codebase is also only compatible with the Windows NT family of operating systems but other operating systems are in the works.

## Usage

1. Download the latest release binaries and extract to any folder (preferrably without requiring administrator permissions).
2. Make sure that the IMU has been paired through bluetooth (or connected by wired serial) and that Windows recognizes the device. Take note of the serial COM port number.
3. Open `GestureServer.exe` and type the serial port number x as `COMx` or `\\.\COMx`.
4. Click the `Set` button and click the `Connect` button while the IMU is connected.
5. When a popup appears that it is connected, press `Ok`.
6. To exit, press `Disconnect` button then close the window.