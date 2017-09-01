#!/usr/bin/env python2
#
#+File: BCUfunctions.py
#
# Python utility functions for the BCU.
#
# generateLUT(): Pixel lookup table generation
#
# MatrixToREC(): Reconstructor matrix conversion to BCU format
#               (also used for the TimeFilter, since the format is the same)
#
# generateOKO(): generates REC, TimeFilter and VoutVector0 matrices to set a certain OKO configuration
#               (all in integer format, must be converted to float before sending to BCU)
#
# loadMatrix(): loads an integer matrix from disk
#
# loadMatrixFloat(): loads a 32-bit floating point matrix from disk
#
# createBCUTables(): writes on disk the pixel LUTs for 16, 20 and 40 pixels quadrant
#
# getCurrentFrameNumber(): gets the current frame # from the BCU
#
# getCurrentFrame(): returns the last frame captured by the BCU
#
# writeBCUfile(): writes on disk a BCU-formatted table
#
# readOffsetsFile(): reads a pupil offsets file from disk
#
# writeOffsetsFile(): writes a pupil offsets file to disk
#
# splitQuadrants(): splits a frame array into four quadrants
#-

from numarray import *
from eev39functions import *
import fits_lib, pyfits
import struct, ll, array, sys, time, os, signal

def read47():
    return ll.CCD_GetCurFrame(3, 1, 1, 1, 0, "MIRCTRL47:FRAME")

#@Function: generateLUT
#
# Function to generate a pixel lookup table for the BCU
#
# Notes about how the function works follow:
#
# Si lavora SEMPRE col punto di vista raster, altrimenti ci si perde.
# 
# Dati in ingresso: offset dei quadranti, lista acq_pupils (post-offset?)
# 
# I pixel sono sempre divisi in 3200+3200, nei primi 3200 ci devono essere
# quelli delle pupille. I pixel delle pupille sono N.
# 
# quindi, abbiamo N slot disponibili con posizioni fissate dal conto delle
# sottoaperture, e altre 6400-N libere.
# 
# 
# 0. inizializzare tutti i pixel della lookuptable a "scartato". La
# lookuptable e' per ora in formato raster.
# 1. si prende la lista acq_pupils (servono 4 liste? in questo modo non ci
# sono piu' offsets... casomai li mettiamo (artificialmente) nel display)
# 2. per ogni sottoapertura (1 pixel dell'acq_pupils) si trovano i quattro
# pixel necessari del CCD.
# 3. l'indirizzo della posizione successiva libera nella BCU viene infilato
# nella lookuptable raster.
# 5. Si ripete per tutti i pixel in pupilla
# 6. Ora si parte dal primo pixel (ordine raster) e si ripetono i punti
# 3,4,5 per ognuno
# 7. Si converte la lookuptable da raster a eev39
# 8. Si converte la lookuptable in BCU_lookuptable, aggiungendo ogni 8 words
# il n. di slopes da calcolare
# 
# Gli offset non esistono piu', al loro posto c'e' un acq_pupils con 4
# pupille.
# Il ricostruttore prende solo le slopes delle pupille, piu' un eventuale
# padding in fondo. Il padding viene naturale, perche' sfrutta i pixel fuori
# pupilla presenti nella BCU subito dopo la pupilla. Le righe corrispondenti
# del ricostruttore sono messe a zero.
# 
# Assumendo che t sia la tabella generata al punto 7, si possono fare due
# funzioni:
# 
# 
# BCUtoRaster:   raster[n] = BCU[t[n]]
# 
# RasterToBCU: BCU[t[n]] = raster[n]
#
# ccd_dim : dimensione lato ccd
# binning : current binning (to find pupil files)
# outfile_for_bcu: file dove salvare la LUT per la BCU
# outfile_for_c  : file dove salvare la LUT per il display
# slopex         : file dove dalvare la LUT per le slope X
# slopey         : file dove dalvare la LUT per le slope Y
#
# nopupil : if true, assumes no pupil file can be found and generates a fake LUT that just downloads
#           all CCD pixels correctly, but that is not useful for slope calculation. Such a LUT is
#           necessary before doing an acq_pupils, otherwise the slope computer would not be able to
#           correctly download at least the first frame.
#@

def generateLUT( ccd_dim, binning, outfile_for_bcu, outfile_for_c, slopex, slopey, nopupil = False, outfile_indpup = None):

    # Fixed number of slopes to do, depending on binning
    # num_lut_slopes = {1: 0x190, 2: 0x64, 3: 0x2a,  4: 0x19, 5: 0x10}

    # slopes-to-do interleave offset
    to_do_interleave = 8

    # Read fits file for each pupil
    pup_dir = "../config/pupils/bin%d/" % binning

    if nopupil:
        # Dummy values in the four quadrants
        pupil1_list = [0,1,2,3]
        pupil2_list = [4,5,6,7] 
        pupil3_list = [8,9,10,11]
        pupil4_list = [12,13,14,15]

    else:
        pupil1_list = fits_lib.readSimpleFits(pup_dir+'pup1.fits')
        pupil2_list = fits_lib.readSimpleFits(pup_dir+'pup2.fits')
        pupil3_list = fits_lib.readSimpleFits(pup_dir+'pup4.fits')
        pupil4_list = fits_lib.readSimpleFits(pup_dir+'pup3.fits')


    REJECT_PIXEL = 0x8000
    REJECT_SLOPE = -1

    LUT_raster = [REJECT_PIXEL] * (ccd_dim * ccd_dim)

    LUT_xslopes = [REJECT_SLOPE] * (ccd_dim * ccd_dim)
    LUT_yslopes = [REJECT_SLOPE] * (ccd_dim * ccd_dim)

    pixel_start_address = 0x04
    slope_start_address = 0x00

    # Add pupils to lookup table
    n_pixel_pup = len(pupil1_list)
    offset = 0
    pixel_addr = pixel_start_address
    slope_addr = slope_start_address
    if n_pixel_pup > 0:
        print "Inserting %d pixels inside the pupil" % n_pixel_pup
        for i in range(n_pixel_pup):
            LUT_raster[ pupil1_list[i]] = pixel_addr + offset + 0
            LUT_raster[ pupil2_list[i]] = pixel_addr + offset + 1
            LUT_raster[ pupil3_list[i]] = pixel_addr + offset + 4 + 0
            LUT_raster[ pupil4_list[i]] = pixel_addr + offset + 4 + 1

            # Increment pixel-related values
            offset = 2-offset
            if offset == 0:
                pixel_addr = pixel_addr + 8

            LUT_xslopes[ pupil1_list[i]] = slope_addr
            LUT_yslopes[ pupil1_list[i]] = slope_addr+1

            # Increment slope-related values
            slope_addr = slope_addr +2

    # Add remaining pixels in whatever order you get
    for i in range(len(LUT_raster)):
        if LUT_raster[i] == REJECT_PIXEL:
            LUT_raster[i] = pixel_addr
            pixel_addr = pixel_addr+1


    # Write an ASCII table for C programs
    if outfile_for_c != "":
        outfile = file( outfile_for_c, "w")
        for i in range(len(LUT_raster)):
            outfile.write( str(LUT_raster[i] - pixel_start_address )+"\n")
        outfile.close()
        print "Written "+outfile_for_c

    # Write an ASCII table for C programs
    if slopex != "":
        outfile = file( slopex, "w")
        for i in range(len(LUT_xslopes)):
            outfile.write( str(LUT_xslopes[i] - slope_start_address )+"\n")
        outfile.close()
        print "Written "+slopex

    # Write an ASCII table for C programs
    if slopey != "":
        outfile = file( slopey, "w")
        for i in range(len(LUT_yslopes)):
            outfile.write( str(LUT_yslopes[i] - slope_start_address )+"\n")
        outfile.close()
        print "Written "+slopey

    LUT_eev39 = FrameToEEV39 (LUT_raster)

    # Now insert the slope counter every 8 pixels
    # Work backwards, otherwise the indexes are not 8-ordered anymore!

    # For odd numbers of pixels, insert the # of slopes to be calculated
    # in the next-to-last position instead of the last

    LUT_BCU = LUT_eev39

    # Force a number of elements multiple of 8
    while len(LUT_BCU) % 8 != 0:
        LUT_BCU.append(0)

    # FORCE TO 1600 SLOPES NO MATTER WHAT THE ACTUAL # OF SLOPES IS
    #num = 1600
    num = n_pixel_pup*2
    
    table_len = len(LUT_BCU)
    first=1
    for pos in range( table_len- to_do_interleave, 0, -to_do_interleave):
        if ccd_dim == 26 and first:
            LUT_BCU.insert(pos, num)
            first=0
        else:
            LUT_BCU.insert( pos, 0)

    # Append the # of slopes to do
    # LUT_BCU.append( n_pixel_pup/2 )

    # FORCE TO 1600
    LUT_BCU.append(0x190)

    # Force an even number of elements
    if  len(LUT_BCU) % 2 != 0:
        LUT_BCU.append( 0)

    if outfile_for_bcu != "":
        outfile = file(outfile_for_bcu, "w")
        for i in range(0, len(LUT_BCU), 2):
            line = "0x%04X%04X" % (LUT_BCU[i+1], LUT_BCU[i])
            outfile.write(line+"\n")
        outfile.close()
        print "Written "+outfile_for_bcu

    # Write ind_pup if requested

    if outfile_indpup:
        outfile = file(outfile_indpup, "w")
        for ind in pupil1_list:
            outfile.write("%d\n" % ind)
        for ind in pupil2_list:
            outfile.write("%d\n" % ind)
        for ind in pupil3_list:
            outfile.write("%d\n" % ind)
        for ind in pupil4_list:
            outfile.write("%d\n" % ind)
        outfile.close()
        print "Written "+outfile_indpup

    return LUT_BCU





#+Function: matrixToREC
#
# Converts an ordinary NxM matrix to BCU reconstructor format
# (with elements interleaved for the DSP calculation)
#
# Input: matrix, number of rows, number of columns (redundant)
#
# Matrix is assumed to be a tables with rows one after the other
#
# The argument's type is not changed 
#-

def matrixToREC( matrix, rows, cols):

    if rows % 2 != 0:
        print "ERROR: number of rows must be a multiple of 2"
        return

    if cols % 4 != 0:
        print "ERROR: number of columns must be a multiple of 4"
        return

    tableout = [0] * len(matrix)

    counter=0
    for y in range(0, rows, 2):
        for x in range( 0, cols, 4):
            
#           tableout[ y*cols +x   ] = matrix[ counter  ]
#           tableout[ y*cols +x +1] = matrix[ counter+1] 
#           tableout[ y*cols +x +2] = matrix[ counter+2]
#           tableout[ y*cols +x +3] = matrix[ counter+3]
#
#           tableout[ (y+1)*cols +x   ] = matrix[ counter+4]
#           tableout[ (y+1)*cols +x +1] = matrix[ counter+5]
#           tableout[ (y+1)*cols +x +2] = matrix[ counter+6]
#           tableout[ (y+1)*cols +x +3] = matrix[ counter+7]

            tableout[ counter] = matrix[ y*cols +x]
            tableout[ counter+1] = matrix[ y*cols +x +1]
            tableout[ counter+2] = matrix[ y*cols +x +2]
            tableout[ counter+3] = matrix[ y*cols +x +3]
            tableout[ counter+4] = matrix[ (y+1)*cols +x ]
            tableout[ counter+5] = matrix[ (y+1)*cols +x +1]
            tableout[ counter+6] = matrix[ (y+1)*cols +x +2]
            tableout[ counter+7] = matrix[ (y+1)*cols +x +3]

            counter = counter+8

    return tableout

#+Function: loadMatrix
#
# Load a matrix from disk from a raw, long-integer file.
# Returns the matrix as a list
#-

def loadMatrix( filename, n_elements):
    
    matrix = array.array('L')
    infile = file( filename, "r")
    matrix.fromfile(infile, n_elements)
    infile.close()

    return matrix.tolist()

#+Function: loadMatrixFloat
#
# Load a matrix from disk from a raw, float file.
# Returns the matrix as a list
#-

def loadMatrixFloat( filename, n_elements):

    matrix = array.array('f')
    infile = file( filename, "r")
    matrix.fromfile(infile, n_elements)
    infile.close()

    return matrix.tolist()


#+Function: createBCUTables
#
# Writes a BCU pixel lookup table on disk
#
# mode: mode directory where to write, without slashes
# name: LUT name (will be placed in the filename)
# offsetx, offsety: offset arrays
# ccd_dim: ccd dimension in pixels (assumes a square CCD)
#
# general notes:
# - no masked out pixels
# - 3x3 binning support is experimental
#
#-

def createBCUTables( mode, name, offsetx, offsety, ccd_dim):

    nomefile = "../config/modes/"+mode+"/pixelLUT."+name+"."+str(ccd_dim)+".dat"
    outfile = file(nomefile, "w")
    table = generateLUT(ccd_dim, offsetx, offsety)
    for i in range(0, len(table), 2):
        line = "0x%04X%04X" % (table[i+1], table[i])
        outfile.write(line+"\n")
    outfile.close()
    print "Written "+nomefile

    return nomefile



def test():

    generateLUT(8)


def test2():

    createBCUTables()   

    rec = range(16*16)

    printframe( matrixToREC(rec, 16, 16), 16)


def reorderFrame( src):
    quadrant_dx = 40
    ccd_dx = 80

    dst = [0] * (quadrant_dx * quadrant_dx *4)
    counter=0

    # The X loop only considers even pixels (odd pixels are filled at the same time)
    for y in range(quadrant_dx):
        for x in range(0, quadrant_dx, 2):
            dst[ y*ccd_dx +x] = src[counter]                    # A1    
            dst[ y*ccd_dx +x +quadrant_dx] = src[counter+1]         # B1
            dst[ y*ccd_dx +x +1] = src[counter+2]               # A2
            dst[ y*ccd_dx +x +quadrant_dx +1] = src[counter+3]      # B2
            dst[ (y+quadrant_dx)*ccd_dx +x] = src[counter+4]            # C1
            dst[ (y+quadrant_dx)*ccd_dx +x +quadrant_dx] = src[counter+5]   # D1
            dst[ (y+quadrant_dx)*ccd_dx +x +1] = src[counter+6]     # C2
            dst[ (y+quadrant_dx)*ccd_dx +x +quadrant_dx +1] = src[counter+7]    # D2

            counter = counter+8

    return dst
    

#+Function: getCurrentFrameNumber_bypointer():
#
# Asks the BCU the current frame # and returns it
#
#-

def getCurrentFrameNumber_bypointer():

    raise Exception("BCUfunctions::getCurrentFrameNumber_bypointer(): this whole file must be obsoleted!")

    ## Look at the frame counter inside the BCU

    buf = struct.unpack( "L",ll.BCU_Read( 0x38018, 1, 148))

    framecounter = (buf[0] /4) / 0x1fb4
    return framecounter


#+Function: getCurrentFrameNumber():
#
# Asks the BCU the current frame # and returns it
#
#-

def getCurrentFrameNumber():
    raise Exception("BCUfunctions::getCurrentFrameNumber_bypointer(): this whole file must be obsoleted!")

    ## Look at the frame counter inside the BCU

    buf = struct.unpack( "L",ll.BCU_Read( 0x3800C, 1, 148))

    framecounter = buf[0]
    return framecounter


#+Function: getFrame
#
# Gets a frame of pixels from the BCU memory
#-

def getFrame(num):

    return ll.CCD_GetFrames( 1, 1, num, 1)

#+Function: getFrames
#
# Gets a series of frames from the live WFS
#
# Setting the raster_order parameter to zero will retrieve
# a frame in BCU ordering, useful for re-upload into the BCU (backgrounds, etc)
#-

def getFrames( num, raster_order=1, buffername='MIRCTRL39:PIPPO'):

    # Modified to acquire only 1 frame at a time
    # from the ll module

    # return ll.CCD_GetCurFrame( 1, num, 1)


    # Ask frames WITHOUT reordering: ONLY GOOD FOR BACKGROUNDS
    first = ll.CCD_GetCurFrame( 1, 1, 1, raster_order, 1, buffername)
    s = first.shape
    frames = zeros(( num, s[1], s[2]), Int32)
    frames[0,:,:] = reshape(first, s[1], s[2])

    if num>1:
        for i in range(num-1):
            frames[i+1,:,:] = reshape( ll.CCD_GetCurFrame( 1, 1, 1, raster_order, 1, buffername), s[1],s[2])

    return frames

#+Function: getSlope
#
# Gets a frame of slopes from the BCU memory
#-

def getSlope(num):

    slope = ll.CCD_GetFrames( 1, 1, num, 2)
    return reorderSlopes(slope)

#+Function: getSlopes
#
# Gets a series of slope frames from the live WFS
#-

def getSlopes( num):

    # Modified to read only 1 frame at a time
    # from the ll module

    # slopes = ll.CCD_GetCurFrame( 1, num, 2)
    # return reorderSlopes(slopes)

    s = ll.CCD_GetCurFrame( 1, 1, 2).shape
    print "S:",s
    slopes = zeros(( num, s[1]), Float32)

    for i in range(num):
        slopes[i,:] = reshape( ll.CCD_GetCurFrame( 1, 1, 2), s[1])

    return slopes

#+Function: reorderSlopes
#
# Workaround for the strange behaviour of numarray slope arrays: reorders the slope frames
# into something more sensible
#-

def reorderSlopes(slopes):

#   n3 = slopes.shape[2]
#   n2 = slopes.shape[1]
#   n1 = slopes.shape[0]

#   data = swapaxes(slopes, 1, 2)
    data = slopes
    return data

    
    
    
#+Function: getCurrentFrame
#
# Asks the BCU the current frame # and retrieve it
#
# Quite inefficient for now, generates lots of messages
#-  

def getCurrentFrame(buffername = 'MIRCTRL39:PIPPO'):
   
    print 'getCurrentFrame()'
    return getFrames( 1, buffername = buffername)

#+Function: getCurrentSlopes
#
# Asks the BCU for the current frame # and retrieve the corresponding slopes
#-

def getCurrentSlopes():

    return getSlopes( 1)

#+Function: getCommand
#
# Gets a frame of commands from the BCU
#-return

def getCommand(num):
    print "asking command for frame "+str(num)
    return ll.CCD_GetFrames(1, 1, num, 3)

#+Function: getCommands
#
# Gets a series of command frames from the live WFS
#-

def getCommands(num):

    # Modified to read only 1 frame at a time
    #
    # return ll.CCD_GetCurFrame(1, num, 3)

    s = ll.CCD_GetCurFrame( 1, 1, 3).shape
    print "S:",s
    cmds = zeros(( num, s[1]), Int32)

    for i in range(num):
        cmds[i,:] = reshape( ll.CCD_GetCurFrame( 1, 1, 3), s[1])

    return cmds

#+Function: getCurrentCommands
#
# Return the current command frame
#-

def getCurrentCommands():

    return getCommands( 1)

#+Function: saveFrame
#
# Saves a frame read from the BCU into a FITS file
#-

def saveFrame( frame, filename, dict):
    '''Save a frame or a series of frames on disk
Parameters:
    frame: frame array
    filename: name of the file to write (FITS file)
    dict: optional dict with file metadata'''
       
    fits_lib.writeSimpleFits( filename, frame, dict)    

#+Function: writeBCUfile
#
# Writes a hex-formatted BCU configuration file
#-

def writeBCUfile( filename, data):
    outfile = file(filename, "w")
    for n in data:
        line = '0x%08X\n' % n
        outfile.write( line)
    outfile.close()

def writeBCUfileFloat( filename, data):
    outfile = file(filename, "w")
    for n in data:
        line = '0x%08X\n' % n
        outfile.write( line)
    outfile.close()
    

def helpBCU():

    print "Available commands are:"
    
    print "getCurrentFrameNumber(): gets the current frame # in the BCU"
    print "getCurrentFrame(): gets the current frame"
    print "getCurrentSlopes(): gets the current slope valuess"
    print "getCurrentCommands(): gets the current OKO commands"
    print "saveFrame( frame, filename): saves a binary frame to disk"
    print ""
    print "generateLUT( ccd_dx): generate a pixel lookup table"
    print "matrixToREC( matrix, rows, cols): converts a matrix to DSP order"
    print "loadMatrix( filename, n_elements): loads an integer matrix from disk"
    print "loadMatrixFloat( filename, n_elements): loads a floating point matrix from disk"
    print "createBCUTables(): creates the BCU lookup tables"


#++Method: readOffsetsFile
#
# Read previous offsets from a disk file
#-

def readOffsetsFile( filename):

    offsetx={}
    offsety={}

    infile = file( filename, "r")
    for q in ['b', 'c', 'd']:
        offsetx[q] = int(infile.readline())
        offsety[q] = int(infile.readline())

    infile.close()
    return (offsetx, offsety)

#++Method: writeOffsetsFile
#
# Save an offset file
#-

def writeOffsetsFile( filename, offsetx, offsety):

    outfile = file( filename, "w")

    for q in ['b','c','d']:
        outfile.write(str(offsetx[q])+"\n")
        outfile.write(str(offsety[q])+"\n")

    outfile.close() 

#+Function: run_idl
#
# Runs an IDL procedure from within a Python program
#
# <idlfile> is the ".pro" filename where the procedure is.
# <command> is the complete IDL command, including any
# argument needed by the procedure
# A temporary "runidl.pro" file will be generated, that will
# compile and run the procedure. The IDL command must be in the path
#-
 
def run_idl(idlfile, command, external=0, waitforexit=1):

    tempfile = "runidl.txt"

    f = file(tempfile, "w")

    if idlfile != "":
        f.write(".r "+idlfile+"\n")
    f.write(command)
    f.write("\nexit\n");
    f.close()

    # print "starting with string "+tempfile

    if external:
        command = "xterm -e idl "+tempfile
        if waitforexit == 0:
            command = command+" &"
        os.system(command)
    else:
        if waitforexit:
            try:
                pid = os.spawnlp( os.P_WAIT, "idl", "idl", tempfile)
            except:
                os.kill(pid, signal.SIGKILL)
        else:
            os.spawnlp( os.P_NOWAIT, "idl", "idl", tempfile)

#+Function: changegain
#
# Changes the gain of a reconstructor matrix

def changegain(gain):

    prefix = "../config/reconstructors/"
    source = "rec_modal_gain1"
    dest = "rec_modal_gain"+str(gain)

    infile = prefix+source
    outfile = prefix+dest

    run_idl("changegain.pro", "changegain, '"+infile+"', '"+outfile+"', "+str(gain))

    ll.RTR_SetREC(1, dest)

#+Function: makePupilsFromCenterRadius
#
# Generates new pupils form a center/radius measure
#
# <diameters>, <cx> and <cy> are 4-element lists with the features of each pupil

def makePupilsFromCenterDiameter( diameters, cx, cy, ccd_dim, binning, use_old = False):

    datafile = "/home/labot/Supervisor/config/pupils/bin%d/pupdata.txt" % binning

    if not use_old:
        f = open(datafile,"w")
        for n in range(4):
            f.write("%5.2f %5.2f %5.2f 0\n" % (diameters[n]/2.0, cx[n], cy[n]))
        f.close()

    return makePupils_common( ccd_dim, binning)
    

# Acquires new pupils
#+Function: makePupils
#
# Acquires new pupils using the acq_pupils.pro routines and generates all needed files

def makePupils(fits_file, ccd_dim, binning):

    idl_file = "/home/labot/idl/adsec_lib/adsec_utilities/acq_pupils_p45.pro"
    idl_cmd = "acq_pupils, %d, /FITS, FILENAME='%s'" % (binning, fits_file)

    run_idl( idl_file, idl_cmd)

    return makePupils_common( ccd_dim, binning)

def makePupils_common(ccd_dim, binning):

    idl_file = "/home/labot/idl/adsec_lib/adsec_utilities/cloop_routines_fits_p45.pro"
    idl_cmd = "generate_masks_fits, %d, %d, inprefix='/home/labot/Supervisor/config/pupils/bin%d/', outprefix='/home/labot/Supervisor/config/pupils/bin%d/', BINNING=%d" % (ccd_dim, ccd_dim, binning, binning, binning)

    run_idl( idl_file, idl_cmd)

    timestamp = time.strftime("%Y%m%d-%H:%M")

    prefix = "/home/labot/Supervisor/config/lookuptables/bin%d/%s" % (binning, timestamp)
    try:
        os.makedirs(prefix)
    except:
        print "Error creating directory: "+prefix

    # Copy original info about centers and diameters
    datafile = "/home/labot/Supervisor/config/pupils/bin%d/pupdata.txt" % binning
    os.system("/bin/cp %s %s" % (datafile, prefix+"/pupdata.txt"))

    outfile_table  = "%s/slopecomp" % prefix
    outfile_displ  = "%s/display" % prefix
    outfile_slopex = "%s/slopex" % prefix
    outfile_slopey = "%s/slopey" % prefix
    outfile_indpup = "%s/indpup" % prefix

    generateLUT(ccd_dim, binning, outfile_table, outfile_displ, outfile_slopex, outfile_slopey, outfile_indpup = outfile_indpup) 

    return timestamp


#+Function: makeNoPupils
#
# Generates the fake lookuptables necessary before acquiring the pupils

def makeNoPupils():

    ccd_dims = {1: 80, 2: 40, 3: 26, 4: 20, 5: 16}

    for bin in ccd_dims.keys():
        ccd_dim = ccd_dims[bin]
        prefix = "../config/lookuptables/bin%d/nopupils" % bin
        outfile_table  = prefix +'/slopecomp'
        outfile_displ  = prefix +'/display'
        outfile_slopex = prefix +'/slopex'
        outfile_slopey = prefix +'/slopey'
        generateLUT( ccd_dim, bin, outfile_table, outfile_displ, outfile_slopex, outfile_slopey, nopupil = True)


#+Function: averageFrames
#
# Computes the average of a series of frames, compressing the first index
#
# frames is assumed to be 3D series of frames
#-
def averageFrames(frames):

    shape = frames.shape
    numelements = shape[0]

    avg = frames[0]
    for i in range(numelements-1):
        avg = avg+frames[i+1]

    avg = avg / numelements

    return avg


#+Function: splitQuadrants
#
# Given a frame, splits it into four quadrants
# returns a tuple with the four quadrants
#
# frame is assumed to be a bidimensional array
# the frame width must be divisible by two


def splitQuadrants(frame):

    shape = frame.shape

    dx = shape[0]
    dy = shape[1]
    q_dx = dx/2
    q_dy = dy/2

    quad1 = frame[0:q_dx, 0:q_dy]
    quad2 = frame[q_dx:dx, 0:q_dy]
    quad3 = frame[0:q_dx, q_dy:dy]
    quad4 = frame[0:q_dx, q_dy:dy] 

    return (quad1, quad2, quad3, quad4)
    

