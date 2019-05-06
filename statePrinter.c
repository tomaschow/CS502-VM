/****************************************************************************
 StatePrinter.c

 Read Appendix D - output generation - in order to understand how
 to use these routines.

 The first portion of this file is SCHEDULER_PRINTER.

 Revision History:
 1.1    December 1990    Initial release
 2.1    April 2001       Added memory_printer
 2.2    July  2002       Make code appropriate for undergrads.
 3.0    August  2004:    Modified to support memory mapped IO
 3.60   August  2012:    Used student supplied code to add support
 .                       for Mac machines
 4.10   December 2013:   Remove SP_setup_file and SP_setup_action
 .                       Roll SP_ACTION_MODE into the routine SP_setup
 .                       Define some new states.
 4.20   May 2015:        The way this code HAS been written is inherently
 .                       STATEFUL.  The code had a number of modes that needed
 .                       to be filled in.  Unfortunately, that meant that
 .                       different threads tangled with each other.
 4.40    March 2017      Place locks around these routines so they will be
                         single threaded.
 ****************************************************************************/

#include                 "syscalls.h"
#include                 "z502.h"
#include                 "protos.h"
#include                 "stdio.h"
#include                 "string.h"

#include "global.h"
#if defined LINUX || defined MAC
#include                 <unistd.h>
#endif

//
//      This string is printed out as the header

#define         SP_HEADER_STRING        \
" Time Target   Action Run   State    Populations \n"

// Note these locks are at the top end of the allowed range of user locks
#define         SP_USER_LOCK                0xF0
#define         MP_USER_LOCK                0xF1
// Used to do locking and unlocking
#define         DO_LOCK                     1
#define         DO_UNLOCK                   0
#define         SUSPEND_UNTIL_LOCKED        TRUE
#define         DO_NOT_SUSPEND              FALSE

// Prototypes for support routines
short SPDoOutput(char *text);
void SPLineSetup(char *, char *, INT16, INT16 *);

/****************************************************************************
 SPPrintLine

 SP_print_line prints out all the data that has been defined in the
 SP_INPUT_DATA structure by the caller.  The goal here is to make this
 print operation as atomic as possible.  Unfortunately the print statement,
 as executed by the native OS is not atomic, so we've place a lock here.
 ALL the data needed for the output is in the structure SP_INPUT_DATA which
 is described in syscalls.h    An example of the use of this code is
 found in sample.c
 ****************************************************************************/

short SPPrintLine(SP_INPUT_DATA *Input) {
	char OutputLine[900];
	char temp[60];
	INT32 LockResult;

	// Note we are using an OS level lock here - the same type as a student
	// might use in an OS.  This is because the statePrinter is being executed
	// at the OS level, not within the hardware.
	READ_MODIFY(MEMORY_INTERLOCK_BASE + SP_USER_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
				&LockResult);

	// Used to get the current_time;
	MEMORY_MAPPED_IO mmio;

	// print out the header
	sprintf(OutputLine, "%s", SP_HEADER_STRING);

	//  Get the current time and place it in the output string
	mmio.Mode = Z502ReturnValue;
	mmio.Field1 = mmio.Field2 = mmio.Field3 = 0;
	MEM_READ(Z502Clock, &mmio);
	sprintf(temp, "%5d", (int) mmio.Field1 % 100000);
	strcat(OutputLine, temp);

	// If user defines the target PID, place it here
	if (Input->TargetPID >= 0)
		sprintf(temp, "%5d  ", Input->TargetPID);
	else
		sprintf(temp, "%s", "       ");
	(void) strcat(OutputLine, temp);

	sprintf(temp, " %8s", Input->TargetAction); /* Action       */
	(void) strcat(OutputLine, temp);

	sprintf(temp, " %3d   ", Input->CurrentlyRunningPID);
	(void) strcat(OutputLine, temp);

//   Multiple running processes is not implemented in Release 4.20
	SPLineSetup(OutputLine, "RUNNING:", Input->NumberOfRunningProcesses,
			Input->RunningProcessPIDs);
	SPLineSetup(OutputLine, "READY  :", Input->NumberOfReadyProcesses,
			Input->ReadyProcessPIDs);
	SPLineSetup(OutputLine, "SUS-PRC:", Input->NumberOfProcSuspendedProcesses,
			Input->ProcSuspendedProcessPIDs);
	SPLineSetup(OutputLine, "SUS-TMR:", Input->NumberOfTimerSuspendedProcesses,
			Input->TimerSuspendedProcessPIDs);
	SPLineSetup(OutputLine, "SUS-MSG:",
			Input->NumberOfMessageSuspendedProcesses,
			Input->MessageSuspendedProcessPIDs);
	SPLineSetup(OutputLine, "SUS-DSK:", Input->NumberOfDiskSuspendedProcesses,
			Input->DiskSuspendedProcessPIDs);
	SPLineSetup(OutputLine, "TERM'S :", Input->NumberOfTerminatedProcesses,
			Input->TerminatedProcessPIDs);

	SPDoOutput(OutputLine);    // We've accumulated the whole line - print it.
	READ_MODIFY(MEMORY_INTERLOCK_BASE + SP_USER_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&LockResult);
	return 0;

} // End of SP_print_line

/****************************************************************************
 SPLineSetup

 Takes input from the user and packages it in a pretty fashion
 char OutputText -   the running string onto which all text is added
 char *mode -        character string describing the kind of pids we're working with
 INT16 Number -      how many processes are there in this category
 INT16 ArrayOfPids - the process IDs in this category
 ****************************************************************************/
void SPLineSetup(char *OutputText, char *mode, INT16 Number,
		INT16 ArrayOfPids[]) {
	char Temp[64];
	int index;
	if (Number > 0) {    // There are pids to deal with

		strcat(OutputText, mode);
		for (index = 0; index < Number; index++) {
			sprintf(Temp, " %d", ArrayOfPids[index]);
			(void) strcat(OutputText, Temp);
		}
		strcat(OutputText, "\n                            ");
	}
} // End of SPLineSetup
/****************************************************************************
 SPDoOutput

 This little routine simply directs output to the screen.
 ****************************************************************************/

short SPDoOutput(char *text) {
	printf("%s\n", text);
	return 0;
} /* End of SP_do_output */

/****************************************************************************

 Read Appendix D - output generation - in order to understand how
 to use these routines.

 The second portion of this source file is MEMORY_PRINTER.

 This is the definition of the table we will produce here.

 A  Frame  0000000000111111111122222222223333333333444444444455555555556666
 B  Frame  0123456789012345678901234567890123456789012345678901234567890123
 C  PID    0000000001111
 D  VPN    0000000110000
 E  VPN    0000111000000
 F  VPN    0000000220000
 G  VPN    0123567230123
 H  VMR    7775555447777

 The rows mean the following:
 A - B:     The frame number.  Note how the first column is "00" and the
 .          last column is "63".
 C:         The Process ID of the process having it's virtual page in the
 .          frame table.
 D - G:     The virtual page number of the process that's mapped to that
 .          frame.  Again the number (from 0 to 1023 possible) is written
 .          vertically.
 H:         The state of the page.   Valid = 4, Modified = 2,
 .          Referenced = 1.  These are OR'd (or added) together.

 Example: The page in frame 6 is virtual page 107 in  process 0.  That
 page has been made valid and has been referenced.
 ****************************************************************************/

/****************************************************************************
 MPPrintLine

 Outputs everything we know about the state of the physical memory.  Again,
 your task is to fill in the MP_INPUT_DATA structure which is described
 in syscalls.h  An example of the use of this code is found in sample.c
 ****************************************************************************/

short MPPrintLine(MP_INPUT_DATA *Input) {
	char OutputLine[1200];
	INT32 index;
	INT32 Temporary;
	INT32 LockResult;
	char temp[120];
	char output_line3[NUMBER_PHYSICAL_PAGES + 5];
	char output_line4[NUMBER_PHYSICAL_PAGES + 5];
	char output_line5[NUMBER_PHYSICAL_PAGES + 5];
	char output_line6[NUMBER_PHYSICAL_PAGES + 5];
	char output_line7[NUMBER_PHYSICAL_PAGES + 5];
	char output_line8[NUMBER_PHYSICAL_PAGES + 5];

	READ_MODIFY(MEMORY_INTERLOCK_BASE + MP_USER_LOCK, DO_LOCK, SUSPEND_UNTIL_LOCKED,
				&LockResult);
//  Header Line
	strcpy(OutputLine, "\n                       PHYSICAL MEMORY STATE\n");

//  First Line
	strcat(OutputLine,
			"Frame 0000000000111111111122222222223333333333444444444455555555556666\n");

//  Second Line
	strcat(OutputLine,
			"Frame 0123456789012345678901234567890123456789012345678901234567890123\n");

//  Third - Eighth Line
	strcpy(output_line3,
			"                                                                 \n");
	strcpy(output_line4,
			"                                                                 \n");
	strcpy(output_line5,
			"                                                                 \n");
	strcpy(output_line6,
			"                                                                 \n");
	strcpy(output_line7,
			"                                                                 \n");
	strcpy(output_line8,
			"                                                                 \n");

// Here we take the input data and arrange it artfully for our output

	for (index = 0; index < NUMBER_PHYSICAL_PAGES ; index++) {
		if (Input->frames[index].InUse == TRUE) {
			output_line3[index] = (char) (Input->frames[index].Pid + 48);
			Temporary = Input->frames[index].LogicalPage;
			output_line4[index] = (char) (Temporary / 1000) + 48;
			output_line5[index] = (char) ((Temporary / 100) % 10) + 48;
			output_line6[index] = (char) ((Temporary / 10) % 10) + 48;
			output_line7[index] = (char) ((Temporary) % 10) + 48;
			output_line8[index] = (char) Input->frames[index].State + 48;
		}
	}
	sprintf(temp, "PID   %s", output_line3);
	strcat(OutputLine, temp);
	sprintf(temp, "VPN   %s", output_line4);
	strcat(OutputLine, temp);
	sprintf(temp, "VPN   %s", output_line5);
	strcat(OutputLine, temp);
	sprintf(temp, "VPN   %s", output_line6);
	strcat(OutputLine, temp);
	sprintf(temp, "VPN   %s", output_line7);
	strcat(OutputLine, temp);
	sprintf(temp, "VMR   %s", output_line8);
	strcat(OutputLine, temp);

	SPDoOutput(OutputLine);
	READ_MODIFY(MEMORY_INTERLOCK_BASE + MP_USER_LOCK, DO_UNLOCK, SUSPEND_UNTIL_LOCKED,
			&LockResult);

	return 0;
}    // End of MPPrintLine
