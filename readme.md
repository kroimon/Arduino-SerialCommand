SerialCommand
=============
A Wiring/Arduino library to tokenize and parse commands received over a serial port.

The original version of this library was written by [Steven Cogswell](http://husks.wordpress.com) (published May 23, 2011 in his blog post ["A Minimal Arduino Library for Processing Serial Commands"](http://husks.wordpress.com/2011/05/23/a-minimal-arduino-library-for-processing-serial-commands/)).

My changes are based on a heavily modified version with smaller footprint and cleaned up code by Stefan Rado.  They add support for a return entered on an empty line, a second line terminator, and the allowance of non-printable characters in the input buffer when necessary when talking to a Davis weather station.
