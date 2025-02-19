# Version ???

## General Features

- Sensor decoder activate delay and deactivate delay now support milliseconds

## Broker

- Sensor decoder added attribute activateDelayMs and deactivateDelayMs for configuring delays in milliseconds
- Protocol version bumped to 9 (sensor delay changes)
- Added code to keep sensor compatible with old decoders
- NetworkDevices not store protocol version
- All socket ports listed on a single file
- Added unit test for sensors

## Emulator

- Fixed pins initial state
- Support to custom EEPROM and custom names for modules

## LiteDecoder

- Decoders storage version bumped to 18

## SharpTerminal

- Support to new sensor with milliseconds delay

## SharpEEPromViewer

- Support to new sensor with milliseconds delay and legacy sensors

## Code

- Updated to stop using deprecated OpenSSL functions for computing SHA1
- Parser is using StringView
- Better organization of Common and Shared libs

# Version 0.9.0

## General Features

- Added support to config flip interval on quad inverter
- Lua sections now publish state to JMRI thought VirtualSensors
- Added support to rename devices and better connection support for unconfigured devices

## Broker

- Support to unnamed devices (auto generate a name with IP)
- Better validation of json data
- fixed sample "MyRailroad"

## SharpTerminal

- Auto reconnect when Broker is closed
- Server selection screen accepts ENTER as a shortcut
- Now uses NetCore
- Added csproj file (removed from CMAKE)

## SharpEEPromViewer

- Support to legacy (deprecated) lumps
- Support to new lumps
- Now uses NetCore
- Added csproj file (removed from CMAKE)

## Scripts

- section scripts seems to be working

## Embedded

- Auto generate mac address
- Updated Session and NetUdp lumps
- cfg command now only requires name, server port is optional (use default port if not provided)
- added clr command to erase EEPROM
- arduino will blink according to state: 
    - slow flash: network init
    - fast flag: trying to connect
    - on: connected
    - quick pulse while on: packet received

## Code

- Added DecoderWeakPointer
- Added own implementation of DirectoryMonitor (win32 only for now)
- Removed code to convert win32 error messages and replaced with std::system_category
- Moved DirectoryMonitor to its own library
- Removed built in DirectoryMonitor
- Fixed unit tests inside visual studio
- Improved SignalDecoder tests
- Added code to detect usage of special arduino pins, like ethercard pins
- fixed lua script to correctly handle sensor events
- ScriptService shows debug info when a Lua function blows up...

# Version 0.8.0 - 04/06/2023

## General Features
- Added QuadInverter decoder
- Added support to Lua scripts

## Broker
- Fix: exception on certain terminal client disconnect behaviors
- win32: SetThreadName for easier debugging
- Standarized log messages

## Code

- Fixed vscode project
- Code cleanup on embedded console
- Test case for EventHub
- BrokerLib: Thinkers linked list now is double linked for fast removal
- Common: Fixed Socket move assignement
- BrokerLib: EventHub now has a pool and internal list, avoid excessive mem alloc.
- BrokerLib: renamed "Messenger" to EventHub 
- Fixed SignalDecoder tests
- Added UnitTest for EventHub

# Version 0.7.1 - 17/10/2022

## Broker
- Fix: Correctly handling broken devices messages and avoiding infinite message loop
- Fix: SignalController now correctlys handles WaitTurnOff timeout avoiding crash
- Added name to Thinkers for easier debugging

# Version 0.7.0 - 16/10/2022

## General Features
- Added TurntableAutoInverterDecoder for easy turntable polarity inversion

## SharpTerminal
- Allow user to set signal aspects from SignalDecoder panel
- Display RemoteDevice free ram value
- Added support to TurntableAutoInverterDecoder

## SharpEEPromViewer
- Correctly showing decoders slots
- Added support to TurntableAutoInverterDecoder

## Code

- Using sigslot library (https://github.com/palacaze/sigslot), removed manual listeners code from Services
- SignalManager does not needs a regular update, now it is fully based on events
- NetworkDevice now only relies on Events, does not need fixed udpates anymore
- Improved state exchange protocol for server and clients, avoid too may extra messages
- Fixed possible bug on arduino servo programmer task management
- Bumped protocol version to 6
- Bumped EEPROM versoin for decoders
- Fixed bug on unknown messages handling that sends null to the log

# Version 0.6.2 - 05/09/2022

## General Features

- Servo programmer is complete!
- Support to download EEPROM image from Arduinos for debugging purposes
- Created EEPRomViewer

## Broker
- Added network thread to NetworkDevice
- Better timeout control on NetworkDevice
- Fix bug in message processing from SharpTerminal
- Added Task system

## Embedded
- Improvements to programmer to avoid Servo stall

## SharpTerminal

- Added parameters to allow manual connection to be configured
 