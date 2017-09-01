//+File: JoeCtrl.c
//
// Control program for LittleJoe camera CCD
//-

#ifdef _WIN32
#include "winsock2.h"
typedef unsigned int socklen_t;
#undef NO_ERROR
#endif

#include <vector>
#include <string>

extern "C" {

#include <stdio.h>
#include <string.h>             // strncmp()
#include <stdlib.h>             // atoi()
#include <stdarg.h>
#include <time.h>               // time(), localtime()
//#include <unistd.h>             // sleep()
#include <pthread.h>

// Joe related libs
#include "hwlib/netseriallib.h"
#include "hwlib/joelib.h"


// General Supervisor libs
#include "base/thrdlib.h"
}

#include "AOApp.h"
#include "AOStates.h"
#include "JoeCtrl.h"


#ifndef __VISAO_NO_MAIN
//4 Nov 2010: VISAO_NO_MAIN flag added to turn off compilation of main()
//            to allow derivations of JoeCtrl 
//            by Jared R. Males, Steward Observatory

// Program identification

int VersMajor = 1;
int VersMinor = 0;
const char *Date = "Dec 2004";

#endif //__VISAO_NO_MAIN

int debug=0;



#define JOE_ADDR_LEN            15                      // Length of idrive string address
#define ERRMSG_LEN              32                      // Length of an error message

int cur_xbin, cur_ybin, cur_speed;
int default_xbin, default_ybin, default_speed, default_black;

#ifndef __VISAO_NO_MAIN
//4 Nov 2010: VISAO_NO_MAIN flag added to turn off compilation of main()
//            to allow derivations of JoeCtrl 
//            by Jared R. Males, Steward Observatory

//+Entry  help
//
// help prints an usage message
//

void help()
{
        printf("\nJoeCtrl - Vers. %d.%d     A. Puglisi, %s\n\n", VersMajor, VersMinor, Date);
        printf("Usage: JoeCtrl [-v] [-h] [-f config file] [<server>]\n\n");
        printf("       -f       configuration file (defaults to \"config\")\n");
        printf("       -v       verbose\n");
        printf("       -i       interactive mode\n");
        printf("       <server> Server numeric address (default: \"127.0.0.1\")\n");
        printf("       -h       prints this message\n\n");
}

#endif //__VISAO_NO_MAIN

JoeCtrl::JoeCtrl( int argc, char **argv) throw (AOException) : AOApp( argc, argv)
{
   Create();
}

void JoeCtrl::Create() throw (AOException)
{
   try {
    _ccdName  = (std::string) ConfigDictionary()["ccdName"]; 
    _ccdNetAddr = (std::string) ConfigDictionary()["ccdNetAddr"];
    _ccdNetPort = ConfigDictionary()["ccdNetPort"];
    _ccdDx    = ConfigDictionary()["ccdXdim"];
    _ccdDy    = ConfigDictionary()["ccdYdim"];
    _ccdDefaultXbin = ConfigDictionary()["ccdDefaultXbin"];
    _ccdDefaultYbin = ConfigDictionary()["ccdDefaultYbin"];
    _ccdDefaultSpeed = ConfigDictionary()["ccdDefaultSpeed"];
    _ccdDefaultBlack = ConfigDictionary()["ccdDefaultBlack"];
    _ccdBlacksNum    = ConfigDictionary()["ccdBlacksNum"];
    _minRep          = ConfigDictionary()["minRep"];
    _maxRep          = ConfigDictionary()["maxRep"];
    _maxNumSpeeds    = ConfigDictionary()["maxNumSpeeds"];
    _maxNumBins      = ConfigDictionary()["maxNumBins"];
    _startProgramSet = ConfigDictionary()["startProgramSet"];
    _fanCtrlActive   = ConfigDictionary()["fanCtrlActive"]; 
    if (_fanCtrlActive) {
        _fanOnTemp       = ConfigDictionary()["fanOnTemp"];
        _fanOffTemp      = ConfigDictionary()["fanOffTemp"];
    }
   } catch (Config_File_Exception &e) {
     printf("%s\n", e.what().c_str());
    _logger->log( Logger::LOG_LEV_FATAL, "%s", e.what().c_str());
     throw AOException("Fatal: Missing configuration data");
   }


    _ccdBlacks.resize( _ccdBlacksNum);

    _tempsLogger = Logger::get("TEMPERATURES", Logger::LOG_LEV_INFO, "TELEMETRY");
    
    // Create the specific LittleJoe arrays
    LoadJoeDiskFiles();
    EraseLocalMemory();

    _ccdGain = 0;

}


void JoeCtrl::PostInit()
{
   // ----------- Initial status
   setCurState(STATE_NOCONNECTION);
   var_errmsg.Set("Starting up");
}


void JoeCtrl::Run()
{   
        int iterations=0;
        while(!TimeToDie())
            {
            try {
              // Do iterative step of the controller
              DoFSM();

              // Read temperatures every now and then
              if (iterations++ % 5 == 0)
                  ReadJoeTemps();
            }
            catch (AOException &e) {
               _logger->log( Logger::LOG_LEV_ERROR, "Caugth exception: %s", e.what().c_str());
            }
          }

        exit(0);
}

//@Function
//
// DoFSM()                      perform controller functions and manage states
//
// Switches the controller from one state to another. States can be changed asynchronously
// from this thread or the listening thread, and this function will properly react to a new state.
//
// Many states include a msleep() function to slow down the thread when immediate action is not necessary
//@

int JoeCtrl::DoFSM(void)
{
        int status;
        int stat = NO_ERROR;

        status = getCurState(); 

        // Always check if we can reach the LittleJoe
        // If not, reset everything and start again
        if (status != STATE_NOCONNECTION)
                if (TestJoeLink() != NO_ERROR)
                        {
                        setCurState(STATE_NOCONNECTION);
                        EraseLocalMemory();
                        }

        status = getCurState(); 

        switch(status)
                {
                // At first, try to start network
                case STATE_NOCONNECTION:
                stat = SetupNetwork();
                if (stat == NO_ERROR)
                        setCurState(STATE_CONNECTED);
                break;

                // After network start, configure LittleJoe
                // Configuration ensures at the end that LittleJoe is turned off
                case STATE_CONNECTED:
                stat = FirstJoeConfig();
                if (stat == NO_ERROR)
                     Stop();
                break;

                // When ready to go, do nothing
                case STATE_READY:
                case STATE_OPERATING:
                msleep(4000);
                break;

                // Unknown states should not exist
                default:
                msleep(1000);
                break;
                }

        // Always set state again (for watchdogs)
        setCurState(getCurState());

        // Return the generated error code, if any
        return stat;
}


//@Function
//
// Stop                        Stop CCD integration
//
// Stops Littlejoe integration. Changes to STATE_READY.
//@

int JoeCtrl::Stop()
{
        int stat;

        _logger->log( Logger::LOG_LEV_DEBUG, "Stopping CCD");
        stat = StopJoe();
        if (stat == NO_ERROR)
                {
                setCurState(STATE_READY);
                var_enable_cur.Set(0);
                }

        return stat;
}

//@Function
//
// SwitchOn                             change from "switched off" to "ready"
//@

int JoeCtrl::Start()
{
        int stat;
        _logger->log( Logger::LOG_LEV_DEBUG, "Starting CCD");

        stat = StartJoe();
        if (stat == NO_ERROR)
                {
                setCurState(STATE_OPERATING);
                var_enable_cur.Set(1);
                }
 
        return stat;
}


// +Entry
//
// SetupVars      create the MsgD-RTDB variables for this CCD
//
// Creates all the variables needed for CCD control.
// Register for notification for all "global" variables

void JoeCtrl::SetupVars()
{

   try {
      var_name = RTDBvar( MyFullName(), "NAME", NO_DIR, CHAR_VARIABLE, _ccdName.size()+1);
      var_name.Set(_ccdName);


      var_enable_req = RTDBvar( MyFullName(), "ENABLE", REQ_VAR);
      var_enable_cur = RTDBvar( MyFullName(), "ENABLE", CUR_VAR);

      var_enable_cur.Set(0);

      var_errmsg = RTDBvar( MyFullName(), "ERRMSG", NO_DIR, CHAR_VARIABLE, ERRMSG_LEN);

      var_dx = RTDBvar( MyFullName(), "DX");
      var_dy = RTDBvar( MyFullName(), "DY");

      var_xbins = RTDBvar( MyFullName(), "XBINS", NO_DIR, INT_VARIABLE, _maxNumBins);
      var_ybins = RTDBvar( MyFullName(), "YBINS", NO_DIR, INT_VARIABLE, _maxNumBins);
      var_speeds = RTDBvar( MyFullName(), "SPEEDS", NO_DIR, INT_VARIABLE, _maxNumSpeeds);

      var_xbin_req = RTDBvar( MyFullName(), "XBIN", REQ_VAR);
      var_ybin_req = RTDBvar( MyFullName(), "YBIN", REQ_VAR);
      var_xbin_cur = RTDBvar( MyFullName(), "XBIN", CUR_VAR);
      var_ybin_cur = RTDBvar( MyFullName(), "YBIN", CUR_VAR);

      var_speed_cur = RTDBvar( MyFullName(), "SPEED", CUR_VAR);
      var_speed_req = RTDBvar( MyFullName(), "SPEED", REQ_VAR);

      var_black_cur = RTDBvar( MyFullName(), "BLACK", CUR_VAR, INT_VARIABLE, _ccdBlacks.size());
      var_black_req = RTDBvar( MyFullName(), "BLACK", REQ_VAR, INT_VARIABLE, _ccdBlacks.size());

      var_framerate_cur = RTDBvar( MyFullName(), "FRMRT", CUR_VAR, REAL_VARIABLE);
      var_framerate_req = RTDBvar( MyFullName(), "FRMRT", REQ_VAR, REAL_VARIABLE);

      var_rep_cur = RTDBvar( MyFullName(), "REP", CUR_VAR);
      var_rep_req = RTDBvar( MyFullName(), "REP", REQ_VAR);

      var_gain_cur = RTDBvar( MyFullName(), "GAIN", CUR_VAR);
      var_gain_req = RTDBvar( MyFullName(), "GAIN", REQ_VAR);
      var_gain_cur.Set( 0, 0, FORCE_SEND);
      var_gain_req.Set( 0, 0, FORCE_SEND);
      
      var_temps = RTDBvar( MyFullName(), "TEMPS", NO_DIR, INT_VARIABLE, 3);

      var_fanReq = RTDBvar( (std::string)ConfigDictionary()["fanReqVar"], INT_VARIABLE, 1, false);

      var_programs = RTDBvar( MyFullName(), "PROGRAMS", NO_DIR, CHAR_VARIABLE, 1024);
      var_program_cur =  RTDBvar( MyFullName(), "PROGRAM", CUR_VAR, CHAR_VARIABLE, 128);
      var_program_req =  RTDBvar( MyFullName(), "PROGRAM", REQ_VAR, CHAR_VARIABLE, 128);

      var_ttamp_cur = RTDBvar("ttctrl.L", "AMP_RADIUS", CUR_VAR, REAL_VARIABLE, 1, false);
      var_ttamp_error = RTDBvar( MyFullName(), "TTAMP_ERROR", NO_DIR, INT_VARIABLE, 1);
      var_ttamp_error.Set(0);

      var_clamp_cur = RTDBvar( MyFullName(), "CLAMP", CUR_VAR);
      var_clamp_req = RTDBvar( MyFullName(), "CLAMP", REQ_VAR);

      var_sample_cur = RTDBvar( MyFullName(), "SAMPLE", CUR_VAR);
      var_sample_req = RTDBvar( MyFullName(), "SAMPLE", REQ_VAR);


      string prog_names = "";
      for (vector<string>::iterator iter = program_names.begin(); iter != program_names.end(); iter++) 
          prog_names += (*iter)+";";
      var_programs.Set(prog_names);

      var_dx.Set( _ccdDy);
      var_dy.Set( _ccdDy);

      // Fill the XBINS and YBINS array up to the maximum length
      unsigned int i;
      vector<int> binning;
      for ( i=0; i < _ccdXbins.size(); i++)
         binning.push_back( _ccdXbins[i]);
      for ( i =  _ccdXbins.size(); i<  _maxNumBins; i++)
         binning.push_back(-1);
      var_xbins.Set( binning);

      binning.clear();
      for ( i=0; i < _ccdYbins.size(); i++)
         binning.push_back( _ccdYbins[i]);
      for ( i =  _ccdYbins.size(); i<  _maxNumBins; i++)
         binning.push_back(-1);

      var_ybins.Set( binning);

      Notify( var_enable_req, EnableReqChanged);
      Notify( var_xbin_req, XbinReqChanged);
      Notify( var_ybin_req, YbinReqChanged);
      Notify( var_speed_req, SpeedReqChanged);
      Notify( var_black_req, BlackReqChanged);
      Notify( var_framerate_req, FrameRateReqChanged);
      Notify( var_rep_req, RepReqChanged);
      Notify( var_program_req, ProgramReqChanged);

      Notify( var_gain_req, GainReqChanged);
      Notify( var_clamp_req, ClampReqChanged);
      Notify( var_sample_req, SampleReqChanged);
      
      setCurState(STATE_NOCONNECTION);

   }  catch (AOVarException &e) {
      _logger->log(Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
      throw AOException("Error creating RTDB variables");
   }

}

//@Method StateChange
//
// Automatically called by AOApp when a state change occurs.
// Used here to invalidate "CUR" variables when losing connections
//@

void JoeCtrl::StateChange( int oldstate, int state)
{
   if ((state == STATE_NOCONNECTION) && (oldstate != STATE_NOCONNECTION))
         {
         var_enable_cur.Set( 0,0, FORCE_SEND);
         var_xbin_cur.Set( 0, 0, FORCE_SEND);
         var_ybin_cur.Set( 0, 0, FORCE_SEND);
         var_speed_cur.Set( 0, 0, FORCE_SEND);
         //var_framerate_cur.Set( 0, 0, FORCE_SEND);
         var_rep_cur.Set( 0, 0, FORCE_SEND);

         for (unsigned int i=0; i< _ccdBlacks.size(); i++)
            var_black_cur.Set( 0, i, NO_SEND);
         var_black_cur.Send();
         }
}


        // -------------------------
        // Disable/Enable request
        //
        // Start or stop the Joe camera


int JoeCtrl::EnableReqChanged( void *pt, Variable *var)
{
    int enable, enabled, stat;
    JoeCtrl *ctrl = (JoeCtrl *)pt;


    stat = NO_ERROR;
    enable = var->Value.Lv[0];

    Logger::get()->log( Logger::LOG_LEV_INFO, "Enable request: %d", enable);
    ctrl->var_enable_req.Set(enable, 0, NO_SEND);  // Mirror value locally
    ctrl->var_enable_cur.Get(&enabled);      // Current value

    // Ignore invalid values
    if ((enable != 0) && (enable != 1))
            return NO_ERROR;

    // Switch on if requested
    if ((enable == 1) && (enabled == 0))
            stat = ctrl->Start();

    // Switch off if requested
    if ((enable == 0) && (enabled == 1))
            stat = ctrl->Stop();

    if (stat == NO_ERROR)
       ctrl->var_enable_cur.Set(enable);

    Logger::get()->log( Logger::LOG_LEV_INFO, "Enable completed");
    return stat;
}

        // -------------------------------
        // Pixel readout speed (CCDnSPEED variable) and binning (CCDnXBIN, CCDnYBIN)
        //
        // Change the Joe program to the one matching the requested speed and binning

        //
        //  ReprogramJoe();    // Questo prende tutte le var_speed, var_xbin ecc. e prova
                               // a riconfigurare LittleJoe, SE SONO VALIDE.
                               // In caso di successo updata tutte le variabili
                               // E' una funzione intelligente che carica nuovi programmi,
                               // si ricorda quelli caricati eccetera.


int JoeCtrl::XbinReqChanged( void *pt, Variable *var)
{
   JoeCtrl *ctrl = (JoeCtrl *)pt;

   int stat = ctrl->checkTTAmp();
   if (stat != NO_ERROR)
       return stat;

   ctrl->var_xbin_req.Set( (int) var->Value.Lv[0], 0, NO_SEND);
   return ctrl->ReprogramJoe();
}
int JoeCtrl::YbinReqChanged( void *pt, Variable *var)
{
   JoeCtrl *ctrl = (JoeCtrl *)pt;

   int stat = ctrl->checkTTAmp();
   if (stat != NO_ERROR)
       return stat;

   ctrl->var_ybin_req.Set( (int)var->Value.Lv[0], 0, NO_SEND);
   return ctrl->ReprogramJoe();
}
int JoeCtrl::SpeedReqChanged( void *pt, Variable *var)
{
   JoeCtrl *ctrl = (JoeCtrl *)pt;

   int stat = ctrl->checkTTAmp();
   if (stat != NO_ERROR)
       return stat;

   ctrl->var_speed_req.Set( (int) var->Value.Lv[0], 0, NO_SEND);
   return ctrl->ReprogramJoe();
}

        // --------------------------------------
        // Black levels (CCDnBLACKS variable)
        //

int JoeCtrl::BlackReqChanged( void *pt, Variable *var)
{
   int black,cur_black,stat;
   JoeCtrl *ctrl = (JoeCtrl *)pt;
   unsigned int i;

   // Set the new black levels, if different from the current ones
   for (i=0; i< ctrl->_ccdBlacks.size(); i++)
           {
           black = var->Value.Lv[i];
           ctrl->var_black_cur.Get(i, &cur_black);

           Logger::get()->log( Logger::LOG_LEV_DEBUG, "Setting black level %d to %d", i, black);
           if (black != cur_black)
                   {
                   if ((stat=SetJoeBlack( i, black)) != NO_ERROR)
                           return stat;
                   }
           ctrl->var_black_cur.Set( black, (int)i,FORCE_SEND);
           }

   return NO_ERROR;
}

int JoeCtrl::ClampReqChanged( void *pt, Variable *var)
{ 
   int stat;
   JoeCtrl *ctrl = (JoeCtrl *)pt;
   int level = var->Value.Lv[0];

   stat = SetJoeClampLevel( level);
   if (stat == NO_ERROR)
      ctrl->var_clamp_cur.Set(level);

   return NO_ERROR;
}

int JoeCtrl::SampleReqChanged( void *pt, Variable *var)
{ 
   int stat;
   JoeCtrl *ctrl = (JoeCtrl *)pt;
   int level = var->Value.Lv[0];

   stat = SetJoeSampleLevel( level);
   if (stat == NO_ERROR)
      ctrl->var_sample_cur.Set(level);

   return NO_ERROR;
}


        // --------------------------------------
        // Repetitions (CCDnREP variable)
        //
       
int JoeCtrl::RepReqChanged( void *pt, Variable *var)
{ 
   int stat;
   JoeCtrl *ctrl = (JoeCtrl *)pt;
   int rep = var->Value.Lv[0];

   // Prevent changes when TT modulation amplitude is not zero
   stat = ctrl->checkTTAmp();
   if (stat != NO_ERROR)
       return stat;
    

   if ((rep>= ctrl->_minRep) && (rep<= ctrl->_maxRep))
           {
           stat = SetJoeRepetitions( rep);
           if (stat == NO_ERROR)
                   {
                   ctrl->var_rep_cur.Set(rep);
                   stat = ctrl->ComputeFramerate();
                   }
           return stat;
           }
   else
           return JOE_OUT_OF_RANGE_ERROR;
}

        // --------------------------------------
        // Direct program selection (PROGRAMS_CUR variable)
        //

int JoeCtrl::ProgramReqChanged( void *pt, Variable *var)
{
   JoeCtrl *ctrl = (JoeCtrl *)pt;

   int stat = ctrl->checkTTAmp();
   if (stat != NO_ERROR)
       return stat;
   
   string name = string(var->Value.Sv, var->H.NItems);
   littlejoe_program *program = ctrl->GetProgram(name);

   ctrl->var_xbin_req.Set( program->binx);
   ctrl->var_ybin_req.Set( program->biny);
   ctrl->var_speed_req.Set( program->readout_speed);

   stat = ctrl->ReprogramJoe();
   if (stat == NO_ERROR)
   	ctrl->var_program_cur.Set(name); 

   return stat;
}

int JoeCtrl::GainReqChanged( void *pt, Variable *var)
{
   JoeCtrl *ctrl = (JoeCtrl *)pt;

   return ctrl->SetGain(var->Value.Lv[0]);

}

        // ------------------------------
        // Framerate (CCDnFRMRT variable)

int JoeCtrl::FrameRateReqChanged( void *pt, Variable *var)
{
   JoeCtrl *ctrl = (JoeCtrl *)pt;
   return ctrl->ChangeFramerate( var->Value.Dv[0], 0);
}

//+Function: ChangeFramerate
//
// Changes the framerate selecting the most convenient readout speed
// and repetition number, without changing the binning.
// Updates the CURSPEED, CURREP e CURFRMRT variables.
// Can be called at any time.
//
// If bestspeed is set, it will search for the most convenient readout. Otherwise,
// the readout is not changed.
//-

int JoeCtrl::ChangeFramerate( double framerate, int bestspeed)
{
        int xbin, ybin, cur_speed;
        int found_speed=-1;
        int found_set=-1;
        int found_program=-1;
        unsigned int i,j;

        // Safety check (avoid division by zero and nonsensical framerates)
        if (framerate <=0)
                return VALUE_OUT_OF_RANGE_ERROR;

        // Prevent changes when TT modulation amplitude is not zero
        int stat = checkTTAmp();
        if (stat != NO_ERROR)
            return stat;
    

        var_xbin_cur.Get(&xbin);
        var_ybin_cur.Get(&ybin);
        var_speed_cur.Get(&cur_speed);

        _logger->log( Logger::LOG_LEV_DEBUG, "Requested frequency: %5.2f", framerate);

        for (i=0; i<ondisk.size(); i++)
                for (j=0; j< ondisk[i].programs.size(); j++)
                        {
                        // Skip programs with the wrong binning
                        if ((xbin != ondisk[i].programs[j].binx) ||
                            (ybin != ondisk[i].programs[j].biny))
                                continue;

                        // Delays are in microseconds
                        double mintime = ondisk[i].programs[j].delay_base;
                        double maxtime = mintime + ondisk[i].programs[j].delay_inc * 65535;

                        double max_framerate = 1e6/mintime;
                        double min_framerate = 1e6/maxtime;

                        _logger->log( Logger::LOG_LEV_DEBUG, "Program %d %d: max,min = %5.2f, %5.2f", i, j, max_framerate, min_framerate);


                        // Skip programs not able to reach the desired framerate
                        if (bestspeed)
                           if ((framerate < min_framerate) || (framerate > max_framerate))
                                continue;

                        int readout_speed = ondisk[i].programs[j].readout_speed;

                        // Skip programs with a different readout speed from our one (if bestspeed is not set)
                        if ((!bestspeed) && (readout_speed != cur_speed))
                           continue;

                        // Skip programs with a readout speed equal or bigger than the one already found
                        if (found_program>=0)
                                if ( readout_speed >= ondisk[found_set].programs[found_program].readout_speed)
                                        continue;

                        // Program passed all tests
                        found_set = i;
                        found_program = j;
                        found_speed = readout_speed;

                        _logger->log( Logger::LOG_LEV_DEBUG, "Found program: %d %d %d", found_set, found_program, found_speed);
                        }

        // See if we have found one
        if (found_program <0)
                return VALUE_OUT_OF_RANGE_ERROR;

        // Calculate new repetition number
        double base = ondisk[found_set].programs[found_program].delay_base;
        double inc = ondisk[found_set].programs[found_program].delay_inc;
        double goal = 1e6/framerate;

        goal = goal - base;
        goal = goal / inc;

        // May happen if the requested speed is not able to go so fast...
        if (goal<0)
           goal=0;

        // Apply new settings (will call the appropriate handlers in order...)
        var_speed_req.Set(found_speed, 0, FORCE_SEND);
        var_rep_req.Set((int)goal, 0, FORCE_SEND);

        _logger->log( Logger::LOG_LEV_DEBUG, "Base %5.2f, inc %5.2f, goal %5.2f", base, inc, goal);
        _logger->log( Logger::LOG_LEV_DEBUG, "Setting speed %d, repetitions %d", found_speed, (int)goal);

        ShowClampSample();

        // Set clamp and sample levels for 2KHz mode
        if (found_speed == 5000) {
           int clamp  = ConfigDictionary()["clampLevel"];
           int sample = ConfigDictionary()["sampleLevel"];

           var_clamp_req.Set(clamp, 0, FORCE_SEND);
           var_sample_req.Set(sample, 0, FORCE_SEND);
        }

        return NO_ERROR;
}

int JoeCtrl::ShowClampSample() {

   int clamp  = GetJoeClampLevel();
   int sample = GetJoeSampleLevel();

   var_clamp_cur.Set( clamp, 0, FORCE_SEND);
   var_sample_cur.Set( sample, 0, FORCE_SEND);

   printf("Detected clamp and sample values: %d and %d\n", clamp, sample);
 
   return NO_ERROR;
}


//+Function: ComputeFramerate
//
// Computes the current framerate from the current state of LittleJoe control variables.
// Refreshes the MsgD-RTDB CCDnCURFRMRT variable.
//-

int JoeCtrl::ComputeFramerate()
{
        int xbin, ybin, speed, rep;
        double framerate;

        var_speed_cur.Get(&speed);
        var_xbin_cur.Get(&xbin);
        var_ybin_cur.Get(&ybin);
        var_rep_cur.Get(&rep);

        littlejoe_program *program = GetProgram(speed, xbin, ybin);
        if (!program)
                return VALUE_OUT_OF_RANGE_ERROR;

        framerate = 1.0e6/ (program->delay_base + program->delay_inc * rep);

        _logger->log( Logger::LOG_LEV_DEBUG, "Delay base: %f - Delay_inc: %f - Rep: %d - framerate: %f", program->delay_base, program->delay_inc, rep, framerate);

        var_framerate_cur.Set(framerate, 0, FORCE_SEND);
        return NO_ERROR;
}

//+Entry: ExposeSpeeds
//
// Fills the CCDxxSPEEDS variable with each possible
// speed of the current binning, as available in the various programsets

int JoeCtrl::ExposeSpeeds()
{
   int xbin, ybin;
   unsigned int i, j;

   var_xbin_cur.Get(&xbin);
   var_ybin_cur.Get(&ybin);

   int *speeds = new int[ _maxNumSpeeds];
   memset( speeds, 0, sizeof(int) * _maxNumSpeeds);

   int counter=0;
   for (i=0; i<ondisk.size(); i++)
       for (j=0; j< ondisk[i].programs.size(); j++) {
         _logger->log( Logger::LOG_LEV_DEBUG, "Checking program %dx%d, %d kpixel/sec", ondisk[i].programs[j].binx, ondisk[i].programs[j].biny,  ondisk[i].programs[j].readout_speed);
         if ((ondisk[i].programs[j].binx == xbin) &&
             (ondisk[i].programs[j].biny == ybin)) {

             // Prevent inserting duplicate speeds
             int k,found=0;
             if (counter>0) {
                for (k=0; k<counter; k++)
                   if (speeds[k] == ondisk[i].programs[j].readout_speed)
                      found=1;
                if (found)
                      continue;
            }

            speeds[ counter++ ] = ondisk[i].programs[j].readout_speed;
            if (counter  >= _maxNumSpeeds) goto done;		// Prevent buffer overrun
         }
       }

done:
   var_speeds.Set(speeds);

   delete speeds;
   return NO_ERROR;
}


int JoeCtrl::SetGain(int g)
{
   if(g < 0 || g > 3) return -1;

  
   var_gain_req.Set( g, 0, NO_SEND);

   
   return 0;
}


//+Entry: ReprogramJoe
//
// This function switches between the different LittleJoe programs (waveforms)
//
// If the requested parameters are not valid, it fails silently
//
// Watch out: this routine calls itself recursively when an upload if needed. Always take this
// into consideration with changing something.
//
// LittleJoe can only be reprogrammed in the SWITCHEDOFF state. If a request is made when
// not in this state it will be refused, unless the <force> flag is nonzero. In this case
// the routine will attempt to reprogram the LittleJoe anyway (this is necessary, for example,
// for the first configuration)
//-

int JoeCtrl::ReprogramJoe( int force)
{
        int stat, need_upload;
        int speed, xbin, ybin, pos;
        int cur_speed, cur_xbin, cur_ybin;
        int req_gain;
        unsigned int i;
        
        var_speed_req.Get(&speed);
        var_xbin_req.Get(&xbin);
        var_ybin_req.Get(&ybin);
        var_gain_req.Get(&req_gain);

        if( req_gain < 0 || req_gain > 3 ) req_gain = 0;
        
        var_speed_cur.Get(&cur_speed);
        var_xbin_cur.Get(&cur_xbin);
        var_ybin_cur.Get(&cur_ybin);

        
        _logger->log( Logger::LOG_LEV_INFO, "SetJoeProgram(): starting reprogram with bin %dx%d, speed %d, gain %d", xbin, ybin, speed, req_gain);

        // Nothing to do?
        if ((speed == cur_speed) && (xbin == cur_xbin) && (ybin == cur_ybin) && (req_gain == _ccdGain))
           {
           _logger->log( Logger::LOG_LEV_INFO, "SetJoeProgram(): current settings are OK, nothing to do.");
           return NO_ERROR;
           }


        // Stop ccd integration
        stat = Stop();
        if (stat != NO_ERROR)
            return stat;

        _logger->log( Logger::LOG_LEV_INFO, "SetJoeProgram(): integration stopped");

        _logger->log( Logger::LOG_LEV_DEBUG, "Checking program %d, %dx%d", speed, xbin, ybin);
                                
        // See if we already have the program in memory, or if we have to upload it
        pos = GetProgramPos( speed, xbin, ybin, &need_upload);
        _logger->log( Logger::LOG_LEV_DEBUG, "Pos in memory=%d, need_upload = %d", pos, need_upload);

        // If not in current memory
        if (pos == -1)
                {
                // If not in disk files
                if (need_upload == -1)
                        {
                        _logger->log( Logger::LOG_LEV_WARNING, "Program %d, %dx%d is not valid", speed, xbin, ybin);
                        return NO_ERROR;
                        }

                // If in disk files, upload the new program set
                _logger->log( Logger::LOG_LEV_INFO, "Program needs uploading of program set %d", need_upload);

                // Check files
                string control_filename = getCompletePath(ondisk[need_upload].control_filename);
                string pattern_filename = getCompletePath(ondisk[need_upload].pattern_filename);

                if (!Utils::fileExists(control_filename)) {
                	_logger->log( Logger::LOG_LEV_ERROR, "File not found: %s", control_filename.c_str());
                        return FILE_ERROR;
                }

                if (!Utils::fileExists(pattern_filename)) {
                	_logger->log( Logger::LOG_LEV_ERROR, "File not found: %s", pattern_filename.c_str());
                        return FILE_ERROR;
                }

                setCurState(STATE_CONFIGURING);

                _logger->log( Logger::LOG_LEV_INFO, "Starting control file upload (%s)", control_filename.c_str());
                if ((stat = SendXmodemFile( "XMC 1", (char *) getCompletePath( ondisk[need_upload].control_filename).c_str())) != NO_ERROR)
                        return stat;

                _logger->log( Logger::LOG_LEV_INFO, "Starting pattern file upload (%s)", pattern_filename.c_str());
                if ((stat = SendXmodemFile( "XMP 1", (char *) getCompletePath( ondisk[need_upload].pattern_filename).c_str())) != NO_ERROR)
                        return stat;

                UpdateJoeMemory( need_upload);
                _logger->log( Logger::LOG_LEV_INFO, "Upload complete");
                //Stop();    // Prevent unwanted starting of ccd...

                // Call again this routine, now it will find the program inside the LittleJoe memory
                return ReprogramJoe( force);
                }
                
        // Program found in current memory
        _logger->log( Logger::LOG_LEV_INFO, "Program in memory at position %d", pos);

        if ((stat = SetJoeProgram(req_gain*8 + pos)) != NO_ERROR)
           {
           _logger->log( Logger::LOG_LEV_ERROR, "Error in SetJoeProgram(): %d", stat);
           return stat;
           }

        // Update status variables
        var_xbin_cur.Set(xbin, 0, FORCE_SEND);
        var_ybin_cur.Set(ybin, 0, FORCE_SEND);
        var_speed_cur.Set(speed, 0, FORCE_SEND);

        _ccdGain = req_gain;
        var_gain_cur.Set(_ccdGain, 0, FORCE_SEND);

        
        // Set default black levels, if specified
        littlejoe_program *program = GetProgram(speed, xbin, ybin);

        if (program)
           for (i=0; i< _ccdBlacks.size(); i++)
              if (program->black_levels[i] >=0)
                  if ((stat = SetJoeBlack(i, program->black_levels[i])) != NO_ERROR)
                     _logger->log( Logger::LOG_LEV_ERROR, "Error in SetJoeBlacklevels(): (%d) %s", stat, lao_strerror(stat));

        // Update black level variables
        for (i=0; i< _ccdBlacks.size(); i++)
           {
           int level = GetBlackLevel(i);
           if (level>=0)
               var_black_cur.Set( GetBlackLevel(i), (int)i, NO_SEND);
           else
               return level;
           }

        var_black_cur.Send();

        // Since the program also stores run state, fix it if needed
        int enable;
        var_enable_req.Get(&enable);
        _logger->log( Logger::LOG_LEV_INFO, "SetJoeProgram(): resuming enabled state %d", enable);
        if (enable) {
          stat = Stop();
          stat = Start();
        }
        else
           stat = Stop();
        if (stat != NO_ERROR)
            return stat;

        // Show new available speeds
        ExposeSpeeds();

        // Show clamp and sample levels
        ShowClampSample();

        // Update shown framerate
        return ComputeFramerate();
}

// +Entry
//
// SetupNetwork       setup network connection with LittleJoe
//
// This function sets up the network connection with the LittleJoe CCD Camera.
// After the network is up, the communication is tested with the TestJoe() function.
//
// Return value: zero or a negative error code.

int JoeCtrl::SetupNetwork()
{
        int result, stat;

        // Close the previous connection, if any
        SerialClose();
        msleep(1000);

        _logger->log( Logger::LOG_LEV_INFO, "Connecting to %s:%d", (char *)_ccdNetAddr.c_str(), _ccdNetPort);
        // Setup serial/network interface
        if (( result = InitJoeLink( (char *)_ccdNetAddr.c_str(), _ccdNetPort)) != NO_ERROR)
                {
                _logger->log( Logger::LOG_LEV_ERROR, "Error configuring network: (%d) %s", result, lao_strerror(result));
                return NETWORK_ERROR;
                }

        _logger->log( Logger::LOG_LEV_INFO, "Network reconfigured OK");    

        // Now check our friend Joe
        stat = PLAIN_ERROR(CheckJoe());
        if (stat)
                {
                _logger->log( Logger::LOG_LEV_ERROR, "Error during LittleJoe testing: (%d) %s", stat, (char *)lao_strerror( stat));
                return stat;
                }

        return NO_ERROR;
}

//+Entry
//
// ReadJoeTemps()    read the LittleJoe CCD camera temperatures
//
// Read the CCD temperature from the serial interface and sets
// the corresponding variables in the MsgD-RTDB
//
// Return value: zero or a negative error code

int JoeCtrl::ReadJoeTemps()
{
        float t1, t2, t3;
        int temps[3];
        int stat;

        int state;
        state = getCurState();

        if (state == STATE_NOCONNECTION)
                return NO_ERROR;

        stat = GetJoeTemperature( &t1, &t2, &t3);
        if (stat != NO_ERROR)
                return stat;

        temps[0] = (int)t1;
        temps[1] = (int)t2;
        temps[2] = (int)t3;

        _tempsLogger->log( Logger::LOG_LEV_INFO, "%d %d %d", temps[0], temps[1], temps[2]);

        var_temps.Set(temps, FORCE_SEND);

        // Fan management
        if (_fanCtrlActive) {
             if (t1< _fanOffTemp)
                 var_fanReq.Set(0, 0, FORCE_SEND);
             if (t1> _fanOnTemp)
                 var_fanReq.Set(1, 0, FORCE_SEND);
	}

        return NO_ERROR;
}

//+Function
//
// FirstJoeConfig       Sets the LittleJoe controller in a known state
//

int JoeCtrl::FirstJoeConfig()
{
    int stat;
    unsigned int i;

        StopJoe();

        var_xbin_req.Set( default_xbin, 0, NO_SEND);
        var_ybin_req.Set( default_ybin, 0, NO_SEND);
        var_speed_req.Set( default_speed, 0, NO_SEND);

        for (i=0; i< _ccdBlacks.size(); i++)
        { 
                if ((stat=SetJoeBlack( i, default_black)) != NO_ERROR)
                    return stat;        

                var_black_cur.Set( default_black, (int)i, FORCE_SEND);
        }         

        ShowClampSample();

   return NO_ERROR;

}

std::string JoeCtrl::getCompletePath( std::string filename)
{
   // Get the path relative to the .conf file
   return ConfigDictionary().getDir()+"/"+filename;

}



//+Function GetProgram
//
// Finds the program with the specified characteristics and returns a pointer to it.
// Returns NULL if the program is not found.
//-

littlejoe_program *JoeCtrl::GetProgram( int speed, int xbin, int ybin)
{
        unsigned int i,j;

        for (i=0; i<ondisk.size(); i++)
                for (j=0; j< ondisk[i].programs.size(); j++)
                        if ((ondisk[i].programs[j].binx == xbin) &&
                            (ondisk[i].programs[j].biny == ybin) &&
                            (ondisk[i].programs[j].readout_speed == speed))
                                return &(ondisk[i].programs[j]);
                
        return NULL;
}

//+Function GetProgram
//
// Finds the program with the specified characteristics and returns a pointer to it.
// Returns NULL if the program is not found.
//-

littlejoe_program *JoeCtrl::GetProgram( string name)
{
        unsigned int i,j;

        for (i=0; i<ondisk.size(); i++)
                for (j=0; j< ondisk[i].programs.size(); j++)
			if (ondisk[i].programs[j].name == name)
                                return &(ondisk[i].programs[j]);

        return NULL;
}




//+Function
//
// GetProgramPos
//
// Returns the position into the LittleJoe controller memory of the specified program.
// If not found, returns -1 and puts into need_upload the file number to upload.
// If a file with the specified program is not found, need_upload will be -1 as well.
//
// If <use_memory> is set to false, it will not look in the current little joe memory,
// but just return the file number to upload.
//-


int JoeCtrl::GetProgramPos( int speed, int xbin, int ybin, int *need_upload, bool use_memory)
{
        unsigned int i,j;
        i=0;

        // Search in the current LittleJoe memory
        if (use_memory) {
        	vector<littlejoe_program>::iterator iter;
        	for (iter= memory.programs.begin(); iter != memory.programs.end(); iter++) {
                	if ((iter->binx == xbin) &&
                    	    (iter->biny == ybin) &&
                    	    (iter->readout_speed == speed)) {
                        	if (need_upload)
                                	*need_upload=0;
                        	return i;
                   	}
                	i++;
        	}    
	}

        _logger->log( Logger::LOG_LEV_DEBUG, "Not in memory");

        // Search in the disk files
        if (need_upload)
                for (i=0; i<ondisk.size(); i++)
                        for (j=0; j< ondisk[i].programs.size(); j++)
                                if ((ondisk[i].programs[j].binx == xbin) &&
                                    (ondisk[i].programs[j].biny == ybin) &&
                                    (ondisk[i].programs[j].readout_speed == speed))
                                        {
                                        *need_upload = i;
                                        return -1;
                                        }
        _logger->log( Logger::LOG_LEV_DEBUG, "Not on disk");        
        if (need_upload)
                *need_upload=-1;
        return -1;
}

//+Function
//
// UpdateJoeMemory
//
// To be called after a program has been successfully downloaded to the LittleJoe controller
//-

int JoeCtrl::UpdateJoeMemory( unsigned int uploaded_file)
{
        if (uploaded_file >= ondisk.size())
                return JOE_OUT_OF_RANGE_ERROR;

   memory = ondisk[uploaded_file];

        return NO_ERROR;
}


//+Function: DumpProgramset
//
// Dumps a programset on screen
//-

int DumpProgramset( littlejoe_programset programset)
{
   int i=0;

        printf("Programset: %s\n", programset.name.c_str());
        printf("Control filename: %s\n", programset.control_filename.c_str());
        printf("Pattern filename: %s\n", programset.pattern_filename.c_str());
        printf("Number of programs: %d\n", programset.programs.size());

    vector<littlejoe_program>::iterator iter;
        for (iter= programset.programs.begin(); iter != programset.programs.end(); iter++)
                {
                printf("Program %d, name:%s\n", i++, iter->name.c_str());
                printf("Bin, speed: %dx%d, %d\n", iter->binx, iter->biny, iter->readout_speed);
                printf("Delay base, inc: %f,%f\n", iter->delay_base, iter->delay_inc);
                }
        
        return NO_ERROR;
}


//@Function: EraseLocalMemory
//
// Erases the local copy of LittleJoe's internal memory
//@

int JoeCtrl::EraseLocalMemory()
{
    memory.programs.clear();

    // If requested, start assuming a certain program set is available
    if (_startProgramSet >= 0)
       SetLocalMemory( _startProgramSet);

        return NO_ERROR;
}

//@Function: SetLocalMemory
//
// Synch the local memory information to a certain programset
//@

int JoeCtrl::SetLocalMemory( unsigned int programset_num)
{
    memory = ondisk[programset_num];
    return NO_ERROR;
}
        

//@Function: LoadJoeDiskFiles
//
// Load the configuration files specifying the different
// program sets that can be uploaded to LittleJoe
//
//@

int JoeCtrl::LoadJoeDiskFiles()
{
    int num, i;
    std::string prefix;

    num = ConfigDictionary()["num_programsets"];

    ondisk.resize(num);
    program_names.clear();

    for (i=0; i<num; i++)
         {
         char param[32];

         sprintf( param, "programset%d", i);
         Config_File *subtree = ConfigDictionary().extract(param);
         ondisk[i] = ReadProgramSet( *subtree);
         delete subtree;
         }

    if (debug)
        for (i=0; i<num; i++)
                DumpProgramset(ondisk[i]);

    _logger->log( Logger::LOG_LEV_INFO, "%d programsets loaded.", ondisk.size());
    return 0;
}

// Reads a configuration file with the parameters of an entire LittleJoe program set

littlejoe_programset JoeCtrl::ReadProgramSet( Config_File &cfg)
{
        int size;
        unsigned int i;

        littlejoe_programset programset;

        programset.name = (std::string) cfg["name"];
        programset.control_filename = (std::string) cfg["control_filename"];
        programset.pattern_filename = (std::string) cfg["pattern_filename"];
        size = cfg["num_programs"];

        if ((size<=0) || (size > 1000))
                {
                _logger->log( Logger::LOG_LEV_WARNING, "Skipping programset %s because it has %d programs", programset.name.c_str(), size);
                return programset;
                }

        programset.programs.resize( size);

        for (i=0; i< programset.programs.size(); i++)
                {
                char par_name[32];
                sprintf( par_name, "program%d", i);

                _logger->log( Logger::LOG_LEV_DEBUG, "Reading program %d of %d", i, programset.programs.size());
                Config_File *subtree = cfg.extract(par_name);
                programset.programs[i] = ReadProgram( *subtree);
                delete subtree;
                }

        return programset;
}

// Read a configuration file with the parameters of a single LittleJoe program

littlejoe_program JoeCtrl::ReadProgram( Config_File &cfg)
{
        littlejoe_program program;

        program.name = (std::string) cfg["name"];
        program.readout_speed = cfg["readout_speed"];
        program.binx = cfg["binx"];
        program.biny = cfg["biny"];
        program.delay_base = cfg["delay_base"];
        program.delay_inc = cfg["delay_inc"];

        // Read default black levels if they exist, otherwise set to -1
        for (int i=0; i<4; i++) {
           try {
              char str[10];
              sprintf( str, "black%d", i+1);
              program.black_levels[i] = cfg[str];
           } catch (Config_File_Exception &e) {
              program.black_levels[i] = -1;
           }
        }

        _logger->log( Logger::LOG_LEV_DEBUG, "Found: speed %d, binx %d, biny %d, base %f, inc %f", program.readout_speed, program.binx, program.biny, program.delay_base, program.delay_inc);

        // Build the arrays listing each possible value
        insert_value( _ccdXbins, program.binx);
        insert_value( _ccdYbins, program.biny);
//        insert_value( ccd_speeds, program.readout_speed);

        program_names.push_back(program.name);

        return program;
}

int JoeCtrl::insert_value( vector<int> &array, int value)
{
    vector<int>::iterator iter;
    for ( iter = array.begin(); iter != array.end(); iter++)
        if (*iter == value)
            return NO_ERROR;

    array.push_back(value);
    return NO_ERROR;
}

int JoeCtrl::checkTTAmp() {

    return NO_ERROR;
   //Disabled for old board

/*
    double amp;

    var_ttamp_cur.Update();
    var_ttamp_cur.Get(&amp);
    
    if (amp!=0) {
        _logger->log( Logger::LOG_LEV_ERROR, "Tip/tilt mirror modulation amplitude is not zero!");
        var_ttamp_error.Set(1, 0, FORCE_SEND);
        return VALUE_OUT_OF_RANGE_ERROR;
    }

    return NO_ERROR;*/
}




#ifndef __VISAO_NO_MAIN
//4 Nov 2010: VISAO_NO_MAIN flag added to turn off compilation of main()
//            to allow derivations of JoeCtrl 
//            by Jared R. Males, Steward Observatory

// Main

int main( int argc, char **argv) {

      SetVersion(VersMajor,VersMinor);

      try {
         JoeCtrl *c;

         c = new JoeCtrl( argc, argv);

          c->Exec();

          delete c;
      } catch (AOException &e) {
          Logger::get()->log( Logger::LOG_LEV_FATAL, "%s:%d: %s", __FILE__, __LINE__, e.what().c_str());
      }
}
                                                                             
#endif //__VISAO_NO_MAIN                                                                
                    
