#!/usr/bin/env python2
#
from eev39functions import *
import fits_lib
import time, os, signal
import calib
from AdOpt import frames_lib
import numpy as np


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

def generateLUT2( ccdDim, binning, outfile_for_bcu, outfile_for_c, slopex, slopey, nopupil = False, outfile_indpup = None):

    print 'generateLUT2(). ccdDim=', ccdDim, '   binning=',binning
    ccdDim = ccdDim/binning

    # Pupils definition
    PUP_A=0
    PUP_B=1
    PUP_C=2
    PUP_D=3

    # Base LUT section
    class LUTsection:
       A1=-1
       A2=-1
       B1=-1
       B2=-1
       C1=-1
       C2=-1
       D1=-1
       D2=-1
       N=-1

    # Read fits file for each pupil
    pupDir = calib.LUTsDir('ccd39', binning)

    pups = []
    if nopupil:
        # Dummy values in the four quadrants
        pups.append( [0,1,2,3])
        pups.append( [4,5,6,7])
        pups.append( [8,9,10,11])
        pups.append( [12,13,14,15])

    else:
	try:
        	pups.append( map( int, fits_lib.readSimpleFits(pupDir+'/pup1.fits')))
        	pups.append( map( int, fits_lib.readSimpleFits(pupDir+'/pup2.fits')))
        	pups.append( map( int, fits_lib.readSimpleFits(pupDir+'/pup4.fits')))
        	pups.append( map( int, fits_lib.readSimpleFits(pupDir+'/pup3.fits')))
	except IOError, e:
		print
		print e
		print 'ERROR----> Pupils not generated.'
		print
	 	return		

    if len(pups[0])%2 == 1:
        pups[0].append( max(pups[0])+1)
        pups[1].append( max(pups[1])+1)
        pups[2].append( max(pups[2])+1)
        pups[3].append( max(pups[3])+1)

    REJECT_PIXEL = 0x8000
    REJECT_SLOPE = -1

    pixelStartAddress = 0x04
    slopeStartAddress = 0x00

    nSubaps = len(pups[0])
    nPups   = 4

    # Build maps
    raster   = np.arange( ccdDim*ccdDim, dtype=np.int32)
    eev39    = np.array( FrameToEEV39(raster))
    pupMap   = np.zeros( ccdDim*ccdDim, dtype=np.int32)-1     # pupMap[pxId]   = pup      [0..3] or -1
    subapMap = np.zero(  ccdDim*ccdDim, dtype=np.int32)-1    # subapMap[pxId] = subapId  [0..1599]  or -1
    for p in range(nPups):
       pupMap[pups[p]]=p
       for subapId in range(nSubaps):
          subapMap[pups[p][subapId]] = subapId

    # Order in which the subapertures are read out
    subapMap_eev39 = np.array( FrameToEev39(raster))
    subapMap_eev39_completed = subapMap_eev39*0

    completedSubaps = np.array(nSubaps)
    
    for idx in range(len(subapMap_eev39)):
       subapIdx = subapMap_eev39[idx]
       if subapIdx>=0:
          completed_subaps[subapIdx] +=1
          if completed_subaps[subapIdx]==4:
             subapMap_eev39_completed[idx] = subapIdx

    DSPidx = subapMap_eev39_completed[ np.nonzero( subapMap_eev39_completed)]
    


    raster = np.zeros((ccdDim,ccdDim))
    raster.flat[ pupil1_list]=1
    raster.flat[ pupil2_list]=2
    raster.flat[ pupil3_list]=3
    raster.flat[ pupil4_list]=4

    eev39 = np.array( FrameToEEV39 (LUT_raster.flatten()))





def generateLUT( ccd_dim, binning, outfile_for_bcu, outfile_for_c, slopex, slopey, nopupil = False, outfile_indpup = None):

    # Fixed number of slopes to do, depending on binning
    # num_lut_slopes = {1: 0x190, 2: 0x64, 3: 0x2a,  4: 0x19, 5: 0x10}

    print 'generateLUT(). ccd_dim=', ccd_dim, '   binning=',binning

    ccd_dim = ccd_dim/binning

    print 'generateLUT(): binned CCD dimension: ',ccd_dim

    # slopes-to-do interleave offset
    to_do_interleave = 8

    # Read fits file for each pupil
    pup_dir = calib.LUTsDir('ccd39', binning)

    if nopupil:
        # Dummy values in the four quadrants
        pupil1_list = [0,1,2,3]
        pupil2_list = [4,5,6,7] 
        pupil3_list = [8,9,10,11]
        pupil4_list = [12,13,14,15]

    else:
	try:
        	pupil1_list = map( int, fits_lib.readSimpleFits(pup_dir+'/pup1.fits'))
        	pupil2_list = map( int, fits_lib.readSimpleFits(pup_dir+'/pup2.fits'))
        	pupil3_list = map( int, fits_lib.readSimpleFits(pup_dir+'/pup4.fits'))
        	pupil4_list = map( int, fits_lib.readSimpleFits(pup_dir+'/pup3.fits'))
	except IOError, e:
		print
		print e
		print 'ERROR----> Pupils not generated.'
		print
	 	return		


    REJECT_PIXEL = 0x8000
    REJECT_SLOPE = -1

    LUT_raster = [REJECT_PIXEL] * (ccd_dim * ccd_dim)

    LUT_xslopes = [REJECT_SLOPE] * (ccd_dim * ccd_dim)
    LUT_yslopes = [REJECT_SLOPE] * (ccd_dim * ccd_dim)

    LUT_subaps = [REJECT_SLOPE] * (ccd_dim * ccd_dim)

    pixel_start_address = 0x04
    slope_start_address = 0x00

    if len(pupil1_list)%2 == 1:
        pupil1_list.append( max(pupil1_list)+1)
        pupil2_list.append( max(pupil2_list)+1)
        pupil3_list.append( max(pupil3_list)+1)
        pupil4_list.append( max(pupil4_list)+1)

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

            LUT_subaps[ pupil1_list[i]] = i
            LUT_subaps[ pupil2_list[i]] = i
            LUT_subaps[ pupil3_list[i]] = i
            LUT_subaps[ pupil4_list[i]] = i
 
            print 'index %d - pixel_addr %d - offset %d' % (i, pixel_addr, offset)
            print 'index %d -  pixel %d  - addr %d'  % (i, pupil1_list[i], LUT_raster[ pupil1_list[i]])
            print 'index %d -  pixel %d  - addr %d'  % (i, pupil2_list[i], LUT_raster[ pupil2_list[i]])
            print 'index %d -  pixel %d  - addr %d'  % (i, pupil3_list[i], LUT_raster[ pupil3_list[i]])
            print 'index %d -  pixel %d  - addr %d'  % (i, pupil4_list[i], LUT_raster[ pupil4_list[i]])

            # Increment pixel-related values
            offset = 2-offset
            if offset == 0:
                pixel_addr = pixel_addr + 8

            LUT_xslopes[ pupil1_list[i]] = slope_addr
            LUT_yslopes[ pupil1_list[i]] = slope_addr+1

            # Replicate y slopes in the X frame - both slopes visible at the same time
            LUT_xslopes[ pupil2_list[i]] = slope_addr+1

            # Increment slope-related values
            slope_addr = slope_addr +2

    # Add remaining pixels in whatever order you get
    for i in range(len(LUT_raster)):
        
        if LUT_raster[i] == REJECT_PIXEL:
            
            # Watch out for the 2 holes of two pixels each which happen if we had an ODD number of pixels in the pupil
            if (offset == 2) and (i<=4):
                if i<4:
                    print 'Trovato n. dispari di subap'
                    offs = [2,2,4,4]
                    addr = pixel_addr + offs[i]
                else:
                    pixel_addr += 4
                    addr = pixel_addr
            else:
                addr = pixel_addr

            LUT_raster[i] = addr
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
    LUT_subaps_eev39 = FrameToEEV39(LUT_subaps)


    # Now insert the slope counter every 8 pixels
    # Work backwards, otherwise the indexes are not 8-ordered anymore!

    # For odd numbers of pixels, insert the # of slopes to be calculated
    # in the next-to-last position instead of the last

    LUT_BCU = LUT_eev39

    # Force a number of elements multiple of 8
    while len(LUT_BCU) % 8 != 0:
        LUT_BCU.append(0)

    slopes_to_do = []
    completed_subaps= np.zeros(800)
    for idx in range( 0, len(LUT_BCU), to_do_interleave):
       for p in range(to_do_interleave):
          subap_idx = LUT_subaps_eev39[idx+p]
          completed_subaps[subap_idx] +=1
       num_subaps = (completed_subaps ==4).sum()
       num_subaps -=4   # Safety factor
       if num_subaps<0:
           num_subaps=0
       slopes_to_do.append( num_subaps*2/4)      # 2 slopes/subap, /4 for BCU purposes

    # FORCE TO 1600 slopes (400 for bcu purposes)
    num_slopes = 0x190

    # Set total no. of slopes as soon as the last slope arrives
    # Back off 1 slope to prevent the DSP from finishing too early
    tmp = np.array(slopes_to_do)
    pos = np.argmax(tmp)
    tmp[pos:] = num_slopes-1

    fits_lib.writeSimpleFits( outfile_for_bcu+'_'+'slopesToDo.fits', tmp, overwrite=True)

    slopes_to_do = tmp
    print
    print 'slopes_to_do array:'
    print
    print  slopes_to_do
    print

    table_len = len(LUT_BCU)
    first=1
    slopes_to_do_pos=-1
    for pos in range( table_len- to_do_interleave, 0, -to_do_interleave):
        # Suspect bin3 bug
        if binning == 3 and first:
            LUT_BCU.insert(pos, num_slopes)
            first=0
            slopes_to_do_pos-=1
        else:
            LUT_BCU.insert( pos, slopes_to_do[slopes_to_do_pos])
            slopes_to_do_pos-=1


    if binning != 3:
        LUT_BCU.append(num_slopes)

    # Force an even number of elements
    if len(LUT_BCU) % 2 != 0:
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

    print 'run_idl(): %s' % command

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

#@Function: makePupilsFromRTDBVars
#
# Wrapper around makePupilsFromCenterDiameter, reading the arguments from RTDB variables
# Diameter can be enlarged with the optional parameter <enlarge> to help in HOT-style optimization
#@

def makePupilsFromRTDBVars( app, enlarge=0):

    from AdOpt import cfg
    side = cfg.side

    ccdW    = app.ReadVar( cfg.varname('ccd39', 'DX')).Value()
    binning = app.ReadVar( cfg.varname('ccd39', 'XBIN.CUR')).Value()

    pupils = frames_lib.getCurPupilsFromRTDB( app, next=True)

    diam = []
    cx = []
    cy = []

    for i in range(4):
        diam.append( pupils[i].diameter + enlarge)
        cx.append(pupils[i].cx)
        cy.append(pupils[i].cy)

    return makePupilsFromCenterDiameter( diam, cx, cy, ccdW, binning)
  

#@Function: makePupilsFromCenterDiameter
#
# Generates new pupils form a center/radius measure
#
# <diameters>, <cx> and <cy> are 4-element lists with the features of each pupil
#@


def makePupilsFromCenterDiameter( diameters, cx, cy, ccd_dim, binning, use_old = False):

    datafile = calib.LUTsDir('ccd39', binning) + '/pupdata.txt'

    print 'makePupilsFromCenterDiameter(): writing '+datafile

    if not use_old:
        f = open(datafile,"w")
        for n in range(4):
            f.write("%5.2f %5.2f %5.2f 0\n" % (diameters[n]/2.0, cx[n], cy[n]))
        f.close()

    return makePupils_common( ccd_dim, binning)

#@Function: movePupils
#
# Moves a pupil by the specified amount in x and y. Returns a new tracking number
#@

def movePupils( old_tracknum, mov_x, mov_y, ccd_dim, binning):

    offset = mov_x + (ccd_dim/binning)*mov_y;

    calibdir = os.path.join(calib.LUTsDir('ccd39', binning))
    pupdir = os.path.join(calibdir,old_tracknum)

    pup1 = fits_lib.readSimpleFits( os.path.join(pupdir, 'pup1.fits'))
    pup2 = fits_lib.readSimpleFits( os.path.join(pupdir, 'pup2.fits'))
    pup3 = fits_lib.readSimpleFits( os.path.join(pupdir, 'pup3.fits'))
    pup4 = fits_lib.readSimpleFits( os.path.join(pupdir, 'pup4.fits'))

    pup1 += offset
    pup2 += offset
    pup3 += offset
    pup4 += offset

    hdr = {}
    hdr['SHIFT_X'] = mov_x
    hdr['SHIFT_Y'] = mov_y
    hdr['PUPFROM'] = old_tracknum

    fits_lib.writeSimpleFits( os.path.join(calibdir, 'pup1.fits'), pup1, hdr, overwrite=True) 
    fits_lib.writeSimpleFits( os.path.join(calibdir, 'pup2.fits'), pup2, hdr, overwrite=True) 
    fits_lib.writeSimpleFits( os.path.join(calibdir, 'pup3.fits'), pup3, hdr, overwrite=True) 
    fits_lib.writeSimpleFits( os.path.join(calibdir, 'pup4.fits'), pup4, hdr, overwrite=True) 

    pupdata = file( os.path.join(pupdir, 'pupdata.txt')).readlines()
    newpupdata = file( os.path.join(calibdir, 'pupdata.txt'), 'w')
    for pup in pupdata:
        diam, cx, cy, dummy = pup.split()
        cx = float(cx)+mov_x
        cy = float(cy)+mov_y
        pup = '%s %5.2f %5.2f %s\n' % (diam, cx, cy, dummy)
        newpupdata.write(pup)

    newpupdata.close()

    return makePupils_fromMasks( ccd_dim, binning)

#@Function: movePupils4
#
# Moves each pupil by a different amount. Returns a new tracking number
# mov_x and mov_y are an array[4] of movements to apply.
#@

def movePupils4( old_tracknum, mov_x, mov_y, ccd_dim, binning):

    offset0 = mov_x[0] + (ccd_dim/binning)*mov_y[0];
    offset1 = mov_x[1] + (ccd_dim/binning)*mov_y[1];
    offset2 = mov_x[2] + (ccd_dim/binning)*mov_y[2];
    offset3 = mov_x[3] + (ccd_dim/binning)*mov_y[3];

    calibdir = os.path.join(calib.LUTsDir('ccd39', binning))
    pupdir = os.path.join(calibdir,old_tracknum)

    pup1 = fits_lib.readSimpleFits( os.path.join(pupdir, 'pup1.fits'))
    pup2 = fits_lib.readSimpleFits( os.path.join(pupdir, 'pup2.fits'))
    pup3 = fits_lib.readSimpleFits( os.path.join(pupdir, 'pup3.fits'))
    pup4 = fits_lib.readSimpleFits( os.path.join(pupdir, 'pup4.fits'))

    pup1 += offset0
    pup2 += offset1
    pup3 += offset2
    pup4 += offset3

    fits_lib.writeSimpleFits( os.path.join(calibdir, 'pup1.fits'), pup1, overwrite=True) 
    fits_lib.writeSimpleFits( os.path.join(calibdir, 'pup2.fits'), pup2, overwrite=True) 
    fits_lib.writeSimpleFits( os.path.join(calibdir, 'pup3.fits'), pup3, overwrite=True) 
    fits_lib.writeSimpleFits( os.path.join(calibdir, 'pup4.fits'), pup4, overwrite=True) 

    pupdata = file( os.path.join(pupdir, 'pupdata.txt')).readlines()
    newpupdata = file( os.path.join(calibdir, 'pupdata.txt'), 'w')
    count=0
    for pup in pupdata:
        diam, cx, cy, dummy = pup.split()
        cx = float(cx)+mov_x[count]
        cy = float(cy)+mov_y[count]
        pup = '%s %5.2f %5.2f %s\n' % (diam, cx, cy, dummy)
        newpupdata.write(pup)
        count += 1

    newpupdata.close()

    return makePupils_fromMasks( ccd_dim, binning)

#+Function: makePupils
#
# Acquires new pupils using the acq_pupils.pro routines and generates all needed files

def makePupils(fits_file, ccd_dim, binning):

    idl_file = "/home/labot/idl/adsec_lib/adsec_utilities/acq_pupils_p45.pro"
    idl_cmd = "acq_pupils, %d, /FITS, FILENAME='%s'" % (binning, fits_file)

    run_idl( idl_file, idl_cmd)

    return makePupils_common( ccd_dim, binning)

def makePupils_hot( ccd_dim, binning):

    return makePupils_fromMasks( ccd_dim, binning)

def makePupils_common(ccd_dim, binning):

    idl_file = "cloop_routines_fits_lbt.pro"   # Will find this in the default IDL path
    idl_cmd = "generate_masks_fits, %d, %d, inprefix='%s', outprefix='%s', BINNING=%d" % (ccd_dim, ccd_dim, calib.LUTsDir('ccd39', binning) + '/', calib.LUTsDir('ccd39', binning)  + '/', binning)

    run_idl( idl_file, idl_cmd)

    return makePupils_fromMasks( ccd_dim, binning)

def makePupils_fromBin1( ccd_dim, binning, bin1_tracknum):
    '''
    Generates new pupil masks by digitally binning the bin1 pupils.
    <from_pups>: tracknum of bin1 pupils to bin
    '''
    from AdOpt import fits_lib
    from AdOpt.wrappers import idl

    tmpfile1 = '/tmp/pupils.fits'
    tmpfile2 = '/tmp/pupils_binned.fits'

    bin1_prefix = calib.LUTsDir('ccd39', 1)+'/'+bin1_tracknum
    outdir = calib.LUTsDir('ccd39', binning)

    pupdata= file(bin1_prefix+'/pupdata.txt').readlines()
    pupdata_out= file(outdir+'/pupdata.txt', 'w')
    for n in range(len(pupdata)):
        radius, cx, cy, sd = map(float, pupdata[n].split())
        radius /= binning
        cx /= binning
        cy /= binning
        sd /= binning
        pupdata_out.write('%5.2f %5.2f %5.2f %5.2f\n' % (radius, cx, cy, sd))
    pupdata_out.close()


    indpup = file(bin1_prefix+'/indpup').readlines()
    f = np.zeros((ccd_dim,ccd_dim))

    f.flat[ [int(x) for x in indpup]] =1

    fits_lib.writeSimpleFits( tmpfile1, f, overwrite=True)

    cmd = "dummy = bin_ccd39(dummy2, %d, INPUT_FILE='%s', OUTPUT_FILE='%s')" % (binning, tmpfile1, tmpfile2)
    print cmd
    idl.ex(cmd)

    f = fits_lib.readSimpleFits(tmpfile2)
    sogliola = (binning*binning)/2
    f[ np.where( f< sogliola) ]=0
    f[ np.where( f>=sogliola)]=1

    idx = np.where(f.flat==1)
    f.flat[idx] = idx

    q_dim = (ccd_dim/binning)/2
    size = q_dim*q_dim

    fits_lib.writeSimpleFits(outdir+'/pup1.fits', np.array(filter( None, f[q_dim:, 0:q_dim].reshape(size).tolist())), overwrite=True)
    fits_lib.writeSimpleFits(outdir+'/pup2.fits', np.array(filter( None, f[q_dim:, q_dim:].reshape(size).tolist())), overwrite=True)
    fits_lib.writeSimpleFits(outdir+'/pup3.fits', np.array(filter( None, f[0:q_dim, 0:q_dim].reshape(size).tolist())), overwrite=True)
    fits_lib.writeSimpleFits(outdir+'/pup4.fits', np.array(filter( None, f[0:q_dim, q_dim:].reshape(size).tolist())), overwrite=True)

    return makePupils_fromMasks( ccd_dim, binning)



def makePupils_fromMasks( ccd_dim, binning):

    timestamp = time.strftime("%Y%m%d-%H%M%S")
    prefix = calib.LUTsDir('ccd39', binning) + '/' + timestamp
    try:
        os.makedirs(prefix)
    except:
        print "Error creating directory: "+prefix

    # Copy original info about centers and diameters
    lutdir = calib.LUTsDir('ccd39', binning)
    datafile = lutdir + '/pupdata.txt'

    print 'makePupils_fromMasks(): moving from %s to %s' % (datafile, prefix+"/pupdata.txt")

    cmd = '/bin/mv -f "%s" "%s"' % (datafile, prefix+"/pupdata.txt")
    print cmd
    os.system(cmd)

    outfile_table  = "%s/bcuLUT" % prefix
    outfile_displ  = "%s/display" % prefix
    outfile_slopex = "%s/slopex" % prefix
    outfile_slopey = "%s/slopey" % prefix
    outfile_indpup = "%s/indpup" % prefix

    generateLUT(ccd_dim, binning, outfile_table, outfile_displ, outfile_slopex, outfile_slopey, outfile_indpup = outfile_indpup) 

    print 'makePupils_common: moving fits file from %s to %s' % (lutdir, prefix)
    cmd = '/bin/mv -f "%s" "%s"' % ( lutdir +'/pup1.fits', prefix)
    print cmd
    os.system(cmd)
    os.system('/bin/mv -f "%s" "%s"' % ( lutdir +'/pup2.fits', prefix))
    os.system('/bin/mv -f "%s" "%s"' % ( lutdir +'/pup3.fits', prefix))
    os.system('/bin/mv -f "%s" "%s"' % ( lutdir +'/pup4.fits', prefix))

    return timestamp


#+Function: makeNoPupils
#
# Generates the fake lookuptables necessary before acquiring the pupils

def makeNoPupils():

    ccd_dims = {1: 80, 2: 40, 3: 26, 4: 20, 5: 16}


    for bin in ccd_dims.keys():
        prefix = calib.LUTsDir('ccd39', bin) + '/nopupils'
        ccd_dim = ccd_dims[bin]
        outfile_table  = prefix +'/bcuLUT'
        outfile_displ  = prefix +'/display'
        outfile_slopex = prefix +'/slopex'
        outfile_slopey = prefix +'/slopey'
        generateLUT( ccd_dim, bin, outfile_table, outfile_displ, outfile_slopex, outfile_slopey, nopupil = True)



