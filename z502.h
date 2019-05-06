/*********************************************************************

        z502.h

   This include file is used only by the Z502.

   Revision History:
   1.3  July    1992:   Make structures smaller. Reduce size of SECTOR.
   2.0  January 2000:   Small changes
   2.1  May     2001:   Redefine disk layout.  Define HARDWARE_STATS
   2.2  July    2002:   Make code appropriate for undergrads.
   3.0 August   2004:   Modified to support memory mapped IO
   3.1 August   2004:   hardware interrupt runs on separate thread
   3.11 August  2004:   Support for OS level locking
   3.53 NOVEMBER 2011:  Changed CONTEXT so the space allocated for
                        REGs is long - didn't matter until trying
                        to store addresses.
*********************************************************************/

#ifndef  Z502_H
#define  Z502_H
#ifdef  NT
#define THREAD_PRIORITY_LOW           THREAD_PRIORITY_BELOW_NORMAL
#define THREAD_PRIORITY_HIGH          THREAD_PRIORITY_TIME_CRITICAL
#define LOCK_TYPE                     HANDLE
// Eliminates warnings of deprecated functions with Visual C++
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef NT
#define THREAD_PRIORITY_LOW                 1
#define THREAD_PRIORITY_HIGH                2
#define LOCK_TYPE                       pthread_mutex_t
#endif
#define LESS_FAVORABLE_PRIORITY             -5
#define MORE_FAVORABLE_PRIORITY              5

#define         USER_MODE                               (short)0
#define         KERNEL_MODE                             (short)1

#define         COST_OF_MEMORY_ACCESS           1L
#define         COST_OF_MEMORY_MAPPED_IO        1L
#define         COST_OF_DISK_ACCESS             8L
#define         COST_OF_DELAY                   2L
#define         COST_OF_CLOCK                   3L
#define         COST_OF_TIMER                   2L
#define         COST_OF_MAKE_CONTEXT            20L
#define         COST_OF_DESTROY_CONTEXT         10L
#define         COST_OF_SWITCH_CONTEXT          15L
#define         COST_OF_SOFTWARE_TRAP           5L
#define         COST_OF_CPU_INSTRUCTION         1L
#define         COST_OF_CALL                    2L

#ifndef NULL
#define         NULL                            0
#endif

#define         EVENT_STRUCTURE_ID              (unsigned char)124
#define         SECTOR_STRUCTURE_ID             (unsigned char)125
#define         CONTEXT_STRUCTURE_ID            (unsigned char)126

#define         EVENT_RING_BUFFER_SIZE          16

/*  STAT_VECTOR is a two dimensional array.  The first
    dimension can take on values shown here.  The
    second dimension holds the error or device type.     */

#define         SV_ACTIVE                       (short)0
#define         SV_VALUE                        (short)1
#define         SV_TID                          (short)2
#define         SV_DIMENSION                    (short)3

#define         MAX_THREAD_TABLE_SIZE            MAX_NUMBER_OF_USER_THREADS+5

typedef struct
    {
    INT32               *queue;
    INT32               time_of_event;
    INT16               ring_buffer_location;
    INT16               event_error;
    INT16               event_type;
    unsigned char       structure_id;
} EVENT;

/* Supports history which is dumped on a hardware panic */

typedef struct {
    INT32               time_of_request;
    INT32               expected_time_of_event;
    INT32               real_time_of_event;
    INT16               event_error;
    INT16               event_type;
} RING_EVENT;


// Structure modified 07/2015 - Issue 7 in Rev 4.11 buglist
typedef struct {
    INT32               context_switches;
    INT32               NumberRunningProcesses;
    INT32               TimeWeightedNumberRunningProcesses;
    INT32               disk_reads[MAX_NUMBER_OF_DISKS];
    INT32               disk_writes[MAX_NUMBER_OF_DISKS];
    INT32               time_disk_busy[MAX_NUMBER_OF_DISKS];
    INT32               number_charge_times;
    INT32               number_faults;
} HARDWARE_STATS;

typedef struct {
    INT32               *queue;
    INT16               structure_id;
    INT16               disk_id;
    INT16               sector;
    char                sector_data[PGSIZE];
} SECTOR;

typedef struct {
    unsigned char       StructureID;          // A unique ID so we know it's a CONTEXT
    void                *CodeEntry;           // Location where program starts
    UINT16              *PageTablePointer;    // Address of page table for this process
    INT32               ContextStartCount;     // How many times this context has been started
 //   INT16               PC;                    // Current address of the process
 //   INT32               CallType;
    INT16               ProgramMode;          // When last run, is it KEERNEL or USER
 //   INT16               mode_at_first_interrupt;
    BOOL                FaultInProgress;
} Z502CONTEXT;

// We create a thread for every potential process a user might create.
// This is the information we need for each thread.

typedef struct {
        int OurLocalID;
        int ThreadID;
        int CurrentState;
        Z502CONTEXT *Context;
        UINT32 Condition;
        UINT32 Mutex;
        INT16 Mode;
} THREAD_INFO;

// These are the states defined for a thread and stored in CurrentState
#define         UNINITIALIZED                      0
#define         CREATED                            1
#define         SUSPENDED_WAITING_FOR_CONTEXT      2
#define         SUSPENDED_WAITING_FOR_FIRST_SCHED  3
#define         ACTIVE                             4
#define         SUSPENDED_AFTER_BEING_ACTIVE       5


typedef struct {
    EVENT               *EventPtr;
    INT16               LastSector;
    INT16               DiskInUse;
    char                *Source;
    char                *Destination;
    INT16               Action;
} DISK_STATE;

typedef struct
    {
    INT16               sector;
    INT16               action;
    char                *buffer;
} MEMORY_MAPPED_DISK_STATE;

typedef struct
    {
    EVENT               *event_ptr;
    INT16               timer_in_use;
} TIMER_STATE;

#endif
