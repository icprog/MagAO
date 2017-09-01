//@File: pixelview.cpp
//
// Python module for fast pixel drawing
//
//
//
//-

#define USE_QWT

#include <Python.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/time.h>   // gettimeofday
#include <sys/shm.h>

#include <stdio.h>
#include <math.h>

#include "qwidget.h"
#include "qbrush.h"
#include "qpixmap.h"
#include "qpainter.h"
#include "qcolor.h"
#include "qimage.h"
#include "qlabel.h"
#include "qdrawutil.h"

#include <string>
#include <vector>

#include "Paths.h"

extern "C" {
#include "../lib/base/errordb.h"
}

#define HAS_SLOPES_IN_SLOPECOMPUTER
#include "bcu_diag.h"

#include "drawlib/drawobjects.h"

#ifdef USE_QWT
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_engine.h>
#endif


// Debug output
static int debug_output =0;
static int debug_output2 =0;

// Dati globali del modulo: pixmap, brushes, n. pixel e dimensioni

QPixmap *pixmap = NULL;
QBrush brushes[256];
QPen pens[256];
QColor colors[256];
QBrush red_brush;

#ifdef USE_QWT
QwtPlot *myPlotX = NULL;
QwtPlot *myPlotY = NULL;
QwtPlotCurve *plotCurveX = NULL;
QwtPlotCurve *plotCurveY = NULL;
double *hcut = NULL;
double *hcut_x = NULL;
double *vcut = NULL;
double *vcut_x = NULL;
double *rms_data = NULL;
double *rms_data_x = NULL;
#define RMS_BUFSIZE (512)
#endif

AbstractDiagnClass *diagbuf = NULL;

ccdFrame *background = NULL;
Slopes   *slopesnull = NULL;

int nx, ny;
int npixels;
float xfactor, yfactor;
int ccd_type;
char *ccdName = (char *)"";
int hotspot_x=-1, hotspot_y=-1;
int bit_window;
int binning;
int vwp_x, vwp_y, vwp_w, vwp_h;

int ccd_max_value = 0;
int init_ok=0;

// Indpup cache
#define MAX_INDPUP (6400)
int indpup[MAX_INDPUP];
int n_subap=0;


// Pupil intensity and actual pupil pixel count
int intensity[4];
int intensity_count[4];
float32 slope_rms=0;

int is_slope=0;       // nonzero if the last downloaded frame was a slope frame


// Zooming rect

int zoom_x1=0, zoom_y1=0, zoom_x2=0, zoom_y2=0;


// Funzioni chiamabili da Python, dichiarate extern "C"

extern "C" {
void initpixelview( void);
static PyObject *init( PyObject *, PyObject *);
static PyObject *setup( PyObject *, PyObject *);
static PyObject *PyGetPixelValue( PyObject *, PyObject *);
static PyObject *PyGetPupilIntensity( PyObject *, PyObject *);
static PyObject *PyGetPupilIntensityCount( PyObject *, PyObject *);
static PyObject *PyGetSlopeRms( PyObject *, PyObject *);
static PyObject *preparePixels( PyObject *, PyObject *);
static PyObject *drawPixels( PyObject *, PyObject *);
static PyObject *blitImage( PyObject *, PyObject *);
static PyObject *setHotSpot( PyObject *, PyObject *);
static PyObject *setDark( PyObject *, PyObject *);
static PyObject *setSlopesnull( PyObject *, PyObject *);
static PyObject *setBitWindow( PyObject *, PyObject *);
static PyObject *setViewport( PyObject *, PyObject *);
static PyObject *setZoomingRect( PyObject *, PyObject *);
static PyObject *finish( PyObject *, PyObject *);
static PyObject *initplot( PyObject *, PyObject *);
static PyObject *replot( PyObject *, PyObject *);
static PyObject *PyFillCutPlots( PyObject *, PyObject *);
void initGraphics();
}

void calcPupilIntensity( int pup_num);
void addRmsValue( double value);



// Registrazione dei metodi chiamabili da Python

static struct PyMethodDef pixelview_methods[] = {
    {"init", init, METH_VARARGS, "First init"}, 
    {"setup", setup, METH_VARARGS, "Module setup"}, 
    {"setBitWindow", setBitWindow, METH_VARARGS, "Bit window setup"},
    {"setViewport", setViewport, METH_VARARGS, "Set viewport"},
    {"preparePixels", preparePixels, METH_VARARGS, "Prepare pixels"},
    {"drawPixels", drawPixels, METH_VARARGS, "Draw pixels"},
    {"blitImage", blitImage, METH_VARARGS, "Blit image"},
    {"setDark", setDark, METH_VARARGS, "set a new dark"},
    {"setSlopesnull", setSlopesnull, METH_VARARGS, "set a new slopes null"},
    {"setHotSpot", setHotSpot, METH_VARARGS, "set pyramid hot spot"},
    {"getPixelValue", PyGetPixelValue, METH_VARARGS, "Returns the value of a single pixel"},
    {"getPupilIntensity", PyGetPupilIntensity, METH_VARARGS, "Returns the intensity of each pupil"},
    {"getSlopeRms", PyGetSlopeRms, METH_VARARGS, "Returns the current slope rms"},
    {"getPupilIntensityCount", PyGetPupilIntensityCount, METH_VARARGS, "Returns the intensity of each pupil"},
    {"setZoomingRect", setZoomingRect, METH_VARARGS, "Sets the current zooming rect"},
    {"initplot", initplot, METH_VARARGS, "Init plot"},
    {"replot", replot, METH_VARARGS, "Replot"},
    {"fillCutPlots", PyFillCutPlots, METH_VARARGS, "Fill plots"},
    {"finish", finish, METH_VARARGS, "Module shutdown"},
    {NULL, NULL, 0, NULL}
};

//+Function: initpixelview

// Function called by Python import mechanism

// This function is the equivalent of Python classes' __init__, and is
// automaticaly called by the interpreter when importing the module.
// The needed paint objects (a pixmap and many brushes) are created, and other
// default values are set.
//-

void initpixelview()
{
   (void) Py_InitModule("pixelview", pixelview_methods);

	// Set current intensity to zero
	memset(intensity,0, sizeof(int)*4);	
   memset(intensity_count, 0, sizeof(int)*4);

}

void clean()
{
    if (diagbuf)
      {
      delete diagbuf;
      diagbuf = NULL;
      }
}

//+Function: initGraphics
//
// initializes QT graphic objects
//-

void initGraphics()
{
    // Create pixmap and brushes

   if (pixmap)
	   delete pixmap;

   pixmap = new QPixmap( 1, 1, -1);

   printf("Pixmap depth: %d\n", pixmap->depth());
   for ( int i=0; i<256; i++)
	{
	brushes[i].setStyle( QBrush::SolidPattern);
	brushes[i].setColor( QColor( i, i, i));
   pens[i].setStyle( Qt::SolidLine);
   pens[i].setColor( QColor( i, i, i));
   colors[i] = QColor(i,i,i);
	}

   red_brush.setStyle( QBrush::SolidPattern);
   red_brush.setColor( Qt::red);

    // Set the bit window on the high 8 bits.
    bit_window = 0;
}


//+Function: init

// called to set global parameters like ccd type

// Calling syntax from Python: setup( ccd_type)

static PyObject *init( PyObject *self, PyObject *args)
{

	if (!PyArg_ParseTuple( args, "i", &ccd_type))
		{
		printf("Error parsing arguments\n");
		return NULL;
		}

    switch (ccd_type)
      {
      case 39:
      ccd_max_value = 16383;
      ccdName = (char *)"ccd39";
      break;

      case 47:
      ccd_max_value = 16383;
      ccdName = (char *)"ccd47";
      break;

      case 100:
      ccd_max_value = 1023;
      ccdName = (char *)"thorlabs";
      break;

      case 200:
      ccd_max_value = 16383;
      ccdName = (char *)"irtc";
      break;

      default:
      break;
      }

#ifdef USE_QWT
    rms_data = new double[RMS_BUFSIZE];
    rms_data_x = new double[RMS_BUFSIZE];
    memset(rms_data, 0, sizeof(double)* RMS_BUFSIZE);
    memset(rms_data_x,0, sizeof(double)* RMS_BUFSIZE);
    for (int x=0; x<RMS_BUFSIZE; x++) {
       rms_data[x]=1;
       rms_data_x[x] = x;
    }
#endif


	return Py_BuildValue("i", NO_ERROR);

}

static PyObject *replot( PyObject *self, PyObject *args)
{
   int n;
   int w=-1;
   int h=-1;

	if (!PyArg_ParseTuple( args, "i|ii", &n, &w, &h))
		{
		printf("Error parsing arguments\n");
		return NULL;
		}

#ifdef USE_QWT

   if (ccd_type == 39)
        addRmsValue(slope_rms);

   QwtPlot *p = NULL;
   if (n==0)
      p = myPlotX;
   else if (n==1)
      p = myPlotY;
   else
      return Py_BuildValue("i", NO_ERROR);

   if (p) {
      if (w!=-1) {
         p->resize(w,h);
      }
      p->replot();
   }
#endif

	return Py_BuildValue("i", NO_ERROR);
}


static PyObject *initplot( PyObject *self, PyObject *args)
{
   long winID0=-1;
   long winID1=-1;

#ifdef USE_QWT
	if (!PyArg_ParseTuple( args, "l|l", &winID0, &winID1))
		{
		printf("Error parsing arguments\n");
		return NULL;
		}

	// Find the widget to draw into
	QWidget *widgetX = QWidget::find( winID0);
	if (!widgetX) {
		printf("Error: no widget found for %d\n", (int)winID0);
		return NULL;
		}
   else {
      myPlotX = new QwtPlot(widgetX);
      myPlotX->enableAxis(QwtPlot::xBottom,false);
      myPlotX->enableAxis(QwtPlot::yLeft,true);
      myPlotX->setAxisScaleEngine(QwtPlot::yLeft, new QwtLog10ScaleEngine);


      plotCurveX = new QwtPlotCurve("Curve1");
      plotCurveX->attach(myPlotX);

      myPlotX->replot();
   }

   if (winID1 != -1) {
	   QWidget *widgetY = QWidget::find( winID1);
	   if (!widgetY) {
		   printf("Error: no widget found for %d\n", (int)winID1);
		   return NULL;
		}
      else {

         myPlotY = new QwtPlot(widgetY);
         myPlotY->enableAxis(QwtPlot::xBottom,true);
         myPlotY->enableAxis(QwtPlot::yLeft,false);
         myPlotY->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);

         plotCurveY = new QwtPlotCurve("Curve1");
         plotCurveY->attach(myPlotY);

         myPlotY->replot();
      }
   }

#endif

	return Py_BuildValue("i", NO_ERROR);
}

//+Function: setup

// called to set viewer (CCD) size and pixel size

// Calling syntax from Python: setup( pixels_x, pixels_y, pixel_size)

// This function must be called before any viewer operation to set
// the viewer size, expressed both in pixel number (X and Y axis), and
// in single pixel dimension (in screen pixels). It can also be called
// at any time to change the viewer's appereance or to adapt to a CCD
// change (rare)
//-

static PyObject *setup( PyObject *self, PyObject *args)
{

	if (!PyArg_ParseTuple( args, "iiffiiiii", &nx, &ny, &xfactor, &yfactor, &binning,
                                             &vwp_x, &vwp_y, &vwp_w, &vwp_h))
		{
		printf("Error parsing arguments\n");
		return NULL;
		}

	if (!pixmap)
		initGraphics();

	// Avoid any zero argument
	
	if (xfactor==0)
		xfactor =1;
	if (yfactor==0)
		yfactor =1;
	if (nx<1)
		nx=1;
	if (ny<1)
		ny=1;

	if (debug_output)
		{
		printf("xpix, ypix: %5.2f,%5.2f\n", xfactor, yfactor);
		printf("nx, ny: %d,%d\n", nx, ny);
		}

	int xdim = (int)(nx * xfactor);
	int ydim = (int)(ny * yfactor);
	pixmap->resize( xdim, ydim);
   pixmap->setOptimization( QPixmap::BestOptim);

    if (debug_output)
      printf("<<---- PIXMAP RESIZE OK\n");

    npixels = nx*ny;

    if (debug_output)
	      printf("Npixels: %d\n", npixels);


    clean();

    switch (ccd_type)
      {
      case 39:
      diagbuf = new OptLoopDiagnClass();
      break;

      case 47:
      diagbuf = new TechViewerDiagnClass();
      break;

      // thorlabs
      case 100:
      diagbuf = new ThorlabsDiagnClass();
      break;

      // irtc
      case 200:
      diagbuf = new IrtcDiagnClass();
      break;
      }

    // Plot data
#ifdef USE_QWT
    if (hcut)
       delete[] hcut;
    if (vcut)
       delete[] vcut;
    if (hcut_x)
       delete[] hcut_x;
    if (vcut_x)
       delete[] vcut_x;
    hcut = new double[nx];
    vcut = new double[ny];
    hcut_x = new double[nx];
    vcut_x = new double[ny];
#endif

   if (ccd_type == 39) {
  // open the right pupilsfile for the current binning
      string pupilsfile = Arcetri::Paths::CurIndpup( ccdName);
      FILE *fp = fopen( pupilsfile.c_str(), "r");

      int count=0;
      indpup[count]=-1;
      if (fp) {
         printf("Loading indpup %s\n", pupilsfile.c_str());
         while (!feof(fp) && count<MAX_INDPUP-1) {
			   int value;
			   if (fscanf(fp, "%d", &value) == 1) 
                indpup[count++] = value;
         }
         n_subap=count/4;
         indpup[count]=-1;
		   fclose(fp);
			}
		}


    if (debug_output)
         {
         printf("Viewport: %d,%d -> %dx%d\n", vwp_x, vwp_y, vwp_w, vwp_h);
         printf("Exiting setup\n");
         }

	return Py_BuildValue("i", NO_ERROR);
}

//@Function: setBitWindow 
//
// sets the visible bit window
//
// Calling syntax from Python: setBitWindow( bit)
//
// This function sets the visible bit window for pixel values.
// Viewer visible values are in the range 0-255. These must be re-mapped
// on CCD possible values, which are usually in the 0-65535 range.
// The argument for this function is the number of bits to right-shift
// the pixel value. For example, a value of 8 will display only the
// high byte of pixel values. A value of 0 will display only the low byte.
//@

static PyObject *setBitWindow( PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple( args, "i", &bit_window))
		return NULL;

	return Py_BuildValue("i", 1);
}

static PyObject *setViewport( PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple( args, "iiii", &vwp_x, &vwp_y, &vwp_w, &vwp_h))
		return NULL;

	return Py_BuildValue("i", NO_ERROR);
}

static PyObject *setZoomingRect( PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple( args, "iiii", &zoom_x1, &zoom_y1, &zoom_x2, &zoom_y2))
		return NULL;

	return Py_BuildValue("i", NO_ERROR);
}

static PyObject *setHotSpot( PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple( args, "ii", &hotspot_x, &hotspot_y))
		return NULL;

	return Py_BuildValue("i", NO_ERROR);
}

//@Function: setDark
//
// Sets a new background
//
// Receives a binary buffer with the background pixels to be
// subtracted from the image
//@

static PyObject *setDark( PyObject *self, PyObject *args)
{
	char *filename;

	if (!PyArg_ParseTuple( args, "s", &filename))
		return NULL;

   if (background)
      delete background;

   background = new ccdFrame(ccdName);
   background->load(filename);

	return Py_BuildValue("i", 0);
}

//@Function: setSlopesnull
//
// Sets a new slopes null frame
//
// Receives a binary buffer with the slope values
//@

static PyObject *setSlopesnull( PyObject *self, PyObject *args)
{
   char *filename;

   if (!PyArg_ParseTuple( args, "s", &filename))
      return NULL;

   if (slopesnull)
      delete slopesnull;

   slopesnull = new Slopes(ccdName);
   slopesnull->loadRaw(filename);

   return Py_BuildValue("i", 0);
}

//@Function: CCDtoScreen
//
// Converts from CCD coordinates to screen coordinates.
//
//@

void CCDtoScreen( int x, int y, int *screen_x, int *screen_y)
{
   // thorlabs
   if ((ccd_type == 100) or (ccd_type == 200))
       {
       *screen_y = y;
       *screen_x = x;
       return;
       }

	*screen_y = nx-x-1;
	*screen_x = y;
}

void CCDtoScreenF( float x, float y, float *screen_x, float *screen_y)
{
   // thorlabs
   if ((ccd_type == 100) or (ccd_type == 200))
         {
         *screen_y = y;
         *screen_x = x;
         return;
         }

   *screen_y = nx-x-1;
   *screen_x = y;
}


//@Function: ScreenToCCD
//
// Converts from CCD coordinates to screen coordinates.
//
//@

void ScreenToCCD( int x, int y, int *ccd_x, int *ccd_y)
{
   // thorlabs
   if ((ccd_type == 100) or (ccd_type == 200))
       {
       *ccd_y = y;
       *ccd_x = x;
       return;
       }

	*ccd_y = x;
	*ccd_x = ny-y-1;
}

void FrameCCDtoScreen( uint16 *src, uint16 *dest)
{
  // CCDtoScreen remapping
  for (int y=0; y<ny; y++)
     for ( int x=0; x<nx; x++)
        {
        int screen_x, screen_y;
        CCDtoScreen( x, y, &screen_x, &screen_y);
        int coord1 = y*nx +x;
        int coord2 = screen_y * nx + screen_x;
        dest[coord2] = src[coord1];
	//printf("src[%d] = dest[%d]\n", coord2, coord1);
        }
}


//@Function: preparePixels
//
// Preprocess a raw pixel buffer so that pixels are ready for display.
// Operations:
// - pixel/slope selection
// - pixel reordering
// - background subtraction
//
// Calling syntax from Python:
//
// preparePixels( pixelbuffer, subtractBackground, show_slopes)

static PyObject *preparePixels( PyObject *self, PyObject *args)
{
	int subtractbackground=0;
   int show_slopes=0;
   unsigned char *pixeldata;
   int pixellen;
   int checkWhiteFrame;

	if (!PyArg_ParseTuple( args, "s#|iiii", &pixeldata, &pixellen, &subtractbackground, &show_slopes, &checkWhiteFrame))
		return NULL;

   if (!diagbuf)
         {
         printf("diagbuf is NULL, skipping\n");
	      return Py_BuildValue("i", 1);
         }

   diagbuf->setData( pixeldata);

   if (debug_output) printf("Pixel data set!\n");

   init_ok=1;

   if (checkWhiteFrame) {
      int n=100;
      int start_pixel = 300*1024;;
      if (pixellen<n+start_pixel)
         start_pixel=pixellen-n;
      uint16 *pixels = (uint16*)diagbuf->pixels_raster();
      int whiteCount=0;
      for (int i=start_pixel; i<n+start_pixel; i++) {
         if (debug_output2) printf("%d ", pixels[i]);
         if (pixels[i] == 65535)
            whiteCount++;
      }
      if (debug_output2) printf("\n");
      if (whiteCount == n) {
         if (debug_output) printf("White frame detected\n");
	      return Py_BuildValue("i", 1);
      }

   }

   // Background subtraction

   if ((subtractbackground) && (background)) {
       if (debug_output) printf("Background subtraction active\n");
       if (diagbuf->pixels_len() == background->len()) {
           uint16 *pixels = (uint16*)diagbuf->pixels_raster();
           uint16 *backg  = (uint16*)background->raster();

           // Subtract background, but keep saturated pixels at their maximum value

           for (int c=0; c<npixels; c++)
             if ( pixels[c] != ccd_max_value)
                pixels[c] -= backg[c];
       } else
          printf("Skipping background subtraction: frame size does not match (frame: %d - background: %d)\n", diagbuf->pixels_len(), background->len());
   }

    if (debug_output) printf("Done reordering\n");

    // Set external flag
    is_slope =  (show_slopes) ? 1: 0;

    // Calculate slope rms
    if (ccd_type == 39) {
       int x,y, slope_count=0;
       float slope_avg=0;
       for (x=0; x<nx; x++)
          for (y=0; y<ny; y++) {
             float slopevalue = diagbuf->slopes_raster()[y*nx+x];
             if (slopevalue != 0) {
                slope_count++;
                slope_avg += slopevalue;
             }
          }
       if (slope_count == 0) {
          slope_rms = 0;
          return Py_BuildValue("i", 0);
       }
       slope_avg /= slope_count;

       float stddev=0;
       for (x=0; x<nx; x++)
          for (y=0; y<ny; y++) {
             float slopevalue = diagbuf->slopes_raster()[y*nx+x];
             if (slopevalue != 0)
                stddev += (slopevalue - slope_avg) * (slopevalue - slope_avg);
          }
       stddev = sqrt( stddev / slope_count);

       slope_rms = stddev;
    }
    else {
       slope_rms=0;
    }


    return Py_BuildValue("i", 0);
}	




//@Function: drawPixels
//
// draws the pixels found in the pixel buffer
//
// Calling syntax from Python:
//
// drawPixels( drawPupils, rtpupils, pup_split, diam, cx, cy)
//
// This function renders the contents of the pixel buffer into
// an internal pixmap. Pixels will
// be reordered using the appropriate BCU reorder table.
//
// Additional flags are:
// - drawPupils: if nonzero, will superimpose the selected pupils on theimage
//@


static PyObject *drawPixels( PyObject *self, PyObject *args)
{
	int drawpupils=0;
   int rtpupils=0;
   int pup_split=0;
   PyObject *diam, *cx, *cy;
   int log= 0;
   int gain0=0;

	if (!PyArg_ParseTuple( args, "iiiOOO|ii", &drawpupils, &rtpupils, &pup_split, &diam, &cx, &cy, &log, &gain0))
		return NULL;

   if (!init_ok)
         return Py_BuildValue("i", 1);

   if (!diagbuf)
         {
         printf("diagbuf is NULL, skipping\n");
         }

   if ((!(diagbuf->pixels_raster())) || (!pixmap))
         {
         printf("diagbuf->pixels_raster() or pixmap is NULL, skipping\n");
         return Py_BuildValue("i", 1);
         }


	// Draw pixels, shifting them according to the bit window 
	QPainter painter( pixmap);

   if (debug_output) printf("Start drawing\n");
   struct timeval start_time;
   gettimeofday(&start_time, NULL);

   painter.setPen(pens[0]);

   // Avoid division by zero
   if (xfactor==0) xfactor=1;
   if (yfactor==0) yfactor=1;

   int drawn=0;
   int xrec = (xfactor<1)? 1 : (int)xfactor;
   int yrec = (yfactor<1)? 1 : (int)yfactor;
   int x_inc = (xfactor<1)? int(1.0/xfactor) : 1;
   int y_inc = (yfactor<1)? int(1.0/yfactor) : 1;

   int min_x = (int)(vwp_x / xfactor);
   int min_y = (int)(vwp_y / yfactor);
   int max_x = min_x + (int)vwp_w/xfactor +1;
   int max_y = min_y + (int)vwp_h/yfactor +1;
   if (max_x > nx)
      max_x = nx;
   if (max_y > ny)
      max_y = ny;

   if (debug_output)
      {
      printf("viewport: %d,%d -> %dx%d\n", vwp_x, vwp_y, vwp_w, vwp_h);
      printf("x,y limits: %d,%d - %d,%d\n", min_x, min_y, max_x, max_y);
      }

   QPen oldPen = painter.pen();
   painter.setPen( Qt::NoPen);
   QBrush *lastbrush = NULL;

   float maxlog = logf(ccd_max_value);

	for (int y=min_y; y< max_y; y+=y_inc)
		for (int x=min_x; x< max_x; x+=x_inc)
			{
			int value=0;
         int saturated=0;
         float slopevalue=0;
         int xs, ys;


         // Get either the slope value or the pixel value
         if (is_slope) {

	    // First half, slopes null

            if (x < nx/2) {
                ScreenToCCD( x+nx/2, y, &xs, &ys);
                slopevalue=0;
                if (slopesnull)
                    slopevalue = ((float32*) slopesnull->raster())[ys*nx+xs];
            } else {
                // Second half, realtime slopes
                ScreenToCCD( x, y, &xs, &ys);
                slopevalue = diagbuf->slopes_raster()[ys*nx+xs];
            }

/*
           ScreenToCCD( x, y, &xs, &ys);
           slopevalue = diagbuf->slopes_raster()[ys*nx+xs];
*/

           value = int(((slopevalue*bit_window/10.0)+2)*255/4);
         } else {
            ScreenToCCD( x, y, &xs, &ys);
            value = diagbuf->pixels_raster()[ys*nx+xs];

            // Zero out "negative" pixels
            if (value > 32767)
               value=0;
            if (value == ccd_max_value)
               saturated = true;

            if (log) {
               value = value/(bit_window==0)?1:bit_window;
               if (value<1) value=1;
               value = int( logf(value) * ccd_max_value / maxlog);
               value =  value / (ccd_max_value/255);
            } else {
               value = (int) ((float)value * (bit_window/2.0));
               value = value / (ccd_max_value/255);
            }
         }

        // Set limits
         if (value <0) value =0;
         if (value > 255) value = 255;

         int x1 = (int)(x*xfactor);
         int y1 = (int)(y*yfactor);

         drawn++;

         QBrush *usebrush;
         if (saturated)
            usebrush = &red_brush;
         else
            usebrush = &brushes[value];

         if (lastbrush != usebrush) {
            lastbrush = usebrush;
            painter.setBrush(*usebrush);
         }
         painter.drawRect( x1, y1, xrec, yrec);

         }


   painter.setPen( oldPen);

   struct timeval end_time;
   gettimeofday(&end_time, NULL);


   if (debug_output) printf("End drawing, drawn %d pixels\n", drawn);

   double difftime = end_time.tv_sec - start_time.tv_sec;
   difftime += (end_time.tv_usec - start_time.tv_usec) / 1e6;

   if (debug_output) printf("Elapsed time: %5.2f ms\n", difftime * 1e3);

   if ((hotspot_x>=0) && (hotspot_y>=0)) 
      {
      int b = binning;		// Get binning only once to avoid race conditions
      if (b>0) {
         painter.setPen(Qt::green);

         int x1, y1, x2, y2, x3, y3, x4, y4;
         CCDtoScreen( hotspot_x/b-10, hotspot_y/b, &x1, &y1);
         CCDtoScreen( hotspot_x/b+10, hotspot_y/b, &x2, &y2);
         CCDtoScreen( hotspot_x/b, hotspot_y/b-10, &x3, &y3);
         CCDtoScreen( hotspot_x/b, hotspot_y/b+10, &x4, &y4);

         painter.moveTo( (int)(x1 *xfactor), (int)(y1 * yfactor));
         painter.lineTo( (int)(x2 *xfactor), (int)(y2 * yfactor));
         painter.moveTo( (int)(x3 *xfactor), (int)(y3 * yfactor));
         painter.lineTo( (int)(x4 *xfactor), (int)(y4 * yfactor));
         }
     }


    if (gain0) {
       painter.setPen(Qt::yellow);
       painter.setFont( QFont( "times", 20, 75));
       painter.drawText( 10, 20, "LOOP PAUSED");
    }


   if ((diam) && (cx) && (cy) && (rtpupils))
      {
      painter.setPen(Qt::red);
      painter.setBrush(Qt::NoBrush);
      int num =1;
      if (pup_split) num =4;
      for (int i=0; i<num; i++)
         {
         double d = PyFloat_AsDouble( PySequence_GetItem( diam, i));
         double x = PyFloat_AsDouble( PySequence_GetItem( cx, i));
         double y = PyFloat_AsDouble( PySequence_GetItem( cy, i));

         float x1 = x-d/2.0;
         float y1 = y-d/2.0;
         float x2 = x+d/2.0;
         float y2 = y+d/2.0;

         float s_x, s_y;
         float s_x1, s_y1, s_x2, s_y2;
         CCDtoScreenF( x1, y1, &s_x1, &s_y1);
         CCDtoScreenF( x2, y2, &s_x2, &s_y2);
         CCDtoScreenF( x, y, &s_x, &s_y);

         painter.drawEllipse( (int)(s_x1 * xfactor), (int)(s_y1 * yfactor), (int)((s_x2-s_x1)*xfactor), (int)((s_y2-s_y1)*yfactor));

         painter.moveTo( (int)(s_x *xfactor), (int)(s_y1 * yfactor));
         painter.lineTo( (int)(s_x *xfactor), (int)(s_y2 * yfactor));
         painter.moveTo( (int)(s_x1 *xfactor), (int)(s_y * yfactor));
         painter.lineTo( (int)(s_x2 *xfactor), (int)(s_y * yfactor));

         }
      }

   for (int i=0; i<4; i++)
      calcPupilIntensity(i);

	// Draw the pupils if requested
	if (drawpupils) {	
		painter.setRasterOp( Qt::XorROP);
		int pitch = nx;
      int count=0;
      int value;

      while((value = indpup[count++]) != -1) {
			int y = value / pitch;
			int x = value % pitch;
			int screen_x, screen_y;

         //printf("%04d %04d: %03d, %03d\n", value, count-1, x, y);

			CCDtoScreen( x, y, &screen_x, &screen_y);
			painter.fillRect( (int)(screen_x*xfactor), (int)(screen_y*yfactor), xrec, yrec, red_brush);
		}
		painter.setRasterOp( Qt::CopyROP);
	}

   // Draw zooming rect if necessary
   if ((zoom_x2 - zoom_x1 >1) && (zoom_y2 - zoom_y1) >1)
      {
      painter.setPen( Qt::red);
      painter.drawRect( zoom_x1, zoom_y1, zoom_x2 - zoom_x1, zoom_y2 - zoom_y1); 
      }

   // Draw objects
   drawobject *obj;
   char objname[64];
   strcpy( objname, "ccdviewer_");
   strncat( objname, ccdName,54);
   while((obj = getObject(objname)) != NULL)
      {
      obj->Draw(&painter, xfactor, yfactor, &CCDtoScreenF);
      delete obj;
      }
		
	painter.end();

	return Py_BuildValue("i", 1);
}	

void calcPupilIntensity( int pup_num)
{
   if (!diagbuf)
      return;
   if (!(diagbuf->pixels_raster()))
      return;

   if (n_subap<1)
      return;

   int total = 0;
   int count = 0;
   int pitch=nx;
   if (pitch==0)
      return;

   for (int i=0; i<n_subap; i++) {
      int index = indpup[i + n_subap*pup_num];

      int x = index % pitch;
      int y = index / pitch;

      uint16 pixel = diagbuf->pixels_raster()[ y*nx +x];
      if (pixel < 32767)
         total += pixel;
      else
         total -= (65536-pixel);
      count++;
   }

   if (count != 0)
      intensity[pup_num] = total/count;
   else
      intensity[pup_num] = total;
   intensity_count[pup_num] = count;
}


static PyObject *blitImage( PyObject *self, PyObject *args)
{
	long winID;

	if (!PyArg_ParseTuple( args, "l", &winID))
		return NULL;

   if (!diagbuf)
      return Py_BuildValue("i", 1);
   if (!(diagbuf->pixels_raster()))
      return Py_BuildValue("i", 1);

	// Find the widget to draw into
	QWidget *widget = QWidget::find( winID);
	if (!widget)
		{
		printf("Error: no widget found for %d\n", (int)winID);
		return NULL;
		}


   if (debug_output) printf("Start blit to widged %d, coords %d,%d\n", (unsigned int)widget, vwp_x, vwp_y);

   
	bitBlt( widget, 0, 0, pixmap, 0, 0, -1, -1, Qt::CopyROP);

   if (debug_output) printf("End blit\n");

	return Py_BuildValue("i", 1);
}	

//@Function: getPixelValue
//
// Returns the value of the specified pixel, as found in the pixel buffer
//@

static PyObject *PyGetPixelValue( PyObject *self, PyObject *args)
{
	int x,y;
	int int_value=0;
   float32 float_value=0;

	if (!PyArg_ParseTuple( args, "ii", &x, &y))
		return NULL;

   if (!diagbuf)
	   return Py_BuildValue("f", 0);
   if (!(diagbuf->pixels_raster()))
	   return Py_BuildValue("f", 0);

    if (is_slope) {
        int xs, ys;
        if (x < nx/2) {
           ScreenToCCD( x+nx/2, y, &xs, &ys);
           if (slopesnull)
               float_value = ((float32*) slopesnull->raster())[ys*nx+xs];
        } else {
           ScreenToCCD( x, y, &xs, &ys);
           float32 *ptr = diagbuf->slopes_raster();
           if (ptr)
              float_value = ptr[ xs +ys * nx];
        }
        return Py_BuildValue("f", float_value);
    }
    else {
        int xs, ys;
        uint16 *ptr;

        ScreenToCCD( x, y, &xs, &ys);
        ptr = diagbuf->pixels_raster();
        if (ptr)
	         int_value = ptr[ xs + ys*nx];
        if (int_value > 32767)
           int_value -= 65536;
	    return Py_BuildValue("i", int_value);
    }
}

//@Function: fillCutPlots
//
// Fills the plots with the horizontal and vertical cuts at the specified pixel
//@

static PyObject *PyFillCutPlots( PyObject *self, PyObject *args)
{
	int xp,yp;
   double hmin, hmax, vmin, vmax;

#ifdef USE_QWT
   hmin=vmin=1e9;
   hmax=vmax=-1e9;

   if (xfactor==0) xfactor=1;
   if (yfactor==0) yfactor=1;

   int min_x = (int)(vwp_x / xfactor);
   int min_y = (int)(vwp_y / yfactor);
   int max_x = min_x + (int)vwp_w/xfactor +1;
   int max_y = min_y + (int)vwp_h/yfactor +1;
   if (max_x > nx)
      max_x = nx;
   if (max_y > ny)
      max_y = ny;


	if (!PyArg_ParseTuple( args, "ii", &xp, &yp))
		return NULL;

   if (!diagbuf)
	   return Py_BuildValue("f", 0);
   if (!(diagbuf->pixels_raster()))
	   return Py_BuildValue("f", 0);


    if (is_slope) {
        float32 *ptr = diagbuf->slopes_raster();
        if (!ptr)
           return Py_BuildValue("f", 0);
        for(int x=min_x; x<max_x; x++) {
           hcut[x] = (double) ptr[x +yp*nx];
           hcut_x[x] = x;
           if (hcut[x]<hmin)
              hmin = hcut[x];
           if (hcut[x]>hmax)
              hmax = hcut[x];
        }
        for(int y=min_y; y<max_y; y++) {
           vcut[y] = (double) ptr[xp +y*nx];
           vcut_x[y] = y;
           if (vcut[y]<vmin)
              vmin = vcut[y];
           if (hcut[y]>vmax)
              vmax = vcut[y];
        }
    }
    else {
        int xs, ys;
        uint16 *ptr = diagbuf->pixels_raster();
        if (!ptr)
           return Py_BuildValue("f", 0);
        for(int x=min_x; x<max_x; x++) {
           ScreenToCCD( x, yp, &xs, &ys);
           double p = (double) ptr[xs +ys*nx];
           if ((p >32767) || (p <1))
               p =1;
           if (p<hmin)
              hmin = p;
           if (p>hmax)
              hmax = p;
           hcut_x[x-min_x] = x;
           hcut[x-min_x] =p;
        }
        for(int y=min_y; y<max_y; y++) {
           ScreenToCCD( xp, y, &xs, &ys);
           double p = (double) ptr[xs +ys*nx];
           if ((p >32767) || (p <1))
               p =1;
           if (p<vmin)
              vmin = p;
           if (p>vmax)
              vmax = p;
           vcut_x[y-min_y] = min_y-y-1;
           vcut[y-min_y] = p;
        }
    }

    plotCurveX->setData( hcut_x, hcut, max_x-min_x);
    plotCurveY->setData( vcut, vcut_x, max_y-min_y);

    // Calc a good decimal range

    float e_hmin = exp10(int(log10(hmin))); 
    hmin = hmin - fmod(hmin, e_hmin);
    if (hmin<1) hmin=1;
    float e_vmin = exp10(int(log10(vmin))); 
    vmin = vmin - fmod(vmin, e_vmin);
    if (vmin<1) vmin=1;
     
    myPlotX->setAxisScale( QwtPlot::yLeft, hmin, hmax, hmax-hmin);
    myPlotY->setAxisScale( QwtPlot::xBottom, vmin, vmax, vmax-vmin);
#endif

	 return Py_BuildValue("i", NO_ERROR);
}


//@Function: addRmsValue
//
// Add a value to the rms plot
//@

void addRmsValue( double value)
{
#ifdef USE_QWT
   // static int xpos=1;
	// int xp,yp;
   double hmin, hmax; // vmin, vmax;


   int min_x=0;
   int max_x = RMS_BUFSIZE-1;

   // Add value at the end of the plot
   memmove( rms_data, rms_data+1, sizeof(double)*(RMS_BUFSIZE-1));
   rms_data[RMS_BUFSIZE-1] = value;

   plotCurveX->setData( rms_data_x, rms_data, RMS_BUFSIZE);

#endif

}







//@Function: getPupilIntensity
//
// Returns a four-element tuple with the 4 pupils intensity
//@

static PyObject *PyGetPupilIntensity( PyObject *self, PyObject *args)
{
	return Py_BuildValue("(iiii)", intensity[0], intensity[1], intensity[2], intensity[3]);
}

//@Function: getSlopeRms
//
// Returns the current slope rms value
//@

static PyObject *PyGetSlopeRms( PyObject *self, PyObject *args)
{
	return Py_BuildValue("f", slope_rms);
}

//@Function: getPupilIntensityCount
//
// Returns a four-element tuple with the 4 pupils intensity counts
//@

static PyObject *PyGetPupilIntensityCount( PyObject *self, PyObject *args)
{
	return Py_BuildValue("(iiii)", intensity_count[0], intensity_count[1], intensity_count[2], intensity_count[3]);
}


//@Function: finish
//
// Module shutdown
//@

static PyObject *finish( PyObject *self, PyObject *args)
{
   clean();
	return Py_BuildValue("i", 1);
}	

