/**
 * @mainpage    Lightweight Message-Passing Embedded Kernel
 *
 * @section     Project Information
 *              This kernel enables multi-tasking capabilities 
 *				in an embedded environment.
 *				It also provides message-passing capabilities between processes and
 * 				user Input/Output functionality via message-passing.
 *				 
 * @section		Kernel calls
 *				Kernel functionality and information can be accessed by a process via
 *				kernel calls.
 *				The supported kernel calls can be found in calls.h
 *				Message-passing is done via kernel calls.
 *
 * @section		Message Passing System
 *				This kernel uses a "boxing" system for message passing.
 *				A process must bind itself to an available message box in order 
 * 				to engage in inter-process communications.
 *				these message boxes allow the process to receive and send messages 
 *				to other processes.
 *				A process can have multiple message boxes bound to it, and can unbind
 *				them at any time via an unbind kernel call.
 *				A process doesn't directly use messages in IPC, but rather it uses
 *				the send & recv kernel calls to send byte streams of specified length
 *				to a specific message box (denoted by its box ID) 
 *				from a message box owned by the process.
 *
 * @subsection	Process Request Transaction
 *				A common interaction between process is sending a message to one with
 *				data pertaining a specific action to be performed by the destination
 *				process, and then receive a message from the same process with data
 *				related to the action performed.
 *				This is how a process communicates with the IO Server, which is 
 *				responsible for displaying and receiving data to/from the user, 
 *				although the request transaction is encapsulated with the 
 *				send_user and recv_user kernel calls.
 *				To make these transaction process faster however, a "request" kernel
 *				call is offered that performs the sending and receiving with one
 *				kernel call instead of requiring two or more kernel calls to perform
 *				it.
 *
 * @section 	IO Server
 *				The kernel comes with a IO server process that enables data to be
 *				displayed and received to/by the user via a terminal.
 *				Accessing the IO server can be done via a request call to the IO_BOX
 *				message box, and the req_data must be a IO_metadata structure which
 *				contains information that the server requires to perform process
 *				requests to output data to user or receive data from user.
 *
 * subsection	IO Server Terminal mode
 *				the IO server doubles up as a terminal program that can take in user
 *				commands to enable process' IO permissions, and to view system 
 *				process' information.
 *				currently supported commands are:
 *				sysview:
 *					displays information about the system processes.
 *				setio pid# pid# pid#... :
 *					used to enable IO permissions for the system processes.
 *					can be used with no pid# following it to enable IO permissions
 *					for all processes.
 *				cleario pid# pid# pid#... :
 *					used to disable IO permissions for the system processes.
 *					can be used with no pid# following it to disable IO permissions
 *					for all processes.
 *				run:
 *					Places IO server process in the background 
 *					and allows user processes to run.
 *
 *				Whenever the terminal process is running, the user-processes won't 
 *				be able to run. the 'run' command must be inputted by the user to 
 *				place the terminal in the background.
 * 				The IO server can be set to be in terminal mode at anytime by 
 *				inputting ctrl+c, which will then pause all user processes.
 *
 * @section     Communications
 *              The IO server uses UART to communicate with the user,
 *				which can be accessed by a computer via a Serial COM port 
 *				and an emulated terminal program like PuTTY.
 *
 * @subsection  Serial Port Settings
 *              * 115200 baud rate,
 *              * 8 data bits,
 *              * 1 stop bit,
 *              * NO parity,
 *              * NO flow control.
 *
 *              Check device manager (or equivalent) to see which 
 *				COM port the board is connected to.
 *                  - Name of board is "Stellaris Virtual Serial Port"
 *
 *              It is also recommended that you enable implicit CR in every LR & 
 *				implicit LR in every CR on your terminal settings.
 */