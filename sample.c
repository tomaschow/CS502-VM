/**************************************************************************

 This routine, "sample", is designed to show you how to interface
 to various hardware functions and how to use scheduler_printer.
 It is provided solely as an example; it is NOT part of the normal
 code involved in building your Operating System.

 Revision History:
 1.3 July     1992: Initial code written.
 1.4 December 1992: Add test to size host machine memory.
 2.2 July     2002: Make code appropriate for undergrads.
 3.0 August   2004: Modified to support memory mapped IO
 3.1 August   2004: hardware interrupt runs on separate thread
 3.11 August  2004: Support for OS level locking
 3.12 Sept.   2004: Minor changes
 3.30 Aug     2006: Change threading and disk tests to be
                    consistant with current hardware.
 3.41         2008: Fix bug associated with running on 64-bit machines.
 4.10         2014: Minor changes to SP_Print functionality
 4.20         2015: Make thread-safe.  Support multiprocessors
 4.40         2017: Race conditions on disk tests on LINUX cause test
                    to crash.
 **************************************************************************/

#include                 "global.h"
#include                 "syscalls.h"
#include                 "z502.h"
#include                 "protos.h"
#include                 "stdio.h"
#include                 "stdlib.h"
#include                 "string.h"
#ifdef NT
#include                 <windows.h>
#include                 <winbase.h>
#include                 <sys/types.h>
#endif

#define                  NUM_RAND_BUCKETS          128
#define                  DO_LOCK                     1
#define                  DO_UNLOCK                   0
#define                  SUSPEND_UNTIL_LOCKED        TRUE
#define                  DO_NOT_SUSPEND              FALSE

//  Prototypes For Functions In This File
void starting_point_for_new_context(void);
void DoOnelock(void);
void DoOneTrylock(void);
void DoOneUnlock(void);
int GetSampleTimeNow(void);

// This is a prototype for Z502.c
// These prototypes are used here for test debugging, but should
//     not be used by students
int CreateAThread(void *ThreadStartAddress, INT32 *data);

// Since this is just sample code, and it doesn't have the benefit
// of a real delay routine as you will be implementing, this
// code just does a loop.  You do NOT NOT NOT want to do this
// since it will simply use up far too much time in your code.
void DoASleep(INT32 TimeUnit);

// extern UINT16 *Z502_PAGE_TBL_ADDR;

char Success[] = "      Action Failed\0        Action Succeeded";
#define          SPART          22

void SampleCode(void) {
	INT32 i, j, k;             // Index & counters
	INT32 Counter = 0;
	long Value;
	MEMORY_MAPPED_IO mmio;        // Structure used for hardware interface

	short *PAGE_TBL_ADDR;

	INT32 disk_id, sector; /* Used for disk requests */
	char disk_buffer_write[PGSIZE ];
	char disk_buffer_read[PGSIZE ];
	char physical_memory_write[PGSIZE ];
	char physical_memory_read[PGSIZE ];

	short random_buckets[NUM_RAND_BUCKETS];
	INT32 LockResult;

	SP_INPUT_DATA SPData;    // Used to feed SchedulerPrinter
	MP_INPUT_DATA MPData;    // Used to feed MemoryPrinter

	INT32 Status;
	INT32 Temp, Temp1;

	/*********************************************************************
	 Show the interface to the delay timer.
	 Eventually the timer will interrupt ( in base.c there's a handler for
	 this ), but here in sample.c we're merely showing the interface
	 to start the call.
	 *********************************************************************/

	// Check the status of the timer - is it running or not
	printf("Starting execution of the sample code\n");
	mmio.Mode = Z502Status;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502Timer, &mmio);
	Status = mmio.Field1;
	if (Status == DEVICE_FREE)
		printf("Got expected (DEVICE_FREE) result for Status of Timer\n");
	else
		printf("Got erroneous result for Status of Timer\n");

	// Start the timer - here's the sequence to use
	mmio.Mode = Z502Start;
	mmio.Field1 = 777;   // You pick the time units
	mmio.Field2 = mmio.Field3 = 0;
	MEM_WRITE(Z502Timer, &mmio);

	// Check the status of the timer - is it running or not
	mmio.Mode = Z502Status;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502Timer, &mmio);
	Status = mmio.Field1;
	if (Status == DEVICE_IN_USE)
		printf("Got expected result (DEVICE_IN_USE) for Status of Timer\n");
	else
		printf("Got erroneous result for Status of Timer\n");

	printf(
			"\nThe next output from the Interrupt Handler should report that \n");
	printf("   interrupt of device 4 has occurred with no error.\n");

	// Go idle until the interrupt occurs
	mmio.Mode = Z502Action;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_WRITE(Z502Idle, &mmio);       //  Let the interrupt for this timer occur
	DoASleep(10);                       // Give it a little more time

	//  Now we're going to try an illegal time and ensure that the fault
	// handler reports an illegal status.  It's important to try to start the time
	// and then do the printf here.  That gives the interrupt handler a chance to
	// run before we get the status here.
	mmio.Mode = Z502Start;
	mmio.Field1 = -77;   // You pick the time units - but it's illegal
	mmio.Field2 = mmio.Field3 = 0;
	MEM_WRITE(Z502Timer, &mmio);
	printf("\nThe output from the Interrupt Handler should report that \n");
	printf("   the interrupt handler found device ID 4 with status %d.\n",
			(int) ERR_BAD_PARAM);
	DoASleep(10);                       // Give it a little more time
	printf("\n");

	// Check the status of the timer - is it running or not
	mmio.Mode = Z502Status;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502Timer, &mmio);
	Status = mmio.Field1;
	if (Status == DEVICE_FREE)
		printf("Got expected result for Status of Timer\n");
	else
		printf("Got erroneous result for Status of Timer\n");
	fflush(stdout);

	/*  The interrupt handler will have exercised the code doing
	 Z502InterruptDevice - getting information about the interrupt and
	 then clearing it.  But they will have been tested only for
	 "correct" usage.
	 Let's try a few erroneous/illegal operations.     */

	// Get cause of interrupt - there should be no interrupt that
	// is waiting so we should get an error.  Check it.
	mmio.Mode = Z502GetInterruptInfo;
	MEM_READ(Z502InterruptDevice, &mmio);
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	if (mmio.Field4 == ERR_NO_DEVICE_FOUND)
		printf("Got expected result for Status when no DeviceFound \n");
	else
		printf("Got erroneous result for Status when no DeviceFound\n");


	/*********************************************************************
	 Show the interface to the Z502_CLOCK.
	 This is easy - all we're going to do is read it twice and make sure
	 the time is incrementing - the second time should be greater than
         the first.
	 *********************************************************************/

	mmio.Mode = Z502ReturnValue;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502Clock, &mmio);
	Temp = (INT32)mmio.Field1;
	mmio.Mode = Z502ReturnValue;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502Clock, &mmio);
	Temp1 = (INT32)mmio.Field1;
	if (Temp1 > Temp)
		printf("\nThe clock time incremented correctly - %d, %d\n", Temp,
				Temp1);
	else
		printf("\nThe clock time did NOT increment correctly - %d, %d\n",
				Temp, Temp1);

	/*********************************************************************
	 Show the interface to the disk read and write.
	 Eventually the disk will interrupt ( in base.c there's a handler for
	 this ), but here in sample.c we're merely showing the interface
	 to start the call.
	 *********************************************************************/
#define   SAMPLE_LEGAL_DISK_ID    1
#define   SAMPLE_LEGAL_SECTOR_LOCATION   3
	// Now see that the disk IS NOT running
	mmio.Mode = Z502Status;
	mmio.Field1 = SAMPLE_LEGAL_DISK_ID;
	mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502Disk, &mmio);
	printf("Disk Test 1: Time = %d ",  GetSampleTimeNow());
	if (mmio.Field2 == DEVICE_FREE)    // Disk hasn't been used - should be free
		printf("Got expected result for Disk Status\n");
	else
		printf("Got erroneous result for Disk Status - Device not free.\n");

	// Start the disk by writing a block of data
	mmio.Mode = Z502DiskWrite;
	mmio.Field1 = SAMPLE_LEGAL_DISK_ID;
	mmio.Field2 = SAMPLE_LEGAL_SECTOR_LOCATION;
	mmio.Field3 = (long) disk_buffer_write;

	//   Put data into the buffer being written
	strncpy(disk_buffer_write, "123456789abcdef", 15);
	// Do the hardware call to put data on disk
	MEM_WRITE(Z502Disk, &mmio);

	// Now see that the disk IS running
	mmio.Mode = Z502Status;
	mmio.Field1 = SAMPLE_LEGAL_DISK_ID;
	mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502Disk, &mmio);
	printf("Disk Test 2: Time = %d   ",  GetSampleTimeNow());
	if (mmio.Field2 == DEVICE_IN_USE)        // Disk should report being used
		printf("Got expected result for Disk Status\n");
	else
		printf( "Got erroneous result for Disk Status - Device is free.\n");

        // We've just started the disk request.  But the system call making that 
        // request has returned to this code.   So how do we know when that disk 
        // write is complete?
        // The Z502 architecture has a feature called "IDLE".  The call to idle 
        // assures that we the thread of execution doesn't return to this code 
        // until the interrupt is about to start.  Actually, we return after an 
        // idle when the time for the next interrupt is seen; that does NOT mean 
        // the interrupt has completed when the code returns here.
	
	mmio.Mode = Z502Action;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_WRITE(Z502Idle, &mmio);
        // The call to IDLE should indicate that the correct amount of time has occurred for the interrupt to start.
        // That DOESN'T mean the interrupt has finished.  That interrupt handling code is running on a different thread.
        // So we now wait until the interrupt has done its work and the disk is now free.
	mmio.Field2 = DEVICE_IN_USE;
	while (mmio.Field2 != DEVICE_FREE) {
		mmio.Mode = Z502Status;
		mmio.Field1 = SAMPLE_LEGAL_DISK_ID;
		mmio.Field2 = mmio.Field3 = 0;
		MEM_READ(Z502Disk, &mmio);
	}

	// The disk should now be done.  Make sure it's idle
	mmio.Mode = Z502Status;
	mmio.Field1 = SAMPLE_LEGAL_DISK_ID;
	mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502Disk, &mmio);
	printf("Disk Test 3: Time = %d   ",  GetSampleTimeNow());
	if (mmio.Field2 == DEVICE_FREE)        // Disk should be free
		printf("Got expected result for Disk Status\n");
	else
		printf("Got erroneous result for Disk Status - Device not free.\n");

	// Now we read the data back from the disk.  If we're lucky,
	// we'll read the same thing we wrote!                     
	// Start the disk by reading a block of data
	mmio.Mode = Z502DiskRead;
	mmio.Field1 = SAMPLE_LEGAL_DISK_ID; // Pick same disk location
	mmio.Field2 = SAMPLE_LEGAL_SECTOR_LOCATION;
	mmio.Field3 = (long) disk_buffer_read;

	// Do the hardware call to read data from disk
	MEM_WRITE(Z502Disk, &mmio);
	printf("Disk Test 4: Time = %d Started read of data\n",  GetSampleTimeNow());

	// Wait for the disk action to complete.
	// This is the same action used previously.
	// NOTE - you should never loop waiting for the action to complete.  The idle assures other work can be
        //        done, and the while loop is merely a way to avoid a race condition.
	mmio.Mode = Z502Action;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_WRITE(Z502Idle, &mmio);
	mmio.Field2 = DEVICE_IN_USE;
	while (mmio.Field2 != DEVICE_FREE) {
		mmio.Mode = Z502Status;
		mmio.Field1 = SAMPLE_LEGAL_DISK_ID;
		mmio.Field2 = mmio.Field3 = 0;
		MEM_READ(Z502Disk, &mmio);
	}
	printf("Disk Test 5: Time = %d Completed read of data\n\n",  GetSampleTimeNow());

	printf("The disk data written is: %s\n", disk_buffer_write);
	printf("The disk data read    is: %s\n\n", disk_buffer_read);

	/*********************************************************************
	 Let's try some intentional errors to see what happens
	 *********************************************************************/
#define   SAMPLE_ILLEGAL_DISK_ID           MAX_NUMBER_OF_DISKS + 2
#define   SAMPLE_ILLEGAL_SECTOR_LOCATION   NUMBER_LOGICAL_SECTORS + 1

	// Try an illegal Disk ID
	mmio.Mode = Z502DiskWrite;
	mmio.Field1 = SAMPLE_ILLEGAL_DISK_ID;
	mmio.Field2 = SAMPLE_LEGAL_SECTOR_LOCATION;
	mmio.Field3 = (long) disk_buffer_write;
	MEM_WRITE(Z502Disk, &mmio);
	if (mmio.Field4 == ERR_BAD_PARAM)
		printf(
				"Disk Test 6: Got expected result for Disk when using illegal ID\n");
	else
		printf(
				"Disk Test 6: Got erroneous result for Disk when using illegal ID\n");

	mmio.Mode = Z502DiskWrite;
	mmio.Field1 = SAMPLE_LEGAL_DISK_ID;
	mmio.Field2 = SAMPLE_ILLEGAL_SECTOR_LOCATION;
	mmio.Field3 = (long) disk_buffer_write;
	MEM_WRITE(Z502Disk, &mmio);

	if (mmio.Field4 == ERR_BAD_PARAM)
		printf(
				"Disk Test 7: Got expected result for Disk when using bad parameter\n");
	else
		printf(
				"Disk Test 7: Got erroneous result for Disk when using bad parameter\n");

	mmio.Mode = Z502Status;
	mmio.Field1 = SAMPLE_ILLEGAL_DISK_ID;
	mmio.Field2 = 0;
	mmio.Field3 = 0;
	MEM_WRITE(Z502Disk, &mmio);

	if (mmio.Field4 == ERR_BAD_DEVICE_ID)
		printf(
				"Disk Test 8: Expected result for Disk Status when using bad ID\n");
	else
		printf(
				"Disk Test 8: Erroneous result for Disk Status when using bad ID\n");

	/*********************************************************************
	 Some of the tests put thousands of pages of data on the disk.  Let's
	 see if we can do that here.   The pages ARE being written to the disks,
	 but the interrupt handler doesn't show all of them happening because
	 it's not catching multiple interrupts.
	 NOTE:  This test should have all the disks in action at the same time
	 *********************************************************************/

	disk_id = 0;
	sector = 1;

	printf("\nThe following section tests multiple disks in operation\n");
	printf("at the same time.  It will take a few seconds\n");

	for (j = 0; j < NUMBER_VIRTUAL_PAGES + 100 /* arbitrary # */; j++) {
		if (disk_id > MAX_NUMBER_OF_DISKS - 2) {
			disk_id = -1;
			sector++;
		}
		disk_id++;
		// First make sure the disk is free - it may be in use from
		// a previous request.
		mmio.Field2 = DEVICE_IN_USE;
		Counter = 0;
		while (mmio.Field2 == DEVICE_IN_USE) {
			mmio.Mode = Z502Action;
			mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
			MEM_WRITE(Z502Idle, &mmio);

			// Now go back and see if disk is done
			mmio.Mode = Z502Status;
			mmio.Field1 = disk_id;
			mmio.Field2 = mmio.Field3 = 0;
			MEM_READ(Z502Disk, &mmio);
			if (Counter > 500) {
				printf("PANIC 1:  Error in multiple disk test in sample.c\n");
                                Counter = 0;
			}
			Counter++;
		}
		mmio.Mode = Z502DiskWrite;
		mmio.Field1 = disk_id;
		mmio.Field2 = sector;
		mmio.Field3 = (long) disk_buffer_write;
		MEM_WRITE(Z502Disk, &mmio);
		if (mmio.Field4 != ERR_SUCCESS) {
			printf("PANIC 2:  Error in multiple disk test in sample.c\n");
		}
	}    // End of for j

	// Give things a chance to settle down - any last interrupts left?
	mmio.Mode = Z502Action;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_WRITE(Z502Idle, &mmio);
	mmio.Mode = Z502Action;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_WRITE(Z502Idle, &mmio);
	printf("End of multi-disk test\n");

	/*********************************************************************
	 Do a physical memory access to check out that it works.
	 This is a mechanism that allows the OS to access physical memory
	 directly without using a page table.
	 *********************************************************************/
	printf("\nStarting test of physical memory write and read.\n");
	for (i = 0; i < PGSIZE ; i++) {
		physical_memory_write[i] = i;
	}
	Z502WritePhysicalMemory(17, (char *) physical_memory_write);
	Z502ReadPhysicalMemory(17, (char *) physical_memory_read);
	for (i = 0; i < PGSIZE ; i++) {
		if (physical_memory_write[i] != physical_memory_read[i])
			printf("Error in Physical Memory Access\n");
	}
	printf("Completed test of physical memory write and read.\n");

	/*********************************************************************
	 Show the interface to read and write of real memory
	 It turns out, that though these are hardware calls, the Z502
	 assumes the calls are being made in user mode.  Remember that this
	 code Sample is running in Kernel Mode so the results will look different
	 from a memory request that is initiated in User Mode in Test2a.
	 What you see here IS the way that the OS can manipulate page tables
	 and access memory.
	 *********************************************************************/

	mmio.Mode = Z502GetPageTable;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502Context, &mmio);
	PAGE_TBL_ADDR = (short *) mmio.Field1;   // Gives us the page table
	// Set to VALID the logical page 0 and have it point at physical
	// frame 0.
	i = PTBL_VALID_BIT;
	PAGE_TBL_ADDR[0] = (UINT16) i;
	i = 73;                   // Data to be written
	MEM_WRITE(0, &i);
	MEM_READ(0, &j);          // Now read it back
	//  WE expect the data read back to be the same as what we wrote
	if (i == j)
		printf("Memory write and read completed successfully\n");
	else
		printf("Memory write and read were NOT successful.\n");

	/*********************************************************************
	 This is the interface to the locking mechanism.  These are hardware
	 interlocks.  We need to test that they work here.  This is the
	 interface we'll be using.

	 void    Z502_READ_MODIFY( INT32 VirtualAddress, INT32 NewLockValue,
	 INT32 Suspend, INT32 *LockResult )

	 We've defined these above to help remember them.
	 #define                  DO_LOCK                     1
	 #define                  DO_UNLOCK                   0
	 #define                  SUSPEND_UNTIL_LOCKED        TRUE
	 #define                  DO_NOT_SUSPEND              FALSE

	 *********************************************************************/

	printf("++++  Starting Hardware Interlock Testing   ++++\n");

	//  TRY A SERIES OF CALLS AS DESCRIBED HERE
	printf("These tests map into the matrix describing the Z502_READ_MODIFY\n");
	printf("     described in Appendix A\n\n");

	printf(
			"1.  Start State = Unlocked:  Action (Thread 1) = Lock: End State = Locked\n");
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED,
			&LockResult);
	printf("%s\n", &(Success[SPART * LockResult]));

	printf(
			"2.  Start State = locked(1): Action (Thread 1) = unLock: End State = UnLocked\n");
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&LockResult);
	printf("%s\n", &(Success[SPART * LockResult]));

	printf(
			"3.  Start State = Unlocked:  Action (Thread 1) = unLock: End State = UnLocked\n");
	printf("    An Error is Expected\n");
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&LockResult);
	printf("%s\n", &(Success[SPART * LockResult]));

	printf(
			"4.  Start State = unlocked:  Action (Thread 1) = tryLock: End State = Locked\n");
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_LOCK, DO_NOT_SUSPEND, &LockResult);
	printf("%s\n", &(Success[SPART * LockResult]));

	printf(
			"5.  Start State = Locked(1): Action (Thread 1) = tryLock: End State = Locked\n");
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_LOCK, DO_NOT_SUSPEND, &LockResult);
	printf("%s\n", &(Success[SPART * LockResult]));

	printf(
			"6.  Start State = locked(1): Action (Thread 1) = unLock: End State = UnLocked\n");
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&LockResult);
	printf("%s\n", &(Success[SPART * LockResult]));

	printf(
			"7.  Start State = Unlocked:  Action (Thread 1) = Lock: End State = Locked\n");
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED,
			&LockResult);
	printf("%s\n", &(Success[SPART * LockResult]));

	//  A thread that locks an item it has already locked will succeed
	printf(
			"8.  Start State = locked(1): Action (Thread 1) = Lock: End State = Locked\n");
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED,
			&LockResult);
	printf("%s\n", &(Success[SPART * LockResult]));

	// Locking a thread that's already locked - but do it with a thread
	// other than the one that did the locking.
	printf(
			"9.  Start State = Locked(1): Action (Thread 2) = tryLock: End State = Locked\n");
	printf("    An Error is Expected\n");
	Status = CreateAThread((int *) DoOnelock, &Temp);
	DoASleep(100); /*  Wait for that thread to finish   */

	// Unlock a thread that's already locked - but unlock it with a thread
	// other than the one that did the locking.
	printf(
			"10. Start State = Locked(1): Action (Thread 2) = unLock: End State = Locked\n");
	printf("    An Error is Expected\n");
	Status = CreateAThread((int *) DoOneUnlock, &Temp);
	DoASleep(100); /*  Wait for that thread to finish   */

	// The second thread will try to get the lock held by the first thread.  This is OK
	// but the second thread will suspend until the first thread releases the lock.
	printf(
			"11. Start State = Locked(1): Action (Thread 2) = Lock: End State = Locked\n");
	Status = CreateAThread((int *) DoOneTrylock, &Temp);
	DoASleep(100); /*  Wait for that thread to finish   */

	//  The first thread does an unlock.  This means the second thread is now able to get
	//  the lock so there is a "relock" when thread two succeeds.
	printf(
			"12. Start State = locked(1): Action (Thread 1) = unLock: End State = Locked(by 2)\n");
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&LockResult);
	printf("%s\n", &(Success[SPART * LockResult]));
	DoASleep(100); /*  Wait for locking action of 2nd thread to finish   */

	printf(
			"13. Start State = Locked(2): Action (Thread 3) = unLock: End State = Locked(2)\n");
	printf("    An Error is Expected\n");
	Status = CreateAThread((int *) DoOneUnlock, &Temp);
	DoASleep(100); /*  Wait for that thread to finish   */

	printf(
			"14. Start State = Locked(2): Action (Thread 1) = tryLock: End State = Locked(2)\n");
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_LOCK, DO_NOT_SUSPEND, &LockResult);
	printf("%s\n", &(Success[SPART * LockResult]));

	printf(
			"15. Start State = locked(2): Action (Thread 1) = unLock: End State = Locked(2)\n");
	printf("    An Error is Expected\n");
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&LockResult);
	printf("%s\n", &(Success[SPART * LockResult]));

	printf("++++  END of hardware interlock code  ++++\n\n");

	/*********************************************************************
	 Show the interface to the CONTEXT calls.

	 The best place to see this usage is in os_init in base.c
	 *********************************************************************/

	/*********************************************************************
	 Show the interface to the scheduler printer.
	 The main job here is to fill in the structure that is passed
	 to the printer.
	 *********************************************************************/
	memset(&SPData, 0, sizeof(SP_INPUT_DATA));
	strcpy(SPData.TargetAction, "Create");
	SPData.CurrentlyRunningPID = 2;
	SPData.TargetPID = 3;
	// The NumberOfRunningProcesses as used here is for a future implementation
	// when we are running multiple processors.  For right now, set this to 0
	// so it won't be printed out.
	SPData.NumberOfRunningProcesses = 0;

	SPData.NumberOfReadyProcesses = 3;   // Processes ready to run
	for (i = 0; i <= SPData.NumberOfReadyProcesses; i++) {
		SPData.ReadyProcessPIDs[i] = i;
	}
	SPData.NumberOfProcSuspendedProcesses = 4;
	for (i = 0; i <= SPData.NumberOfProcSuspendedProcesses; i++) {
		SPData.ProcSuspendedProcessPIDs[i] = i + 3;
	}
	SPData.NumberOfMessageSuspendedProcesses = 2;
	for (i = 0; i <= SPData.NumberOfMessageSuspendedProcesses; i++) {
		SPData.MessageSuspendedProcessPIDs[i] = i + 16;
	}
	SPData.NumberOfTimerSuspendedProcesses = 6;
	for (i = 0; i <= SPData.NumberOfTimerSuspendedProcesses; i++) {
		SPData.TimerSuspendedProcessPIDs[i] = i + 8;
	}
	SPData.NumberOfDiskSuspendedProcesses = 1;
	for (i = 0; i <= SPData.NumberOfDiskSuspendedProcesses; i++) {
		SPData.DiskSuspendedProcessPIDs[i] = i + 15;
	}
	SPData.NumberOfTerminatedProcesses = 0;   // Not used at this time

	CALL(SPPrintLine(&SPData));

	/*********************************************************************
	 Show the interface to the memory_printer.
	 We're just inputting random values here to show the interface.
	 *********************************************************************/

	memset(&MPData, 0, sizeof(MP_INPUT_DATA));  // Good practice - clean up

	for (j = 0; j < NUMBER_PHYSICAL_PAGES ; j = j + 2) {
		MPData.frames[j].InUse = TRUE;
		MPData.frames[j].Pid = (j / 2) % 10;
		MPData.frames[j].LogicalPage = j * 16 + 10;
		MPData.frames[j].State = (j / 2) % 8;
	}
	MPPrintLine(&MPData);

	/*********************************************************************
	 Show how the skewed random numbers work on this platform.
	 *********************************************************************/
	for (j = 0; j < NUM_RAND_BUCKETS; j++)
		random_buckets[j] = 0;

	for (j = 0; j < 100000; j++) {
		GetSkewedRandomNumber(&Value, NUM_RAND_BUCKETS);
		random_buckets[Value]++;
	}
	printf("\nTesting that your platform produces correctly skewed random numbers.\n");
	printf("Each row should (with some exceptions) have higher count than the previous.\n");
	printf("    Range:   Counts in this range.\n");
	for (j = 0; j < NUM_RAND_BUCKETS; j = j + 8) {
		k = 0;
		for (i = j; i < j + 8; i++)
			k += random_buckets[i];
		printf("%3d - %3d:  %d\n", j, j + 7, k);
	}

	/*********************************************************************
	 Show the interface to the Z502Halt.
	 Note that the program will end NOW, since we don't return
	 from the command.
	 *********************************************************************/

	MEM_WRITE(Z502Halt, 0);

}

void starting_point_for_new_context(void) {
}

void DoOnelock(void) {
	INT32 LockResult;
	printf("      Thread 2 - about to do a lock\n");
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_LOCK, SUSPEND_UNTIL_LOCKED,
			&LockResult);
	printf("      Thread 2 Lock:  %s\n", &(Success[SPART * LockResult]));
//    DestroyThread( 0 );
}
void DoOneTrylock(void) {
	INT32 LockResult;
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_LOCK, DO_NOT_SUSPEND, &LockResult);
	printf("      Thread 2 TryLock:  %s\n", &(Success[SPART * LockResult]));
//    DestroyThread( 0 );
}
void DoOneUnlock(void) {
	INT32 LockResult;
	READ_MODIFY(MEMORY_INTERLOCK_BASE, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&LockResult);
	printf("      Thread 2 UnLock:  %s\n", &(Success[SPART * LockResult]));
//    DestroyThread( 0 );
}
// This routine takes the place of a real sleep function such as you will
// write in your code.  Do NOT use this kind of loop routine in your code.
void DoASleep(INT32 LoopUnit) {
	int i, j;
	static int k = 0;
	for ( i = 0; i < LoopUnit; i++ ) {
		for ( j = 0; j < 10000000; j++)
			k = k + j;
	}
}
// This is a simple routine to ask the hardware for the time.  Note that
// the time used by the Z502 simulation is not "real" time.
int GetSampleTimeNow()  {
	MEMORY_MAPPED_IO mmio;        // Structure used for hardware interface
        mmio.Mode = Z502ReturnValue;
        mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
        MEM_READ(Z502Clock, &mmio);
        return  mmio.Field1;
}
