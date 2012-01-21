/******************************************************************************* 
SerialCommand - An Arduino library to tokenize and parse commands received over
a serial port. 
Copyright (C) 2011 Steven Cogswell  <steven.cogswell@gmail.com>
http://husks.wordpress.com  

Version 20110523B.   

See SerialCommand.h for version history. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
***********************************************************************************/

#include "WProgram.h"
#include <string.h>
#include "SerialCommand.h"

// Constructor makes sure some things are set. 
SerialCommand::SerialCommand()
{
	strncpy(delim," ",MAXDELIMETER);  // strtok_r needs a null-terminated string
	term='\r';   // return character, default terminator for commands
	numCommand=0;    // Number of callback handlers installed
	clearBuffer(); 
}

//
// Initialize the command buffer being processed to all null characters
//
void SerialCommand::clearBuffer()
{
	for (int i=0; i<SERIALCOMMANDBUFFER; i++) 
	{
		buffer[i]='\0';
	}
	bufPos=0; 
}

// Retrieve the next token ("word" or "argument") from the Command buffer.  
// returns a NULL if no more tokens exist.   
char *SerialCommand::next() 
{
	char *nextToken;
	nextToken = strtok_r(NULL, delim, &last); 
	return nextToken; 
}

// This checks the Serial stream for characters, and assembles them into a buffer.  
// When the terminator character (default '\r') is seen, it starts parsing the 
// buffer for a prefix command, and calls handlers setup by addCommand() member
void SerialCommand::readSerial() 
{
	while (Serial.available() > 0) 
	{
		int i; 
		boolean matched; 
		inChar=Serial.read();   // Read single available character, there may be more waiting
		#ifdef SERIALCOMMANDDEBUG
		Serial.print(inChar);   // Echo back to serial stream
		#endif
		if (inChar==term) {     // Check for the terminator (default '\r') meaning end of command
			#ifdef SERIALCOMMANDDEBUG
			Serial.print("Received: "); 
			Serial.println(buffer);
		    #endif
			bufPos=0;           // Reset to start of buffer
			token = strtok_r(buffer,delim,&last);   // Search for command at start of buffer
			if (token == NULL) return; 
			matched=false; 
			for (i=0; i<numCommand; i++) {
				#ifdef SERIALCOMMANDDEBUG
				Serial.print("Comparing ["); 
				Serial.print(token); 
				Serial.print("] to [");
				Serial.print(CommandList[i].command);
				Serial.println("]");
				#endif
				// Compare the found command against the list of known commands for a match
				if (strncmp(token,CommandList[i].command,SERIALCOMMANDBUFFER) == 0) 
				{
					#ifdef SERIALCOMMANDDEBUG
					Serial.print("Matched Command: "); 
					Serial.println(token);
					#endif
					// Execute the stored handler function for the command
					(*CommandList[i].function)(); 
					clearBuffer(); 
					matched=true; 
					break; 
				}
			}
			if (matched==false) {
				(*defaultHandler)(); 
				clearBuffer(); 
			}

		}
		if (isprint(inChar))   // Only printable characters into the buffer
		{
			buffer[bufPos++]=inChar;   // Put character into buffer
			buffer[bufPos]='\0';  // Null terminate
			if (bufPos > SERIALCOMMANDBUFFER-1) bufPos=0; // wrap buffer around if full  
		}
	}
}

// Adds a "command" and a handler function to the list of available commands.  
// This is used for matching a found token in the buffer, and gives the pointer
// to the handler function to deal with it. 
void SerialCommand::addCommand(char command[], void (*function)())
{
	if (numCommand < MAXSERIALCOMMANDS) {
		#ifdef SERIALCOMMANDDEBUG
		Serial.print(numCommand); 
		Serial.print("-"); 
		Serial.print("Adding command for "); 
		Serial.println(command); 
		#endif
		
		strncpy(CommandList[numCommand].command,command,SERIALCOMMANDBUFFER); 
		CommandList[numCommand].function = function; 
		numCommand++; 
	} else {
		// In this case, you tried to push more commands into the buffer than it is compiled to hold.  
		// Not much we can do since there is no real visible error assertion, we just ignore adding
		// the command
		#ifdef SERIALCOMMANDDEBUG
		Serial.println("Too many handlers - recompile changing MAXSERIALCOMMANDS"); 
		#endif 
	}
}

// This sets up a handler to be called in the event that the receveived command string
// isn't in the list of things with handlers.
void SerialCommand::addDefaultHandler(void (*function)())
{
	defaultHandler = function;
}