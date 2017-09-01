
#include "Paths.h"
#include <sstream>
#include "Utils.h"
using namespace std;

#include <cstdlib>
#include <iostream>
#include <dirent.h>
#include <string.h>


using namespace Arcetri;

   string Paths::calibDir() {
      return Utils::getAdoptRoot()+"/" + string("/calib/");
   }
   
   string Paths::WfsCalibDir() {
      return calibDir() + "/wfs/current";
   }

   string Paths::WfsSetupsDir() {
      return WfsCalibDir()+ string("/setups");
   }

  string Paths::AdSecDataDir( bool absolute, string date, string trackingNum) {
     string path = "";
     if (date != "") {
        if (date.size() >8)
           date = date.substr(0, 8);
        path = date+"/";

        if (trackingNum != "")
           path += "Data_"+ trackingNum + "/";
     }

     if (absolute)
        path = "/towerdata/adsec_data/"+path;

     return path;
  }

   string Paths::AdSecCalibDir( bool absolute) {
      if (absolute)
         return calibDir() + "/adsec/current/";
      else
         return "";     // Reduces to nothing because relative paths starts from this directory!
   }

   string Paths::FlatsDir( bool absolute) {
      string path = "/adsec_calib/flat/Applicable";
      return getenv("ADOPT_MEAS")+path;
   }

   string Paths::removePrefix( string absPath) {
      string prefix = AdSecCalibDir();
      if (absPath.compare(0, prefix.size(), prefix) == 0)
         return absPath.substr( prefix.size());
      else
         return absPath;
   }

   string Paths::M2CDir( string m2c, bool absolute) {
     if (m2c.compare(""))
        return AdSecCalibDir( absolute)+"M2C/"+m2c+"/";
     else
        return AdSecCalibDir( absolute)+"M2C/";
   }

   string Paths::DisturbDir( string m2c, bool absolute) {
      if (m2c.compare(""))
         return M2CDir( m2c, absolute)+"disturb/";
      else
         return AdSecCalibDir( absolute)+"CMD/disturb/";
   }

   string Paths::GainDir( string m2c, bool absolute) {
      return M2CDir( m2c, absolute)+"gain/";
   }

   string Paths::M2CFile( string m2c, bool absolute) {
     return M2CDir( m2c, absolute)+"m2c.fits";
   }

   string Paths::AmpEnvelopeFile( string m2c, bool absolute) {
     return M2CDir( m2c, absolute)+"amp_envelope.fits";
   }

   string Paths::RECsDir( string m2c, bool absolute) {
     return M2CDir( m2c, absolute)+"RECs/";
   }

   string Paths::IntmatAcqDir( string m2c, string acq, bool absolute) {
      if (acq.compare(""))
         return M2CDir( m2c, absolute)+"intmatAcq/"+acq+"/";
      else
         return M2CDir( m2c, absolute)+"intmatAcq/";
   }

   string Paths::PsfOptDir( string m2c, string timestamp, bool absolute) {
      if (timestamp.compare(""))
         return M2CDir( m2c, absolute)+"psfOpt/"+timestamp+"/";
      else
         return M2CDir( m2c, absolute)+"psfOpt/";
   }

   string Paths::ModesAmpDir( string m2c, bool absolute) {
     return M2CDir( m2c, absolute)+"modesAmp/";
   }

   string Paths::BackgDir( string ccd, int binning) {
      return WfsCalibDir() + "/" + ccd + "/backgrounds/" + formatBinning(binning);
   }

   string Paths::LUTsDir( string ccd) {
      return WfsCalibDir() + "/" + ccd + "/LUTs";
   }

   string Paths::slopenullDir( int binning) {
      return WfsCalibDir() + "/slopenulls/" + formatBinning(binning);
   }

   string Paths::dspDir() {
      return WfsCalibDir() + "/dsp_programs";
   }

   string Paths::gainsDir() {
      return WfsCalibDir() + "/gains";
   }

   string Paths::CurBackgFilename( string ccd, int binning) {
      return BackgDir( ccd, binning) + "/current";
   }

   string Paths::CurLUTDir( string ccd) {
      return LUTsDir( ccd) + "/current";
   }

   string Paths::CurDisplayLUT( string ccd) {
      return LUTsDir( ccd) + "/current/display";
   }

   string Paths::CurSlopeXLUT( string ccd) {
      return LUTsDir( ccd) + "/current/slopex";
   }

   string Paths::CurSlopeYLUT( string ccd) {
      return LUTsDir( ccd) + "/current/slopey";
   }

   string Paths::CurBcuLUT( string ccd) {
      return LUTsDir( ccd) + "/current/bcuLUT";
   }

   string Paths::CurIndpup( string ccd) {
      return LUTsDir( ccd) + "/current/indpup";
   }

   string Paths::CurSlopenull( int binning) {
      return slopenullDir( binning) + "/current";
   }

   string Paths::formatBinning( int binning) {
      ostringstream ostr;
      ostr << "bin" << binning;
      return ostr.str();
   }


vector<string> Paths::readDir( string path, int type) {
   DIR *dirp;
   struct dirent *entry;
   vector<string> list;

   if ((dirp = opendir(path.c_str()))) {         // Additional parenthesis to remove warning
      while ((entry = readdir(dirp))) {          // Additional parenthesis to remove warning
         if ((!strcmp(entry->d_name, ".")) || (!strcmp(entry->d_name, "..")))
            continue;
         if (entry->d_type == type)
            list.push_back(entry->d_name);
      }
      closedir(dirp);
   }
   return list;
}


