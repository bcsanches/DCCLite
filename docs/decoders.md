Decoders
--------

On [DCCLite](intro.md) context a decoder is an entity on a (device)[devices.md] (Arduino node) that talk with physical devices. 

For example, if you have a LED on your layout and would like to use DCCLite to turn it on / off, you should configure an (OutputDevice)[output_dev.md] on any **device** for controlling the LED.

This means that *decoders* for DCCLite are like physical *decoders* on an DCC system, but represented as *virtual decoder* on a **device**.
