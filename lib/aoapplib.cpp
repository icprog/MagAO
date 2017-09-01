#include "aoapplib.h"

using namespace Arcetri;

void AOAppLib::setLoggerLevel(string aoapp, string logger, int level)  throw (AOException)
{
    //@C
    //Serialize the object
    //@
    LogLevelModifier llm(logger,level);

    std::ostringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << llm; 

    std::string sbuf = oss.str();
    size_t  lungh = sbuf.size();
    const char* buffer = sbuf.c_str();

    int stat = thSendMsg(lungh, (char*) (aoapp.c_str()), AO_SETLOGLEV, DISCARD_FLAG, (void*)(buffer));
    if(IS_ERROR(stat)) {
        std::ostringstream oss;
        oss << "Error (" << lao_strerror(stat) << ") in setLoggerLevel " ;
        throw AOException(oss.str(), stat);
    }
    return;
}

NameLevelDict AOAppLib::getLoggerNamesAndLevels(string aoapp, int timeout)  throw (AOException)
{
    int stat;
    NameLevelDict ret;
    //@C
    //Send the request and wait for the reply
    //@
    stat = thSendMsg(0, (char*) (aoapp.c_str()), AO_GETLOGNAMELEV, NOHANDLE_FLAG, NULL); 
    if(IS_ERROR(stat)) {
        std::ostringstream oss;
        oss << "Error (" << lao_strerror(stat) << ") in getLoggerNamesAndLevels";
        throw AOException(oss.str(), stat);
    }
    MsgBuf *msgb=thWaitMsg(ANY_MSG, (char*) (aoapp.c_str()) ,stat,timeout,&stat);
    if(!msgb) {
        std::ostringstream oss;
        oss << "Error (" << lao_strerror(stat) << ") in getLoggerNamesAndLevels " ;
        throw AOException(oss.str(), stat);
    }
    unsigned int mcode=HDR_CODE(msgb);
    switch (mcode) {
        case ACK :
            //@C
            //Deserialize the Dict object received as a reply.
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
            throw AOException("getLoggerNamesAndLevels error ", MSG_LEN_ERROR, __FILE__, __LINE__);
            break;
        default :
            thRelease(msgb);
            throw AOException("getLoggerNamesAndLevels error ", UNEXPECTED_REPLY_ERROR, __FILE__, __LINE__);
    }
    return ret;

}

