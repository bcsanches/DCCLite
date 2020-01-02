| Build | Status (github) |
|-------|-----------------|
| MSVC  | [![Build status](https://ci.appveyor.com/api/projects/status/vk4a1wgr532h5nlc/branch/master?svg=true)](https://ci.appveyor.com/project/bcsanches/dcclite/branch/master)|
| Linux | [![Travis Build Status](https://travis-ci.org/bcsanches/DCCLite.svg?branch=master)](https://travis-ci.org/bcsanches/DCCLite)  |

What's DccLite
--------------

DccLite is an open source software for controlling a model railroad. It is loosely based on [DCC++][6], but it does not implement a full command station like it. This software is aimed for those who need to control model railroad accessories, like turnouts, lights, read input from sensors, etc.

User Documentation
------------------

If you would like to know more, please visit our (working in progress) wiki at: [DCCLite Documentation](https://github.com/bcsanches/DCCLite/wiki)


Dependencies
------------

This code needs the following libraries to be built:

- [Fmt][3]
- [JsonCreator][2]
- [RapidJson][1]
- [spdlog][4]

DCC++
-------
Why not use DCC++?

Until the current date (December 2019) DCC++ does not have an easy or standard way to serve multiples Arduinos for a large model railroad. Right now, only for controlling my model railroad staging yard I need four Arduinos for turnouts.

So, DCC++ does not allow me:
- Have multiple Arduinos working together on the same layout / network
- Allow simple configuration throught config files and without needing to use serial port commands
- Configure and use Servos on Turnouts

On the other side, this project is not aimed as a replacement for DCC++, but also as a independent system for controlling model railroad devices. It does not include code or support for controlling trains, like DCC++.

License
-------

All code is licensed under the [MPLv2 License][5].

[1]: https://github.com/Tencent/rapidjson/
[2]: https://github.com/bcsanches/JsonCreator
[3]: https://github.com/fmtlib/fmt
[4]: https://github.com/gabime/spdlog
[5]: https://choosealicense.com/licenses/mpl-2.0/
[6]: https://sites.google.com/site/dccppsite/
