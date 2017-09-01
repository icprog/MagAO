#include "diagnlib.h"
#include "Logger.h"

extern "C"{
#include "base/thrdlib.h"
}

#include <sstream>
#include <string>
#include <boost/archive/binary_oarchive.hpp> //serialize
#include <boost/archive/binary_iarchive.hpp> //serialize

using namespace Arcetri;
using namespace Arcetri::Diagnostic;

int SetDiagnVarParam(
        char*               diagnapp, //@P{diagnapp}: DiagnApp's name
        const ParamDict  &  pdict,    //@P{pdict}: ParamDict containing the DiagnParam object for each DiagnVar
        int                 tmout     //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                      // If 0, wait forever. If -1, do not request acknowledgement.   
        )                             // @R: Number of items modified if tmout >= 0
{
    int stat; 

    std::ostringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << pdict; 

    std::string sbuf = oss.str();
    size_t  lungh = sbuf.size();
    const char* buffer = sbuf.c_str();

    if(tmout>=0) {           // We want to check the reply
        stat = thSendMsg(lungh, diagnapp, DIAGNSETPARAM, NOHANDLE_FLAG, (void*)(buffer)); 
        if(IS_ERROR(stat)){
            std::ostringstream oss;
            oss << "Error (" << lao_strerror(stat) << ") in SetDiagnVarParam " ;
            throw AOException(oss.str(), stat);
        }
        MsgBuf *msgb=thWaitMsg(ACK,diagnapp,stat,tmout,&stat);
        if(!msgb) {
            std::ostringstream oss;
            oss << "Error (" << lao_strerror(stat) << ") in SetDiagnVarParam " ;
            throw AOException(oss.str(), stat);  
        } 
        int count =  *((int*)(MSG_BODY(msgb)));
        thRelease(msgb);
        return count;
    } else {                // We do not want to check the reply
        stat = thSendMsg(lungh, diagnapp, DIAGNSETPARAM, DISCARD_FLAG, (void*)(buffer));
        if(IS_ERROR(stat)) {
            std::ostringstream oss;
            oss << "Error (" << lao_strerror(stat) << ") in SetDiagnVarParam " ;
            throw AOException(oss.str(), stat);
        }
        return NO_ERROR;
    }

    return 0;
}

int SetDiagnVarParam(
        char*               diagnapp,  //@P{diagnapp}: DiagnApp's name
        const DiagnWhich &  which,
        const DiagnParam &  param,     //@P{param}: Parameter object
        int                 tmout      //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                       // If 0, wait forever. If -1, do not request acknowledgement.    
        )                              // @R: Number of items modified if tmout >=0 
                                       //     NO_ERROR if tmout < 0
{
    ParamDict       wp;
    wp[which]=param; 

    return SetDiagnVarParam(diagnapp, wp, tmout);
}


//std::map<DiagnWhich, DiagnParam>  GetDiagnVarParam(
ParamDict  GetDiagnVarParam(
        char*               diagnapp,  //@P{diagnapp}: DiagnApp's name
        const DiagnWhich &  which,     //@P{which}: DiagnVar's filter.  
        int                 tmout      //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                       // If 0, wait forever.    
        )                              // @R: completion code
{
    int stat;
    ParamDict  ret;
    Logger* logger = Logger::get("DIAGNLIB");

    // TODO tutto sto casino di stream e strings si puo' migliorare? 
    //@C
    //Serialize the DiagnWhich object
    //@
    std::ostringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << which;

    // const char* buffer = oss.str().c_str();
    std::string sbuf = oss.str();
    size_t  lungh = sbuf.size();
    const char* buffer = sbuf.c_str();

    //@C
    //Send the request and wait for the reply
    //@
    stat = thSendMsg(lungh, diagnapp, DIAGNGETPARAM, NOHANDLE_FLAG, (void*)(buffer)); 
    logger->log(Logger::LOG_LEV_DEBUG, "Stat %d lungh %d [%s:%d]", stat, lungh, __FILE__, __LINE__);
    if(IS_ERROR(stat)) {
        std::ostringstream oss;
        oss << "Error (" << lao_strerror(stat) << ") in GetDiagnVarParam";
        throw AOException(oss.str(), stat);
    }
    MsgBuf *msgb=thWaitMsg(ANY_MSG,diagnapp,stat,tmout,&stat);
    if(!msgb) {
        std::ostringstream oss;
        oss << "Error (" << lao_strerror(stat) << ") in GetDiagnVarParam " ;
        throw AOException(oss.str(), stat);
    }
    unsigned int mcode=HDR_CODE(msgb);
    switch (mcode) {
        case ACK :
            //@C
            //Deserialize the ParamDict object received as a reply.
            //@
            {
            unsigned int len = HDR_LEN(msgb);
            std::istringstream iss( std::string( (const char *)(MSG_BODY(msgb)), len));
            boost::archive::binary_iarchive ia(iss);
            ia >> ret;
            thRelease(msgb);
            break;
            }
        case NAK :
            thRelease(msgb);
            throw AOException("GetDiagnParam error ", MSG_LEN_ERROR, __FILE__, __LINE__);
            break;
        default :
            thRelease(msgb);
            throw AOException("GetDiagnParam error ", UNEXPECTED_REPLY_ERROR, __FILE__, __LINE__);
    }
    return ret;
;
}


ValueDict  GetDiagnValue(
        char*               diagnapp,  //@P{diagnapp}: DiagnApp's name
        const DiagnWhich &  which,     //@P{which}: DiagnVar's filter.  
        int                 tmout      //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                       // If 0, wait forever.    
        )                              // @R: completion code
{
    int stat;
    ValueDict  ret;

    // TODO tutto sto casino di stream e strings si puo' migliorare? 
    //@C
    //Serialize the DiagnWhich object
    //@
    std::ostringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << which;

    // const char* buffer = oss.str().c_str();
    std::string sbuf = oss.str();
    size_t  lungh = sbuf.size();
    const char* buffer = sbuf.c_str();

    //@C
    //Send the request and wait for the reply
    //@
    stat = thSendMsg(lungh, diagnapp, DIAGNGETVALUE, NOHANDLE_FLAG, (void*)(buffer)); 
    //printf("[GetDiagnValue] Stat %d lungh %d Content: ", stat, lungh);
    //for (int i =0; i<lungh; i++) printf("%c",buffer[i]); printf("\n");
    if(IS_ERROR(stat)) {
        std::ostringstream oss;
        oss << "Error (" << lao_strerror(stat) << ") in GetDiagnValue [1]";
        throw AOException(oss.str(), stat);
    }
    MsgBuf *msgb=thWaitMsg(ANY_MSG,diagnapp,stat,tmout,&stat);
    if(!msgb) {
        std::ostringstream oss;
        oss << "Error (" << lao_strerror(stat) << ") in GetDiagnValue [2]" ;
        throw AOException(oss.str(), stat);
    }
    unsigned int mcode=HDR_CODE(msgb);
    switch (mcode) {
        case ACK :
           //@C
            //Deserialize the ParamValue object received as a reply.
            //@
            {
            unsigned int len = HDR_LEN(msgb);
            std::istringstream iss( std::string( (const char *)(MSG_BODY(msgb)), len));
            boost::archive::binary_iarchive ia(iss);
            ia >> ret;
            thRelease(msgb);
            break;
            }
        case NAK :
            thRelease(msgb);
            throw AOException("GetDiagnValue error [3]", MSG_LEN_ERROR);
            break;
        
        default :
            thRelease(msgb);
            throw AOException("GetDiagnValue error [4]", UNEXPECTED_REPLY_ERROR);
    }
    return ret;
}

//TODO is identical to GetDiagnValue: template?

BufferDict  GetDiagnBuffer(
        char*               diagnapp,  //@P{diagnapp}: DiagnApp's name
        const DiagnWhich &  which,     //@P{which}: DiagnVar's filter.  
        int                 tmout      //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                       // If 0, wait forever.    
        )                              // @R: completion code
{
    int stat;
    BufferDict  ret;

    // TODO tutto sto casino di stream e strings si puo' migliorare? 
    //@C
    //Serialize the DiagnWhich object
    //@
    std::ostringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << which;

    // const char* buffer = oss.str().c_str();
    std::string sbuf = oss.str();
    size_t  lungh = sbuf.size();
    const char* buffer = sbuf.c_str();

    //@C
    //Send the request and wait for the reply
    //@
    stat = thSendMsg(lungh, diagnapp, DIAGNGETBUFFER, NOHANDLE_FLAG, (void*)(buffer)); 
    //printf("[GetDiagnBuffer] Stat %d lungh %d Content: ", stat, lungh);
    //for (int i =0; i<lungh; i++) printf("%c",buffer[i]); printf("\n");
    if(IS_ERROR(stat)) {
        std::ostringstream oss;
        oss << "Error (" << lao_strerror(stat) << ") in GetDiagnBuffer";
        throw AOException(oss.str(), stat);
    }
    MsgBuf *msgb=thWaitMsg(ANY_MSG,diagnapp,stat,tmout,&stat);
    if(!msgb) {
        std::ostringstream oss;
        oss << "Error (" << lao_strerror(stat) << ") in GetDiagnBuffer " ;
        throw AOException(oss.str(), stat);
    }
    unsigned int mcode=HDR_CODE(msgb);
    switch (mcode) {
        case ACK :
            //@C
            //Deserialize the ParamBuffer object received as a reply.
            //@
            {
            unsigned int len = HDR_LEN(msgb);
            //std::cout << "DIAGNGETBUFFER arrived from " << HDR_FROM(msgb) << " len " << len << " seqnum " << HDR_SEQN(msgb) << std::endl;   
            std::istringstream iss( std::string( (const char *)(MSG_BODY(msgb)), len));
            boost::archive::binary_iarchive ia(iss);
            ia >> ret;
            thRelease(msgb);
            break;
            }
        case NAK :
            thRelease(msgb);
            throw AOException("GetDiagnBuffer error ", MSG_LEN_ERROR, __FILE__, __LINE__);
            break;
        
        default :
            thRelease(msgb);
            throw AOException("GetDiagnBuffer error ", UNEXPECTED_REPLY_ERROR, __FILE__, __LINE__);
    }
    return ret;
}



#include <boost/serialization/string.hpp>
int DumpDiagnVarParams(
        char*               diagnapp,  //@P{diagnapp}: DiagnApp's name
        char*               filename,  //@P{filename}: filename where the params are dumped.
        int                 tmout      //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                       // If 0, wait forever. If -1, do not request acknowledgement.    
        )                              // @R: Number of items modified if tmout >=0 
                                       //     NO_ERROR if tmout < 0
{
    int stat;
	string strfilename = string(filename);

    // TODO tutto sto casino di stream e strings si puo' migliorare? 
    //@C
    //Serialize the ParamDict object
    //@

    std::ostringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << strfilename; 

    std::string sbuf = oss.str();
    size_t  lungh = sbuf.size();
    const char* buffer = sbuf.c_str();

    if(tmout>=0) {           // We want to check the reply
        stat = thSendMsg(lungh, diagnapp, DIAGNDUMPPARAMS, NOHANDLE_FLAG, (void*)(buffer)); 
        if(IS_ERROR(stat)){
            std::ostringstream oss;
            oss << "Error (" << lao_strerror(stat) << ") in DumpDiagnVarParams " ;
            throw AOException(oss.str(), stat);
        }
        MsgBuf *msgb=thWaitMsg(ACK,diagnapp,stat,tmout,&stat);
        if(!msgb) {
            std::ostringstream oss;
            oss << "Error (" << lao_strerror(stat) << ") in DumpDiagnVarParams " ;
            throw AOException(oss.str(), stat);  
        } 
        int count =  *((int*)(MSG_BODY(msgb)));
        thRelease(msgb);
        return count;
    } else {                // We do not want to check the reply
        stat = thSendMsg(lungh, diagnapp, DIAGNDUMPPARAMS, DISCARD_FLAG, (void*)(buffer));
        if(IS_ERROR(stat)) {
            std::ostringstream oss;
            oss << "Error (" << lao_strerror(stat) << ") in DumpDiagnVarParams " ;
            throw AOException(oss.str(), stat);
        }
        return NO_ERROR;
    }
}

//
//
int LoadDiagnVarParams(
        char*               diagnapp,  //@P{diagnapp}: DiagnApp's name
        char*               filename,  //@P{filename}: filename where the params are dumped.
        int                 tmout      //@P{tmout}: Timeout in milliseconds, to wait for acknowledgement. 
                                       // If 0, wait forever. If -1, do not request acknowledgement.    
        )                              // @R: Number of items modified if tmout >=0 
                                       //     NO_ERROR if tmout < 0
{
    int stat = NO_ERROR;

	string strfilename = string(filename);

    std::ostringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << strfilename; 

    std::string sbuf = oss.str();
    size_t  lungh = sbuf.size();
    const char* buffer = sbuf.c_str();

    int flag = (tmout>=0) ? NOHANDLE_FLAG : DISCARD_FLAG;
    stat = thSendMsg(lungh, diagnapp, DIAGNLOADPARAMS, flag, (void*)(buffer)); 
    if(IS_ERROR(stat)){
        std::ostringstream oss;
        oss << "Error (" << lao_strerror(stat) << ") in DumpDiagnVarParams " ;
        throw AOException(oss.str(), stat);
    }
    if (tmout>=0) { 
      MsgBuf *msgb=thWaitMsg(ACK,diagnapp,stat,tmout,&stat);
      if(!msgb) {
         std::ostringstream oss;
         oss << "Error (" << lao_strerror(stat) << ") in DumpDiagnVarParams " ;
         throw AOException(oss.str(), stat);  
      }
      stat =  *((int*)(MSG_BODY(msgb)));
      thRelease(msgb);
    } 
    return stat;
}

int LoadOfflineDiagnFrame(char* diagnapp,  //@P{diagnapp}: DiagnApp's name
						   uint32 frameI) 	//@P{frameI}: number of the frame to load
{
	int stat;

    stat = thSendMsgPl(0, diagnapp, DIAGNLOADOFFLINEFRAME, frameI,  DISCARD_FLAG, NULL);
    if(IS_ERROR(stat)) {
        std::ostringstream oss;
        oss << "Error (" << lao_strerror(stat) << ") in LoadOfflineDiagnFrame " ;
        throw AOException(oss.str(), stat);
    }
    return NO_ERROR;
}


