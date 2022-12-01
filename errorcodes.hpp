#ifndef _ERRORCODES_H
#define _ERRORCODES_H

const int NO_ERROR      = 0b00000;
const int USAGE_ERROR   = 0b00001;
const int MEMORY_ERROR  = 0b00010;
const int FILE_ERROR    = 0b00100;
const int NETWORK_ERROR = 0b01000;
const int SYSTEM_ERROR  = 0b10000;

const int DISCONNECTED = -1;
const int UNEXPECTED_DISCONNECT = -2;

// NOTE: Should have use enum classes

#endif  // _ERRORCODES_H
