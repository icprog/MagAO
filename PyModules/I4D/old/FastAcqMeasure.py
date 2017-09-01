from Scripting.App import *
from Scripting.Modify import *
from Scripting.Data import *
from Scripting.Config import *
from Scripting.Measurements import *
from Scripting.Show import *
import time, os, os.path, gc
from PhazeMonkey.Util import FastAcq

from Commons import Constants

# You will need to select the proper import statement depending on your framegrabber.
# Older framegrabbers use the first line, newer use the second. You will get an
# appropriate error message if you choose wrong.
#from PhazeMonkey.Hardware.FrameSource import IFC56_ext as IFC_ext
from PhazeMonkey.Hardware.FrameSource import IFC58_ext as IFC_ext

'''
This script demonstrates the use of StreamToDisk, which is the fastest way
to capture large amounts of raw data.  You pass a path and optional filename,
the number of frames to capture, and various options, and it saves a sequence
of frames to disk.

There are two functions in this script: acquire() and calibrate(). Acquire()
acquires the data from the camera and streams it to disk with very little
processing, for maximum acquisition speed. The only processing it does perform-
packing the camera bits- actually increases the throughput. Calibrate() reads
in the streamed files, applies hardware-specific corrections, and saves the
calibrated frames. (It might be possible to perform the calibrate step independent
of the hardware with additional work, but it is quite fast, taking about the same
time as the acquisition itself.)

StreamToDisk returns a tuple containing information about each frame and about
the camera.

  (frameInfo, camInfo) = IFC_ext.StreamToDisk( folder_path, num_meas [, options] )

folder_path is the folder in which to store the data.  It must already exist.
num_meas is the number of frames to capture.  For large frames and high rates,
  the system may not be able to keep up forever.  Capturing will stop if
  it can't keep up.

OPTIONS:
Options to the method may be passed as part of the argument tuple, or as keywords.
They are described here in the order they should appear if not using keywords.

  filename          Base name of the files to be created.  Default is 'meas'.  The actual 
                    file names have _N appended, where N is an integer starting at 0.

  x0, dx, y0, dy    The limits of the frame to save.  If you don't need the entire frame,
                    you can select a rectangular subsection with these parameters.  This
                    increases the maximum sustainable frame rate.  Default is full frame.

  pack              Pack the camera bits in the output file.  This defaults to 1,
                    which is usually the best choice as it reduces the amount of data
                    written to disk, and thus increases the maximum frame rate (and
                    saves disk space).  However, you can set it to 0 if you want to
                    save the data as 16-bit numbers in the file.

RETURN VALUES:
frameInfo is an array of tuples, each representing information about one frame. 
The size of this array is the number of frames actually saved to disk.  Here is
what each entry in the array contains, in the order they appear in the tuple:
  acquisitionTime   Time the frame was acquired (relative to the first one) in microseconds
  openTime          Time spent opening and closing the file for this frame, in milliseconds
  writeTime         Time spent writing the file for this frame, in milliseconds
  thisSeqNum        Sequence number of this frame (should match the index of the frame)
  possiblyBad       bit 0 is the possiblyBad bit set by framegrabber, bit 1 is set if
                     the capture was aborted at this frame.
  numLocked         Number of locked frames when this frame was captured.  Should be 1.
  numLost           Number lost when extracting the frames from memory.
  numMissed         Number of frames taken by the camera but not captured by the framegrabber
  nextSeqNum        The sequence number of the last frame put into the ring buffer by the
                     framegrabber.  If this grows to 40 more than thisSeqNum, the ring buffer
                     will become full and the acquisition will stop.
  waiting           The number of frames waiting in the ring buffer, which should be equal
                     to nextSeqNum - thisSeqNum


camInfo is a dictionary of information grabbed from the camera.  It is mostly unintersting
except for debugging.  Look at the keys in the dictionary to see what's in it.  (You may
need the Coreco IFC software manual to understand their meanings.)

FILE FORMAT:
The files saved contain a 1024-bit header at the start, followed by the raw data, saved
in native byte order (little-endian on all Windows machines, and hopefully will stay that way).
Before saving, the data is usually packed, based on the word-size of the camera.  Packing
can be inhibited by the 'pack=0' keyword, as noted above.

The header is in keyword-value format, where the keywords are delimited by an = sign,
and the value is delimited by a \n character.  String values are denoted by the use
of a double = sign.  For example:

X0=0
Y0=512
WORD_SIZE=12
SOMETHING_FLOATY=100.23
CAMERA_NAME==Illunis XV27,000,000

A null character indicates the end of the header info.  Here are
the keywords used:

HEADER_LENGTH   Can be used to extend the header past 1024 bytes (NOT YET IMPLEMENTED.
                 If this keyword is missing, the header is 1024 bytes.

X0              Horizontal start of the data, in pixels

Y0              Vertical start of the data, in pixels

DX              Width of the data, in pixels

DY              Height of the data, in pixels

WORD_SIZE       Number of bits per pixel.  If not 16, the data is packed

WORD_SHIFT      Only present if data not packed.  Shift the data by this many
                 bits to recover the correct values.  Negative numbers mean
                 shift right, positive mean shift left.

ACQ_TIME        Time (relative to the start of an acquisition) when the frame
                 was collected from the camera by the frame grabber.



KNOWN ISSUES:

1. You must use the PACK option. This is the best mode for almost all applications anyway.

2. X0, Y0, DX, and DY currently must be multiples of 4. The script will automatically
truncate non-multiples to the next lowest multiple of 4.

'''

''' ======== Define some user-selectable parameters ======== '''
_folder_root = Constants.I4D_DATA_PATH
_raw_folder_path = Constants.I4D_RAW_DATA_PATH
_calibrated_folder_path = Constants.I4D_CALIB_DATA_PATH
_pack = 1
# Use the following to limit the acquisition area, or use defaults of 0, 0, -1, -1 for max area.
_x0 = 0
_y0 = 0
_dx = -1         # Default of -1 uses max area
_dy = -1         # Default of -1 uses max area
_meas_sample = 1 # keep every nth measurement, delete the others
''' ======== End user-selectable parameters. ======== '''




# Get the framesource. This represents the camera and framegrabber.
fs = Store.FetchKey('hardware.inventory.framesource')


try:
    x_cam_size, y_cam_size = IFC_ext.GetSize()
except Exception, e:
    MessageBox("Could not acquire; you might need to modify this script.\nSee lines 10-14 of this script. (Error: %s)" % str(e))


fmr = fs.GetFactoryRect()
if fmr:
    fmr_l, fmr_t, fmr_w, fmr_h = fmr # unpack
else:
    fmr_l, fmr_t, fmr_w, fmr_h = 0, 0, x_cam_size, y_cam_size

# Removed for Pyro usage: this lock the server, waiting for the user input, but 
# unfortunately the user is remote !!!
#TextBox('cam size: %d, %d.\nfmr: %d, %d, %d, %d' % (x_cam_size, y_cam_size, fmr_l, fmr_t, fmr_w, fmr_h))


# The user's x0, y0, dx, dy are referenced to the factory rectangle.
# Convert to full-frame coordinates.
_x0 = _x0 + fmr_l
_y0 = _y0 + fmr_t
if _dx == -1 or _dx > fmr_w:
    _dx = fmr_w 
if _dy == -1 or _dy > fmr_h:
    _dy = fmr_h

_x0 = (_x0/4)*4
_y0 = (_y0/4)*4
_dx = (_dx/4)*4
_dy = (_dy/4)*4

# Removed for Pyro usage: this lock the server, waiting for the user input, but 
# unfortunately the user is remote !!!
#TextBox('x0: %d, y0: %d, dx: %d, dy: %d ' % (x0, y0, dx, dy))

if not os.path.exists(_folder_root):
    os.mkdir(_folder_root)

progress = None # progress bar window


#
# Acquire data form camera and stream to disk
#
def __acquire(nMeas, burstFolder, fileNamePrefix, enableTrigger):
    
    fullRawPath = _raw_folder_path + '/' + burstFolder
    
    if not os.path.exists(fullRawPath):
        os.mkdir(fullRawPath)

    # Gather info to be saved in the file header.
    xpix_val = GetValue(fs.GetPixelSpacing())
    xpix_unit = GetUnit(fs.GetPixelSpacing())
    maxint = fs.GetMaxInt()
    wedge = Store.FetchFloat('hardware.wedge')
    wavelength_val = GetValue(fs.wavelength)
    wavelength_unit = GetUnit(fs.wavelength)
    head_serial = Store.FetchKey('sys.head_serial', '000')
    date = time.ctime()
    #date = int(time.time())

    # Set up any triggers, shutters, etc in preparation for grabbing.
    fs.SetTriggeredMode(enableTrigger)
    fs.PreGrab(hint=None)
    
    # Collect and save a number of frames
    (frameInfo, camInfo) = IFC_ext.StreamToDisk( fullRawPath, 
                                                 nMeas,
                                                 pack=_pack, 
                                                 filename=fileNamePrefix,
                                                 x0=_x0, dx=_dx,
                                                 y0=_y0, dy=_dy,
                                                 xpix_val=xpix_val,
                                                 xpix_unit=xpix_unit,
                                                 maxint=maxint,
                                                 wedge=wedge,
                                                 wavelength_val=wavelength_val,
                                                 wavelength_unit=wavelength_unit,
                                                 head_serial=head_serial,
                                                 date=date)

    # Stop any triggers, shutters, etc.
    fs.PostGrab()
    
    numDone = len( frameInfo )
    lastTime = 0
    
    tmsgs = "Open\tWrite\tSeq\tBad\tLock\tLost\tMissed\tNext\tWaiting\n"
    for frame in frameInfo:
        thisTime = frame[0]
        tmsg = '%6.2f\t%6.2f\t%d\t%d\t%d\t%d\t%d\t%d\t%d' % frame[1:]
        dmsg = ' Delta_t=%.2f' % ((thisTime-lastTime)/1000)
        print tmsg + " " + dmsg
        tmsgs = tmsgs + tmsg + "\n"
        lastTime = thisTime
    #TextBox(tmsgs)
    
    lastTime /= 1000
    if numDone > 1:
        msg = 'Total time %.2f ms for %d frames = %.2f ms/frame' % (lastTime, numDone, lastTime/(numDone-1))
        #print msg
        #MessageBox(msg)
    
    print camInfo
    nCollected = gc.collect()
    
    return (frameInfo, camInfo)
    
    
#
# Read acquired files, apply hardware-specific corrections, and save calibrated frames.
# 
def __calibrate(burstFolder):
    
    fullRawPath = _raw_folder_path + '/' + burstFolder
    fullCalibPath = _calibrated_folder_path + '/' + burstFolder
    
    if not os.path.exists(fullRawPath):
        MessageBox('Input path %s does not exist.' % fullRawPath)
        return
    
    if not os.path.exists(fullCalibPath):
        os.mkdir(fullCalibPath)
    
    input_filenames = os.listdir(fullRawPath)

    # If the returned array is not the full array, paste it into a full array.
    # This ensures the corrections are applied to the correct pixels.
    is_subarray = (x_cam_size != _dx or y_cam_size != _dy)

    progress = CreateProgressWindow('Calibrating measurements', maximum=len(input_filenames))
    i = 0
    for input_filename in input_filenames:
        progress.Update(i, 'Calibrating measurement %d of %d' % (i, len(input_filenames)))
        i+=1

        input_fullpath = os.path.join(fullRawPath, input_filename)
        output_fullpath = os.path.join(fullCalibPath, input_filename)
    
        # Read a file
        arrayAndInfo = FastAcq.ReadRawFromDisk(input_fullpath)
        
        if arrayAndInfo is not None: # check that it worked
            (array, info) = arrayAndInfo
            array = array.astype('s')
            
            # paste into full array if necessary, indexed to full camera frame
            if is_subarray:
                full_array = Numeric.zeros((y_cam_size, x_cam_size), 's') # 'w'
                full_array[_y0:_y0+_dy,_x0:_x0+_dx] = array
            else:
                full_array = array
            
            # Apply calibrations
            fs.SetFullFrame(full_array)
            fs.ApplyCorrections()
            #fs.ApplyFactoryRect() we're restricting to fmr in acq
            fs.ApplySampling()
            full_array = fs.GetFullFrame()
            red = fs.GetReduction() or 1

            # parse out of full array, 
            if is_subarray:
                array = full_array[(_y0)/red:(_y0+_dy)/red,(_x0)/red:(_x0+_dx)/red]
            else:
                array = full_array

            array = array.copy() # make continguous

            # Write the corrected file, with header info unchanged (XXX pass info dict)
            # ATTENTION: from the GUI is possible to straem to disk only the pixels inside
            # the detector mask: this increase the lenght sustainable frame burst!
            FastAcq.WriteRawToDisk(array, output_fullpath, pack=_pack,
                                   x0=_x0/red, y0=_y0/red, dx=_dx/red, dy=_dy/red,
                                   xpix_val=info.get('xpix_val'),
                                   xpix_unit=info.get('xpix_unit'),
                                   maxint=info.get('maxint'),
                                   wedge=info.get('wedge'),
                                   wavelength_val=info.get('wavelength_val'),
                                   wavelength_unit=info.get('wavelength_unit'),
                                   head_serial=info.get('head_serial'),
                                   date=info.get('date'),
                                   wordSize=int(info.get('WORD_SIZE')),
                                   acq_time=info.get('ACQ_TIME'),
                                   )

    progress.Close()



# 
# Delete every nth measurement from the raw data folder.
# If n = 1, do nothing
#
def __sample(n):
    if n<=1:
        return # n must be greater than 1

    if not os.path.exists(_raw_folder_path):
        MessageBox('Input path %s does not exist.' % _raw_folder_path)
        return
    
    if not os.path.exists(_calibrated_folder_path):
        os.mkdir(_calibrated_folder_path)

    input_filenames = os.listdir(_raw_folder_path)
    for i in range(len(input_filenames)):
        if i%n:
            input_fullpath = os.path.join(_raw_folder_path, input_filenames[i])
            os.remove(input_fullpath)
    
    
### MAIN METHOD ###
def acquire(nMeasures, burstFolder, fileNamePrefix, enableTrigger):
    
    try:
        (frameInfo, camInfo) = __acquire(nMeasures, burstFolder, fileNamePrefix, enableTrigger)
        __sample(_meas_sample)   
        return (frameInfo, camInfo)
    except Exception, e:
        if progress:
            progress.Close()
        raise

def calibrate(burstFolder):
    try:
        __calibrate(burstFolder)    
    except Exception, e:
        if progress:
            progress.Close()
        raise
    
    

