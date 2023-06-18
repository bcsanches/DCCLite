# Version ?????

## Code

- Added own implementation of DirectoryMonitor (win32 only for now)

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
 