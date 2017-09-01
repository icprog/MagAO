#ifndef COMMANDSENDER_H_INCLUDE
#define COMMANDSENDER_H_INCLUDE

#include "Logger.h"
#include "AOExcept.h"
#include "AOApp.h"


using namespace Arcetri;

namespace Arcetri {
	
	namespace Bcu {

		/*
		 * @Class{API}: CommandSenderException
		 * Root class for exceptions thorwn by CommandSender.
		 * @
		 */ 
		class CommandSenderException: public AOException {
			public:
				CommandSenderException(string id, string m):AOException(m) { exception_id = id; }
		};
		
		/*
		 * @Class{API}: CommandSenderException
		 * Exception thrown by CommandSender.
		 * The command have been sent but the answer is not a succes, but a FAULT.
		 * @
		 */
		class RequestFaultException: public CommandSenderException {
			public:
				RequestFaultException():CommandSenderException("RequestFaultException","The request has failed") {};
		};
		
		/*
		 * @Class{API}: CommandSenderTimeoutException
		 * Exception thrown by CommandSender.
		 * The specified timeout has expired before the answer came back.
		 * @
		 */
		class CommandSenderTimeoutException: public CommandSenderException {
			public:
				CommandSenderTimeoutException():CommandSenderException("CommandSenderTimeoutException","Timeout expired") {};
		};
		
		/*
		 * @Class{API}: CommandSendingException
		 * Exception thrown by CommandSender.
		 * The command hasn't been sent, for some reasons (bcu not existing; impossible 
		 * to allocate the shared memory buffer; thSendMsg failed).
		 * @
		 */ 
		class CommandSendingException: public CommandSenderException {
			public:
				CommandSendingException():CommandSenderException("CommandSendingException","Impossible to send the command") {};
		};
		
		/*
		 * @Class{API}: CommandSendingException
		 * Exception thrown by CommandSender.
		 * The command hasn't been sent, for some reasons (bcu not existing; impossible 
		 * to allocate the shared memory buffer; thSendMsg failed).
		 * @
		 */ 
		class CommandLengthException: public CommandSenderException {
			public:
				CommandLengthException(int datalen):CommandSenderException("CommandLengthException","")
              {
              ostringstream ss;
              ss << "Incorrect request length: " << datalen << " bytes";
              _message = ss.str(); 
              }
		};
		
		
		/*
		 * @Class{API}: CommandSender
		 * Thread-safe version of the "bcucommand" library.
		 * 
		 * A command sender allow a client to send commands to a pool
		 * of BCUs and check the command result using the thrown exceptions.
		 * 
		 * All the thrown exceptions are returned by pointer because they
		 * all derive from "CommandSenderException", and by this way can 
		 * be handled in a polimorphic way: usually the client could catch
		 * only a generic "CommandSenderException*" AND the specific
		 * "CommandSenderTimeoutException*" (see sendMultiBcuCommand(...) 
		 * above), avoiding the full list.
		 */
		class CommandSender {
			
			public:
			
				/*
				 * Creates a thread-safe object able to send commands
				 * to a set of BCUs.
				 * 
				 * The same object can be used to perform an indefinite sequence
				 * of sendMultiBcuCommand(...), while a CommandSenderTimeoutException
				 * isn't returned (see sendMultiBcuCommand(...) above)
				 * 
				 * NOTE: the client must remember to delete the exception !!!
				 * 
				 */
				CommandSender();
				/*
				 * Destroy the object taking care of detach and - if nobody is attached -
				 * clear the shared memory buffer.
				 */
				~CommandSender();

				
				/*
				 * @Method{API}: sendMultiBcuCommand
				 *  Send a bcu command allowing to specify an arbitrary bcu range. Allow to 
				 * address multiple Dsp for each Bcu.
				 * Check for the existance of the addressed Bcus.
				 * 
				 * If the command is succesfully completed (command sent and successfully
				 * executed) the method terminates.
				 * 
				 * Otherwise an exception is thrown:
				 * 
				 *  - CommandSendingException: the command hasn't been sent, for some reasons
				 * 							   (bcu not existing; impossible to allocate the shared 
				 * 							   memory buffer; thSendMsg failed)
				 * 
				 * 	- RequestFaultException: the command have been sent but the answer is
				 * 							 not a succes, but a FAULT.
				 * 
				 *	- CommandSenderTimeoutException: the specified timeout has expired before
				 * 									 the answer came back.
				 * 
				 * 				 
				 * !!! VERY IMPORTANT !!!
				 * If a CommandSenderTimeoutException is thrown the client MUST
				 * destroy the CommandSender and NOT reuse it for the next request,
				 * because is possible that the attached shared memory is still 
				 * used by the MirrorCtrl.
				 * @
				 */
				void sendMultiBcuCommand(int priority, 			 // @P{priority}: see Constants class in lib/BcuCommon.h
							 int firstBcu, 			 // @P{firstBcu}: first Bcu to address
							 int lastBcu,  			 // @P{lastBcu}: last Bcu to address
							 int firstDsp, 			 // @P{firstDsp}: first Dsp to address
							 int lastDsp, 			 // @P{lastDsp}: last Dsp to address
							 int opcode, 			 // @P{opcode}: see OpCodes class .
											 //             Note that the SAME/SEQ type refers to both 
											 //			    BCU and DSP !!!
							 int address, 			 // @P{address}: data address on bcu.
							 int datalen, 			 // @P{datalenBytes}: Length of data for a single Bcu and Dsp.
											 // Its meaning depends on "opcode" parameter:
											 //  - For a SAME "opcode": match the size of "data" parameter
											 //  - For a SEQ "opcode": when multiplied for BcuNum*DspNum match 
											 //    the size of "data" parameter
							 unsigned char *data,	 	 // @P{data}: reference to data buffer
							 int maxClientTimeout_ms, 	 // @P{maxClientTimeout_ms}: Max timeout allowed by the client. When timeout
											 //  					     expires a CommandSenderTimeoutException  is thrown.
							 int flags)			 // @P{flags}: see Constants class
									throw (RequestFaultException, CommandSenderTimeoutException, CommandSendingException);
				
				
				/*
				 * @Method{API}: sendMultiBcuCommand
				 * Overloading of the @see{sendMultiBcuCommand} method.
				 * 
				 * Send a command to a single Bcu or to a set of Bcu using some special 
				 * IDs (Constants::BCU_ALL, Constants::BCUMIRROR_ALL). 
				 * @
				 */
				void sendMultiBcuCommand(int priority, 				 //see class Priority
							 int bcuId,  				 //see class Constants
							 int firstDsp, int lastDsp, 
							 int opcode, 				 //see class OpCodes
							 int address, int datalen, 
							 unsigned char *data, 
							 int maxClientTimeout_ms, 	 // Max timeout allowed by the client
							 int flags)	 
								throw (RequestFaultException, CommandSenderTimeoutException, CommandSendingException);
		
		
			private:
			
				// For internal use only (got from WfsConstants.h and AdSecConstants.h)
				int _BCU_39;
				int _BCU_47;
				int _BCU_SWITCH;
				int _BCUMIRROR_0;
				int _BCUMIRROR_1;
				int _BCUMIRROR_2;
				int _BCUMIRROR_3;
				int _BCUMIRROR_4;
				int _BCUMIRROR_5;
				int _BCUMIRROR_ALL;
				int _BCU_ALL;
				
                /*
                 * name of the mirrorctrl server to which requests are sent
                 */
                string _mirrorctrl;     
			
				/*
				 * Custom shared memory 
				 */
				 class SharedMemoryException: public CommandSenderException {
					public:
						SharedMemoryException(string m):CommandSenderException("SharedMemoryException", m) {};
				};
				
				struct SharedMem {
				 	int shm_id;                     // Shared memory id
				 	int blocksize;                  // Size of shared memory buffer
				 	unsigned char *shm_addr;        // Shared memory buffer address
				} *_shared;
				
				void shmAlloc(int size) throw(SharedMemoryException);
				void shmFree();
				
				/*
				 * A stupid logger
				 */
				Logger* _logger;
		};
		
	}

}

#endif /*COMMANDSENDER_H_INCLUDE*/
