
#ifndef __AOI_DD_h__
#define __AOI_DD_h__

#include "arblib/aoArb/AOArbConst.h"

#define CMD_RUNNING 2
#define CMD_RUNNING_STR "running . . ."
#define CMD_SUCCESS 1
#define CMD_SUCCESS_STR "success"
#define CMD_DEFAULT 0
#define CMD_DEFAULT_STR ""
#define CMD_FAILURE -1
#define CMD_FAILURE_STR "failed"

/** \todo find where the above are defined in the TCS-AOS tree*/


struct AOI_command_info
{
   int tmout;
   int stat;
   std::string errMsg;

   //presetflat
   std::string flatspec;
   //presetao
   std::string aomode;
   std::string wfsSpec;
   double cindex;
   double rocoordx;
   double rocoordy;
   double mag;
   bool adcTracking;
   //checkref
   double result[3];
   //acquireref
   bool repoint;
   //refine
   std::string method;
   //modify
   int nmodes;
   double freq;
   int nbins;
   double ttmod;
   std::string filter1;
   std::string filter2;
   std::string filter3;
   //offsetxy
   double x;
   double y;
   //offsetz
   double z;
   //userpanic
   std::string reason;
   //setnewinstrument
   std::string instr;
   std::string focalSt;
   
};

void init_AOI_command_info(AOI_command_info *com);

struct AOI_Commands_list
{
   AOI_command_info presetflat;
   AOI_command_info presetvisao;
   AOI_command_info presetao;
   AOI_command_info acqguider;
   AOI_command_info checkref;
   AOI_command_info acquireref;
   AOI_command_info refine;
   AOI_command_info modify;
   AOI_command_info startao;
   AOI_command_info offsetxy;
   AOI_command_info offsetz;
   AOI_command_info stop;
   AOI_command_info pause;
   AOI_command_info resume;
   AOI_command_info userpanic;
   AOI_command_info setnewinstrument;
   AOI_command_info wfson;
   AOI_command_info wfsoff;
   AOI_command_info adsecon;
   AOI_command_info adsecoff;
   AOI_command_info adsecset;
   AOI_command_info adsecrest;
   
   int updateargs;
   int busy;
};

void init_AOI_Commands_list(AOI_Commands_list * com);

struct AOI_AO_params
{
   double dx;
   double dy;
   std::string slnull;
   std::string filter1;
   std::string filter2;
   std::string filter3;
   double freq;
   double gain;
   double mag;
   int nbins;
   int nmodes;
   double r0;
   double snmode;
   double strehl;
   double ttmodul;
};

void init_AOI_AO_params(AOI_AO_params *par);

struct AOI_wfs_info
{
   int enabled;
   int active;
   int health;
   int ccdbin;
   double ccdfreq;
   int tv_binning;
   double tv_exptime;
   std::string filter1;
   double filter1_pos;
   double filter1_reqpos;
   double filter1_startpos; //The home position
   double filter1_homingpos; //the position command to start homing
   double filter1_abortpos; //the position command to abort movement

   std::string tv_filter2;
   std::string tv_filter3;
   int counts;
   int no_subaps;
   std::string status;
   int led;
   double mod_ampl;
   std::string msg;
   int pyramid_pos[2];
   char tvImgVar[65536];
   std::string tv_filename0;
   std::string tv_filename1;
   double tv_angle;
   
   double rerotator_angle;

   double baysidex;
   int baysidex_enabled;
   double baysidey;
   int baysidey_enabled;
   double baysidez;
   int baysidez_enabled;
   
};

void init_AOI_wfs_info(AOI_wfs_info *wfs);

struct AOI_AO_DD
{
   int ao_ready;
   int correctedmodes;
   
   std::string status;
   std::string reconstructor;
   int loopon;
   std::string mode;
   std::string msg;
   int ofl_enabled;
   int sl_ready;
   double strehl;
   std::string wfs_source;

   std::string loop_gains;
   
   double tt_amp[3];
   double tt_freq[3];
   double tt_offset[3];
   
   //Data from the gainSetter
   int nmodes;
   int homiddle; //the cutoff between ho1 and ho2
   double gain_tt;
   double gain_ho1;
   double gain_ho2;
   
   
   AOI_AO_params param;
};

void init_AOI_AO_DD(AOI_AO_DD *ao);

struct AOI_ADSEC_DD
{
   double anem_speed[12];
   int anem_upd;
   int elev_upd;
   int health;
   int contamination;
   int nwact;
   std::string popmsg;
   std::string shape;
   int coil_status;
   int pwr_status;
   int tss_status;
   std::string status;
   int led;
   std::string msg;
   double safeskip_perc;
   double offload[3];
   std::string fl_filename;
};

void init_AOI_ADSEC_DD(AOI_ADSEC_DD *ads);

struct AOI_side_DD
{
   AOI_Commands_list cmds;
   
   AOI_AO_DD ao;
   AOI_ADSEC_DD adsec;

   int labmode;
   int idlstat;
   
   AOI_wfs_info wfs1;
   
   //AOI_wfs_info wfs2;
   int vmajor;
   int vminor;
   std::string starttime;
   std::string msgdip;
   std::string msgdident;
   std::string msgdstat;
   std::string conntime;
   int tel_enabled;
   std::string logdir;

   int updateargs;
   int connected;
   int sstat_color;
   std::string servstat;
};

void init_AOI_side_DD(AOI_side_DD *side);

struct AOI_DD_mountpos
{
   double position;
   double time;
};

struct AOI_DD_hexapod
{
   double abs_pos[6];
   double set_pos[6];
   double enc_pos[6];
};

struct AOI_DD_dome
{
   double az;
   int stat;
};

struct AOI_DD_rotator
{
   int roi;
   double angle;
   double offset;
   int following;
};

struct AOI_DD_catalog
{
   double ra;
   double dec;
   double epoch;
   double rotOff;
   std::string rotMode;
   std::string obj;
   std::string obsinst;
   std::string obsname;
};

struct AOI_DD_environment
{
   double wxtemp;
   double wxpres;
   double wxhumid;
   double wxwind;
   double wxwdir;
   double wxdewpoint;
   double wxpwvest;

   double ttruss;
   double tcell;
   double tseccell;
   double tambient;

   double dimmfwhm;
   int dimmtime;
   double mag1fwhm;
   int mag1time;
   double mag2fwhm;
   int mag2time;

};

struct AOI_DD
{
   //std::string tgtname;
   std::string dateobs;
   int ut;
   int st;

   double ra;
   double dec;
   double epoch;
   double ha;
   double am;
   AOI_DD_rotator rotator;

   int istracking;
   int isguiding;
   int isslewing;
   int guider_ismoving;

   AOI_DD_mountpos az;
   AOI_DD_mountpos el;
   double zd;
   double pa;
   AOI_DD_dome dome;

   AOI_DD_catalog cat;

   AOI_DD_hexapod hexapod;

   AOI_DD_environment environ;
   AOI_side_DD side;

   double avgwfe;
   double stdwfe;
   double instwfe;

   int nodInProgress;
};


void init_AOI_DD(AOI_DD * aoi);

    
#endif //__AOI_info_h__
