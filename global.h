/***************************************************************************

  global.h

      This include file is used by both the OS502 and
      the Z502 Simulator.

      Revision History:
        1.0 August 1990:        first release
        1.1 Jan    1991:        Additional disks added
        1.3 July   1992:        Make as many defines as possible
                                into shorts since PCs are happier.
        1.6 June   1995:        Minor changes
        2.0 Jan    2000:        A large number of small changes.
        2.1 May    2001:        Redefine disk layout
        2.2 July   2002:        Make code appropriate for undergrads.
        3.0 August 2004:        Modified to support memory mapped IO
        3.1 August 2004:        Implement Memory Mapped IO
        3.11 August 2004:       Implement OS level locking capability
        3.13 Nov.   2004        Change priorities of LINUX threads
        3.40 August 2008        Fix code for 64 bit addresses
        3.41 Sept.  2008        Change Z502_ARG to support 64 bit addr.
        3.50 August 2009        Minor cosmetics
        3.60 August 2012        Updates with student generated code to
                                support MACs
        4.00 August 2013        Major revision of how tests run.  There are
                                now many threads, one for each process.  All
                                macros in the tests have disappeared.
        4.01 October 2013       Small fix in test1j.
        4.20 May 2015:          Serious changes to MemoryMappedIO.  Made atomic
        .                       StatePrinter made atomic
        4.30 Jan 2016           StudentConfiguration.h created  This file
        .                       cleaned up, documented and simplified.
        4.40 Mar 2017           Fixed many bugs
****************************************************************************/
#ifndef GLOBAL_H_
#define GLOBAL_H_
#include   "studentConfiguration.h"

#define         CURRENT_REL                     "4.40"

//      These are Portability enhancements

typedef         int                             INT32;
typedef         unsigned int                   UINT32;
typedef         short                           INT16;
typedef         unsigned short                  UINT16;
typedef         int                             BOOL;

/***************************************************************************
  Used internally in the Z502 and a few other places,
  these represent the upper limit on the number of threads
  allowed by the simulation and the max number of processors
  used by the simulation.
***************************************************************************/

#define MAX_NUMBER_OF_USER_THREADS               25
#define MAX_NUMBER_OF_PROCESSORS             (short)32
#define MULTIPROCESSOR_IMPLEMENTED               TRUE

#define         FALSE                           (BOOL)0
#define         TRUE                            (BOOL)1

/***************************************************************************
  These parameters define the memory structure and page table
  mechanism.
***************************************************************************/
// How many physical pages of memory exist in the Z502
#define    NUMBER_PHYSICAL_PAGES           (short)64
// How many virtual pages of memory exist in the Z502
#define    NUMBER_VIRTUAL_PAGES             1024
// The number of bytes in a page
#define    PGSIZE                           (short)16

/***************************************************************************
     Meaning of locations in a page table entry
     These are bit masks that define the location in a page table
     where the hardware understands the location.
     You can see how these are used in z502.c in the MemoryCommon code
***************************************************************************/
#define         PTBL_VALID_BIT                  0x8000
#define         PTBL_MODIFIED_BIT               0x4000
#define         PTBL_REFERENCED_BIT             0x2000
#define         PTBL_PHYS_PG_NO                 0x0FFF

//     These are the memory mapped IO Functions

#define      Z502Halt                  Z502Idle+1
#define      Z502Idle                  Z502InterruptDevice+1
#define      Z502InterruptDevice       Z502Clock+1
#define      Z502Clock                 Z502Timer+1
#define      Z502Timer                 Z502Disk+1
#define      Z502Disk                  Z502Context+1
#define      Z502Context               Z502Processor+1
#define      Z502Processor             Z502MEM_MAPPED_MIN+1
#define      Z502MEM_MAPPED_MIN        0x7FF00000

// These are the Memory Mapped IO Modes

#define      Z502Action                    0
#define      Z502GetInterruptInfo          1
#define      Z502ReturnValue               3
#define      Z502Start                     4
#define      Z502Status                    5
#define      Z502DiskRead                  6
#define      Z502DiskWrite                 7
#define      Z502CheckDisk                 8
#define      Z502InitializeContext         9
#define      Z502StartContext             10
#define      Z502GetPageTable             11
#define      Z502GetCurrentContext        12
#define      Z502SetProcessorNumber       13
#define      Z502GetProcessorNumber       14

// This is the memory Mapped IO Data Structure.  It is an integral
// part of all Mapped IO.  It's required that this be filled in by
// the OS before making a call to the hardware.

typedef struct  {
	int          Mode;
	long         Field1;
	long         Field2;
	long         Field3;
	long         Field4;
} MEMORY_MAPPED_IO;

//  These are the allowable locations for hardware synchronization support
#define      MEMORY_INTERLOCK_BASE     0x7FE00000
#define      MEMORY_INTERLOCK_SIZE     0x00000100

//  The maximum number of disks we will support, along with the
//  number of sectors on each disk
#define         MAX_NUMBER_OF_DISKS             (short)8
#define         NUMBER_LOGICAL_SECTORS          (short)2048
#define         MAX_NUMBER_INODES               (short)31

/*****************************************************************
    These are the device IDs that are produced when an interrupt
    or fault occurs.  Further explanation can be found in Appendix A
******************************************************************/
// This trap results in the execution of code in svc
#define         SOFTWARE_TRAP                   (short)0

// These traps results in the execution of code in FaultHandler                           */
#define         CPU_ERROR                       (short)1
#define         INVALID_MEMORY                  (short)2
#define         INVALID_PHYSICAL_MEMORY         (short)3
#define         PRIVILEGED_INSTRUCTION          (short)4

// These traps results in the execution of code in InterruptHandler
#define         TIMER_INTERRUPT                 (short)4
#define         DISK_INTERRUPT                  (short)5
#define         DISK_INTERRUPT_DISK0            (short)5
#define         DISK_INTERRUPT_DISK1            (short)6
//      ... we could define other explicit disk names here

#define         LARGEST_STAT_VECTOR_INDEX       DISK_INTERRUPT + \
                                                MAX_NUMBER_OF_DISKS - 1

// These are the codes that are returned upon execution of
//   a hardware request.

#define         ERR_SUCCESS                             0L
#define         ERR_BAD_PARAM                           1L
#define         ERR_NO_PREVIOUS_WRITE                   2L
#define         ERR_ILLEGAL_ADDRESS                     3L
#define         ERR_DISK_IN_USE                         4L
#define         ERR_BAD_DEVICE_ID                       5L
#define         ERR_NO_DEVICE_FOUND                     6L
#define         DEVICE_IN_USE                           7L
#define         DEVICE_FREE                             8L
#define         ERR_Z502_INTERNAL_BUG                   20L
#define         ERR_OS502_GENERATED_BUG                 21L

/*      Definition of the TO_VECTOR array.  The TO_VECTOR
        contains pointers to the routines which will handle
        hardware exceptions.  The pointers are accessed with
        these indices:                                          */

#define         TO_VECTOR_INT_HANDLER_ADDR              (short)0
#define         TO_VECTOR_FAULT_HANDLER_ADDR            (short)1
#define         TO_VECTOR_TRAP_HANDLER_ADDR             (short)2
#define         TO_VECTOR_TYPES                         (short)3

        // Miscellaneous

#define         SUSPEND_CURRENT_CONTEXT_ONLY            (short)0
#define         START_NEW_CONTEXT_ONLY                  (short)1
#define         START_NEW_CONTEXT_AND_SUSPEND           (short)2


#endif /* GLOBAL_H_ */
