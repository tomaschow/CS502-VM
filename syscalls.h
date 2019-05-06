/*********************************************************************
 syscalls.h

 This include file is used by only the OS502.

 Revision History:
 1.0 August 1990:        Initial release
 1.1 Jan 1991:           Make system calls portable
 by using union of pointer
 and long.  Add incls for
 scheduler_printer.
 1.2 Dec 1991;           Allow interrupts to occur in user code and in CALL
                         statements.
 1.5 Aug 1993;           Add READ_MODIFY & DEFINE_SHARED_AREA support.
 2.0 Jan 2000;           Small changes
 2.1 May 2001;           Fix STEP macro.  DISK macros.
 2.2 Jul 2002;           Make code appropriate for undergrads.
 2.3 Aug 2004;           Modify Memory defines to work in kernel
 3.1 Aug 2004:           hardware interrupt runs on separate thread
 3.11 Aug 2004:          Support for OS level locking
 3.30 Jul 2006:          Modify POP_THE_STACK to apply to base only
 4.10 Dec 2013:          Define new support for state printer
 4.20 Feb 2014:          Move setting of MODE to the hardware
 4.30 Feb 2016:          Many new File System system calls defined.
 *********************************************************************/
#ifndef  SYSCALLS_H
#define  SYSCALLS_H

#include        "global.h"
#include        "stdio.h"

/* Definition of System Call numbers                        */

#define         SYSNUM_MEM_READ                        0
#define         SYSNUM_MEM_WRITE                       1
#define         SYSNUM_READ_MODIFY                     2
#define         SYSNUM_GET_TIME_OF_DAY                 3
#define         SYSNUM_SLEEP                           4
#define         SYSNUM_GET_PROCESS_ID                  5
#define         SYSNUM_CREATE_PROCESS                  6
#define         SYSNUM_TERMINATE_PROCESS               7
#define         SYSNUM_SUSPEND_PROCESS                 8
#define         SYSNUM_RESUME_PROCESS                  9
#define         SYSNUM_CHANGE_PRIORITY                 10
#define         SYSNUM_SEND_MESSAGE                    11
#define         SYSNUM_RECEIVE_MESSAGE                 12
#define         SYSNUM_PHYSICAL_DISK_READ              13
#define         SYSNUM_PHYSICAL_DISK_WRITE             14
#define         SYSNUM_DEFINE_SHARED_AREA              15
#define         SYSNUM_FORMAT                          16
#define         SYSNUM_CHECK_DISK                      17
#define         SYSNUM_OPEN_DIR                        18
#define         SYSNUM_OPEN_FILE                       19
#define         SYSNUM_CREATE_DIR                      20
#define         SYSNUM_CREATE_FILE                     21
#define         SYSNUM_READ_FILE                       22
#define         SYSNUM_WRITE_FILE                      23
#define         SYSNUM_CLOSE_FILE                      24
#define         SYSNUM_DIR_CONTENTS                    25
#define         SYSNUM_DELETE_DIR                      26
#define         SYSNUM_DELETE_FILE                     27

// This structure defines the format used for all system calls.
// For each call, the structure is filled in and then its address
// is used as an argument to call SVC.

#define         MAX_NUMBER_ARGUMENTS                   8
typedef struct    {
    int  NumberOfArguments;          // This includes the System Call Number
    int  SystemCallNumber;
    long *Argument[MAX_NUMBER_ARGUMENTS];
} SYSTEM_CALL_DATA;


extern void ChargeTimeAndCheckEvents(INT32);
extern int BaseThread();

#ifndef COST_OF_CALL
#define         COST_OF_CALL                            2L
#endif
#ifndef COST_OF_SOFTWARE_TRAP
#define         COST_OF_SOFTWARE_TRAP                   5L
#endif
#ifndef         COST_OF_CPU_INSTRUCTION
#define         COST_OF_CPU_INSTRUCTION                 1L
#endif

#define         CALL( fff )       {                             \
                ChargeTimeAndCheckEvents( COST_OF_CALL );       \
                fff;                                            \
                }                                               \


//      Macros used to make the test programs more readable

/*
 * In C, we can do a #define.  This produces a macro.  Then every
 * place the macro name is used, it expands to include all the code
 * defined here.  It greatly improves the readability of the code.
 */


#define    MEM_READ( arg1, arg2 )    Z502MemoryRead( arg1, (INT32 *)arg2 )

#define    MEM_WRITE( arg1, arg2 )   Z502MemoryWrite( arg1, (INT32 *)arg2 )

#define    READ_MODIFY( arg1, arg2, arg3, arg4 )                               \
	         Z502MemoryReadModify( arg1, arg2, arg3, arg4 )



#define         GET_TIME_OF_DAY( arg1 )      {                                 \
                SYSTEM_CALL_DATA *SystemCallData =                             \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof (SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 2;                         \
                SystemCallData->SystemCallNumber = SYSNUM_GET_TIME_OF_DAY;     \
                SystemCallData->Argument[0] = (long *)arg1;                    \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );             \
                SoftwareTrap(SystemCallData);                                  \
                free(SystemCallData);                                          \
                }                                                              \

#define         SLEEP( arg1 )                {                                 \
                SYSTEM_CALL_DATA *SystemCallData =                             \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof (SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 2;                         \
                SystemCallData->SystemCallNumber = SYSNUM_SLEEP;               \
                SystemCallData->Argument[0] = (long *)arg1;                    \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );             \
                SoftwareTrap(SystemCallData);                                  \
                free(SystemCallData);                                          \
                }                                                              \

#define         CREATE_PROCESS( arg1, arg2, arg3, arg4, arg5 )   {             \
                SYSTEM_CALL_DATA *SystemCallData =                             \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof (SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 6;                         \
                SystemCallData->SystemCallNumber = SYSNUM_CREATE_PROCESS;      \
                SystemCallData->Argument[0] = (long *)arg1;                    \
                SystemCallData->Argument[1] = (long *)arg2;                    \
                SystemCallData->Argument[2] = (long *)arg3;                    \
                SystemCallData->Argument[3] = (long *)arg4;                    \
                SystemCallData->Argument[4] = (long *)arg5;                    \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );             \
                SoftwareTrap(SystemCallData);                                  \
                free(SystemCallData);                                          \
                }                                                              \


#define         GET_PROCESS_ID( arg1, arg2, arg3)   {                          \
                SYSTEM_CALL_DATA *SystemCallData =                             \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof (SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 4;                         \
                SystemCallData->SystemCallNumber = SYSNUM_GET_PROCESS_ID;      \
                SystemCallData->Argument[0] = (long *)arg1;                    \
                SystemCallData->Argument[1] = (long *)arg2;                    \
                SystemCallData->Argument[2] = (long *)arg3;                    \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );             \
                SoftwareTrap(SystemCallData);                                  \
                free(SystemCallData);                                          \
                }                                                              \


#define         TERMINATE_PROCESS( arg1, arg2 )      {                        \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 3;                        \
                SystemCallData->SystemCallNumber = SYSNUM_TERMINATE_PROCESS;  \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }                                                             \


#define         SUSPEND_PROCESS( arg1, arg2 )      {                          \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 3;                        \
                SystemCallData->SystemCallNumber = SYSNUM_SUSPEND_PROCESS;    \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }                                                             \

#define         RESUME_PROCESS( arg1, arg2 )      {                           \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 3;                        \
                SystemCallData->SystemCallNumber = SYSNUM_RESUME_PROCESS;     \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }                                                             \


#define         CHANGE_PRIORITY( arg1, arg2, arg3)   {                         \
                SYSTEM_CALL_DATA *SystemCallData =                             \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof (SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 4;                         \
                SystemCallData->SystemCallNumber = SYSNUM_CHANGE_PRIORITY;     \
                SystemCallData->Argument[0] = (long *)arg1;                    \
                SystemCallData->Argument[1] = (long *)arg2;                    \
                SystemCallData->Argument[2] = (long *)arg3;                    \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );             \
                SoftwareTrap(SystemCallData);                                  \
                free(SystemCallData);                                          \
                }                                                              \



#define         SEND_MESSAGE( arg1, arg2, arg3, arg4 )   {                     \
                SYSTEM_CALL_DATA *SystemCallData =                             \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof (SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 5;                         \
                SystemCallData->SystemCallNumber = SYSNUM_SEND_MESSAGE;        \
                SystemCallData->Argument[0] = (long *)arg1;                    \
                SystemCallData->Argument[1] = (long *)arg2;                    \
                SystemCallData->Argument[2] = (long *)arg3;                    \
                SystemCallData->Argument[3] = (long *)arg4;                    \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );             \
                SoftwareTrap(SystemCallData);                                  \
                free(SystemCallData);                                          \
                }                                                              \


#define         RECEIVE_MESSAGE( arg1, arg2, arg3, arg4, arg5, arg6 )   {      \
                SYSTEM_CALL_DATA *SystemCallData =                             \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof (SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 7;                         \
                SystemCallData->SystemCallNumber = SYSNUM_RECEIVE_MESSAGE;     \
                SystemCallData->Argument[0] = (long *)arg1;                    \
                SystemCallData->Argument[1] = (long *)arg2;                    \
                SystemCallData->Argument[2] = (long *)arg3;                    \
                SystemCallData->Argument[3] = (long *)arg4;                    \
                SystemCallData->Argument[4] = (long *)arg5;                    \
                SystemCallData->Argument[5] = (long *)arg6;                    \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );             \
                SoftwareTrap(SystemCallData);                                  \
                free(SystemCallData);                                          \
                }                                                              \


#define         PHYSICAL_DISK_READ( arg1, arg2, arg3)   {                      \
                SYSTEM_CALL_DATA *SystemCallData =                             \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof (SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 4;                         \
                SystemCallData->SystemCallNumber = SYSNUM_PHYSICAL_DISK_READ;  \
                SystemCallData->Argument[0] = (long *)arg1;                    \
                SystemCallData->Argument[1] = (long *)arg2;                    \
                SystemCallData->Argument[2] = (long *)arg3;                    \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );             \
                SoftwareTrap(SystemCallData);                                  \
                free(SystemCallData);                                          \
                }                                                              \


#define         PHYSICAL_DISK_WRITE( arg1, arg2, arg3)   {                     \
                SYSTEM_CALL_DATA *SystemCallData =                             \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof (SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 4;                         \
                SystemCallData->SystemCallNumber = SYSNUM_PHYSICAL_DISK_WRITE; \
                SystemCallData->Argument[0] = (long *)arg1;                    \
                SystemCallData->Argument[1] = (long *)arg2;                    \
                SystemCallData->Argument[2] = (long *)arg3;                    \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );             \
                SoftwareTrap(SystemCallData);                                  \
                free(SystemCallData);                                          \
                }                                                              \


#define         DEFINE_SHARED_AREA( arg1, arg2, arg3, arg4, arg5)   {          \
                SYSTEM_CALL_DATA *SystemCallData =                             \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof (SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 6;                         \
                SystemCallData->SystemCallNumber = SYSNUM_DEFINE_SHARED_AREA;  \
                SystemCallData->Argument[0] = (long *)arg1;                    \
                SystemCallData->Argument[1] = (long *)arg2;                    \
                SystemCallData->Argument[2] = (long *)arg3;                    \
                SystemCallData->Argument[3] = (long *)arg4;                    \
                SystemCallData->Argument[4] = (long *)arg5;                    \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );             \
                SoftwareTrap(SystemCallData);                                  \
                free(SystemCallData);                                          \
                }                                                              \

#define         FORMAT( arg1, arg2 )      {                                   \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 3;                        \
                SystemCallData->SystemCallNumber = SYSNUM_FORMAT;             \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }                                                             \

#define         CHECK_DISK( arg1, arg2 )      {                               \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 3;                        \
                SystemCallData->SystemCallNumber = SYSNUM_CHECK_DISK;         \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }

#define         OPEN_DIR( arg1, arg2, arg3 )      {                           \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 4;                        \
                SystemCallData->SystemCallNumber = SYSNUM_OPEN_DIR;           \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                SystemCallData->Argument[2] = (long *)arg3;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }

#define         OPEN_FILE( arg1, arg2, arg3 )      {                          \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 4;                        \
                SystemCallData->SystemCallNumber = SYSNUM_OPEN_FILE;          \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                SystemCallData->Argument[2] = (long *)arg3;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }

#define         CREATE_DIR( arg1, arg2 )      {                               \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 3;                        \
                SystemCallData->SystemCallNumber = SYSNUM_CREATE_DIR;         \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }

#define         CREATE_FILE( arg1, arg2 )      {                              \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 3;                        \
                SystemCallData->SystemCallNumber = SYSNUM_CREATE_FILE;        \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }

#define         READ_FILE( arg1, arg2, arg3, arg4 )      {                    \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 5;                        \
                SystemCallData->SystemCallNumber = SYSNUM_READ_FILE;          \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                SystemCallData->Argument[2] = (long *)arg3;                   \
                SystemCallData->Argument[3] = (long *)arg4;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }

#define         WRITE_FILE( arg1, arg2, arg3, arg4 )      {                   \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 5;                        \
                SystemCallData->SystemCallNumber = SYSNUM_WRITE_FILE;         \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                SystemCallData->Argument[2] = (long *)arg3;                   \
                SystemCallData->Argument[3] = (long *)arg4;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }
#define         CLOSE_FILE( arg1, arg2 )      {                   \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 3;                        \
                SystemCallData->SystemCallNumber = SYSNUM_CLOSE_FILE;         \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }

#define         DIR_CONTENTS( arg1 )      {                                   \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 2;                        \
                SystemCallData->SystemCallNumber = SYSNUM_DIR_CONTENTS;       \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }
#define         DELETE_DIR( arg1, arg2 )      {                               \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 3;                        \
                SystemCallData->SystemCallNumber = SYSNUM_DELETE_DIR;         \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }

#define         DELETE_FILE( arg1, arg2 )      {                              \
                SYSTEM_CALL_DATA *SystemCallData =                            \
                     (SYSTEM_CALL_DATA *)calloc(1, sizeof(SYSTEM_CALL_DATA)); \
                SystemCallData->NumberOfArguments = 3;                        \
                SystemCallData->SystemCallNumber = SYSNUM_DELETE_FILE;        \
                SystemCallData->Argument[0] = (long *)arg1;                   \
                SystemCallData->Argument[1] = (long *)arg2;                   \
                ChargeTimeAndCheckEvents( COST_OF_SOFTWARE_TRAP );            \
                SoftwareTrap(SystemCallData);                                 \
                free(SystemCallData);                                         \
                }

/*      This section includes items needed in the scheduler printer.
 It's also useful for those routines that want to communicate
 with the scheduler printer.                                       */

#define         SP_MAX_NUMBER_OF_PIDS         (INT16)14
#define         SP_LENGTH_OF_ACTION           (INT16)8

// This structure is the means of communicating with the Scheduler Printer
// The OS prepares data in this structure, and then passes it to to the Scheduler
// Printer to be output.
typedef struct {
	char    TargetAction[SP_LENGTH_OF_ACTION+2];      // What action is performed
	INT16   CurrentlyRunningPID;                      // Pid of process making request
	INT16   TargetPID;                                // Action is being done on what PID
	INT16   NumberOfRunningProcesses;                 // At least 1 - duplicates CurrentlyRunningPID
	INT16   RunningProcessPIDs[SP_MAX_NUMBER_OF_PIDS];// For multiprocessor, this may be more than one
	INT16   NumberOfReadyProcesses;                   // Number of Ready To Run Processes
	INT16   ReadyProcessPIDs[SP_MAX_NUMBER_OF_PIDS];  // PIDs of those Ready To Run
	INT16   NumberOfProcSuspendedProcesses;           // Processes suspended by another process
	INT16   ProcSuspendedProcessPIDs[SP_MAX_NUMBER_OF_PIDS];
    INT16   NumberOfMessageSuspendedProcesses;        // Processes suspended because waiting for message
   	INT16   MessageSuspendedProcessPIDs[SP_MAX_NUMBER_OF_PIDS];
	INT16   NumberOfTimerSuspendedProcesses;          // Processes suspended because waiting for timer
	INT16   TimerSuspendedProcessPIDs[SP_MAX_NUMBER_OF_PIDS];
	INT16   NumberOfDiskSuspendedProcesses;           // Processes suspended because waiting for disk
	INT16   DiskSuspendedProcessPIDs[SP_MAX_NUMBER_OF_PIDS];
	INT16   NumberOfTerminatedProcesses;              // Processes no longer active - terminated
	INT16   TerminatedProcessPIDs[SP_MAX_NUMBER_OF_PIDS];
} SP_INPUT_DATA;

typedef struct {
	INT16  InUse;         // TRUE == in use, FALSE == not in use
	INT16  Pid;           // The Process holding this frame
	INT16  LogicalPage;   // The logical page in that process
	INT16  State;         // The state of the frame.
} MP_FRAME_DATA;

//    These are the definitions of the State in the MP_FRAME_DATA
//    Frame is Valid - the physical frame contains real data:
//    Frame is Modified - some process has written to it and it is dirty:
//    Frame is Referenced - some process has written or read it:
#define   FRAME_VALID      4
#define   FRAME_MODIFIED   2
#define   FRAME_REFERENCED 1

typedef struct {
	MP_FRAME_DATA  frames[NUMBER_PHYSICAL_PAGES];
} MP_INPUT_DATA;
#endif
