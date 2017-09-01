// JoeCtrl.h header file

#include <vector>
#include <string>

#include "stdconfig.h"

typedef struct
{
    std::string name;
    int readout_speed;
    int binx;
    int biny;
    float32 delay_base;
    float32 delay_inc;
    int black_levels[4];  // This should be long as the highest no. of black level possible
} littlejoe_program;

typedef struct
{
   std::string name;
   std::string control_filename;
   std::string pattern_filename;
   std::vector<littlejoe_program> programs;
} littlejoe_programset;


int DumpProgramset( littlejoe_programset programset);

//@Class: JoeCtrl
//
// Extends an AOApp to control a SciMeasure CCD

class JoeCtrl: public AOApp {

   public:

      JoeCtrl( int argc, char **argv) throw (AOException);

protected:
   void Create(void) throw (AOException);

   int SetupNetwork(void);
   int ReadJoeTemps(void);			// Refresh MsgD-RTDB variables about Joe's temperature
   int ReprogramJoe( int force=0);
   int FirstJoeConfig(void);

   // local FSM 
   int DoFSM(void);

   int Start(void);
   int Stop(void);

   int GetProgramPos( int speed, int xbin, int ybin, int *need_upload, bool use_memory=true);
   int UpdateJoeMemory( unsigned int uploaded_file);
   int LoadJoeDiskFiles(void);
   int EraseLocalMemory(void);
   int SetLocalMemory( unsigned int programset_num);

   int ComputeFramerate();
   int ChangeFramerate( double framerate, int bestspeed=1);
   littlejoe_program *GetProgram( int speed, int xbin, int ybin);
   littlejoe_program *GetProgram( string name);
   int ExposeSpeeds();
   int ShowClampSample();

   int SetGain(int g);
   
   int insert_value( std::vector<int> &array, int value);

   littlejoe_program ReadProgram( Config_File &cfg);
   littlejoe_programset ReadProgramSet( Config_File &cfg);

protected:

   // VIRTUAL - Setups variables in RTDB
   void SetupVars(void);

   // VIRTUAL - Perform post-initialization settings
   void PostInit();

   // VIRTUAL - Run
   // switches the stage from one state to the other
   void Run();

   // VIRTUAL - StateChange
   // Used to invalidate CUR variables when losing connection
   void StateChange( int oldstate, int state);

   // RTDB handles
   static int EnableReqChanged( void *pt, Variable *var);
   static int XbinReqChanged( void *pt, Variable *var);
   static int YbinReqChanged( void *pt, Variable *var);
   static int SpeedReqChanged( void *pt, Variable *var);
   static int BlackReqChanged( void *pt, Variable *var);
   static int FrameRateReqChanged( void *pt, Variable *var);
   static int RepReqChanged( void *pt, Variable *var);
   static int ProgramReqChanged( void *pt, Variable *var);
   static int ClampReqChanged( void *pt, Variable *var);
   static int SampleReqChanged( void *pt, Variable *var);

   static int GainReqChanged( void *pt, Variable *var);
   
   // Get the complete path of a program/pattern file
   std::string getCompletePath( std::string filename);

   // TT modulation amplitude check
   int checkTTAmp();

protected:

   // CCD number (set from cfg file)
   int  _ccdNum; 

   // CCD name
   std::string _ccdName;

   // CCD network address
   std::string _ccdNetAddr;
   int         _ccdNetPort;

   // CCD parameters
   int          _ccdDx;
   int          _ccdDy;
   unsigned int _ccdNumSpeeds;
   int          _maxNumSpeeds;
   int          _maxNumBins;
   std::vector<int>  _ccdXbins;
   std::vector<int>  _ccdYbins;
   std::vector<int>  _ccdSpeeds;
   std::vector<int>  _ccdBlacks;
   int          _ccdBlacksNum;
   int          _minRep;
   int          _maxRep;

   // Default parameters
   int _ccdDefaultXbin;
   int _ccdDefaultYbin;
   int _ccdDefaultSpeed;
   int _ccdDefaultBlack;

   int _ccdDefaultClamp;
   int _ccdDefaultSample;


   //Gain
   int _ccdGain;
   
   // Struct representing LittleJoe's internal memory
   littlejoe_programset memory;
   int _startProgramSet;

   // Struct array representing files that can be uploaded to LittleJoe (each one is a memory dump)
   std::vector<littlejoe_programset> ondisk;
   std::vector<string> program_names;

   // RTDB variables

   RTDBvar var_name, var_status, var_errmsg, var_enable_cur, var_enable_req;
   RTDBvar var_dx, var_dy, var_xbins, var_ybins, var_speeds;
   RTDBvar var_xbin_cur, var_ybin_cur, var_speed_cur, var_black_cur;
   RTDBvar var_xbin_req, var_ybin_req, var_speed_req, var_black_req;
   RTDBvar var_temps, var_framerate_cur, var_framerate_req;
   RTDBvar var_rep_cur, var_rep_req;
   RTDBvar var_fanReq;
   RTDBvar var_programs, var_program_cur, var_program_req;

   RTDBvar var_gain_cur, var_gain_req;
   RTDBvar var_ttamp_cur, var_ttamp_error;

   RTDBvar var_clamp_cur, var_clamp_req;
   RTDBvar var_sample_cur, var_sample_req;

   
   // Temperature thresholds for fan on/off
   int _fanCtrlActive;
   int _fanOnTemp;
   int _fanOffTemp;

   // Temperature logger

   Logger *_tempsLogger;

};

