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
- prodcons - Sample producer/consumer processes 
- yacb (Yet Another Circular Buffer) - Circular buffer sample

## Building ##
mlib requires the [UTF-8 Library](https://github.com/neacsum/utf8). in addition The test program requires the [UTPP Library](https://github.com/neacsum/utpp).

The preferred method is to use the [CPM - C/C++ Package Manager](https://github.com/neacsum/cpm/). Download the [CPM executable](https://github.com/neacsum/cpm/releases/latest/download/cpm.exe) and, from the root of the development tree, issue the `cpm` command:
```
  cpm -u https://github.com/neacsum/mlib.git mlib
```

You can also use the `BUILD.bat` script will create all libraries and test programs:
```
c:\mlib>build all
```

## Documentation ##
You can find Doxygen generated documentation [here](https://neacsum.github.io/mlib/index.html).

There are also some articles about some of the features:
 - [Error code objects](https://neacsu.net/docs/programming/error_codes/cp_article/)
 - [Windows Sockets Streams](https://neacsu.net/docs/programming/sockstreams/windows-socket-streams/)
 - [Windows Sockets Streams Part II - Multi-Threaded TCP Servers](https://neacsu.net/docs/programming/sockstreams/multi-threaded-servers/)
 - [Windows Sockets Streams Part III - HTTP Server](https://neacsu.net/docs/programming/sockstreams/httpserver/)
 - [YACB - Yet Another Circular Buffer](https://neacsu.net/docs/programming/circular_buffer/)
 - [Producer/Consumer Queues in C++](https://neacsu.net/docs/programming/producer-consumer-queues/)
 - [C++ Replacement for `getopt`](https://neacsu.net/docs/programming/getopt/)

## License ##
[MIT License](https://github.com/neacsum/mlib/blob/master/LICENSE)
