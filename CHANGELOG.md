# Changelog

All notable changes to the Razors Edge project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2025-11-07

### Added

#### Testing Infrastructure
- **Automated Testing Framework**: PlatformIO native unit tests for critical modules
  - Safety system test suite (15 test cases)
  - Motor controller test suite (13 test cases)
  - Test infrastructure in `test/` directory
  - GitHub Actions CI/CD integration for automated testing

#### Thread Safety Improvements
- **Fixed Thread Safety Issues**: Replaced volatile with std::atomic
  - Critical safety flags now use std::atomic for lock-free thread safety
  - Proper mutex-protected access for non-atomic shared data
  - Added comprehensive documentation of thread safety requirements
  - Eliminated race conditions in SharedSystemData structure

#### Logging Framework
- **Structured Logging System**: Professional logging with levels and formatting
  - Log levels: ERROR, WARN, INFO, DEBUG, TRACE
  - Timestamp support
  - Optional color-coded output
  - Module-specific tagging
  - Compile-time log level control
  - Global convenience macros (LOG_ERROR, LOG_WARN, etc.)

#### Configuration Management
- **NVS-Based Configuration**: Persistent settings with ConfigManager
  - Pedal calibration storage
  - Safety limits configuration
  - WiFi credentials management
  - Motor balance trim values
  - Display preferences
  - Factory reset capability
  - Runtime configuration updates

#### Over-The-Air (OTA) Updates
- **OTA Manager**: Secure firmware updates over WiFi
  - Password-protected updates
  - Progress tracking and callbacks
  - Safety interlocks (disable during driving)
  - Error handling and recovery
  - Automatic rollback on failure
  - Integration with web interface

#### Web Security
- **Authentication System**: Secure web interface access
  - Session-based authentication
  - Configurable username/password
  - Session timeout management
  - IP-based session validation
  - Token-based API access
  - WebServer integration helpers

#### Build System
- **Multiple Build Configurations**: Optimized builds for different scenarios
  - `release`: Production build (optimized, minimal logging)
  - `debug`: Development build (full logging, debugging symbols)
  - `esp32dev`: Default development build
  - `test`: Hardware-in-the-loop testing
  - `native`: Native testing (runs on PC)
  - Build number and git commit tracking

#### CI/CD Pipeline
- **GitHub Actions Workflow**: Automated build and test pipeline
  - Multi-environment builds (esp32dev, debug, release)
  - Automated testing on push/PR
  - Static code analysis
  - Firmware artifact generation
  - Automatic release builds
  - Build number and version tracking

#### Hardware Abstraction Layer
- **HAL Interfaces**: Testable hardware abstractions
  - IMotorDriver interface with MDD20A and Mock implementations
  - ICurrentSensor interface with INA228 and Mock implementations
  - IGPIO interface for Arduino and Mock GPIO
  - Enables unit testing without hardware
  - Simplified hardware swapping

#### Version Management
- **Semantic Versioning**: Comprehensive version tracking
  - Major.Minor.Patch versioning
  - Build number from git commit count
  - Git commit hash tracking
  - Build timestamp
  - Version information API
  - JSON version endpoint for web interface

### Changed

- **CoreManager.h**: Updated SharedSystemData to use std::atomic for thread safety
- **platformio.ini**: Restructured with multiple build environments
- **Build flags**: Added version macros and build configuration flags

### Technical Details

#### New Files Added
```
src/
├── Logger.h              # Structured logging framework
├── ConfigManager.h       # NVS configuration management
├── OTAManager.h         # Over-the-air update manager
├── WebAuth.h            # Web authentication system
├── Version.h            # Version information
└── HardwareAbstraction.h # Hardware abstraction layer

test/
├── test_safety_system/
│   └── test_safety.cpp   # Safety system tests
└── test_motor_controller/
    └── test_motor.cpp    # Motor controller tests

.github/workflows/
└── ci.yml               # GitHub Actions CI/CD pipeline
```

#### Dependencies
- No additional external dependencies required
- All new features use existing Arduino/ESP32 libraries
- Preferences library for NVS storage
- ArduinoOTA for firmware updates

#### Breaking Changes
- None - all additions are backward compatible
- Existing code continues to work without modification

#### Migration Notes
- To use new features, include appropriate headers in main.cpp
- Configuration manager should be initialized early in setup()
- Logger can replace existing Serial.print() statements gradually
- OTA updates require WiFi to be enabled
- Web authentication is optional and can be disabled

### Performance Impact
- Minimal overhead from logging (can be disabled at compile time)
- std::atomic operations are lock-free and fast
- Configuration reads from NVS only at startup
- OTA only active when explicitly enabled

### Security Improvements
- Session-based authentication for web interface
- Password-protected OTA updates
- Secure credential storage in NVS
- IP validation for sessions
- Configurable session timeouts

### Testing
- 28 unit tests covering safety and motor control
- CI/CD pipeline runs tests on every commit
- Mock hardware interfaces enable extensive testing
- Hardware-in-the-loop test environment available

### Documentation
- Inline code documentation for all new modules
- This CHANGELOG documents all changes
- Thread safety requirements documented in CoreManager.h
- Build configuration options documented in platformio.ini

---

## [1.0.0] - 2025-11-06

### Initial Release
- Dual-core ESP32 architecture
- 6-speed electronic transmission
- Dual motor control with current monitoring
- GPS geofencing system
- WiFi web interface
- OLED dashboard
- Multi-layer safety systems
- LED lighting control
- Power management with INA228 sensors
- Comprehensive documentation

---

**Version Format**: Major.Minor.Patch
- **Major**: Breaking changes or major features
- **Minor**: New features, backward compatible
- **Patch**: Bug fixes and minor improvements
