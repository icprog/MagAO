/* 
 * @File: adsec_comm.h
 * Bcu communication IDL interface for AdSec
 * 
 * Defines the entry point for the Idl Wrapper Routines
 * for the LBT672 Adaptive Secondary Mirror control and diagnostic 
 *
 * err = CALL_EXTERNAL(sc.dll_name, 'read_seq',               $
 *                      bcu_num, first_dsp, last_dsp, dsp_address, data_length,   $
 *                      set_PD
 * read_seq.pro IDL call_external function example 
 * 
 * 
 * ATTENTION: in progress moving to idl_wraplib.dlm !!!
 *
 * @
 */

// Error codes
#ifndef NO_ERROR
#define NO_ERROR    (0L)
#endif /* NO_ERROR */


//Functions prototypes
extern "C"{
	
	/*
	 * Used to load IDL callable functions defined by DLM
	 */
	int IDL_Load(void);
	
	
	// -------------------- PROTOTYPES FOR OLD-STYLE CALL-EXTERNAL ----------------------- //

	
	/*
	 * @Function{API}: sendMultiBcuCommand
	 * Send a command to a set of MirrorBcu.
	 * 
	 * This is a wrapper function to call lib/CommandSender::sendMultiBcuCommand(...)
	 * taking care of:
	 *  - Addressing the BCUs with IDs defined in Bcu::Constants
	 * 	- Managing the CommandSender creation/destruction
	 * 	- Handling the exceptions and converting them to PLAIN ERRORS
	 * 
	 * IMPORTANT: the opcode include the SEQ/SAME "flag", and it refers to both  BCU and DSP.
	 * 			  This means that is impossible to perform an operation with a buffer hosting
	 * 			  SEQ data for DSPs and SAME data for BCUs (or vice/versa).
	 * 			  This choice is based on the fact that the set of numBcu MirrorBcu are intended as an
	 * 			  unique big Bcu, composed of numDspPerBcu*numBcu.
	 * @
	 */
	IDL_INT sendMultiBcuCommand(int priority, 			// @P{priority}: see Constants class in lib/BcuCommon.h
								int firstBcu, 			// @P{firstBcu}: first Bcu to address
								int lastBcu, 			// @P{lastBcu}: last Bcu to address
								int firstDsp, 			// @P{firstDsp}: first Dsp to address
								int lastDsp, 			// @P{lastDsp}: last Dsp to address
								int opcode, 			// @P{opcode}: see OpCodes class in lib/BcuCommon.h
														//             Note that the SAME/SEQ type refers to both 
														//			   BCU and DSP !!!
								int address, 			// @P{address}: data address on bcu.
								int datalenBytes, 		// @P{datalenBytes}: Length of data for a single Bcu and Dsp.
														// Its meaning depends on "opcode" parameter:
														//  - For a SAME "opcode": match the size of "data" parameter
														//  - For a SEQ "opcode": when multiplied for BcuNum*DspNum match 
														//    the size of "data" parameter
								unsigned char *data, 	// @P{data}: reference to data buffer
								int timeout_ms, 		// @P{timeout_ms}: accepted timeout. When timeout expires a
														//                 TIMEOUT_ERROR is returned.
								int flags); 			// @P{flags}: see Constants class in lib/BcuCommon.h
	
	/*
	 * @Function{API}: read_seq
	 * Perform a READ_SEQ command (single BCU version)
	 * 
	 * Note that SEQ is referred to both addressed Bcu and Dsp.
	 * Please see @see{sendMultiBcuCommand} for more details.
	 * @
	 */
	IDL_LONG read_seq(int Argc, void *Argv[]);
	
	/*
	 * @Function{API}: write_seq
	 * Perform a WRITE_SEQ command (single BCU version)
	 * 
	 * Note that SEQ is referred to both addressed Bcu and Dsp.
	 * Please see @see{sendMultiBcuCommand} for more details.
	 * @
	 */
	IDL_LONG write_seq(int Argc, void *Argv[]);
	
	/*
	 * @Function{API}: write_same_multi
	 * Perform a WRITE_SAME command (single BCU version)
	 * 
	 * Note that SAME is referred to both addressed Bcu and Dsp.
	 * Please see @see{sendMultiBcuCommand} for more details.
	 * @
	 */
	IDL_LONG write_same(int Argc, void *Argv[]);
	
	/*
	 * @Function{API}: read_seq_multi{API}
	 * Perform a READ_SEQ command (multi BCU version)
	 * 
	 * Note that SEQ is referred to both addresesd Bcu and Dsp.
	 * Please see @see{sendMultiBcuCommand} for more details.
	 * @
	 */
	IDL_LONG read_seq_multi(int Argc, void *Argv[]);
	
	/*
	 * @Function{API}: write_seq_multi
	 * Perform a WRITE_SEQ command (multi BCU version)
	 * 
	 * Note that SEQ is referred to both addressed Bcu and Dsp.
	 * Please see @see{sendMultiBcuCommand} for more details.
	 * @
	 */
	IDL_LONG write_seq_multi(int Argc, void *Argv[]);
	
	/*
	 * @Function{API}: write_same_multi
	 * Perform a WRITE_SAME command (multi BCU version)
	 * 
	 * Note that SAME is referred to both addressed Bcu and Dsp.
	 * Please see @see{sendMultiBcuCommand} for more details.
	 * @
	 */
	IDL_LONG write_same_multi(int Argc, void *Argv[]);
	
	
	IDL_LONG idl_setup( int lArgc, void *Argv[]);
	IDL_LONG smreadbuf(int Argc, void *Argv[]);
	IDL_LONG idl_close( int lArgc, void *Argv[]);
	
	IDL_LONG clear_mem(int Argc, void *Argv[]);
	IDL_LONG lock_flash(int Argc, void *Argv[]);
	IDL_LONG unlock_flash(int Argc, void *Argv[]);
	
	IDL_LONG reset_devices(int Argc, void *Argv[]);
	IDL_LONG reset_devices_multi(int Argc, void *Argv[]);
	IDL_LONG reset_devices_multi_wait(int Argc, void *Argv[]);
	
	IDL_LONG powerup(int Argc, void *Argv[]);
	IDL_LONG createvar(int Argc, void *Argv[]);
	IDL_LONG getvar(int Argc, void *Argv[]);
	IDL_LONG get_vartypes(int Argc, void *Argv[]); 
	IDL_LONG setvar(int Argc, void *Argv[]); 
	IDL_LONG writevar(int Argc, void *Argv[]); 
}

BufInfo *is_attached( char *bufname);
void resize_Info (char *bufname);
int idl_attach( char *bufname, int type, BufInfo **extern_info);


