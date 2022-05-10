# MLIB - Multi-purpose Library #
This is a collection of bits and pieces crafted over the years. It is released
with the hope that other people might find it useful or interesting.

Here you will find stuff as diverse as:
- socket streams that allow you work with C++ io streams over TCP or UDP sockets
- a very competent JSON parser/generator
- an embedded HTTP server
- tools for creating a user interface using HTML
- a powerful error handling mechanism
- parsing functions for NMEA-0183 messages
- serial ports enumeration functions

and many more...

## Contents ##
 - mlib - Main Library
 - mlib_test - Test Program
  
  Example programs:
  - echoserv - An echo server
  - ui-sample - Sample HTML user interface
  - yacb (Yet Another Circular Buffer) - Circular buffer sample

## Building ##
mlib requires the [UTF-8 Library](https://github.com/neacsum/utf8). The test
program requires the [UTPP Library](https://github.com/neacsum/utpp).

`BUILD.bat` script will create all libraries and test programs.

You can use the [CPM - C/C++ Package Manager](https://github.com/neacsum/cpm) to fetch all dependent packages and build them. Just issue the `CPM` command

## Installation ##
All projects have been tested under Visual Studio 2022. The libraries can be 
built in 32 or 64 bit version, with or without debug information.

## Documentation ##
You can find Doxygen generated documentation [here](https://neacsum.github.io/mlib/html/index.html).

Also there are some articles on CodeProject detailing some of the features:
 - [Error code objects](https://www.codeproject.com/Articles/5251693/Cplusplus-Error-Handling-with-Error-Code-Objects)
 - [Windows Sockets Streams](https://www.codeproject.com/Articles/5252621/Windows-Sockets-Streams)
 - [Windows Sockets Streams Part II - Multi-Threaded TCP Servers](https://www.codeproject.com/Articles/5270886/Windows-Sockets-Streams-Part-II-Multi-Threaded-TCP)
 - [Windows Sockets Streams Part III - HTTP Server](https://www.codeproject.com/Articles/5272994/Windows-Sockets-Streams-Part-III-HTTP-Server)
 - [YACB - Yet Another Circular Buffer](https://www.codeproject.com/Articles/5292326/YACB-Yet-Another-Circular-Buffer)
 - [Producer/Consumer Queues in C++](https://www.codeproject.com/Articles/5281878/Producer-Consumer-Queues-in-Cplusplus)

## License ##
[MIT License](https://github.com/neacsum/mlib/LICENSE)