
#include "acq.h"

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
using namespace std;

#include <dirent.h>     // DT_REG
#include <stdlib.h>
#include <curses.h>
#include <sys/stat.h>   // mkdir()
#include <sys/types.h>  // mkdir()


#include "Paths.h"
#include "Utils.h"

#include "intmatlib.h"

extern "C" {
#include "iolib.h"
}

#include "arblib/base/ArbitratorInterface.h"
#include "arblib/wfsArb/WfsInterfaceDefines.h"
#include "arblib/base/CommandsExport.h"
#include "arblib/wfsArb/WfsCommandsExport.h"
#include "arblib/adSecArb/AdSecPTypes.h"
#include "arblib/adSecArb/AdSecCommandsExport.h"
#include "arblib/aoArb/AOCommandsExport.h"
#include "arblib/base/AlertsExport.h"
#include "arblib/base/Alerts.h"


//#include "bcu_diag.h"

using namespace Arcetri;
using namespace Arcetri::Arbitrator;
//using namespace Arcetri::Wfs_Arbitrator;




int shortTimeout = 10*1000;
int longTimeout = 40*1000;
int extraLongTimeout = 1000*1000;

string errstr = "";

char user[] = "LBTO";
char host[] = "obs4";

bool test= false;

vector<string> tracknums;

// If this is defined, no commands are sent to arbitrators except for status requests
#undef TEST_ONLY

void help() {
	printf("\n\n");
	printf("automated image acquisition\n");
	printf("\n");
	printf("Usage:  acq <filename> [-test]\n");
	printf("\n");
	printf("<filename> is the name of a text file which can contains the following commands:\n");
	printf("\n");
	printf("irtc <exptime> [<framerate>]  : set the IRTC exposure time (in seconds) and frame rate (in Hz)\n");
	printf("offset <x> <y>                : execute a closed loop offset in millimeters\n");
	printf("pos <x> <y>                   : set an absolute position in arcsec\n");
	printf("nframes <nframes>             : set the n. of frames for subsequent cubes and darks\n");
	printf("image [<nframes> [<nframes47> [<nframesAO>]]] : save an IRTC cube and optical loop data. Ccd47 defaults to zero frames (not saved). Optical loop data defaults to 4000 frames\n");
        printf("exptime <exptime>       a     : change the PISCES exposure time\n");
	printf("pisces [<nframes> [<nframes47> [<nframesAO>]]] : save a PISCES cube and optical loop data. Ccd47 defaults to zero frames (not saved). Optical loop data defaults to 4000 frames\n");
        printf("cube                          : alias of \"image\"");
	printf("dark [<nframes>]              : save an IRTC dark frames\n");
	printf("piscesdark [<nframes>]        : save a PISCES dark frames\n");
        printf("lamp <on|off>                 : turns on/off the WFS reference source\n");
        printf("savegain                      : saves current gain into an internal register\n");
        printf("restoregain                   : restores previously saved gain\n");
        printf("gainzero                      : sets zero gain\n");
        printf("gain <gain>                   : set specified gain on all modes\n");
        printf("wait <seconds>                : waits for the specified number of seconds\n");
        printf("disturb <filename>            : applies the specified disturbance file\n");
	printf("disturb [sync|ovs|off]        : sets the disturbance for sync WFS / oversampling / none\n");
	printf("flat [<filename>]             : applies the specified flat file (or the default one if nothing is given)\n");
        printf("cameralens [on|off]           : enables/disables camera lens pupil tracking\n");
        printf("openloop                      : opens the AO loop from wfs side\n");
        printf("closeloop                     : closes the AO loop from wfs side\n");
	printf("stopao                        : opens the AO loop from ao arbitrator side\n");
	printf("startao                       : closes the AO loop from ao aoarbitrator\n");
	printf("pauseao                       : pauses the AO loop from ao arbitrator side\n");
	printf("resumeao                      : resumes the AO loop from ao aoarbitrator\n");
        printf("elab                          : run elab-lib on all the tracknumbers acquired up to this point\n");
	printf("log <msg>                     : print message on screen\n");
	printf("autogain_init                 : use all the tracknumbers acquired up to this point to initialize the autogain algorihtm");
	printf("autogain_step                 : use the last tracknumber acquired to perform an autogain step.");
	printf("prompt <msg>                  : display a message and wait for a line of text to be entered.");
        printf("alert [ao|adsec|wfs]          : send an alert message to the specified arbitrator.");
	printf("\n");
	printf("If -test is given, no images are acquired and offsets are only printed but not executed.\n");

}

acq::acq( int argc, char *argv[]) : AOApp( "acq", "L", "127.0.0.1", 0) {

   if (argc <2) {
      printf("Usage:\n");
      printf("acq <filename>\n");
      printf("acq -help\n");
      exit(0);
   }

   if (strcmp(argv[1], "-help") == 0) {
	help();
	exit(0);
   }

   _jobfile = argv[1];
   tracknums.clear();

   if (argc>2)
	if (strcmp(argv[2], "-test") == 0) 
		test=true;

}

acq::~acq() {
   delete _wfsIntf;
   delete _adsecIntf;
}

void acq::PostInit() {
   init();

   _wfsIntf = new ArbitratorInterface( Utils::getClientname("wfsarb"), Logger::LOG_LEV_DEBUG);
   _adsecIntf = new ArbitratorInterface(  Utils::getClientname("adsecarb", true, "ADSEC"), Logger::LOG_LEV_DEBUG);
   _aoarbIntf = new ArbitratorInterface(  Utils::getClientname("AOARB", true, "ADSEC"), Logger::LOG_LEV_DEBUG);
   printf("%s\n",  Utils::getClientname("wfsarb").c_str());
   printf("%s\n",  Utils::getClientname("AOARB", true, "ADSEC").c_str());

   _wfsNotifier = new AlertNotifier( Utils::getClientname("wfsarb"));
   _adsecNotifier = new AlertNotifier( Utils::getClientname("adsecarb", true, "ADSEC"));
   _aoNotifier = new AlertNotifier( Utils::getClientname("AOARB", true, "ADSEC"));


   _lampVarReq = new RTDBvar("picctrl."+Side()+".lamp.REQ", INT_VARIABLE, 1, false);
   _lampVarCur = new RTDBvar("picctrl."+Side()+".lamp.CUR", INT_VARIABLE, 1, false);
   _lamp=0;

   _gainVarCur = new RTDBvar("ADSEC."+Side()+".G_GAIN_A@M_ADSEC", CHAR_VARIABLE, 256, false);

   Notify(*_lampVarCur, lampNotify);
}

int acq::lampNotify(void *pt, Variable *var) {
   ((acq*)pt)->_lamp = var->Value.Lv[0];
   return NO_ERROR;
}


void acq::init() {

}


typedef map<string, vector<string> > Parameters;

string lowercase( string s) {

	for (unsigned int i=0; i<s.size(); i++)
		s[i] = tolower(s[i]);
	return s;
}

void acq::Run() {

   string tracknum = "None";


   ifstream ifs(_jobfile.c_str());

   char line[1024];
   string cmd, v; 

   float xpos=0;
   float ypos=0;

   // Process each line
   while( ifs.good()) {

	char command[1024];
	cmd="";
	vector<string> tokens; // Create vector to hold our words

	tokens.clear();
	ifs.getline(line, 1023);
	stringstream ss(line); // Insert the string into a stream

	ss >> cmd;
	while(ss >> v) tokens.push_back(v);

        if (cmd == "irtc") {
		if (tokens.size() < 1) {
			printf("Error: irtc command must have at least one argument: exp. time (microseconds)\n"); 
			continue;
		}

		int exptime = (int)(atof(tokens[0].c_str())  * 1e6);
		int hz = 100000;
		if (tokens.size()>1)
			hz = atoi(tokens[1].c_str());
	
		int ret=0;
		if (!test) {	
			sprintf(command, "ssh %s@%s \"irc SetIRParams %d %d\"", user, host, exptime, hz);
			ret = system(command);
		}
		if (ret) 
			printf("Error: irtc command exited with status %d\n", ret);
		else
			printf("Set IRTC params: exptime=%d microsec, framerate=%d hz\n", exptime, hz);
	}

	if (cmd == "offset") {

		if (tokens.size() < 2) {
			printf("Error: offset command must have two arguments: X and Y movement (mm)\n");
			continue;
		}

		float x = atof(tokens[0].c_str());
		float y = atof(tokens[1].c_str());
	
		int ret=0;
		if (!test) {	
			//printf("Offset command disabled\n");
			sprintf(command, "ssh %s@%s \"irc OffsetXYAO %4.2f %4.2f right\"", user, host, x, y);
			ret = system(command);
		}
		if (ret)
			printf("Error: offset command exited with status %d\n", ret);
		else {
			xpos +=x;
			ypos +=y;
			printf("Executed offset : x=%5.3f y=%5.3f (mm)\n", x, y);
			printf("Current position: x=%5.3f y=%5.3f (arcsec)\n", xpos/0.6, ypos/0.6);
		}

	}

	if (cmd == "pos") {


		if (tokens.size() < 2) {
			printf("Error: pos command must have two arguments: X and Y absolute position (arcsec)\n");
			continue;
		}

		float x = atof(tokens[0].c_str());
		float y = atof(tokens[1].c_str());

		float offx = x*0.6-xpos;
		float offy = y*0.6-ypos;

		int ret=0;
                if ((offx <0.5) && (offy<0.5)) {
			if (!test) {		
				//printf("Pos command disabled\n");
				sprintf(command, "ssh %s@%s \"irc OffsetXYAO %4.2f %4.2f right\"", user, host, offx, offy);
				ret = system(command);
			}
			if (ret)
				printf("Error: pos command exited with status %d\n", ret);
			else {
				xpos +=offx;
				ypos +=offy;
				printf("Executed offset : x=%5.3f y=%5.3f (mm)\n", offx, offy);
				printf("Current position: x=%5.3f y=%5.3f (arcsec)\n", xpos/0.6, ypos/0.6);
			}
		} else {
			if (!test) {		
				//printf("Pos command disabled\n");
				sprintf(command, "ssh %s@%s \"irc OffsetXYAO %4.2f %4.2f right\" &", user, host, offx, offy);
				ret = system(command);

				// Wait for offset execution
				float m = offx;
				if (offy>offx) m =offy;
				sleep((unsigned int)(m/0.6 * 8 +15));
			}
			if (ret)
				printf("Error: pos command exited with status %d\n", ret);
			else {
				xpos +=offx;
				ypos +=offy;
				printf("Executed offset : x=%5.3f y=%5.3f (mm)\n", offx, offy);
				printf("Current position: x=%5.3f y=%5.3f (arcsec)\n", xpos/0.6, ypos/0.6);
			}
		}

	}
        if (cmd == "cameralens") {
               bool onoff = (tokens[0] == "on") ? true : false;

               Wfs_Arbitrator::autoTrackParams params;
               params.rerotTrack=true;
               params.adcTrack= false;
               params.lensTrack= onoff;
               Wfs_Arbitrator::AutoTrack *arbcmd = new Wfs_Arbitrator::AutoTrack( shortTimeout, params);
               int stat = sendCmdWfs( arbcmd, errstr);
               if (stat) {
                     printf("Error opening loop: %s\n", errstr.c_str());
                     exit(0);
               }
               printf("Camera lens tracking %s\n", (onoff ? "enabled" : "disabled"));

        }

        if (cmd == "prompt") {
                int i;
                for (i=0; i<tokens.size(); i++)
                        printf("%s ", tokens[i].c_str());
                printf("\n");
                char buf[128];
                fgets(buf, 127, stdin);
        }


        if (cmd == "lamp") {
                if (tokens.size() <1) {
			printf("Error: lamp command must have one argument: on/off\n");
			continue;
		}
                int onoff = (tokens[0] == "on") ? 1 : 0;
		_lampVarReq->Set(onoff);
		int timeout=20,i;
                for (i=0; i<timeout; i++) {
			sleep(1);
			if (_lamp == onoff)
				break;
		}
		if (i==timeout) {
			printf("Error: timeout while waiting for lamp answer\n");
			continue;
		}
                if (onoff)
			printf("Lamp turned on\n");
		else
			printf("Lamp turned off\n");
        }

/*
        if (cmd == "gain") {
                if (tokens.size() <1) {
			printf("Error: gain command must have one argument: gain value\n");
			continue;
		}
                float gain = atof(tokens[0].c_str());

                char gainfile[128];
                snprintf(gainfile, "/towerdata/adsec_calib/M2C/KL/gain/gain%3.1f.fits", gain);
		int stat = setGain(gainfile);
		if (stat == 0)
			printf("Gain set to %3.1f\n", gain);
	}
*/


        if (cmd == "savegain") {
                _gainVarCur->Update();
                _savedGain = _gainVarCur->Get(); 
                printf("Gain values saved\n");
        }

        if (cmd == "restoregain") {
		int stat = setGain( _savedGain);
                if (stat == 0)
			printf("Gain restored\n");
	}

	if (cmd == "gainzero") {
		int stat = setGain("/towerdata/adsec_calib/M2C/KL/gain/gain0.fits");
		if (stat == 0)
			printf("Gain set to zero\n");
	}

	if (cmd == "openloop") {
		Command *arbcmd = new Wfs_Arbitrator::StopLoop(shortTimeout);
		int stat = sendCmdWfs( arbcmd, errstr);
		if (stat == 0) {
                        msleep(100);
			Wfs_Arbitrator::stopLoopAdsecParams params;
			params.hold = true;
			Command *cmd2 = new Wfs_Arbitrator::StopLoopAdsec(shortTimeout, params);
			stat = sendCmdWfs(cmd2, errstr);
		}
		if (stat) {
                       	printf("Error opening loop: %s\n", errstr.c_str());
			exit(0);
               	} else {
			printf("Loop opened\n");
		}
	}

	if (cmd == "closeloop") {
		Command* arbcmd = new Wfs_Arbitrator::PrepareAdsec(shortTimeout);
		int stat = sendCmdWfs( arbcmd, errstr);
		if (stat == 0) {
			Command* cmd2 = new Wfs_Arbitrator::CloseLoop(shortTimeout);
			stat = sendCmdWfs( cmd2, errstr);
		} 
		if (stat) {
			printf("Error closing loop: %s\n", errstr.c_str());
			exit(0);
               	} else {
			printf("Loop closed\n");
		}
	}

	if (cmd == "startao") {
	        Command *cmd = new StartAO(shortTimeout);
		int stat = sendCmd( cmd, _aoarbIntf, errstr);
		if (stat) {
			printf("Error closing loop: %s\n", errstr.c_str());
			exit(0);
               	} else {
			printf("Loop closed\n");
		}
	}

	if (cmd == "stopao") {
	        stopParams params = { "Open"};
	        Command *cmd = new Stop(shortTimeout, params);
		int stat = sendCmd( cmd, _aoarbIntf, errstr);
		if (stat) {
                       	printf("Error opening loop: %s\n", errstr.c_str());
			exit(0);
               	} else {
			printf("Loop opened\n");
		}
	}

	if (cmd == "pauseao") {
	        Command *cmd = new Pause(shortTimeout);
		int stat = sendCmd( cmd, _aoarbIntf, errstr);
		if (stat) {
			printf("Error loop pause: %s\n", errstr.c_str());
			exit(0);
               	} else {
			printf("Loop paused\n");
		}
	}


	if (cmd == "resumeao") {
	        Command *cmd = new Resume(shortTimeout);
		int stat = sendCmd( cmd, _aoarbIntf, errstr);
		if (stat) {
			printf("Error loop resume: %s\n", errstr.c_str());
			exit(0);
               	} else {
			printf("Loop resumed\n");
		}
	}


	if (cmd == "disturb") {
		if (tokens.size() <1) {
			printf("Error: disturb command must have one argument: filename, sync or ovs\n");
			continue;
		}
		bool enabledisturb = false;
		bool wfs = false;
		bool ovs = false;
		if (tokens[0].compare("sync") == 0) {
			wfs = true;
			enabledisturb = true;
		}
		if (tokens[0].compare("ovs") == 0) {
			ovs = true;
                        wfs = true;
			enabledisturb = true;
		}
		if (tokens[0].compare("off") == 0) {
			enabledisturb = true;
		}
		if (enabledisturb) {
			Wfs_Arbitrator::enableDisturbParams params = {wfs, ovs};
			Command *arbcmd = new Wfs_Arbitrator::EnableDisturb( shortTimeout, params);
			int stat = sendCmdWfs( arbcmd, errstr);
			if (stat)
                                printf("Error in disturbance command: %s\n", errstr.c_str());
                	else
				printf("Disturbance set to: sync %s - ovs %s\n", (wfs) ? "on" : "off", (ovs) ? "on" : "off");
			continue;
		}

		string filename = tokens[0];
		if (filename.substr(0,1) != "/")
			filename = Paths::AdSecCalibDir( true)+"CMD/disturb/"+filename;

		AdSec_Arbitrator::disturbParams params;
		params._on = true;
		params._disturbFile = filename;
		Command* arbcmd = new AdSec_Arbitrator::SetDisturb(shortTimeout, params);
		int stat = sendCmdAdsec(arbcmd, errstr);
		if (stat)
		    printf("Error in disturb command: %s\n", errstr.c_str());
		else
		    printf("Disturbance set to: %s\n", filename.c_str());
	}
			

	if (cmd == "wait") {
		if (tokens.size() <1) {
			printf("Press <return> to continue...");
			do {} while (getchar() != '\n');
		}
		else {
			double seconds = atof(tokens[0].c_str());
			printf("Waiting %5.2f seconds\n", seconds);
			msleep((int)(seconds*1000));
		}
        }

	if (cmd == "flat") {
		string filename;
		if (tokens.size() <1) 
			filename = "default.sav";
		else
			string filename = tokens[0];
		if (filename.substr(0,1) != "/")
			filename = Paths::FlatsDir( true)+"/"+filename;

		AdSec_Arbitrator::shapeParams params;
		string s = Utils::addExtension( filename, ".sav");
		params._shapeFile = s;
		Command *cmd = new AdSec_Arbitrator::LoadShape( longTimeout, params);
		int stat = sendCmdAdsec(cmd, errstr);
		if (stat)
		    printf("Error in load shape command: %s\n", errstr.c_str());
		else
		    printf("Loaded shape: %s\n", filename.c_str());
	}


	if (cmd == "dark") {
		if (tokens.size() <1) {
			printf("Error: dark command must have one argument: number of frames\n");
			continue;
		}

		int nframes = atoi(tokens[0].c_str());

		if (!test) {
			Command *arbcmd = new Wfs_Arbitrator::CalibrateIRTCDark(extraLongTimeout, nframes);
			int stat = sendCmdWfs( arbcmd, errstr);

			if (stat)
				printf("Error saving dark file: %s\n", errstr.c_str());
                	else 
				printf("Saved IRTC dark\n");
		}
		else {
			printf("Saved IRTC dark\n");
		}
	}


	if (cmd == "exptime") {
		if (tokens.size() <1) {
			printf("Error: exptime command must have one argument: exposure time\n");
			continue;
		}

		float exptime = atof(tokens[0].c_str());
                float bias = 0.8;

                char *fmt = "thaoshell.py -e \"pisces.setexptime(%5.3f)\"";
                char cmd[128];
            
                sprintf(cmd, fmt, exptime - bias);
                int ret = system(cmd);
                if (ret <0)
                    printf("Error setting PISCES exptime\n");
                else
                    printf("PISCES exptime set to %5.1f seconds\n", exptime);
		}


	if (cmd == "piscesdark") {
		if (tokens.size() <1) {
			printf("Error: piscesdark command must have one argument: number of frames\n");
			continue;
		}

		int nframes = atoi(tokens[0].c_str());

                printf("Acquiring PISCES dark...\n");
		if (!test) {
			Command *arbcmd = new Wfs_Arbitrator::CalibratePISCESDark(extraLongTimeout, nframes);
			int stat = sendCmdWfs( arbcmd, errstr);

			if (stat)
				printf("Error saving dark file: %s\n", errstr.c_str());
                	else 
				printf("Saved PISCES dark\n");
		}
		else {
			printf("Saved PISCES dark\n");
		}
	}


	if (cmd == "image") {
		if (tokens.size() <1) {
			printf("Error: image command must have one argument: number of frames\n");
			continue;
		}

		int nframesIrtc = atoi(tokens[0].c_str());
                int nframesPisces=0;
		int nframes47 = 0;
		int nframesAO = 4000;
		if (tokens.size()>1)
			nframes47 = atoi(tokens[1].c_str());
		if (tokens.size()>2)
			nframesAO = atoi(tokens[2].c_str());
		Wfs_Arbitrator::saveOptLoopDataParams params;
		params._nFrames = nframesAO;
		params._nFramesIrtc = nframesIrtc;
		params._nFramesPsf = nframes47;
		params._nFramesPisces = nframesPisces;
		params._saveIrtc = (nframesIrtc > 0) ? true : false;
		params._savePsf = (nframes47 > 0) ? true : false;
		params._savePisces = (nframesPisces > 0) ? true : false;
		params._saveFrames = true;
		params._saveSlopes = true;
		params._saveModes = true;
		params._saveCommands = true;
		params._savePositions = true;
	
		if (!test) {	
			Command *arbcmd = new Wfs_Arbitrator::SaveOptLoopData( extraLongTimeout, params);
			Command *reply;
			int stat = sendCmdWfs( arbcmd, errstr, &reply);

			if (stat)
				printf("Error saving optical loop data: %s\n", errstr.c_str());
			else {
				string tracknum = ((Wfs_Arbitrator::SaveOptLoopData *)reply)->getParams()._trackNum;
				printf("Saved tracknum: %s  (irtc=%d frames  ccd47=%d frames optical loop data=%d frames)\n", tracknum.c_str(), nframesIrtc, nframes47, nframesAO);
                                tracknums.push_back(tracknum);

			}
			if (reply)
				delete reply;
		}
		else {
			printf("Saved tracknum:  test  (irtc=%d frames  ccd47=%d frames optical loop data=%d frames)\n", nframesIrtc, nframes47, nframesAO);
		}
        }

	if (cmd == "pisces") {
		if (tokens.size() <1) {
			printf("Error: pisces command must have one argument: number of frames\n");
			continue;
		}

		int nframesPisces = atoi(tokens[0].c_str());
                int nframesIrtc =0;
		int nframes47 = 0;
		int nframesAO = 4000;
		if (tokens.size()>1)
			nframes47 = atoi(tokens[1].c_str());
		if (tokens.size()>2)
			nframesAO = atoi(tokens[2].c_str());
		Wfs_Arbitrator::saveOptLoopDataParams params;
		params._nFrames = nframesAO;
		params._nFramesIrtc = nframesIrtc;
		params._nFramesPsf = nframes47;
		params._nFramesPisces = nframesPisces;
		params._saveIrtc = (nframesIrtc > 0) ? true : false;
		params._savePsf = (nframes47 > 0) ? true : false;
		params._savePisces = (nframesPisces > 0) ? true : false;
		params._saveFrames = true;
		params._saveSlopes = true;
		params._saveModes = true;
		params._saveCommands = true;
		params._savePositions = true;
	
                printf("Saving PISCES optloopdata...\n");
		if (!test) {	
			Command *arbcmd = new Wfs_Arbitrator::SaveOptLoopData( extraLongTimeout, params);
			Command *reply;
			int stat = sendCmdWfs( arbcmd, errstr, &reply);

			if (stat)
				printf("Error saving optical loop data: %s\n", errstr.c_str());
			else {
				string tracknum = ((Wfs_Arbitrator::SaveOptLoopData *)reply)->getParams()._trackNum;
				printf("Saved tracknum: %s  (pisces=%d frames  ccd47=%d frames optical loop data=%d frames)\n", tracknum.c_str(), nframesPisces, nframes47, nframesAO);
                                tracknums.push_back(tracknum);

			}
			if (reply)
				delete reply;
		}
		else {
			printf("Saved tracknum:  test  (pisces=%d frames  ccd47=%d frames optical loop data=%d frames)\n", nframesPisces, nframes47, nframesAO);
		}
        }

	if (cmd == "elab") {

		// Run IDL command
		char buffer[2048];
		sprintf(buffer, "idl -e \"ao_init & a= obj_new('aodataset', from='%s', to='%s') & log_twiki,a\" 2> /dev/null", tracknums.begin()->c_str(), (tracknums.end()-1)->c_str());
		int ret = system(buffer);

	}

	if (cmd == "alert") {
		if (tokens.size() <1) {
			printf("Error: alert command must have one argument: destination arbitrator\n");
			continue;
		}
                Error alert("User stop", false);
                if (tokens[0].compare("ao")==0) {
                    printf("Sending alert to AO arbitrator\n");
                    _aoNotifier->notifyAlert((Alert*)&alert);
                    printf("Alert sent to AO arbitrator\n");
                }
                else if (tokens[0].compare("wfs")==0) {
                    printf("Sending alert to WFS arbitrator\n");
                    _wfsNotifier->notifyAlert(&alert);
                    printf("Alert sent to WFS arbitrator\n");
                }
                else if (tokens[0].compare("adsec")==0) {
                    printf("Sending alert to ADSEC arbitrator\n");
                    _adsecNotifier->notifyAlert(&alert);
                    printf("Alert sent to ADSEC arbitrator\n");
                }
                else
                    printf("Error: unknown alert destionation %s\n", tokens[0].c_str());

	}

	if (cmd == "autogain_init") {
		if (tokens.size() <2) {
			printf("Error: autogain_init command must have two arguments: first and last mode\n");
			continue;
		}
		int firstmode = atoi(tokens[0].c_str());
		int lastmode  = atoi(tokens[1].c_str());
                char outfile[512];
                memset(outfile, 0, 512);
                snprintf(outfile, 511, "/tmp/autogainref%d_%d.sav", firstmode, lastmode);

		char buffer[2048];
		sprintf(buffer, "idl -e \"ao_init & a= obj_new('aodataset', from='%s', to='%s') & autogain_init, a, firstmode=%d, lastmode=%d, outfile='%s'\" 2> /dev/null", tracknums.begin()->c_str(), (tracknums.end()-1)->c_str(), firstmode, lastmode, outfile);
                printf("%s\n", buffer);
		int ret = system(buffer);
        }

	if (cmd == "autogain_step") {
		if (tokens.size() <2) {
			printf("Error: autogain_step command must have two arguments: first and last mode\n");
			continue;
		}
		int firstmode = atoi(tokens[0].c_str());
		int lastmode  = atoi(tokens[1].c_str());
                char outfile[512];
                memset(outfile, 0, 512);
                snprintf(outfile, 511, "/tmp/autogainref%d_%d.sav", firstmode, lastmode);

		char buffer[2048];
		sprintf(buffer, "idl -e \"ao_init & autogain_step, '%s', firstmode=%d, lastmode=%d, reffile='%s'\" 2> /dev/null", (tracknums.end()-1)->c_str(), firstmode, lastmode, outfile);
		int ret = system(buffer);

	}

	if (cmd == "log") {
		printf("%s: %s\n", Utils::asciiDateAndTime().c_str(), &(line[4]));
	}
   }
}

int acq::setGain( string gainfile) {

    Arcetri::AdSec_Arbitrator::gainParams g;
    g._gainFile = gainfile;
    Command *arbcmd = new Arcetri::AdSec_Arbitrator::SetGain( shortTimeout, g);
    int stat = sendCmdAdsec( arbcmd, errstr);
    if (stat)
	printf("Error setting gain: %s\n", errstr.c_str());
    return stat;
}

int acq::sendCmdAdsec( Command *cmd, string &errstr, Command **reply) {
    return sendCmd( cmd, _adsecIntf, errstr, reply);
}

int acq::sendCmdWfs( Command *cmd, string &errstr, Command **reply) {
    return sendCmd( cmd, _wfsIntf, errstr, reply);
}

int acq::sendCmd( Command *cmd, ArbitratorInterface *intf, string &errstr, Command **reply) {

    int stat = NO_ERROR;
    errstr = "";

    Command *myreply=NULL;
    try {
        myreply = intf->requestCommand(cmd);
    } catch (ArbitratorInterfaceException &e) {
        errstr = e.what();
	stat = -1;
    }
    if (!myreply)
	stat = -1;
    else if (!myreply->isSuccess())
	stat = -1;

    if (reply) {
	if (myreply)
		*reply = myreply;
	else
		*reply = NULL;
	}
    else if (myreply)
	delete myreply;

    return stat;
}


int main( int argc, char *argv[]) {

   try {
      acq *a = new acq( argc, argv);
      a->Exec();
      delete a;
   }
   catch (LoggerFatalException &e) {
      // In this case the logger can't log!!!
      printf("%s\n", e.what().c_str());
   }
   catch (AOException &e) {
      printf("Error in acq: %s", e.what().c_str());
      Logger::get(Logger::LOG_LEV_ERROR)->log(Logger::LOG_LEV_ERROR, "Error in acq: %s", e.what().c_str());
   }

   return 0;
}
  




