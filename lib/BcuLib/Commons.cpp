#include "BcuLib/Commons.h"
#include "BcuLib/CommandSender.h"
#include "BcuLib/BcuMessage.h"

extern "C" {
#include "base/timelib.h"
}


bool Bcu::isBcuReady(int firstBcuId, int lastBcuId, double timeout_s, int delay_ms) {

	CommandSender* commandSender = new CommandSender();

	int wordsToRead = 10;
	BYTE readBuf[wordsToRead * Constants::DWORD_SIZE * (lastBcuId - firstBcuId + 1)];
	
	struct timeval startTime, endTime;
	double elapsedTime_s;
	gettimeofday(&startTime, NULL);
	
	Logger::get()->log(Logger::LOG_LEV_INFO, "Checking for BCUs %d-%d ready...", firstBcuId, lastBcuId);
	bool timedOut = false;
	bool bcuReady = false;
	while(!timedOut && !bcuReady) {
		
		if(commandSender == NULL) {
			commandSender = new CommandSender();
		}
		
		try {
			commandSender->sendMultiBcuCommand(Priority::LOW_PRIORITY, 
										   	   firstBcuId, lastBcuId, 
											   255, 255, 
					 						   OpCodes::MGP_OP_RDSEQ_SDRAM, 
					 						   0, 				// Dsp address
					 						   wordsToRead * Constants::DWORD_SIZE,  	// Datalen
					 						   readBuf,			// Receive buffer
					 						   100, 			// Timeout ms
					 						   Constants::WANTREPLY_FLAG);
	 						    	  
	 		Logger::get()->log(Logger::LOG_LEV_INFO, "BCUs %d-%d ready!", firstBcuId, lastBcuId);
			bcuReady = true;	
		}	
		catch(CommandSenderTimeoutException& e) {
			delete commandSender;	// See sendMultiBcuCommand(...) API for info
			commandSender = NULL;
		}    	  
		catch(CommandSenderException& e) {
			// Do nothing, simply loop again
		}
		
		// Check if timeout expired !!!
		if(!bcuReady) {
			gettimeofday(&endTime, NULL);
			elapsedTime_s = (endTime.tv_usec-startTime.tv_usec)/1E6 + (endTime.tv_sec - startTime.tv_sec);
			if(elapsedTime_s > timeout_s) {
				Logger::get()->log(Logger::LOG_LEV_WARNING, "Timeout %2.1f s expired: BCUs %d-%d still not ready", timeout_s, firstBcuId, lastBcuId);
				timedOut = true;
			}
			
			Logger::get()->log(Logger::LOG_LEV_WARNING, "...waiting %2.1f s", timeout_s-elapsedTime_s);
			msleep(delay_ms);
		}
	}
	
	delete commandSender;
	return bcuReady;
}


bool Bcu::OpCodes::isRead( int opcode) {
	
	if ((opcode == OpCodes::MGP_OP_RDSEQ_DSP) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_FLASH) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_SDRAM) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_SRAM) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_DIAGBUFF)) return true;
	
	return false;

}

bool Bcu::OpCodes::isWrite( int opcode) {
	
	if ((opcode == OpCodes::MGP_OP_RDSEQ_DSP) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_FLASH) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_SDRAM) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_SRAM) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_DIAGBUFF)) return false;
	
	return true;

} 

bool Bcu::OpCodes::isSame(int opcode) {
	
	if ((opcode == OpCodes::MGP_OP_WRSEQ_DSP) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_DSP) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_FLASH) ||
		(opcode == OpCodes::MGP_OP_WRSEQ_SDRAM) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_SDRAM) ||
		(opcode == OpCodes::MGP_OP_WRSEQ_SRAM) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_SRAM) ||
		(opcode == OpCodes::MGP_OP_WRSEQ_DIAGBUFF) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_DIAGBUFF) ||
		(opcode == OpCodes::MGP_OP_WRRD_RELAIS_BOARD)) return false;
			
	return true;
} 


bool Bcu::OpCodes::isSeq(int opcode) {
	
	if ((opcode == MGP_OP_WRSEQ_DSP) ||
		(opcode == MGP_OP_RDSEQ_DSP) ||
		(opcode == MGP_OP_RDSEQ_FLASH) ||
		(opcode == MGP_OP_WRSEQ_SDRAM) ||
		(opcode == MGP_OP_RDSEQ_SDRAM) ||
		(opcode == MGP_OP_WRSEQ_SRAM) ||
		(opcode == MGP_OP_RDSEQ_SRAM) ||
		(opcode == MGP_OP_WRSEQ_DIAGBUFF) ||
		(opcode == MGP_OP_RDSEQ_DIAGBUFF) ||
		(opcode == MGP_OP_WRRD_RELAIS_BOARD)) return true;
			
	return false;
}

int Bcu::OpCodes::getDspFactor(int opcode)  {
	
	if ((opcode == OpCodes::MGP_OP_WRSAME_DSP) ||
		(opcode == OpCodes::MGP_OP_WRSEQ_DSP) ||
		(opcode == OpCodes::MGP_OP_RDSEQ_DSP)) return 1;
	
	return 2;
}

int Bcu::OpCodes::getReadOpCode(string memType) {
	if(memType.compare("DSP") == 0) return OpCodes::MGP_OP_RDSEQ_DSP;
	else if(memType.compare("SRAM") == 0) return OpCodes::MGP_OP_RDSEQ_SRAM;
	else if(memType.compare("SDRAM") == 0) return OpCodes::MGP_OP_RDSEQ_SDRAM;
	else return MGP_OP_CMD_NULL;
}
