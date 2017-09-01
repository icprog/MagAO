#ifndef PATHS_H_INCLUDED
#define PATHS_H_INCLUDED

#include <string>
#include <vector>
using namespace std;

#include <stdlib.h>

namespace Arcetri {

	/*
	 * Configuration of common config and calib paths
	 * 
	 */
	class Paths {

      public:   

         // Directory access functions

         static string calibDir();
         static string WfsCalibDir();
         static string WfsSetupsDir();
         static string BackgDir( string ccd, int binning);
         static string LUTsDir( string ccd);
         static string slopenullDir( int binning);
         static string dspDir();
         static string gainsDir();

         static string formatBinning( int binning);

         static string AdSecCalibDir( bool absolute=false);
         static string FlatsDir( bool absolute=false);
         static string M2CDir( string m2c="", bool absolute=false);
         static string M2CFile( string m2c, bool absolute=false);
         static string AmpEnvelopeFile( string m2c, bool absolute=false);
         static string ModesAmpDir( string m2c, bool absolute=false);
         static string RECsDir( string m2c, bool absolute=false);
         static string IntmatAcqDir( string m2c, string acq="", bool absolute=false);
         static string DisturbDir( string m2c="", bool absolute=false);
         static string GainDir( string m2c="", bool absolute=false);
         static string PsfOptDir( string m2c="", string timestamp="", bool absolute=false);
         static string removePrefix( string absPath);

         static string AdSecDataDir( bool absolute=false, string date="", string trackingNum="");

         // File access functions

         static string CurLUTDir( string ccd);
         static string CurBackgFilename( string ccd, int binning);
         static string CurDisplayLUT( string ccd);
         static string CurSlopeXLUT( string ccd);
         static string CurSlopeYLUT( string ccd);
         static string CurBcuLUT( string ccd);
         static string CurIndpup( string ccd);
         static string CurSlopenull( int binning);

         // Convenience functions

         static vector<string> readDir( string path, int type);

   };

} // namespace Arcetri


#endif // PATHS_H_INCLUDED
