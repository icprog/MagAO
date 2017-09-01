//@File: DiagnApp.cpp
//
// \diagnapp\ class definition
//
// This file contains the definition of \diagnapp\ class
//@

#include <sys/resource.h>  // set/getpriority
#include <boost/functional/hash.hpp>

extern "C" {
    #include "base/thrdlib.h"
}

#include "DiagnApp.h"
#include "DiagnApp_convert.h"
#include "Utils.h"

using namespace Arcetri;

bool DiagnApp::_offline;

//@Member: DiagnApp
//
// Constructor
//@

DiagnApp::DiagnApp( int argc, char **argv)
    : AOApp(argc, argv)
{
    Constructor();

    // Cerca flag "offline" e nomefile (-o[ffline] nomefile)
    char c;
    optind = 0;
    opterr = 0;
    while ((c = getopt (argc, argv, "o:")) != -1) {
        switch(c) {
            case 'o':
                _offline = true;
                _framesHistory = new IFramesHistory(optarg);
                _logger->log(Logger::LOG_LEV_INFO, "Offline mode active (history file: %s)", optarg);
                break;

            case '?':
                break;
        }
    }
}


void DiagnApp::Constructor()
{
    _frame_timestamp = 0;
    _slow_counter = 0;
    _fast_counter = 0;
    _slow_rate = 10;
    _fast_rate = 10;    // just an initial guess. don't care
    _frame_rate = 10;   // just an initial guess. don't care
    //@C
    // Initialize vars used for threading
    //@
    _fast_ready = true;
    _slow_ready = true;
    pthread_cond_init(&_cond_fast, NULL);
    pthread_cond_init(&_cond_slow, NULL);
    pthread_mutex_init(&_mut_fast, NULL);
    pthread_mutex_init(&_mut_slow, NULL);
    //pthread_mutex_init(&_mut_slowvars, NULL);
    pthread_mutex_init(&_mut_fastvars, NULL);

    // Frames history info
    _offline = false;
    _framesHistory = NULL;

    _telemetry = NULL;
}

//@Member: $\sim$ DiagnApp
//
//Standard Destructor
//
//Dump a paramcfgfile containing the diagnvars parameters
//@
DiagnApp::~DiagnApp()
{
   SetTimeToDie(true);     // Tell other threads to die in case this function is called from an exception
    DumpDiagnVarParam();
}


//@Member:UpdateFilteredIterators
//
// Create filters for iterating over subsets of variables
//@
void DiagnApp::UpdateFilteredIterators()
{
    //pthread_mutex_lock(&_mut_slowvars);
    //_logger->log(Logger::LOG_LEV_TRACE, "%s [updatefilterediterators] _mut_slowvars locked: update _slowvars filter [%s:%d]",
    //                       log_time().c_str(), __FILE__, __LINE__);
    //_slowvars    = _vars.Filter(DiagnWhich("*",DiagnWhich::All,DiagnWhich::All,DiagnWhich::Slow,DiagnWhich::Enabled));
    //pthread_mutex_unlock(&_mut_slowvars);
    //_logger->log(Logger::LOG_LEV_TRACE, "%s [updatefilterediterators] _mut_slowvars unlocked: _slowvars filter updated [%s:%d]",
    //        log_time().c_str(), __FILE__, __LINE__);

    // wait the good moment to update iterator of fast variables
    //pthread_mutex_lock(&_mut);
    //_logger->log(Logger::LOG_LEV_TRACE, "%s [updatefilterediterators] mutex locked [%s:%d]",log_time().c_str(), __FILE__, __LINE__);
    //while (_fast_ready == false){
    //    _logger->log(Logger::LOG_LEV_TRACE, "%s [updatefilterediterators] cond_wait [%s:%d]",log_time().c_str(),__FILE__, __LINE__);
    //    pthread_cond_wait(&_cond, &_mut);
    //}

    // Can modify the iterator for the fast loop
    pthread_mutex_lock(&_mut_fastvars);
    _logger->log(Logger::LOG_LEV_TRACE, "[updatefilterediterators] _mut_fastvars locked: update _fastvars filter [%s:%d]", __FILE__, __LINE__);
    _fastvars    = _vars.Filter(DiagnWhich("*",DiagnWhich::All,DiagnWhich::All,DiagnWhich::Fast,DiagnWhich::Enabled));
    pthread_mutex_unlock(&_mut_fastvars);
    _logger->log(Logger::LOG_LEV_TRACE, "[updatefilterediterators] _mut_fastvars unlocked: _fastvars filter updated [%s:%d]", __FILE__, __LINE__);

}

//@Member: InstallHandlers
//
// Install handlers for messages directed to a generic \diagnapp\ .
//
//
// The following messages are handled:
// \begin{itemize}
// \item DIAGNDUMP
// \item DIAGNDUMPPARAMS
// \item DIAGNSETPARAM
// \item DIAGNGETPARAM
// \item DIAGNGETVALUE
// \item DIAGNGETBUFFER
// \end{itemize}
//@
void DiagnApp::InstallHandlers()
{
    int stat;
    _logger->log(Logger::LOG_LEV_INFO, "Installing notification handler for DIAGNDUMP ...");
    if((stat=thHandler(DIAGNDUMP, "*", 0, diagndump_handler, "diagndump", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error in installing notification handler for DIAGNDUMP: %s [%s:%d]",
                lao_strerror(stat),__FILE__, __LINE__);
        throw AOException("thHandler error in installing notification handler for DIAGNDUMP", stat,__FILE__, __LINE__);
    }
    _logger->log(Logger::LOG_LEV_INFO, "Installing notification handler for DIAGNDUMPPARAMS ...");
    if((stat=thHandler(DIAGNDUMPPARAMS, "*", 0, diagndumpparams_handler, "diagndumpparams", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error in installing notification handler for DIAGNDUMPPARAMS: %s [%s:%d]",
                lao_strerror(stat),__FILE__, __LINE__);
        throw AOException("thHandler error in installing notification handler for DIAGNDUMPPARAMS", stat,__FILE__, __LINE__);
    }
    _logger->log(Logger::LOG_LEV_INFO, "Installing notification handler for DIAGNSETPARAM ...");
    if((stat=thHandler(DIAGNSETPARAM, "*", 0, diagnsetparam_handler, "diagnsetparam", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error in installing notification handler for DIAGNSETPARAM: %s [%s:%d]",
                lao_strerror(stat),__FILE__, __LINE__);
        throw AOException("thHandler error in installing notification handler for DIAGNSETPARAM", stat,__FILE__, __LINE__);
    }
    _logger->log(Logger::LOG_LEV_INFO, "Installing notification handler for DIAGNGETPARAM ...");
    if((stat=thHandler(DIAGNGETPARAM, "*", 0, diagngetparam_handler, "diagngetparam", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error in installing notification handler for DIAGNGETPARAM: %s [%s:%d]",
                lao_strerror(stat),__FILE__, __LINE__);
        throw AOException("thHandler error in installing notification handler for DIAGNGETPARAM", stat,__FILE__, __LINE__);
    }
    _logger->log(Logger::LOG_LEV_INFO, "Installing notification handler for DIAGNGETVALUE ...");
    if((stat=thHandler(DIAGNGETVALUE, "*", 0, diagngetvalue_handler, "diagngetvalue", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error in installing notification handler for DIAGNGETVALUE: %s [%s:%d]",
                lao_strerror(stat),__FILE__, __LINE__);
        throw AOException("thHandler error in installing notification handler for DIAGNGETVALUE", stat,__FILE__, __LINE__);
    }
    _logger->log(Logger::LOG_LEV_INFO, "Installing notification handler for DIAGNGETBUFFER ...");
    if((stat=thHandler(DIAGNGETBUFFER, "*", 0, diagngetbuffer_handler, "diagngetbuffer", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error in installing notification handler for DIAGNGETBUFFER: %s [%s:%d]",
                lao_strerror(stat),__FILE__, __LINE__);
        throw AOException("thHandler error in installing notification handler for DIAGNGETBUFFER", stat,__FILE__, __LINE__);
    }
    _logger->log(Logger::LOG_LEV_INFO, "Installing notification handler for DIAGNLOADPARAMS ...");
    if((stat=thHandler(DIAGNLOADPARAMS, "*", 0, diagnloadparams_handler, "diagnloadparams", this))<0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error in installing notification handler for DIAGNLOADPARAMS: %s [%s:%d]",
                lao_strerror(stat),__FILE__, __LINE__);
        throw AOException("thHandler error in installing notification handler for DIAGNLOADPARAMS", stat,__FILE__, __LINE__);
    }
    //Registrare handler for message DIAGNLOADOFFLINEFRAME
    if(_offline) {
        _logger->log(Logger::LOG_LEV_INFO, "Installing notification handler for DIAGNGETBUFFER ...");
        if((stat=thHandler(DIAGNLOADOFFLINEFRAME, "*", 0, diagnloadofflineframe_handler, "diagnloadofflineframe", this))<0) {
            _logger->log(Logger::LOG_LEV_FATAL, "Error in installing notification handler for DIAGNLOADOFFLINEFRAME: %s [%s:%d]",
                    lao_strerror(stat),__FILE__, __LINE__);
            throw AOException("thHandler error in installing notification handler for DIAGNLOADOFFLINEFRAME", stat,__FILE__, __LINE__);
        }
    }
}

//@Function: diagndump_handler
//
// handler of a DIAGNDUMP command. Serialize data in a
// file called YYMMDDhhmss.dump
//@

#include <ctime>
#include <fstream>
int DiagnApp::diagndump_handler(MsgBuf *msgb, void *argp, int /*hndlrQueueSize*/)
{
    DiagnApp *pt = (DiagnApp*)argp;
    char outstr[200];
    time_t t;
    struct tm *tmp;

    t = time(NULL);
    tmp = localtime(&t);
    strftime(outstr, sizeof(outstr), "%y%m%d%H%M%S.dump", tmp);

    // make an archive
    std::ofstream ofs(outstr);
    boost::archive::binary_oarchive oa(ofs);
    oa << pt->_vars;

    thRelease(msgb);
    return NO_ERROR;
}



//@Function: diagndumpparams_handler
//
// handler of a DIAGNDUMPPARAMS command.
//
//@
#include <boost/archive/binary_iarchive.hpp>
#include <sstream>
#include <string>
int DiagnApp::diagndumpparams_handler(MsgBuf *msg, void *argp, int /*hndlrQueueSize*/)
{
    string         dumpfile;
    DiagnApp *pt = (DiagnApp*)argp;

    Logger* logger = Logger::get("DIAGNDUMPPARAMS");

    logger->log(Logger::LOG_LEV_INFO, "received DIAGNDUMPPARAMS message [%s:%d]", __FILE__, __LINE__);

    unsigned int len = HDR_LEN(msg);

    std::string sbuf( (const char *)(MSG_BODY(msg)), len);
    std::istringstream iss(sbuf);
    boost::archive::binary_iarchive ia(iss);
    ia >> dumpfile;
    bool ret = pt->DumpDiagnVarParam(dumpfile);
    if (ret==true)
        logger->log(Logger::LOG_LEV_INFO, "written dumpfile %s [%s:%d]", dumpfile.c_str(), __FILE__, __LINE__);
    else
        logger->log(Logger::LOG_LEV_WARNING, "dumpfile %s was NOT written [%s:%d]", dumpfile.c_str(), __FILE__, __LINE__);

    //@C
    // Acknowledge
    //@
    int stat = thReplyMsg(ACK, sizeof(ret), &ret, msg);
    if(IS_ERROR(stat)){
        logger->log(Logger::LOG_LEV_ERROR, "Error in DiagnApp::diagndumpparams_handler(): %s [%s:%d]",lao_strerror(stat), __FILE__, __LINE__);
        return stat;
        //Dont throw any exception and simply go on //throw AOException("DiagnApp::diagndumpparams_handler()", stat, __FILE__, __LINE__);
    }


    return NO_ERROR;
}


//@Function: diagnloadparams_handler
//
// handler of a DIAGNLOADPARAMS command.
//
//@
int DiagnApp::diagnloadparams_handler(MsgBuf *msg, void *argp, int /*hndlrQueueSize*/)
{
    string configfile;
    int stat;
    DiagnApp *pt = (DiagnApp*)argp;

    Logger* logger = Logger::get("DIAGNLOADPARAMS");
    logger->log(Logger::LOG_LEV_INFO, "received DIAGNLOADPARAMS message [%s:%d]", __FILE__, __LINE__);

    unsigned int len = HDR_LEN(msg);

    std::string sbuf( (const char *)(MSG_BODY(msg)), len);
    std::istringstream iss(sbuf);
    boost::archive::binary_iarchive ia(iss);
    ia >> configfile;

    try {
      pt->UpdateDiagnParam(configfile);
    } catch (Config_File_Exception &e) {
      logger->log(Logger::LOG_LEV_ERROR, "Error in DiagnApp::UpdateDiagnParams(): %s [%s:%d]", e.what().c_str(), __FILE__, __LINE__);
      stat = FILE_ERROR;
    }

    //@C
    // Acknowledge
    //@
    stat = thReplyMsg(ACK, sizeof(stat), &stat, msg);
    if(IS_ERROR(stat)){
        logger->log(Logger::LOG_LEV_ERROR, "Error in DiagnApp::diagnloadparams_handler(): %s [%s:%d]",lao_strerror(stat), __FILE__, __LINE__);
        return stat;
    }

    return NO_ERROR;
}


//@Function: diagnsetparam_handler
//
// handler of a DIAGNSETPARAM command.
//
//@
#include <boost/archive/binary_iarchive.hpp>
#include <sstream>
#include <string>
int DiagnApp::diagnsetparam_handler(MsgBuf *msg, void *argp, int hndlrQueueSize)
{
    ParamDict      pd;
    DiagnWhich     wh;
    DiagnParam     pa;
    DiagnApp *pt = (DiagnApp*)argp;

    static std::vector<WhichParam> v_wp;
    const unsigned int PROCESS_QUEUE = 100;
    int count=0, stat;

    Logger* logger = Logger::get("DIAGNSETPARAM");
    logger->log(Logger::LOG_LEV_INFO, "received SETPARAM message [%s:%d]", __FILE__, __LINE__);

    unsigned int len = HDR_LEN(msg);

    std::string sbuf( (const char *)(MSG_BODY(msg)), len);
    std::istringstream iss(sbuf);
    boost::archive::binary_iarchive ia(iss);
    ia >> pd;

    ParamDict::iterator it;
    WhichParam wp;

    for (it = pd.begin(); it != pd.end(); it++) {
        wp.wh = it->first;
        wp.pa = it->second;
        v_wp.push_back(wp);
    }

    logger->log(Logger::LOG_LEV_INFO, "diagnsetparam_handler(): queue is %d, handler queue %d [%s:%d]", v_wp.size(), hndlrQueueSize, __FILE__, __LINE__);

    // Coda piena oppure fine messaggi
    if ((v_wp.size() >= PROCESS_QUEUE) || (hndlrQueueSize==0)) {
      logger->log(Logger::LOG_LEV_INFO, "diagnsetparam_handler(): processing queue of %d messages [%s:%d]", v_wp.size(), __FILE__, __LINE__);
      count = pt->SetDiagnVarParam(v_wp);
      v_wp.clear();
    }

    logger->log(Logger::LOG_LEV_INFO, "deserialize SETPARAM message: %s ---> %s [%d vars updated][%s:%d]",
          wp.wh.str().c_str(), wp.pa.str().c_str(),count, __FILE__, __LINE__);


    //@C
    // Acknowledge
    //@
    stat = thReplyMsg(ACK, sizeof(count), &count, msg);
    if(IS_ERROR(stat)){
        logger->log(Logger::LOG_LEV_ERROR, "Error in DiagnApp::diagnsetparam_handler(): %s [%s:%d]",lao_strerror(stat), __FILE__, __LINE__);
        return stat;
        //Dont throw any exception and simply go on //throw AOException("DiagnApp::diagnsetparam_handler()", stat, __FILE__, __LINE__);
    }


    return NO_ERROR;
}

//@Function: diagngetparam_handler
//
// handler of a DIAGNGETPARAM command.
//
//@
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <sstream>
#include <string>
int DiagnApp::diagngetparam_handler(MsgBuf *msg, void *argp, int /*hndlrQueueSize*/)
{
    Timing t,t1;
    DiagnWhich  wh;
    ParamDict   mappa;
    DiagnApp *pt = (DiagnApp*)argp;

    Logger* logger = Logger::get("DIAGNGETPARAM");
    t.start();
    //@C
    // Deserialize request containing a DiagnWhich object
    //@
    unsigned int len = HDR_LEN(msg);
    logger->log(Logger::LOG_LEV_DEBUG, "received GETPARAM message from %s [%s:%d]",HDR_FROM(msg), __FILE__, __LINE__);
    t1.start();
    std::string inbuf( (const char *)(MSG_BODY(msg)), len);
    std::istringstream iss( inbuf);
    boost::archive::binary_iarchive ia(iss);
    ia >> wh;
    t1.stop();
    logger->log(Logger::LOG_LEV_DEBUG, "GETPARAM parameters deserialized in %g s [%s:%d]", t1.sec(), __FILE__, __LINE__);

    logger->log(Logger::LOG_LEV_DEBUG, "GETPARAM DiagnWhich is %s [%s:%d]",wh.str().c_str(), __FILE__, __LINE__);

    //@C
    // Get dictionary, serialize it and reply
    //@

    t1.start();
    mappa = pt->GetDiagnVarParam(wh);
    t1.stop();
    logger->log(Logger::LOG_LEV_DEBUG, "GETPARAM got mappa in %g s [%s:%d]", t1.sec(), __FILE__, __LINE__ );

    t1.start();
    std::ostringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << mappa;

    std::string outbuf = oss.str();
    int  lungh = outbuf.size();
    const char* buffer = outbuf.c_str();
    t1.stop();
    logger->log(Logger::LOG_LEV_DEBUG, "GETPARAM reply serialized in %g s [%s:%d]", t1.sec(), __FILE__, __LINE__ );

    logger->log(Logger::LOG_LEV_DEBUG, "GETPARAM reply to %s (len %d) [%s:%d]",HDR_FROM(msg), lungh, __FILE__, __LINE__ );

    int stat = thReplyMsg(ACK, lungh, (void*)(buffer), msg);
    if(IS_ERROR(stat)){
        logger->log(Logger::LOG_LEV_ERROR, "Error in DiagnApp::diagngetparam_handler(): %s [%s:%d]",lao_strerror(stat), __FILE__, __LINE__);
        return stat;
        //Dont throw any exception and simply go on //throw AOException("DiagnApp::diagngetparam_handler()", stat, __FILE__, __LINE__);
    }
    t.stop();
    logger->log(Logger::LOG_LEV_DEBUG, "GETPARAM all done in %g s [%s:%d]", t.sec(), __FILE__, __LINE__ );
    return NO_ERROR;
}

//@Function: diagngetvalue_handler
//
// handler of a DIAGNGETVALUE command.
//
//@
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <sstream>
#include <string>
int DiagnApp::diagngetvalue_handler(MsgBuf *msg, void *argp, int /*hndlrQueueSize*/)
{
    Timing t,t1;
    DiagnWhich  wh;
    ValueDict   mappa;
    DiagnApp *pt = (DiagnApp*)argp;

    Logger* logger = Logger::get("DIAGNGETVALUE");
    t.start();
    //@C
    // Deserialize request containing a DiagnWhich object
    //@
    unsigned int len = HDR_LEN(msg);
    logger->log(Logger::LOG_LEV_DEBUG, "received GETVALUE message from %s [%s:%d]",HDR_FROM(msg),__FILE__, __LINE__);
    t1.start();
    std::string inbuf( (const char *)(MSG_BODY(msg)), len);
    std::istringstream iss( inbuf);
    boost::archive::binary_iarchive ia(iss);
    ia >> wh;
    t1.stop();
    logger->log(Logger::LOG_LEV_DEBUG, "GETVALUE parameters deserialized in %g s [%s:%d]", t1.sec(),__FILE__, __LINE__);

    logger->log(Logger::LOG_LEV_DEBUG, "GETVALUE DiagnWhich is %s [%s:%d]",wh.str().c_str(),__FILE__, __LINE__);

    //@C
    // Get dictionary, serialize it and reply
    //@

    t1.start();
    mappa = pt->GetDiagnValue(wh);
    t1.stop();
    logger->log(Logger::LOG_LEV_DEBUG, "GETVALUE got mappa in %g s [%s:%d]", t1.sec(),__FILE__, __LINE__ );

    t1.start();
    std::ostringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << mappa;

    std::string outbuf = oss.str();
    int  lungh = outbuf.size();
    const char* buffer = outbuf.c_str();
    t1.stop();
    logger->log(Logger::LOG_LEV_DEBUG, "GETVALUE reply serialized in %g s [%s:%d]", t1.sec(),__FILE__, __LINE__ );

    logger->log(Logger::LOG_LEV_DEBUG, "GETVALUE reply to %s (len %d) [%s:%d]",HDR_FROM(msg), lungh,__FILE__, __LINE__ );
    int stat = thReplyMsg(ACK, lungh, (void*)(buffer), msg);
    if(IS_ERROR(stat)){
        logger->log(Logger::LOG_LEV_ERROR, "Error in DiagnApp::diagngetvalue_handler(): %s [%s:%d]",lao_strerror(stat),__FILE__, __LINE__);
        return stat;
        //Dont throw any exception and simply go on //throw AOException("DiagnApp::diagngetvalue_handler()", stat, __FILE__, __LINE__);
    }
    t.stop();
    logger->log(Logger::LOG_LEV_DEBUG, "GETVALUE all done in %g s [%s:%d]", t.sec(),__FILE__, __LINE__ );
    return NO_ERROR;
}

//@Function: diagngetbuffer_handler
//
// handler of a DIAGNGETBUFFER command.
//
// TODO this one is identical to diagngetvalue_handler: template it?
//@
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <sstream>
#include <string>
int DiagnApp::diagngetbuffer_handler(MsgBuf *msg, void *argp, int /*hndlrQueueSize*/)
{
    Timing t,t1;
    DiagnWhich  wh;
    BufferDict   mappa;
    DiagnApp *pt = (DiagnApp*)argp;

    Logger* logger = Logger::get("DIAGNGETBUFFER");
    t.start();
    //@C
    // Deserialize request containing a DiagnWhich object
    //@
    unsigned int len = HDR_LEN(msg);
    logger->log(Logger::LOG_LEV_DEBUG, "received GETBUFFER message from %s [%s:%d]",HDR_FROM(msg),__FILE__, __LINE__);
    t1.start();
    std::string inbuf( (const char *)(MSG_BODY(msg)), len);
    std::istringstream iss( inbuf);
    boost::archive::binary_iarchive ia(iss);
    ia >> wh;
    t1.stop();
    logger->log(Logger::LOG_LEV_DEBUG, "GETBUFFER parameters deserialized in %g s [%s:%d]", t1.sec(),__FILE__, __LINE__);

    logger->log(Logger::LOG_LEV_DEBUG, "GETBUFFER DiagnWhich is %s [%s:%d]",wh.str().c_str(),__FILE__, __LINE__);

    //@C
    // Get dictionary, serialize it and reply
    //@

    t1.start();
    mappa = pt->GetDiagnBuffer(wh);
    t1.stop();
    logger->log(Logger::LOG_LEV_DEBUG, "GETBUFFER got mappa in %g s [%s:%d]", t1.sec(),__FILE__, __LINE__ );

    t1.start();
    std::ostringstream oss;
    boost::archive::binary_oarchive oa(oss);
    oa << mappa;

    std::string outbuf = oss.str();
    int  lungh = outbuf.size();
    const char* buffer = outbuf.c_str();
    t1.stop();
    logger->log(Logger::LOG_LEV_DEBUG, "GETBUFFER reply to %s serialized in %g s (len %d) [%s:%d]",
            HDR_FROM(msg), t1.sec(), lungh,__FILE__, __LINE__ );

    if (lungh >  MAX_MSG_LEN) {
        logger->log(Logger::LOG_LEV_INFO, "GETBUFFER message too long [%s:%d]",__FILE__, __LINE__ );
        int stat = thReplyMsg(NAK, 0, NULL, msg);
        if(IS_ERROR(stat))
            logger->log(Logger::LOG_LEV_ERROR, "Error in DiagnApp::diagngetbuffer_handler(): %s [%s:%d]",lao_strerror(stat),__FILE__, __LINE__);
    } else {
        int stat = thReplyMsg(ACK, lungh, (void*)(buffer), msg);
        if(IS_ERROR(stat))
            logger->log(Logger::LOG_LEV_ERROR, "Error in DiagnApp::diagngetbuffer_handler(): %s [%s:%d]",lao_strerror(stat),__FILE__, __LINE__);
            //Dont throw any exception and simply go on //throw AOException("DiagnApp::diagngetbuffer_handler()", stat, __FILE__, __LINE__);
    }
    t.stop();
    logger->log(Logger::LOG_LEV_DEBUG, "GETBUFFER all done in %g s [%s:%d]", t.sec(),__FILE__, __LINE__ );
    return NO_ERROR;
}


int DiagnApp::diagnloadofflineframe_handler(MsgBuf *msg, void *argp, int /*hndlrQueueSize*/)
{
    Logger* logger = Logger::get("DIAGNLOADOFFLINEFRAME");
    logger->log(Logger::LOG_LEV_DEBUG, "received LOADOFFLINEFRAME message from %s [%s:%d]",HDR_FROM(msg),__FILE__, __LINE__);

    if(_offline) {
		DiagnApp *pt = (DiagnApp*)argp;
		int frameIndex = HDR_PLOAD(msg);
		logger->log(Logger::LOG_LEV_DEBUG, "requested to load offline frame number %d...", frameIndex);
        if (pt->Loadofflineframe(frameIndex)) {
            return NO_ERROR;
        } return VALUE_OUT_OF_RANGE_ERROR;
    }
    else {
        logger->log(Logger::LOG_LEV_WARNING, "offline mode not enabled (start DiagnApp with '-o filename' option)");
        return UNEXPECTED_MSG_ERROR;
    }
}


//@Member: SetupVars
//
//@
void DiagnApp::SetupVars()
{
    try{
        _VarUnprotected = RTDBvar( MyFullName(),  "UNPROTECTED", NO_DIR, INT_VARIABLE, 1);
        _VarUnprotected.Set(true);
    } catch(AOException &e){ std::cerr << e.what() << std::endl; throw(e); }
}


//@Member:Run
//
// Run Diagnostic control loop
//
// CreateDiagnVars
// CreateFilters
// Start slow Thread
// Set RT scheduling
// Loop
// Restore scheduling
//@

void DiagnApp::Run()
{
    pthread_attr_t attribute;
    pthread_t slowthread_id;

 try{
    //@C
    //Create locally defined diagnostic variables and populate \verb+_vars+.
    //(timestamp, framerate, ...)
    //@
    CreateGlobalDiagnVars();

    //@C
    //Create application dependent diagnostic variables and populate \verb+_vars+
    //@
    CreateDiagnVars();

    //@C
    //Generate txt file describing the set of defined diagnostic variables
    //@
    CreateDiagnVarsDescriptionFile();

    //@C
    //Parse \verb+_param_cfg_file+ and update the parameters of the diagnostic variables
    //@
    UpdateDiagnParam();

    //@C
    //Create filters for iterating over "fast" and "slow" variables
    //@
    UpdateFilteredIterators();

    //@C
    // Parse the telemetry_cfg_file (if any) and create the DiagnTelemetry object.
    //@
    try {
        string teleconffile = (string)(ConfigDictionary()["TelemetryCfgFile"]);
        string telelogfile  = (string)(ConfigDictionary()["TelemetryLogFile"]);
        _telemetry    = new DiagnTelemetry(teleconffile, telelogfile, _fastvars, "TELEMETRY", Logger::LOG_LEV_WARNING);
    } catch (...) {
        _logger->log(Logger::LOG_LEV_WARNING, "TelemetryCfgFile / TelemetryLogFile are not specified in conf file. Telemetry disabled  [%s:%d]", __FILE__, __LINE__);
    }

    //@C
    //Compute frequencies (\verb+_fast_rate+ and \verb+_frame_rate+)
    //every \verb+update_freq+ cycles.
    //It is dynamically modified to keep an approximate interval
    //between updates of 1s.
    //@
    unsigned int update_freq=10;

    unsigned int fast_counter_old=0;
    unsigned int frame_counter_old=0;


    //@C
    // Spawn ``slow'' thread
    //@
    int ret;
    pthread_attr_init(&attribute);      // Prepare thread attribute
    //pthread_attr_setdetachstate( &attribute, PTHREAD_CREATE_DETACHED);
    if ((ret = pthread_create( &slowthread_id, &attribute, SlowUpdateThread, this)) != 0) {
        _logger->log(Logger::LOG_LEV_FATAL, "Error spawning thread [%s:%d]", __FILE__, __LINE__);
        throw AOException("Error spawning thread", THREAD_CREATION_ERROR,__FILE__, __LINE__);
    }

    //@C
    // Set priority for ``fast'' variables
    //@
    Utils::setSchedulingRR(99);
    _logger->log(Logger::LOG_LEV_INFO, "policy %d, priority %d [%s, %d]", sched_getscheduler(0), getpriority(PRIO_PROCESS, 0),
            __FILE__, __LINE__);

    Logger* logfast = Logger::get("FAST", Logger::stringToLevel("INF"));


    Timing t;
    t.start();
    while(TimeToDie()==false)
    {
        //@C
        // Here GetRawData is going to be called: set _can_copy = false
        //@
        pthread_mutex_lock(&_mut_can_copy);
        logfast->log(Logger::LOG_LEV_TRACE, "_mut_can_copy: lock on going to GetRawData(), _can_copy=false [%s:%d]",__FILE__, __LINE__);
        _can_copy=false;
        pthread_mutex_unlock(&_mut_can_copy);
        logfast->log(Logger::LOG_LEV_TRACE, "_mut_can_copy: unlock [%s:%d]",__FILE__, __LINE__);


        //@C
        // get raw data
        //@
        GetRawData();
        if (TimeToDie()==true) break; // could have received a TimeToDie while waiting for data

        //@C
        // save timestamp of the last diagnostic frame
        //@
        _frame_timestamp = GetTimeStamp();

        //@C
        // Loop over \verb+_fastvars+ and update
        //@
        pthread_mutex_lock(&_mut_fast);
        logfast->log(Logger::LOG_LEV_TRACE, "_mut_fast locked: _fast_ready=false, Update, _fast_ready=true [%s:%d]",
                __FILE__, __LINE__);
        _fast_ready=false;

        pthread_mutex_lock(&_mut_fastvars);
        logfast->log(Logger::LOG_LEV_TRACE, "_mut_fastvars locked: ResetFunctAction [%s:%d]", __FILE__, __LINE__);
        for (DiagnSet::iterator it = _fastvars->begin(); it != _fastvars->end();  it++)
        {
            DiagnVar& v = **it;
            v.ResetFunctAction();
        }
        logfast->log(Logger::LOG_LEV_TRACE, "Update start [%s:%d]", __FILE__, __LINE__);
        for (DiagnSet::iterator it = _fastvars->begin(); it != _fastvars->end();  it++)
        {
            DiagnVar& v = **it;
            v.Update(_frame_timestamp);
        }
        pthread_mutex_unlock(&_mut_fastvars);
        logfast->log(Logger::LOG_LEV_TRACE, "_mut_fastvars unlocked: Update end [%s:%d]", __FILE__, __LINE__);

        //@C
        // Now data from ``fast'' variables are ready, both raw and processed.
        // It's time to wake up the slow thread.
        //@
        _frame_counter = GetFrameCounter();
        _fast_counter++;
        _fast_ready=true;
        logfast->log(Logger::LOG_LEV_TRACE, "_cond_fast broadcast: _fast_ready=true [%s:%d]", __FILE__, __LINE__);
        pthread_cond_broadcast(&_cond_fast);
        pthread_mutex_unlock(&_mut_fast);
        logfast->log(Logger::LOG_LEV_TRACE, "_mut_fast unlocked  [%s:%d]", __FILE__, __LINE__);

        //@C
        // Here we are exiting from update: set _can_copy = true
        //@
        pthread_mutex_lock(&_mut_can_copy);
        logfast->log(Logger::LOG_LEV_TRACE, "_mut_can_copy: lock on exit form update  [%s:%d]",__FILE__, __LINE__);
        _can_copy=true;
        pthread_cond_broadcast(&_cond_can_copy);
        logfast->log(Logger::LOG_LEV_TRACE, "_cond_can_copy: _can_copy=true [%s:%d]", __FILE__, __LINE__);
        pthread_mutex_unlock(&_mut_can_copy);
        logfast->log(Logger::LOG_LEV_TRACE, "_mut_can_copy unlocked: loop end,  going to sleep[%s:%d]",__FILE__, __LINE__);

        //@C
        // Yield processor: we're RealTime and we need the other threads to be scheduled sometime.
        //@
        //usleep(1000);

        //@C
        // Call InFastLoop(). Entry point for user-defined tasks in extended DiagnApps (log,...)
        //@
        InFastLoop();
        
        //@C
        // do stuff for telemetry
        //@C
        if (_telemetry) _telemetry->LogTelemetry( DiagnApp::GetTimeStamp()  ); // _frame_timestamp);

        /*if (! _telemetry_map.empty() ) {
            static float first_timestamp = _frame_timestamp;
            static map<string,float>  orig = _telemetry_map;
            map<string, float>::iterator it; 
                //,itlow,itup;
           // itlow = _telemetry_map.begin();  // itlow points to first element to be logged
           // itup  = _telemetry_map.upper_bound(_frame_timestamp);   // itup points to first element that it's early now to be logged
           
            float ora  = _frame_timestamp - first_timestamp;
            _logger->log(Logger::LOG_LEV_INFO, "ORA %f [%s:%d]", ora, __FILE__, __LINE__);
            for (multimap<string, float>::iterator  it= _telemetry_map.begin(); it !=_telemetry_map.end();   it++ )
                 _logger->log(Logger::LOG_LEV_INFO, "PRIMA %s %f [%s:%d]",(*it).first.c_str(), (*it).second, __FILE__, __LINE__);


            for ( it=_telemetry_map.begin() ; it != _telemetry_map.end(); it++ ){
                string family = (*it).first;
                float  time_to_log = (*it).second;
                if (time_to_log > ora) continue; 
                _logger->log(Logger::LOG_LEV_INFO, "GUARDO %s %f [%s:%d]",family.c_str(), time_to_log,__FILE__, __LINE__);
                try {
                    DiagnSetPtr vars2log  = _vars.Filter(DiagnWhich(family, DiagnWhich::All, DiagnWhich::All, DiagnWhich::Fast, DiagnWhich::Enabled));
                    if ( vars2log->empty() == false ) {
                        vector<double> vvv = vars2log->GetValues() ;
                        _logger->log(Logger::LOG_LEV_INFO, "LOGGO %s %f %g [%s:%d]",family.c_str(), time_to_log, vvv[0], __FILE__, __LINE__);
                        Logger::get(family)->log_telemetry(Logger::LOG_LEV_INFO,  vvv ) ;
                    }
                } catch (DiagnSetException &e) {
                    _logger->log(Logger::LOG_LEV_DEBUG, "%s", e.what().c_str());
                }
                float delta = orig[family];
                _telemetry_map[family] += delta;
            }
            for (map<string, float>::iterator  it= _telemetry_map.begin(); it !=_telemetry_map.end();   it++ )
                 _logger->log(Logger::LOG_LEV_INFO, "DOPO %s %f [%s:%d]",(*it).first.c_str(), (*it).second,__FILE__, __LINE__);
        }*/


        // compute frequency TODO may be reviewed.
        if (_fast_counter - fast_counter_old == update_freq)
        {
            t.stop();

            //@C
            // Compute hw frame rate
            //@
            if (_frame_counter > frame_counter_old) // to skip hardware resets
                _frame_rate = (float)(_frame_counter-frame_counter_old) / t.sec();

            //@C
            // Compute _fast_rate
            //@
            _fast_rate = (float)update_freq/t.sec();

            logfast->log(Logger::LOG_LEV_INFO, "Processed %5ld cycles of %5d vars in %6.3g s (%6.3g Hz). HW frame rate %7.3g [%s:%d]",
                update_freq, _fastvars->size(), t.sec(), _fast_rate, _frame_rate,__FILE__,__LINE__ );

            frame_counter_old = _frame_counter;
            update_freq = (unsigned int) (update_freq / t.sec() + 1);  // want to pass here once per second.
            fast_counter_old = _fast_counter;
            t.start();

            //@C
            // Call Periodic(). This function is called roughly once per second.
            //@
            Periodic();
        }
    }
 } catch (...) {
     _logger->log(Logger::LOG_LEV_ERROR, "DiagnApp::Run() caught exception. Unlocking fastvars mutex  [%s:%d]", __FILE__, __LINE__);
     pthread_mutex_unlock(&_mut_fastvars);
     pthread_mutex_unlock(&_mut_fast);
     throw;
 }
    // Wait for slow thread to exit
    _logger->log(Logger::LOG_LEV_WARNING, "DiagnApp::Run() joining slowthread [%s:%d]", __FILE__, __LINE__);
    pthread_join(slowthread_id, NULL);

    // TODO go back to normal scheduling
    _logger->log(Logger::LOG_LEV_WARNING, "DiagnApp::Run() exit [%s:%d]", __FILE__, __LINE__);
}


//@Member: log_time
//
//@
#include "time.h"
#include <sstream>
#include <iomanip>
std::string DiagnApp::log_time() // link -lrt
{
    struct timespec tp;
    std::ostringstream oss;
    clock_gettime(CLOCK_REALTIME,&tp);
    float tim = tp.tv_sec % 100 + tp.tv_nsec/1e9;
    oss << std::fixed << std::setfill('0') << std::setw(7) << std::setprecision(4) << tim << " -" ;
    return oss.str();
}

//@Member: ToggleUnprotected
//
// Set \verb+_VarUnprotected+ to true when the diagnostic application can't control
// the safety status of the system for unwanted reasons.
//
//@
void DiagnApp::ToggleUnprotected(bool prtctd)
{
    try{
        _VarUnprotected.Set(prtctd);
    } catch(AOException &e){ std::cerr << e.what() << std::endl; throw(e); }
}


//@Member: SlowUpdateThread
//
// Thread that updates variables flagged ``slow''
//@
void * DiagnApp::SlowUpdateThread(void* argp)
{
    DiagnApp *pt = (DiagnApp*)argp;

    Logger* logslow = Logger::get("SLOW");

    int ret;
    int nice=0;
    if ((ret=setpriority(PRIO_PROCESS, 0, nice)) != 0){
        logslow->log(Logger::LOG_LEV_WARNING, "Error setting priority %d - [error %d] [%s:%d]",nice, ret, __FILE__, __LINE__ );
    }
    logslow->log(Logger::LOG_LEV_INFO, "policy %d, priority %d [%s:%d]", sched_getscheduler(0), getpriority(PRIO_PROCESS, 0),
            __FILE__, __LINE__);
    //@C
    //Compute frequencies (\verb+_fast_rate+ and \verb+_frame_rate+)
    //every \verb+update_freq+ cycles.
    //It is dynamically modified to keep an approximate interval
    //between updates of 1s.
    //@
    unsigned int update_freq=10;
    unsigned int slow_counter_old=0;

    unsigned int fast_counter_copied = 0;

    DiagnSetPtr slowvars;

    Timing t;
    t.start();

    while(TimeToDie()==false){
        //@C
        // make a local copy of slowvars; TODO add mutex
        // TODO probabilmente la feature di cambiare al volo una DiagnVar da slow a fast e' una cazzata
        // se si rimuove, allora questa parte finisce fuori dal loop: chi e' creato slow resta slow e non ci si pensa piu'
        //@
        slowvars    = pt->_vars.Filter(DiagnWhich("*",DiagnWhich::All,DiagnWhich::All,DiagnWhich::Slow,DiagnWhich::Enabled));
        if (slowvars->size() == 0) {
            usleep(200000);  //TODO facacare
            continue;
        }

        //@C
        // Wait on \verb+_cond_can_copy+ that signals the availability of raw data and updated fast variables.
        //@
        pthread_mutex_lock(&pt->_mut_can_copy);
        logslow->log(Logger::LOG_LEV_TRACE, "_mut_can_copy locked: wait _can_copy [%s:%d]", __FILE__, __LINE__);
        while (pt->_can_copy == false || pt->_fast_counter == fast_counter_copied){
            logslow->log(Logger::LOG_LEV_TRACE, "waiting _cond_can_copy for CopyOnLocal  [%s:%d]", __FILE__, __LINE__);
            pthread_cond_wait(&pt->_cond_can_copy, &pt->_mut_can_copy);
        }
        logslow->log(Logger::LOG_LEV_TRACE, "_mut_can_copy locked: going to CopyOnLocal [%s:%d]", __FILE__, __LINE__);

        //@C
        // Copy data to local buffer. Now they are all good. Then probably not.
        //@
        for (DiagnSet::iterator it = slowvars->begin(); it != slowvars->end();  it++) (**it).CopyOnLocal();

        //@C
        // Keep memory of the frame number copied and release mutex.
        //@
        fast_counter_copied = pt->_fast_counter;
        pthread_mutex_unlock(&pt->_mut_can_copy);
        logslow->log(Logger::LOG_LEV_TRACE, "_mut_can_copy unlocked: CopyonLocal done, _slow_ready = false [%d] [%s:%d]",fast_counter_copied, __FILE__, __LINE__);

        //@C
        // Iterate over \verb+slowvars+ and update them.
        //@
        for (DiagnSet::iterator it = slowvars->begin(); it != slowvars->end();  it++)
        {
            DiagnVar& v = **it;
            v.Update(pt->_frame_timestamp);
        }
        pt->_slow_counter++;

        //@C
        // Now blindly save all slow DiagnValues in \verb+_dict_slow+.
        // Just do it every cycle: it cost a litte and is needed by GetDiagnValue();
        // Set \verb+_slow_ready+ to false before starting updating dictionaries
        // Set \verb+_slow_ready+ to true  after having updated the dictionaries  and signal.
        // TODO profile this part, may be too heavy
        //@
        pthread_mutex_lock(&pt->_mut_slow);
        logslow->log(Logger::LOG_LEV_TRACE, "_mut_slow locked: updating dictionaries, slow_ready=false [%s:%d]",  __FILE__, __LINE__);

        pt->_slow_ready=false;
        for (DiagnSet::iterator it = slowvars->begin(); it != slowvars->end();  it++){
            pt->_dict_slow[DiagnWhich( (*it)->Family(), (*it)->Index(), (*it)->Index() ) ] =  (*it)->ExportValues();
            pt->_dict_buffers_slow[DiagnWhich( (*it)->Family(), (*it)->Index(), (*it)->Index() ) ] =  (*it)->ExportBuffers(); //TODO this is may be too heavy ???
        }
        pt->_slow_ready=true;
        logslow->log(Logger::LOG_LEV_TRACE, "_cond_slow broadcast: _slow_ready=true [%s:%d]", __FILE__, __LINE__);
        pthread_cond_broadcast(&pt->_cond_slow);
        pthread_mutex_unlock(&pt->_mut_slow);
        logslow->log(Logger::LOG_LEV_TRACE, "_mut_slow unlocked: dictionaries updated, slow_ready=true [%s:%d]", __FILE__, __LINE__);

        //@C
        // Call InSlowLoop(). Entry point for user-defined tasks (log,...)
        //@
        pt->InSlowLoop();

        // compute _slow_rate TODO review this part
        if (pt->_slow_counter - slow_counter_old == update_freq)
        {
            t.stop();
            pt->_slow_rate = (float)update_freq/t.sec();

            logslow->log(Logger::LOG_LEV_INFO, "Processed %5ld cycles of %5d vars in %6.3g s (%6.3g Hz) [%s:%d]",
                update_freq, slowvars->size(), t.sec(), pt->_slow_rate, __FILE__, __LINE__);

            update_freq = (unsigned int) (update_freq / t.sec() + 1);  // want to pass here once per second.
            slow_counter_old = pt->_slow_counter;
            t.start();
        }
    }
    logslow->log(Logger::LOG_LEV_WARNING, "DiagnApp::SlowUpdateThread() exit [%s:%d]", __FILE__, __LINE__);
    return NO_ERROR;
}

//@Member: SetDiagnVarParam
//
// Modified the parameters of a DiagnVar.
// Can be called with either a single DiagnWhich / DiagnParam specification,
// or with two vectors of WhichParam objects.
//
//@
int  DiagnApp::SetDiagnVarParam(
        DiagnWhich & w,
        DiagnParam & p
        )
{
   std::vector<WhichParam> v_wp;
   WhichParam wp;
   wp.wh = w;
   wp.pa = p;
   v_wp.push_back(wp);
   return SetDiagnVarParam( v_wp);
}

int DiagnApp::SetDiagnVarParam( 
      std::vector<WhichParam> &v_wp
      )
{
    int modified=0;

    Logger* logger = Logger::get("DIAGNSETPARAM");

    //@C
    // fast variables can have their parameters modified only when not updating i.e. when \verb+_fast_ready+ is true.
    //@
    pthread_mutex_lock(&_mut_fast);
    logger->log(Logger::LOG_LEV_TRACE, "_mut_fast locked: wait _fast_ready==true [%s:%d]",__FILE__, __LINE__);
    while (_fast_ready == false){
        logger->log(Logger::LOG_LEV_TRACE, "waiting _cond_fast: ImportParams for fast vars [%s:%d]",__FILE__, __LINE__);
        pthread_cond_wait(&_cond_fast, &_mut_fast);
    }
    logger->log(Logger::LOG_LEV_TRACE, "_mut_fast locked: _fast_ready==true going to ImportParams [%s:%d]", __FILE__, __LINE__);

    std::vector<WhichParam>::iterator itwp;
    for ( itwp = v_wp.begin(); itwp != v_wp.end(); itwp++) {

        DiagnSetPtr vars2modFast  = _vars.Filter(DiagnWhich(itwp->wh.Family(), itwp->wh.From(), itwp->wh.To(), DiagnWhich::Fast));
        for (DiagnSet::iterator it = vars2modFast->begin(); it != vars2modFast->end();  it++) {
            try{
                (*it)->ImportParams(itwp->pa);
                modified++;
            } catch (DiagnRangeException<double>) {
            } catch (DiagnRangeException<float>) {} //do nothing. it's enough that modified is not incremented
        }
    }

    pthread_mutex_unlock(&_mut_fast);
    logger->log(Logger::LOG_LEV_TRACE, "_mut_fast unlocked: ImportParams for fast vars ended [%s:%d]",__FILE__, __LINE__);


    //@C
    // Cannot require that slow variables had their parameters modified only outside the \verb+Update()+ loop, because updating slow variables
    // is tipically a very long process. So here we don't wait for any particular condition and we accept the \verb+ImportParams()+ process to be eventually done
    // during the verb+Update()+ loop.
    //@

    for ( itwp = v_wp.begin(); itwp != v_wp.end(); itwp++) {

      DiagnSetPtr vars2modSlow  = _vars.Filter(DiagnWhich(itwp->wh.Family(), itwp->wh.From(), itwp->wh.To(), DiagnWhich::Slow));
      for (DiagnSet::iterator it = vars2modSlow->begin(); it != vars2modSlow->end();  it++) {
        try {
            (*it)->ImportParams(itwp->pa);
            modified++;
        } catch (DiagnRangeException<double>) {
        } catch (DiagnRangeException<float>) {} //do nothing. it's enough that modified is not incremented
      }
    }

    //@C
    // Call \verb+UpdateFilteredIterators()+ to update the filters matching a subset of variables like \verb+_slowvars+ and \verb+_fastvars+ .
    //@
    UpdateFilteredIterators();

    return modified;
}

//@Member: GetDiagnVarParam
//
//
//@
ParamDict    DiagnApp::GetDiagnVarParam(
        DiagnWhich &which)
{
    DiagnSetPtr vars2modify  = _vars.Filter(DiagnWhich(which.Family(), which.From(), which.To()));

    ParamDict dict;

    for (DiagnSet::iterator it = vars2modify->begin(); it != vars2modify->end();  it++){
        dict[DiagnWhich( (*it)->Family(), (*it)->Index(), (*it)->Index() ) ] =  (*it)->ExportParams();
    }
    return dict;
}

//@Member: GetDiagnValue
//
//
//@
ValueDict    DiagnApp::GetDiagnValue(
        DiagnWhich &which)
{
    DiagnSetPtr vars2getFast;
    DiagnSetPtr vars2getSlow;

    Logger* logger = Logger::get("DIAGNGETVALUE");

    logger->log(Logger::LOG_LEV_TRACE, "look for filter in previous requests  [%s:%d]", __FILE__, __LINE__);

    logger->log(Logger::LOG_LEV_TRACE, "Filter slowvars [%s:%d]", __FILE__, __LINE__);
    vars2getFast  = _vars.Filter(DiagnWhich(which.Family(), which.From(), which.To(), DiagnWhich::Fast));
    vars2getSlow  = _vars.Filter(DiagnWhich(which.Family(), which.From(), which.To(), DiagnWhich::Slow));
    ValueDict dict;

    logger->log(Logger::LOG_LEV_TRACE, "Filters ready. Lock mutex [%s:%d]", __FILE__, __LINE__);
    pthread_mutex_lock(&_mut_fast);
    logger->log(Logger::LOG_LEV_TRACE, "_mut_fast locked: wait _fast_ready==true [%s:%d]", __FILE__, __LINE__);
    while (_fast_ready == false){
        logger->log(Logger::LOG_LEV_TRACE, "waiting _cond_fast [%s:%d]",__FILE__, __LINE__);
        pthread_cond_wait(&_cond_fast, &_mut_fast);
    }
    logger->log(Logger::LOG_LEV_TRACE, "_mut_fast locked: ExportValues for matched fast vars [%s:%d]", __FILE__, __LINE__);
    for (DiagnSet::iterator it = vars2getFast->begin(); it != vars2getFast->end();  it++){
        dict[DiagnWhich( (*it)->Family(), (*it)->Index(), (*it)->Index() ) ] =  (*it)->ExportValues();
    }
    pthread_mutex_unlock(&_mut_fast);
    logger->log(Logger::LOG_LEV_TRACE, "_mut_fast unlocked: fastvars values added to dict [%s:%d]", __FILE__, __LINE__);


    // Now get data for the slow varibles retrieving dict saved in the slow cycle
    pthread_mutex_lock(&_mut_slow);
    logger->log(Logger::LOG_LEV_TRACE, "_mut_slow locked: wait _slow_ready==true  [%s:%d]", __FILE__, __LINE__);
    while (_slow_ready == false){
        logger->log(Logger::LOG_LEV_TRACE, "waiting _cond_slow: ExportValues for slow vars [%s:%d]", __FILE__, __LINE__);
        pthread_cond_wait(&_cond_slow, &_mut_slow);
    }
    logger->log(Logger::LOG_LEV_TRACE, "_mut_slow locked: ExportValues for matched slow vars [%s:%d]", __FILE__, __LINE__);

    for (DiagnSet::iterator it = vars2getSlow->begin(); it != vars2getSlow->end();  it++){
        DiagnWhich wh( (*it)->Family(), (*it)->Index(), (*it)->Index() );
        dict[wh] =  _dict_slow[wh];
    }

    pthread_mutex_unlock(&_mut_slow);
    logger->log(Logger::LOG_LEV_TRACE, "_mut_slow unlocked: slowvars buffers added to dict  [%s:%d]", __FILE__, __LINE__);


    return dict;
}
//@Member: GetDiagnBuffer
//
//
//@
BufferDict    DiagnApp::GetDiagnBuffer(
        DiagnWhich &which)
{
    DiagnSetPtr vars2getFast  = _vars.Filter(DiagnWhich(which.Family(), which.From(), which.To(), DiagnWhich::Fast));
    DiagnSetPtr vars2getSlow  = _vars.Filter(DiagnWhich(which.Family(), which.From(), which.To(), DiagnWhich::Slow));
    BufferDict dict;

    Logger* logger = Logger::get("DIAGNGETBUFFER");

    // Get data from the fast variables
    pthread_mutex_lock(&_mut_fast);
    logger->log(Logger::LOG_LEV_TRACE, "_mut_fast locked: wait _fast_ready==true [%s:%d]", __FILE__, __LINE__);
    while (_fast_ready == false){
        logger->log(Logger::LOG_LEV_TRACE, "waiting _cond_fast: ExportBuffers for fast vars [%s:%d]", __FILE__, __LINE__);
        pthread_cond_wait(&_cond_fast, &_mut_fast);
    }
    logger->log(Logger::LOG_LEV_TRACE, "_mut_fast locked: ExportBuffers for matched fast vars [%s:%d]", __FILE__, __LINE__);

    for (DiagnSet::iterator it = vars2getFast->begin(); it != vars2getFast->end();  it++){
        dict[DiagnWhich( (*it)->Family(), (*it)->Index(), (*it)->Index() ) ] =  (*it)->ExportBuffers();
    }
    pthread_mutex_unlock(&_mut_fast);
    logger->log(Logger::LOG_LEV_TRACE, "_mut_fast unlocked: fastvars buffers added to dict  [%s:%d]",__FILE__, __LINE__);


    // Now get data for the slow varibles retrieving dict saved in the slow cycle
    pthread_mutex_lock(&_mut_slow);
    logger->log(Logger::LOG_LEV_TRACE, "_mut_slow locked: wait _slow_ready==true  [%s:%d]",__FILE__, __LINE__);
    while (_slow_ready == false){
        logger->log(Logger::LOG_LEV_TRACE, "waiting _cond_slow: ExportBuffers for slow vars [%s:%d]", __FILE__, __LINE__);
        pthread_cond_wait(&_cond_slow, &_mut_slow);
    }
    logger->log(Logger::LOG_LEV_TRACE, "_mut_slow locked: ExportBuffers for matched slow vars [%s:%d]", __FILE__, __LINE__);

    for (DiagnSet::iterator it = vars2getSlow->begin(); it != vars2getSlow->end();  it++){
        DiagnWhich wh( (*it)->Family(), (*it)->Index(), (*it)->Index() );
        dict[wh] =  _dict_buffers_slow[wh];
    }

    pthread_mutex_unlock(&_mut_slow);
    logger->log(Logger::LOG_LEV_TRACE, "_mut_slow unlocked: slowvars buffers added to dict  [%s:%d]", __FILE__, __LINE__);

    return dict;
}

//@Member: UpdateDiagnParam
//
// Parse config file of parameter and update variables' parameters.
//
//@
void     DiagnApp::UpdateDiagnParam(string paramfile)
{
    int cnt;
    std::vector<WhichParam> dict = ParseParamCfgFile(paramfile);
    cnt = SetDiagnVarParam(dict);
//    _logger->log(Logger::LOG_LEV_DEBUG, "UpdateDiagnParam: %s ---> %s [%d vars modified] [%s:%d]",
//           (*it).wh.str().c_str(), (*it).pa.str().c_str(), cnt, __FILE__, __LINE__);
    _logger->log(Logger::LOG_LEV_INFO, "loaded %d params from file %s [%s:%d]", dict.size(), paramfile.c_str(), __FILE__, __LINE__);
}

//@Member: ParseParamCfgFile
//
// Parse the config file of parameters and return a WhichParam vector.
//
// Parameter config file name must be declared in DiagnApp config file
// under the tag ParamCfgFile.
// Each line in the config file is composed of a DiagnWhich + a DiagnParam object
// (3 + 8  columns at the time of writing,  but see DiagnWhich and DiagnParam documentation
// for details)
//
//@
#include <fstream>
#include "DiagnWhich.h"
std::vector<WhichParam>    DiagnApp::ParseParamCfgFile(string configfile)
{
    std::string                 filename;
    std::string                 line;
    std::string::size_type      first;
    std::vector<WhichParam>     dict;

    if (configfile != "")
        filename = ConfigDictionary().getDir() + "/" + configfile;
    else
        filename = ConfigDictionary().getDir() + "/" + (std::string)(ConfigDictionary()["ParamCfgFile"]);

    std::ifstream in(filename.c_str());
    if (!in) throw Config_File_Exception("Unable to find configuration file: " + filename, FILE_ERROR, __FILE__, __LINE__);
    while (getline(in,line))
    {
        //Strip blank characters
        first = line.find_first_not_of(" \t");
        if (first != std::string::npos)
            line = line.substr(first, line.find_last_not_of(" \t")-first+1);

        // Remove all leading whitespace
        while ((line.size()>0) && ((line[0] == ' ') || (line[0] == '\t')))
           line = line.substr(1);

        //Checks if comment line
        first = line.find('#');
        //Remove comment ad the end of the line
        if (first != std::string::npos) line = line.substr(0, first);
        // Found only comment or empty line
        if (line.size() <1) continue;
        // Parse text
        istringstream ssline(line);
        WhichParam wp;
        ssline >> wp.wh >> wp.pa;
        _logger->log(Logger::LOG_LEV_INFO, "ParseParamCfgFile: %s %s  [%s:%d]",
                wp.wh.str().c_str(), wp.pa.str().c_str(),  __FILE__, __LINE__);
        dict.push_back(wp);
    }
    return dict;
}

//@Member: ParseTelemetryCfgFile
//
// Parse the config file of telemetry and return a dictionary <Family, TelemetryPeriod>,
// where Family is a string and TelemetryPeriod is the logger period in seconds.
//
// Telemetry config file name must be declared in DiagnApp config file
// under the tag TelemetryCfgFile. In case this tag or the file are absent, no telemetry logging
// will be performed
//
// Each line in the config file is composed of a Family name (string) and a 
// telemetry log period in seconds (float)
//
//@

//multimap<float, string> DiagnApp::ParseTelemetryCfgFile()
/*map<string, float> DiagnApp::ParseTelemetryCfgFile()
{
    map<string, float>       tmpdict;
    map<string, float>::iterator it  ;
    string                        family;
    float                         period;
    std::string                   filename;
    std::string                   line;
    std::string::size_type        first;
    
    try {
        filename = ConfigDictionary().getDir() + "/" + (std::string)(ConfigDictionary()["TelemetryCfgFile"]);
    } catch (...) {
        _logger->log(Logger::LOG_LEV_WARNING, "Telemetry Config File not found. Telemetry disabled  [%s:%d]", __FILE__, __LINE__);
        return tmpdict;
    }
    
    std::ifstream in(filename.c_str());
    if (!in) throw Config_File_Exception("Unable to find configuration file: " + filename);
    while (getline(in,line))
    {
        //Strip blank characters
        first = line.find_first_not_of(" \t");
        if (first != std::string::npos)
            line = line.substr(first, line.find_last_not_of(" \t")-first+1);
        //Checks if comment line
        first = line.find('#');
        //Remove comment ad the end of the line
        if (first != std::string::npos) line = line.substr(0, first);
        // Found only comment or empty line
        if (line.size() <1) continue;
        // Parse text
        istringstream ssline(line);
        //WhichParam wp;
        ssline >> family;
        ExtractFromStream<float>(ssline, period); 
        tmpdict.insert ( pair<string,float>(family, period)  );
        _logger->log(Logger::LOG_LEV_INFO, "ParseTelemetryCfgFile: %s %f  [%s:%d]", family.c_str(), period,  __FILE__, __LINE__);
    }

    return tmpdict;

}*/


//@Member: GetTimeStamp
//
// Return timestamp of the current frame
//
// Unit must be seconds
//@
double DiagnApp::GetTimeStamp()
{
    struct timeval tv;
    int ret;
    double timestamp=0.0;
    static time_t ti=0;
    static bool firsttime=true;

    if (firsttime){
        ret = gettimeofday(&tv, NULL);
        if (ret != 0) throw AOException("[DiagnApp::GetTimeStamp()] gettimeofday failed",GENERIC_SYS_ERROR,__FILE__,__LINE__);
        ti = tv.tv_sec;
        firsttime=false;
    }

    ret = gettimeofday(&tv, NULL);
    if (ret != 0) throw AOException("[DiagnApp::GetTimeStamp()] gettimeofday failed",GENERIC_SYS_ERROR,__FILE__,__LINE__);
    timestamp = (tv.tv_sec-ti)+tv.tv_usec/1000000.;

    _logger->log(Logger::LOG_LEV_TRACE, "[DiagnApp::GetTimeStamp()] timestamp: %g [%s:%d]", timestamp, __FILE__, __LINE__);

    return timestamp;
}



//@Member: CreateDiagnVarsDescriptionFile
//
// Create text file containing the description of the diagnvar set
//
// File name must be specified in the config file
//@
void DiagnApp::CreateDiagnVarsDescriptionFile()
{
    std::string filename = (std::string)(ConfigDictionary()["VarDescriptionFile"]);

    // Create absolute path filename
    string absoluteFilename = _logger->getLogPath() + "/" + filename;
    _logger->log(Logger::LOG_LEV_INFO, "[CreateDiagnVarsDescriptionFile] Writing diagnvar description file '%s' [%s:%d]",
            absoluteFilename.c_str(), __FILE__, __LINE__);

    ofstream myfile (absoluteFilename.c_str());
    if (myfile.is_open())
    {
        myfile << ";;;;;;;;; This is an autogenerated file. Don't edit ;;;;;;;;;;;;;;;;;\n";
        myfile << ";\n; A DiagnVar is specified by a Family name (i.e. 'ChDistAve') and an Index (i.e. 671)\n";
//        myfile << "; The complete name is given by Family and Index in the format '%s%04d' (i.e. ChDistAve-0671) but it is rarely used\n";

        char outstr[200];
        time_t t;
        struct tm *tmp;

        t = time(NULL);
        tmp = localtime(&t);
        strftime(outstr, sizeof(outstr), "%y/%m/%d %H:%M:%S", tmp);

        myfile << ";;;;;; "<<MyFullName()<<" "<< outstr << "\n";

        myfile << "diagnapp                    string     \""<<MyFullName()<<"\"          ; diagnostic application name\n" ;
        strftime(outstr, sizeof(outstr), "%y%m%d%H%M%S", tmp);
        myfile << "date                        string     \""<<outstr<<"\"          ; when this file has been generated\n";

        myfile << "vars_num               long         "<<_vars.size()<<"                   ; number of diagnostic variables\n";


        DiagnSetPtr varsFamilies  = _vars.Filter(DiagnWhich("*", 0, 0));
        myfile << "vars_family_num        long         "<<varsFamilies->size()<<"                   ; number of diagnvar family\n";

        myfile << "vars_family_name       array      ; string array containing diagnvars family names\n";
        myfile << "        string       "<<varsFamilies->size()<<"\n";
        for (DiagnSet::iterator it = varsFamilies->begin(); it != varsFamilies->end();  it++)
            myfile << "        "<<  (*it)->Family()        <<"\n";
        myfile << "end\n\n";

        for (DiagnSet::iterator it = varsFamilies->begin(); it != varsFamilies->end();  it++)
        {
            DiagnSetPtr varsIndex  = _vars.Filter(DiagnWhich( (*it)->Family(), -1, -1));
            myfile << (*it)->Family()     << " structure   ; diagnvar family name \n";
            myfile << "        qty         long       " << varsIndex->size()  << "                   ; how many variables in this family\n";
            myfile << "end\n\n";
        }
        myfile.close();
    }
    else {
        _logger->log(Logger::LOG_LEV_ERROR, "[CreateDiagnVarsDescriptionFile] Unable to write file '%s' [%s:%d]",
            absoluteFilename.c_str(), __FILE__, __LINE__);
    }
    return;
}




//@Member: CreateGlobalDiagnVars
//
//@
void DiagnApp::CreateGlobalDiagnVars()
{

    std::string          groupname;
    //DiagnRange<double>   rng(-2, -1 , 1, 2);
    //size_type            run_mean_len=100;
    //unsigned int         cons_allow_faults=0;
    FunctActionPtr             hAlarm   (new FunctAlarm());
    FunctActionPtr             hWarning (new FunctWarning());

//@C
// /verb+DiagnAppInitTimestamp+: timestamp of the boot of the diagnostic application.
// 
//@

    //@C
    // save timestamp of now  
    //@
    struct timeval tv;
    gettimeofday(&tv, NULL);
    _init_timestamp = tv.tv_sec + tv.tv_usec/1000000.;
    groupname = "DiagnAppInitTimestamp";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctDouble( &(_init_timestamp) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+DiagnAppFrameTimestamp+: frame timestamp. See also DiagnApp::GetTimeStamp()
//@
    groupname = "DiagnAppFrameTimestamp";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctDouble( &(_frame_timestamp) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+DiagnAppFrameCounter+: frame counter. See also DiagnApp::GetFrameCounter()
//@
    groupname = "DiagnAppFrameCounter";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUnsignedInt( &(_frame_counter) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+DiagnAppFastVarsCounter+: fast vars counter. Counts how many loops of FastDiagnVars analysis have been done.
//@
    groupname = "DiagnAppFastVarsCounter";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUnsignedInt( &(_fast_counter) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+DiagnAppSlowVarsCounter+: slow vars counter. Counts how many loops of SlowDiagnVars analysis have been done.
//@
    groupname = "DiagnAppSlowVarsCounter";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctUnsignedInt( &(_slow_counter) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+DiagnAppFrameRate+: frame rate. frame rate, averaged on a ~1s window
//@
    groupname = "DiagnAppFrameRate";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctFloat( &(_frame_rate) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }
//@C
// /verb+DiagnAppFastVarsRate+: fast vars rate. How fast FastDiagnVars are analyzed, averaged on a ~1s window
//@
    groupname = "DiagnAppFastVarsRate";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctFloat( &(_fast_rate) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

//@C
// /verb+DiagnAppSlowVarsRate+: slow vars rate. How fast SlowDiagnVars are analyzed, averaged on a ~1s window
//@
    groupname = "DiagnAppSlowVarsRate";
    {
        _logger->log(Logger::LOG_LEV_DEBUG, "Creating DiagnVar %s-%d", groupname.c_str(), 0);
        FunctPtr fConv (new FunctFloat( &(_slow_rate) ) );
        DiagnVarPtr theVar (new DiagnVar(groupname, 0, fConv, hWarning, hAlarm));
        _vars.Add(theVar);
    }

}




//@Member: DumpDiagnVarParam
//
// Create text file containing the list of diagnvars with the parameters
// The generated file has the same format of a ParamCfgFile.
//
// If dumpfile is not specified, the file is created in the
// log directory and it is named "DIAGNAPPNAME.varsparam.DATE"
// (e.g. HOUSEKPR0.varsparam.080429235959)
//
//@
bool DiagnApp::DumpDiagnVarParam(string dumpfile)
{
    Logger* logger = Logger::get("DIAGNDUMPPARAMS");
    string outfilename;
    if ( dumpfile.compare( string() ) == 0 )
    {
        // what time is it?
        char outstr[200];
        time_t t;
        struct tm *tmp;
        t = time(NULL);
        tmp = localtime(&t);
        strftime(outstr, sizeof(outstr), "%y%m%d%H%M%S", tmp);

        // Create filename
        ostringstream dumpfilestream;
        dumpfilestream << logger->getLogPath() << "/" << MyFullName() << ".varsparam." << outstr;
        outfilename = dumpfilestream.str();
    }  else {
      if (dumpfile[0] != '/')
         outfilename = logger->getLogPath() + "/" + dumpfile;
      else
         outfilename = dumpfile;
    }


    logger->log(Logger::LOG_LEV_INFO, "Writing diagnvar description file '%s' [%s:%d]",
            outfilename.c_str(), __FILE__, __LINE__);

    ofstream myfile (outfilename.c_str());
    if (myfile.is_open())
    {
        myfile << "########  This is an autogenerated file ############\n";
        char outstr[200];
        time_t t;
        struct tm *tmp;

        t = time(NULL);
        tmp = localtime(&t);
        strftime(outstr, sizeof(outstr), "%y/%m/%d %H:%M:%S", tmp);

        myfile << "#  "<<MyFullName()<<" "<< outstr << "\n#\n";
        myfile << "# Each entry must be composed of 11 fields:\n";
        myfile << "# Fields 0-2 identify a filter that matches a set of DiagnVars\n";
        myfile << "# Fields 3-10 are used to specify parameters to be set for the DiagnVars matched by the filter.\n";
        myfile << "#\n";
        myfile << "# Examples of filters:\n";
        myfile << "# Matching all variables:                                   *           All     All\n";
        myfile << "# Matching all ChDistAve vars:                              ChDistAve   All     All\n";
        myfile << "# Matching all variables with index 13:                     *           13      13\n";
        myfile << "# Matching first 100 Modes:                                 Modes       All     99\n";
        myfile << "# Matching all current vars:                                Ch*Curr*    All     All\n";
        myfile << "#\n";
        myfile << "# [Alarm/Warn][Min/Max] specify alarm and warning ranges. Use SI units.\n";
        myfile << "#\n";
        myfile << "# RunningMeanLen specify the length in seconds of the running mean buffer over which the average value is computed.\n";
        myfile << "# \n";
        myfile << "# CAF specify the number of Consecutive Allowed Faults that can be detected before triggering a warning/alarm.\n";
        myfile << "# This can be useful in case of faulty sensors that sometimes output a very wrong value.\n";
        myfile << "#\n";
        myfile << "# Set Enabled to \"disabled\" if you don't want to monitor the matched variables.\n";
        myfile << "#\n";
        myfile << "# Set Slow to \"slow\" if the matched variables are heavy to be computed. The update of the \"slow\" variables is done in a \n";
        myfile << "# dedicated thread at a reduced rate with respect to \"fast\" variables\n";
        myfile << "#\n";
        myfile << "# Entry will be applied in order, from top to bottom. \n";
        myfile << "#\n";
        myfile << "# FamilyName            From    To     AlarmMin    WarnMin      WarnMax     AlarmMax   RunningMeanLen  CAF  Enabled Slow\n";

        DiagnWhich wh = DiagnWhich("*", -1, -1);
        DiagnWhich oldwh;
        DiagnParam oldpa;
        DiagnWhich nowwh;
        DiagnParam nowpa;
        string  oldfam;
        int     oldfrom;
        int     oldto;
        ParamDict mappa = GetDiagnVarParam( wh );
        for (ParamDict::iterator it = mappa.begin(); it != mappa.end();  it++){
            if ( it == mappa.begin() ){ 
                oldwh = (*it).first;
                oldpa = (*it).second;
                oldfrom = oldwh.From();
                oldto   = oldwh.To();
                oldfam  = oldwh.Family();
            }
            nowwh = (*it).first;
            nowpa = (*it).second;
            if (nowwh.Family() == oldwh.Family() && nowpa == oldpa) 
            {
                oldto = nowwh.To();
            } else {   
                myfile << DiagnWhich(oldfam, oldfrom, oldto) <<  oldpa << endl;
                oldfrom = nowwh.From();
                oldto   = nowwh.To();
                oldfam  = nowwh.Family();
            }
            oldwh = nowwh;
            oldpa = nowpa;
        }

        myfile.close();
    }
    else {
        logger->log(Logger::LOG_LEV_ERROR, "Unable to write file '%s' [%s:%d]", outfilename.c_str(), __FILE__, __LINE__);
    }
    if (myfile.fail() == true) return false;
    else return true;
    //TODO more description of failure
}


bool DiagnApp::Loadofflineframe(uint32 frameIndex) {
    static Logger* logger = Logger::get("LOAD_O_FRAME", Logger::LOG_LEV_DEBUG);
    BYTE* frame = _framesHistory->getFrame(frameIndex);    // This is e reference to the allocated frame: do not delete!
    uint32 frameSize = _framesHistory->getFrameSize();
    if(frame == NULL) {
        logger->log(Logger::LOG_LEV_ERROR, "Impossible to load frame %d from file", frameIndex);
        return false;
    }
    else {
        logger->log(Logger::LOG_LEV_DEBUG, "Frame %d succesfully loaded from file", frameIndex);
        StoreOfflineFrame(frame, frameSize); // Virtual method!
        return true;
    }
}


// Dump flag related
//
// Some alarm handlers will call this function and accumulate an value depending on
// the alarm description (long hash val) which can then be read during the InFastLoop
// periodic call.
// 

//@Member: setDumpAlarm
//
//@
void DiagnApp::setDumpAlarm(std::string desc)
{
    // mutex sync
    boost::mutex::scoped_lock lock(_mutex);
    boost::hash<std::string> hash;

    // arbitrary combination of the hash code
    _dumpAlarmHash = _dumpAlarmHash * 101 + hash(desc);
    if (_dumpAlarmHash == 0)
        _dumpAlarmHash = 1;
}


//@Member: getDumpAlarmAndClear
//
//@
long DiagnApp::getDumpAlarmAndClear(bool clear)
{
    // mutex sync
    boost::mutex::scoped_lock lock(_mutex);

    long ret = _dumpAlarmHash;
    
    if (clear)
        _dumpAlarmHash = 0;

    return ret;
}

