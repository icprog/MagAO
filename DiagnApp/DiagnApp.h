//@File: DiagnApp.h
//
// \diagnapp\ class declaration
//
// This file contains the declaration of \diagnapp\ class
//@

#ifndef DIAGNAPP_H_INCLUDED
#define DIAGNAPP_H_INCLUDED

#include <boost/archive/binary_oarchive.hpp> //serialize


#include "AOApp.h"
#include "DiagnSet.h"
#include "DiagnTelemetry.h"
#include "DiagnWhich.h"
#include "mastdiagnlib.h"

using namespace Arcetri;

//@Class: DiagnApp
//
// Base class for LBT-AO Diagnostic Applications
//
//@

class DiagnApp: public AOApp
{
    public:
        DiagnApp( int argc, char **argv) ;
        virtual             ~DiagnApp();

        //////////////////// methods ////////////////
        virtual void            InstallHandlers();
        virtual void            CreateDiagnVars(){};
        virtual void            GetRawData(){};
//@C
// \verb+GetTimeStamp()+ can be overridden by the derived application to return the time [s] associated with the
// current diagnostic frame. Default implementation use the system clock.
//@
        virtual double          GetTimeStamp();
//@C
// \verb+GetFrameCounter()+ must be implemented by the derived application to return a frame counter
// (e.g. the ID of the diagnostic record written by the hardware). This value is used to compute the FrameRate
//@
        virtual unsigned int    GetFrameCounter()=0;
//@C
// \verb+InFastLoop()+ is called at every update in the "fast" thread.
// Available for user defined tasks
//@

        virtual void            InFastLoop(){};
//@C
// \verb+InSlowLoop()+ is called at every update in the "slow" thread.
// Available for user defined tasks
//@

        virtual void            InSlowLoop(){};
//@C
// \verb+Periodic()+ is called no more than once per second in the "fast" thread.
// It can be used for periodic updates or display purposes
//@
        virtual void            Periodic(){};

        virtual void            SetupVars();
        virtual void            Run();

        int                 SetDiagnVarParam( DiagnWhich &, DiagnParam &);
        int                 SetDiagnVarParam( std::vector<WhichParam> &);

        ParamDict           GetDiagnVarParam( DiagnWhich & which);
        ValueDict           GetDiagnValue( DiagnWhich & which);
        BufferDict          GetDiagnBuffer( DiagnWhich & which);
        bool                DumpDiagnVarParam( std::string dumpfile = std::string() );

//@C
// \verb+FrameRate+ return the rate of the diagnostic frames as determined by the counter
// inside the hardware
//@
        float               FrameRate() {return _frame_rate;}
//@C
// \verb+FastRate+ return the number of processed "fast" frames per second
//@
        float               FastRate()  {return _fast_rate;}

//@C
// \verb+getVarUnprotected+ return value of _VarUnprotected RTDB variable.
//@
        bool				isVarUnprotected() { bool varUnprotected;
        										 _VarUnprotected.Get(0, &varUnprotected);
        										 return varUnprotected; }

        // dump flag related
        void                    setDumpAlarm(std::string desc);
        long                    getDumpAlarmAndClear(bool clear = true);

    protected:
        virtual void        UpdateFilteredIterators();
        void                ToggleUnprotected(bool);
        std::string         log_time();

        static bool			isOffline() { return _offline; }
        uint32				getHistorySize() { return _framesHistory->getFramesNumber(); }

    private:
        static int          diagndump_handler(MsgBuf *, void *, int hndlrQueueSize);
        static int          diagnsetparam_handler(MsgBuf*, void*, int hndlrQueueSize);
        static int          diagngetparam_handler(MsgBuf*, void*, int hndlrQueueSize);
        static int          diagngetvalue_handler(MsgBuf*, void*, int hndlrQueueSize);
        static int          diagngetbuffer_handler(MsgBuf*, void*, int hndlrQueueSize);
        static int          diagndumpparams_handler(MsgBuf*, void*, int hndlrQueueSize);
        static int          diagnloadparams_handler(MsgBuf *, void *, int );
        static int 			diagnloadofflineframe_handler(MsgBuf *msg, void *argp, int hndlrQueueSize);

        static void *       	SlowUpdateThread(void*);
        void                	UpdateDiagnParam(string configfile="");
        std::vector<WhichParam>     ParseParamCfgFile(string configname = "");
        map<string, float>     ParseTelemetryCfgFile();

        void                	Constructor();
        void                	CreateDiagnVarsDescriptionFile();
        void                	CreateGlobalDiagnVars();


    protected:
    	/*
    	 * Load a frame from the offline history
    	 * Calls the virtual method StoreOfflineFrame to save it
    	 */
        bool				 	Loadofflineframe(uint32 frameIndex);

        /*
         * Store an offline frame into the buffer used by GetRawData()
         * The frame size is only used to compare the frame size with the
         * raw buffer size, for security reasons.
         */
        virtual void			StoreOfflineFrame(BYTE* /*frame*/, uint32 /*frameSize*/) {};



        ////////////////// variables ///////////////
    public:

    protected:
//@C
//\verb+_vars+ contains all the defined DiagnVars.
//@
        DiagnSet            _vars;
//@C
//\verb+_slowvars+ is a filter matching all the DiagnVars tagged to run in the "slow" thread.
//Take care of race conditions when iterating over this filter: \verb+_mut_slowvars+ must be acquired before
//iterating over \verb+_slowvars+
//@
//        DiagnSetPtr         _slowvars;

//@C
//\verb+_fastvars+ is a filter matching all the DiagnVars tagged to run in the "fast" thread.
//Take care of race conditions when iterating over this filter: \verb+_mut_fastvars+ must be acquired before
//iterating over \verb+_fastvars+
//@
        DiagnSetPtr         _fastvars;
//@C
// \verb+_frame_timestamp+ holds the timestamp of the last processed frame in the fast thread.
// Get this value from the right counter in the diagnostic record or, if unavailable, use system clock.
// Unit must be seconds.
//@
        double              _frame_timestamp;
//@C
// \verb+_init_timestamp+ holds the timestamp of the boot of the diagnostic application in seconds since Epoch.
//@
        double              _init_timestamp;

//@C
//\verb+_frame_counter+ holds the frame number of the last processed frame. 
// Get this value from hardware via GetFrameCounter(). 
//@
        unsigned int        _frame_counter;
//@C
//\verb+_fast_counter+ counts the number of diagnostic frames processed in the fast thread
//@
        unsigned int        _fast_counter;
//@C
//\verb+_slow_counter+ counts the number of diagnostic frames processed in the slow thread
//@
        unsigned int        _slow_counter;

        DiagnTelemetry*     _telemetry;

    private:


//@C
//\verb+_requests+ is a dictionary that holds the requests received through the diagnlib and the search results.
//TODO DOC serve a non rifare piu' volte intutilmente la ricerca.
//@
        std::map<DiagnWhich, DiagnSetPtr>       _requests;

//@C
// \verb+_cond_fast+ is broadcasted by the fast thread when \verb+_fastvars+ are ready, i.e. is associated
// with the condition \verb+_fast_ready == True+.
//@
        pthread_cond_t      _cond_fast;

//@C
//\verb+_cond_slow+
//@
        pthread_cond_t      _cond_slow;

//@C
//\verb+_cond_can_copy+ is used to signal when \verb+_can_copy+ become true.
//@
        pthread_cond_t      _cond_can_copy;

//@C
//\verb+_mut_fastvars+ are used to synchronize access to
//\verb+_fastvars+ mainly to prevent UpdateFilteredIterators to corrupt the iterator over \verb+_slowvars+
//@
        pthread_mutex_t     _mut_fastvars;
//@C
// \verb+_mut_raw+ protect concurrency on the raw data written by \verb+GetRawData()+ and accessed by
// \verb+CopyOnLocal()+ in the slow thread.
//@
//        pthread_mutex_t     _mut_raw;

//@C
//\verb+_mut_fast+ protect concurrency on the access to fast variables
//@
        pthread_mutex_t     _mut_fast;

//@C
// \verb+_mut_slow+ is used to access data saved in the slow thread.
// When you need the values of a set of slow variables, you don't want to wait
// for a slow cycle to end: it can take a very long time.
// See GetDiagnValue and \verb+_dict_slow+.
//@
        pthread_mutex_t     _mut_slow;

//@C
// \verb+_mut_can_copy+ protect concurrency on \verb+_can_copy+.
//@
        pthread_mutex_t     _mut_can_copy;



//@C
//\verb+_dict_slow+ make a dictionary of slow variables DiagnValue to be used
//in \verb+GetDiagnValue()+. This is needed because the process of updating the slow variables can
//be very long and \verb+GetDiagnValue()+ don't want to wait for it. So \verb+_dict_slow+ store a copy of
//all the 'slow' variables to quickly answer to the queries of  \verb+GetDiagnValue()+
//@
        ValueDict           _dict_slow;

//@C
//\verb+_dict_buffer_slow+ make a dictionary of slow variables DiagnBuffer to be used
//in GetDiagnBuffer. See \verb+_dict_slow+.
//@
        BufferDict           _dict_buffers_slow;

//@C
// \verb+_raw_ready+ is set to True after \verb+GetRawData()+.
// \verb+_raw_ready+ is set to False before calling \verb+GetRawData()+.
// The slow thread must wait for the condition \verb+_raw_ready==True+ and \verb+_fast_ready==True+ to call \verb+CopyOnLocal()+.
// To prevent concurrency on \verb+_raw_ready+ use mutex \verb+_mut_raw+.
//@
//        bool                _raw_ready;

//@C
// \verb+_can_copy+ is set to True when raw data is good and fast variables have been updated
// \verb+_can_copy+ is set to False before calling GetRawData.
// To prevent concurrency on \verb+_can_copy+ use mutex \verb+_mut_can_copy+.
// The condition  \verb+_can_copy==True+ triggers the broadcast of \verb+_cond_can_copy+.
//@
        bool                _can_copy;


//@C
// \verb+_fast_ready+ is set to True when a diagnostic frame has been made available and all fast variables have
// been updated.
// \verb+_fast_ready+ is set to False before calling GetRawData
// The slow thread must wait for the condition \verb+_fast_ready==True+ to read and copy the raw data.
// To prevent concurrency on \verb+_fast_ready+ use mutex \verb+_mut+.
// The condition  \verb+_fast_ready==True+ triggers the broadcast of \verb+_cond_fast+.
//@
        bool                _fast_ready;

//@C
// \verb+_slow_ready+ is set to True when all slow variables have been updated and data have been saved to
// to the \verb+_dict_slow+.
// \verb+_slow_ready+ is set to False after having copied data to the local vars and before calling
// updating 'slow' vars.
// To prevent concurrency on \verb+_slow_ready+ use mutex \verb+_mut_slow+.
// The condition  \verb+_slow_ready==True+ triggers the broadcast of \verb+_cond_slow+.
//@
        bool                _slow_ready;

//@C
// \verb+_VarUnprotected+ is set true when the diagnostic application
// is not able to perform the diagnostic checks (e.g. because the raw data
// are not available or \diagnapp\ is set in dummy mode)
//@
        RTDBvar             _VarUnprotected;

//@C
//\verb+_frame_rate+ is the measured rate of diagnostic frames on hardware.
//@
        float               _frame_rate;
//@C
//\verb+_fast_rate+ is the measured rate of frame processed in the fast thread.
//@
        float               _fast_rate;
//@C
//\verb+_slow_rate+ is the measured rate of frame processed in the slow thread.
//@
        float               _slow_rate;

//@C
//\verb+_offline+ is the flag indicating that the frames are loaded from a file instead that
// from a shared memory
//@
        static bool _offline;


//@C
//\verb+_framesHistory+ is the object able to load frames from the file
//@
		IFramesHistory* _framesHistory;

        // dump flag related
        volatile long     _dumpAlarmHash;
        boost::mutex      _mutex;

    
};


#endif
