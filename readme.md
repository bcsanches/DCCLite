# DccLite

## Build Status

| Build | Status (github) |
|-------|-----------------|
| MSVC  | [![Build status](https://ci.appveyor.com/api/projects/status/vk4a1wgr532h5nlc/branch/master?svg=true)](https://ci.appveyor.com/project/bcsanches/dcclite/branch/master)|

## What's DccLite

DccLite is an open source software for controlling a model railroad. It is loosely based on [DCC++][6], but it does not implement a full command station like it. This software is aimed for those who need to control model railroad accessories, like turnouts, lights, read input from sensors, etc.

## User Documentation

If you would like to know more, please visit our (working in progress) wiki at: [DCCLite Documentation](https://github.com/bcsanches/DCCLite/wiki)

## Features
Support to a infinite (depends on your network) number of IOT devices connected to the server for controlling model railroad devices.

You can config and control using this tool, an Arduino or any other board for:
- Turning on / off leds or signals
- Throwing / closing turnouts using servos
- Reading sensor states

And this can be easily integrated with JMRI and use all the rich features from JMRI to control your model railroad devices. 

The main design goal of DCCLite is to be easy as possible to config and specially, to do fine tunning of devices without the need for recompiling code and uploading it to an Arduino. This is specially useful when you have a large model railroad that just one Arduino will not be enough for all devices. 

Another design goal is to allow the installation of the Arduino boards as close as possible to where they are needed. If you have a yard throat with several turnouts and signals, instead of running cables all over the place, you can put a single arduino mega (or perhaps two) close to it. For controlling it, you just need to provide power and connect it to a Ethernet cable connected to your local network. No need for running dozens (or even hundres) of wires from the devices to the Arduino meters away.

### JMRI

This tool can be integrated with [JMRI][12] for easily controlling your devices. The Broker server emulates a DCC++ connection and JMRI will think that it is simple talking with a DCC++ device. But diferent of a regular DCC++ device, DccLite can be composed of several Arduino boards spread all over your model railroad talking with your PC throught an Ethernet network.  

### DCC++

Why not use DCC++?

Until the current date (February 2022) [DCC++][14] does not have an easy or standard way to serve multiples Arduinos for a large model railroad. Right now, only for controlling my model railroad staging yard I need four Arduinos just for turnouts.

So, DCC++ does not allow me:
- Have multiple Arduinos working together on the same layout / network
- Allow simple configuration throught config files and without needing to use serial port commands
- Configure and use Servos on Turnouts

On the other side, this project is not aimed as a replacement for DCC++, but also as a independent system for controlling model railroad devices. It does not include code or support for controlling trains, like DCC++.

### DCC++EX

So, why not use [DCC++Ex][13] and EX-Rail?

This project started before DCC++EX and also before their Automation (EX-Rail) project. Probably most of the DccLite functionality is covered there, but it still requires to compile and keep uploading code on a Arduino or any other IOT device, as DccLite you can simple do this from a config file and let the server software (Broker) automagically upload this to the IOT device. Much simple and easier process for fine tunning your model railroad.

I do not claim that this way of doing things is better or worse for any solution you choose. But, the DccLite is my preferred method and works fine for me.

## Dependencies

This code needs the following libraries to be built:

- [Ethercard][8]
- [Fmt][3]
- [GoogleTest][9]
- [JsonCreator][2]
- [lfwatch][7]
- [magicenum][10]
- [RapidJson][1]
- [spdlog][4]
- [sigslot][15]
- [wxWidgets][11] - Optional, only for building native GUI apps (still in development)
 

## License

All code is licensed under the [MPLv2 License][5].

[1]: https://github.com/Tencent/rapidjson/
[2]: https://github.com/bcsanches/JsonCreator
[3]: https://github.com/fmtlib/fmt
[4]: https://github.com/gabime/spdlog
[5]: https://choosealicense.com/licenses/mpl-2.0/
[6]: https://sites.google.com/site/dccppsite/
[7]: https://github.com/bcsanches/lfwatch
[8]: https://github.com/njh/EtherCard
[9]: https://github.com/google/googletest
[10]: https://github.com/Neargye/magic_enum
[11]: https://github.com/wxWidgets/wxWidgets
[12]: https://www.jmri.org/
[13]: https://dcc-ex.com/
[14]: https://github.com/DccPlusPlus/BaseStation/wiki
[15]: https://github.com/palacaze/sigslot
