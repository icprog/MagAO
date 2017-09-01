#!/usr/bin/env python

import pyfits
import os
import string, types

#+Function: createFits()
#
# Create a new empty FITS object
#
# returns: FITS object
#-

def createFits():
    'Creates a new empty FITS object'

    fitsobj = pyfits.HDUList()
    return fitsobj

#+Function: addToFits()
#
# Add to the FITS object a data section
#
# fitsobj: FITS object as returned from createFITS()
# data: numarray object with data
# hdr: Python dictonary with header key-value pairs
#
# This function may be called multiple times to add
# multiple data sections to a single FITS file
#-


def addToFits( fitsobj, data, hdr):
    'Add a new header/data section to a FITS object'

    # Select between Primary and Image HDU
    if len(fitsobj) <1:
        hdu = pyfits.PrimaryHDU()
    else:
        hdu = pyfits.ImageHDU()

    # Set the FITS data
    hdu.data = data

    # Create the FITS header    
    for k in sorted(hdr.keys()):
        hdu.header.update('HIERARCH '+k, hdr[k])

    # Append to the main fits object and exit
    fitsobj.append(hdu)

#+Function: writeFits()
#
# Writes the FITS object on disk
#
# fitsobj: FITS object to be written
# filename: file to write to
#-
    
def writeFits( fitsobj, filename):
    'Writes the FITS object to disk'
    
    # Write out the file
    fitsobj.writeto(filename)

#+Function: writeSimpleFits
#
# quick function to write a single data matrix
#-

def writeSimpleFits( filename, data, hdr = {}, overwrite = False):
    'Writes a single data matrix on disk'

    if overwrite:
        if os.access( filename, os.F_OK):
            os.unlink( filename)

    fits = createFits()
    addToFits( fits, data, hdr)
    writeFits( fits, filename)

def readSimpleFits( filename):
    fitsobj = pyfits.open(filename)
    data = fitsobj[0].data
    fitsobj.close()

    return data

def readFitsHdr(filename):
    fitsobj = pyfits.open(filename)
    hdr = fitsobj[0].header
    fitsobj.close()
    for card in hdr.ascard:
        card.verify('fix')
    return hdr

def cleanHdr(hdr):
    '''
    Removes standard keywords from a hdr dictionary.
    '''

    keys = ['SIMPLE', 'BITPIX', 'NAXIS', 'NAXIS1', 'NAXIS2', 'NAXIS3', 'END']
    for k in keys:
        try:
            del hdr[k]
        except KeyError:
            pass

    # Remove non-printable characters
    for k in hdr.keys():
        if type(hdr[k]) == types.StringType:
            hdr[k] = filter( lambda x: x in string.printable, hdr[k])

    return hdr


#
# Pyfits read functions are powerful enough that wrappers
# are not necessary.
#
# Typical usage is as follows:
#
# fitsobj = pyfits.open('filename')
#
# numsections = len(fitsobj)
#
# data = fitsobj[0].data # for first section
# hdr = fitsobj[0].hdr
# ...
# data = fitsobj[numsections-1].data
# hdr = fitsobj[numsections-1].header
#
# print hdr.items()
#
# print hdr['type']
#
# fitsobj.close()


        


    
