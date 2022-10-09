# Version 0.6.3 - ??/??/????

## SharpTerminal
- Allow user to set signal aspects from SignalDecoder panel

## Code

- Using sigslot library (https://github.com/palacaze/sigslot), removed manual listeners code from Services
- SignalManager does not needs a regular update, now it is fully based on events
- NetworkDevice now only relies on Events, does not need fixed udpates anymore
- Improved state exchange protocol for server and clients, avoid too may extra messages

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
 