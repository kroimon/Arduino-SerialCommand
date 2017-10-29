/**
 * SerialCommand - A Wiring/Arduino library to tokenize and parse commands
 * received over a serial port.
 * 
 * Copyright (C) 2012 Stefan Rado
 * Copyright (C) 2011 Steven Cogswell <steven.cogswell@gmail.com>
 *                    http://husks.wordpress.com
 * 
 * Version 20120522
 * 
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "SerialCommand.h"

/**
 * Constructor makes sure some things are set.
 */
SerialCommand::SerialCommand(HardwareSerial *serial)
  : commandList(NULL),
    commandCount(0),
    defaultHandler(NULL),
    term('\n'),           // default terminator for commands, newline character
    last(NULL)
{
  _HardSerial = serial;
#ifndef __NO_SERIAL___
  _Serial = NULL;
#ifdef SOFTSERIAL2_
SOFTSERIAL2_ *Serial2;      // Constructor
#endif
#endif
  strcpy(delim, " "); // strtok_r needs a null-terminated string
  clearBuffer();
}

#ifndef __NO_SERIAL___
SerialCommand::SerialCommand(SOFTSERIAL_ *serial)
  : commandList(NULL),
    commandCount(0),
    defaultHandler(NULL),
    term('\n'),           // default terminator for commands, newline character
    last(NULL)
{
  _HardSerial = NULL;
#ifndef  __NO_SERIAL___
  _Serial = serial;
#endif
  strcpy(delim, " "); // strtok_r needs a null-terminated string
  clearBuffer();
}
#endif

/**
 * Adds a "command" and a handler function to the list of available commands.
 * This is used for matching a found token in the buffer, and gives the pointer
 * to the handler function to deal with it.
 */
void SerialCommand::addCommand(const char *command, void (*function)()) {
  #ifdef SERIALCOMMAND_DEBUG
    _HardSerial.print("Adding command (");
    _HardSerial.print(commandCount);
    _HardSerial.print("): ");
    _HardSerial.println(command);
  #endif

  commandList = (SerialCommandCallback *) realloc(commandList, (commandCount + 1) * sizeof(SerialCommandCallback));
  strncpy(commandList[commandCount].command, command, SERIALCOMMAND_MAXCOMMANDLENGTH);
  commandList[commandCount].function = function;
  commandCount++;
}

/**
 * This sets up a handler to be called in the event that the receveived command string
 * isn't in the list of commands.
 */
void SerialCommand::setDefaultHandler(void (*function)(const char *)) {
  defaultHandler = function;
}


/**
 * This checks the Serial stream for characters, and assembles them into a buffer.
 * When the terminator character (default '\n') is seen, it starts parsing the
 * buffer for a prefix command, and calls handlers setup by addCommand() member
 */
void SerialCommand::readSerial() {
  //TODO: This is a total hack...
  if (_HardSerial){
	  while (_HardSerial->available() > 0) {
	    char inChar = _HardSerial->read();   // Read single available character, there may be more waiting
	    #ifdef SERIALCOMMAND_DEBUG
	      _HardSerial.print(inChar);   // Echo back to serial stream
	    #endif

	    if (inChar == term) {     // Check for the terminator (default '\r') meaning end of command
	      #ifdef SERIALCOMMAND_DEBUG
		_HardSerial.print("Received: ");
		_HardSerial.println(buffer);
	      #endif

	      char *command = strtok_r(buffer, delim, &last);   // Search for command at start of buffer
	      if (command != NULL) {
		boolean matched = false;
		for (int i = 0; i < commandCount; i++) {
		  #ifdef SERIALCOMMAND_DEBUG
		    _HardSerial.print("Comparing [");
		    _HardSerial.print(command);
		    _HardSerial.print("] to [");
		    _HardSerial.print(commandList[i].command);
		    _HardSerial.println("]");
		  #endif

		  // Compare the found command against the list of known commands for a match
		  if (strncmp(command, commandList[i].command, SERIALCOMMAND_MAXCOMMANDLENGTH) == 0) {
		    #ifdef SERIALCOMMAND_DEBUG
		      _HardSerial.print("Matched Command: ");
		      _HardSerial.println(command);
		    #endif

		    // Execute the stored handler function for the command
		    (*commandList[i].function)();
		    matched = true;
		    break;
		  }
		}
		if (!matched && (defaultHandler != NULL)) {
		  (*defaultHandler)(command);
		}
	      }
	      clearBuffer();
	    }
	    else if (isprint(inChar)) {     // Only printable characters into the buffer
	      if (bufPos < SERIALCOMMAND_BUFFER) {
		buffer[bufPos++] = inChar;  // Put character into buffer
		buffer[bufPos] = '\0';      // Null terminate
	      } else {
		#ifdef SERIALCOMMAND_DEBUG
		  _HardSerial.println("Line buffer is full - increase SERIALCOMMAND_BUFFER");
		#endif
	      }
	    }
	  }
#ifndef __NO_SERIAL___
  } else if (_Serial){
	  while (_Serial->available() > 0) {
	    char inChar = _Serial->read();   // Read single available character, there may be more waiting
	    #ifdef SERIALCOMMAND_DEBUG
	      _Serial.print(inChar);   // Echo back to serial stream
	    #endif

	    if (inChar == term) {     // Check for the terminator (default '\r') meaning end of command
	      #ifdef SERIALCOMMAND_DEBUG
		_Serial.print("Received: ");
		_Serial.println(buffer);
	      #endif

	      char *command = strtok_r(buffer, delim, &last);   // Search for command at start of buffer
	      if (command != NULL) {
		boolean matched = false;
		for (int i = 0; i < commandCount; i++) {
		  #ifdef SERIALCOMMAND_DEBUG
		    _Serial.print("Comparing [");
		    _Serial.print(command);
		    _Serial.print("] to [");
		    _Serial.print(commandList[i].command);
		    _Serial.println("]");
		  #endif

		  // Compare the found command against the list of known commands for a match
		  if (strncmp(command, commandList[i].command, SERIALCOMMAND_MAXCOMMANDLENGTH) == 0) {
		    #ifdef SERIALCOMMAND_DEBUG
		      _Serial.print("Matched Command: ");
		      _Serial.println(command);
		    #endif

		    // Execute the stored handler function for the command
		    (*commandList[i].function)();
		    matched = true;
		    break;
		  }
		}
		if (!matched && (defaultHandler != NULL)) {
		  (*defaultHandler)(command);
		}
	      }
	      clearBuffer();
	    }
	    else if (isprint(inChar)) {     // Only printable characters into the buffer
	      if (bufPos < SERIALCOMMAND_BUFFER) {
		buffer[bufPos++] = inChar;  // Put character into buffer
		buffer[bufPos] = '\0';      // Null terminate
	      } else {
		#ifdef SERIALCOMMAND_DEBUG
		  _Serial.println("Line buffer is full - increase SERIALCOMMAND_BUFFER");
		#endif
	      }
	    }
	  }
#endif
  }
}

/*
 * Clear the input buffer.
 */
void SerialCommand::clearBuffer() {
  buffer[0] = '\0';
  bufPos = 0;
}

/**
 * Retrieve the next token ("word" or "argument") from the command buffer.
 * Returns NULL if no more tokens exist.
 */
char *SerialCommand::next() {
  return strtok_r(NULL, delim, &last);
}
