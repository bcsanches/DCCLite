| Build | Status (github) |
|-------|-----------------|
| MSVC  | [![Build status](https://ci.appveyor.com/api/projects/status/vk4a1wgr532h5nlc/branch/master?svg=true)](https://ci.appveyor.com/project/bcsanches/dcclite/branch/master)|
| Linux | [![Travis Build Status](https://travis-ci.org/bcsanches/DCCLite.svg?branch=master)](https://travis-ci.org/bcsanches/DCCLite)  |

What's DccLite
--------------

DccLite is an open source software for controlling a model railroad. It is loosely based on [DCC++][6], but it does not implement a full command station like it. This software is aimed for those who need to control model railroad accessories, like turnouts, lights, read input from sensors, etc.

User Documentation
------------------

If you would like to know more, please visit our (working in progress) wiki at: [DCCLite Wiki](https://github.com/bcsanches/DCCLite/wiki)


Dependencies
------------

This code needs the following libraries to be built:

- [Fmt][3]
- [JsonCreator][2]
- [RapidJson][1]
- [spdlog][4]

License
-------

All code is licensed under the [MPLv2 License][5].

[1]: https://github.com/Tencent/rapidjson/
[2]: https://github.com/bcsanches/JsonCreator
[3]: https://github.com/fmtlib/fmt
[4]: https://github.com/gabime/spdlog
[5]: https://choosealicense.com/licenses/mpl-2.0/
[6]: https://sites.google.com/site/dccppsite/
