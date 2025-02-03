# MLIB - Multi-purpose Library #
This is a collection of bits and pieces crafted over the years. It is released
with the hope that other people might find it useful or interesting.

## License ##

The MIT License (MIT)
 
Copyright (c) 1999-2025 Mircea Neacsu

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Objects Inventory ##

### Synchronization ###
- \ref mlib::criticalsection "critical section"
- \ref mlib::event "event"
- \ref mlib::mutex "mutex"
- \ref mlib::semaphore "semaphore"
- \ref mlib::shmem "shared memory"
- \ref mlib::thread "thread"
- \ref mlib::wtimer "waitable timer"
- \ref mlib::syncbase "syncbase" - base class for all synchronization objects

## JSON Parser ##
- \ref json::node an element of a JSON object

### Networking ###
- \ref mlib::inaddr "inaddr"
- \ref mlib::sock "socket"
- \ref mlib::sockbuf
- \ref mlib::generic_sockstream "generic_sockstream" and derived classes
- \ref mlib::tcpserver "TCP server"
- \ref mlib::http::server "HTTP server"
- \ref mlib::http::JSONBridge "JSONBridge" - JSON interface to HTTP server
- \ref mlib::firewall "Windows firewall" configuration

### Error Logging and Handling ###
- \ref mlib::erc "Error code" objects
- \ref mlib::errfac "Error facility"
- \ref syslog() "syslog"
- \ref dprintf() "dprintf" - output using OutputDebugString
- \ref trace.h "TRACE macros"

### Geometry ###
- \ref mlib::convex_hull() "convex_hull" - convex hull calculator
- \ref mlib::Border "border"
- \ref mlib::RotMat "rotmat" - Rotations calculator
- \ref mlib::Point "2D Point"

### SQLITE Encapsulation ###
- \ref mlib::Query "Query"
- \ref mlib::Database "Database"

### Other things ###
- \ref mlib::bitstream "bitstream" - Reading and writing of bit fields
- \ref mlib::ipow() "ipow" - Integer powers through multiplication
- \ref mlib::poly() "poly" - Polynomial evaluation using Horner's scheme
- \ref NMEA-0183 parsing
- \ref mlib::OptParser "OptParser" - Parsing command line options
- \ref mlib::ring_buffer "ring_buffer" - FIFO Circular buffer
- \ref serenum - Enumerators for serial ports
- \ref mlib::statpars "statpars" - Statistical parameters
- \ref mlib::stopwatch "stopwatch" - Another stopwatch timer
- \ref tvops "tvops" - Operations with timeval structures
- \ref mlib::biosuuid() - Retrieve BIOS UUID value
 


