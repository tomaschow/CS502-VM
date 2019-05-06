/*********************************************************************
protos.h

     This include file contains prototypes needed by the builder of an
     Operating System.

     Revision History:
        1.0    August 1990     Initial Release.
        2.1    May    2001     Add memory_printer.
        2.2    July   2002     Make code appropriate for undergrads.
        3.1  August   2004:    hardware interrupt runs on separate thread
        3.11  August  2004:    Support for OS level locking
        4.0   May     2013:    Support for new testing mechanism
        4.3   May     2016:    Support for new test names
*********************************************************************/
#ifndef  PROTOS_H
#define  PROTOS_H

#include     "global.h"
#include     "syscalls.h"

//                      ENTRIES in base.c

void   InterruptHandler( void );
void   FaultHandler( void );
void   svc( SYSTEM_CALL_DATA * );
void   osInit (int argc, char *argv[] );

//                     ENTRIES in sample.c

void   SampleCode(void );

//                      ENTRIES in state_printer.c

short   SPPrintLine( SP_INPUT_DATA * );
short   MPPrintLine( MP_INPUT_DATA * );

//                      ENTRIES in CheckDisk.c
void   CheckDisk( long DiskID, long *ReturnedError );

//                      ENTRIES in test.c

void   test0( void );
void   test1( void );
void   test2( void );
void   test3( void );
void   test4( void );
void   test5( void );
void   test6( void );
void   test7( void );
void   test8( void );
void   test9( void );
void   test10( void );
void   test11( void );
void   test12( void );
void   test13( void );
void   test14( void );
void   test15( void );
void   test16( void );

void   test20( void );
void   test21( void );
void   test22( void );
void   test23( void );
void   test24( void );
void   test25( void );
void   test26( void );
void   test27( void );
void   test28( void );

void   GetSkewedRandomNumber( long*, long );   // Used by sample.c

//                      ENTRIES in z502.c

// This is the only way that the Operating System can access the hardware.
// Some of these entries are used only by the main() in test.c

void   Z502CreateUserThread( void *);
//void   Z502Halt( void );                            //MAKE MEMORYMAPPED IO
//void   Z502Idle( void );                            // MAKE MEMORY MAPPED IO
void   SoftwareTrap(SYSTEM_CALL_DATA *SystemCallData);
void   Z502MemoryRead(INT32, INT32 * );
void   Z502MemoryWrite(INT32, INT32 * );
void   Z502ReadPhysicalMemory( INT32, char *);
void   Z502WritePhysicalMemory( INT32, char *);
void   *Z502PrepareProcessForExecution( void );
void   Z502MemoryReadModify( INT32, INT32, INT32, INT32 * );

#endif // PROTOS_H_
