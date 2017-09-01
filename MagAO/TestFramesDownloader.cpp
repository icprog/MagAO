
#include <iostream>
#include "../contrib/cfitsio/fitsio.h"

#include "../MasterDiagnostic/TechnicalViewer.h"


int write_fits(const char *foutname, int nx, short *im)
{
   fitsfile *outfptr;   /* FITS file pointers defined in fitsio.h */
   int status = 0, i;       /* status must always be initialized = 0  */
   long naxes[2], lpix[2],fpix[2];//, imlong[1024*1024];
   char hfld[81], datestr[80];
   int timeref;
   
   
   //fits_get_system_date(&day, &month, &year, &status );
   fits_get_system_time(datestr, &timeref, &status);
   
   //im = (INT_2BYTES *) malloc(nx*ny*2);
   
   /* Create the output file */
   if ( !fits_create_file(&outfptr, foutname, &status) )
   {
      naxes[0] = nx;
      naxes[1] = nx;
      
      
      if(!fits_create_img(outfptr,SHORT_IMG , 2, naxes, &status))
      {         
         fpix[0] = 1;
         fpix[1] = 1;
         lpix[0] = nx;
         lpix[1] = nx;
         fits_write_subset(outfptr,TSHORT , fpix, lpix, im, &status);
         if(!status)
         {
            for(i=0;i<81;i++) hfld[i] = ' ';
            fits_update_key(outfptr, TSTRING, "DATE", (void *)datestr,"Date this file was written YYYY-mm-dd",  &status);
            //fits_write_history(outfptr, "Generated using rawtofits vers 0.0.0 from raw file:", &status);
         }
      }
      
      
      
      fits_close_file(outfptr,  &status);
   }
   
   /* if error occured, print out error message */
   if (status) fits_report_error(stderr, status);
   return(status);
  
}

namespace VisAO
{

class TestFramesDownloader : public TechnicalViewer
{
   public:

      TestFramesDownloader(string bcuName,              
                      string remoteIp, int remotePort, 
                      int frameCounterOffsetDw,
                      int frameSizeDw,
                      int bitPerPixel,
                      string shmBufProducerName, 
                      string baseShmBufName,    
                      int numFrames = -1);
      /*
       * Must be implemented by the derived class.
       */
      virtual void storeFrame()throw(NotInizializedBufferException);

};

/*TestFramesDownloader::TestFramesDownloader(string bcuName, string remoteIp, int remotePort, int frameSizeDw, int numFrames) :
           AbstractFramesDownloader(bcuName, remoteIp, remotePort, frameSizeDw, numFrames)*/
TestFramesDownloader::TestFramesDownloader(string bcuName, string remoteIp, int remotePort, int frameCounterOffsetDw, int frameSizeDw, int bitPerPixel,
                     string shmBufProducerName, string baseShmBufName,  int numFrames) :   TechnicalViewer(bcuName, remoteIp, remotePort, 
                     frameCounterOffsetDw, frameSizeDw, bitPerPixel, shmBufProducerName, baseShmBufName,numFrames)         
{
   
}

void TestFramesDownloader::storeFrame() throw(NotInizializedBufferException)
{
   static int i = 0;
   
   char fname[50];
   
   snprintf(fname, 25, "udpfits_%i.fits", i);
   
   std::cerr << "Storing " << i << ". . .\n";
   
   write_fits(fname, 1024, (short *)(_frame + 4*Constants::DWORD_SIZE));
   i++;
}


}
#include <errno.h>

int main()
{
   VisAO::TestFramesDownloader * tfd;
   Logger* _logger;
   _logger = Logger::get("BCU47");
   _logger->setLevel(Logger::LOG_LEV_TRACE);
   _logger->log(Logger::LOG_LEV_INFO, "Initializing");
   std::cout << "Initializing\n";
   try
   {
   tfd = new VisAO::TestFramesDownloader("BCU47", "visaosup.visao", 10016, 0, 524296, 16, "", "", 1);
   }
   catch(MasterDiagnosticException e)
   {
      std::cout << e.what() << "\n";
      throw;
   }
   //tfd = new VisAO::TestFramesDownloader("BCU47", "visaosup.visao", 10016, 0, 2056, 16, "", "", 1);
   std::cout << "Starting\n";
   errno = 0;
   tfd->run();
   std::cout << errno << "\n";
   std::cout << "Done\n";
}
