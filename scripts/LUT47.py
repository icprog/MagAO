#!/usr/bin/env python2
#
#+File: BCUfunctions.py
#
# Python utility functions for the BCU.
#
# generateLUT(): Pixel lookup table generation
#-

import numarray


def generateLUT47(binning=1, outfile_for_bcu='', outfile_for_c=''):
    LUT_SIZE = 29120     # dimensione in word della LUT di scarico AIA2PIO alla 
                         # memoria del DSP (solo i pixel senza il counter ogni 8 pixel)
    M_SIZE = 1024        # frame n of columns
    N_SIZE = 1024        # frame n of rows

    to_do_interleave = 8
    pixel_start_address = 0x04

    M = M_SIZE/int(binning)
    N = N_SIZE/int(binning)

    a=range(pixel_start_address,pixel_start_address+LUT_SIZE)
    # Now insert the slope counter every to_do_interleave pixel
    pos = to_do_interleave
    cum = to_do_interleave
    while pos<len(a):
        a.insert(pos,cum)
        pos += to_do_interleave+1
        cum += to_do_interleave
    #a.append(LUT_SIZE) # (come da file .doc)    
    a.append(0) # (come da file matlab)
    
    # lut[1::2]*65536 + lut[0::2]
    even = a[0::2]
    odd  = a[1::2]
    if len(odd)<len(even):
        odd.append(0)
    lut = numarray.add(numarray.multiply(odd,65536),even)

    if outfile_for_bcu != "":
        outfile = file(outfile_for_bcu, "w")
        for i in lut:
            line = "0x%08X" % (i)
            outfile.write(line+"\n")
        outfile.close()
        print "Written "+outfile_for_bcu

    if outfile_for_c != "":
        outfile = file(outfile_for_c, "w")
        for j in range(0,N):
            for i in range(0,M/2):
                line = "%d" % ( (N-j-1)*M + 2*i )
                outfile.write(line+"\n")
            for i in range(M/2, M):
                line = "%d" % ( (N-j-1)*M + 2*(M-i)-1 )
                outfile.write(line+"\n")
        outfile.close()
        print "Written "+outfile_for_c

    return lut
    

if __name__ == "__main__":
	import sys
	generateLUT47(sys.argv[1],sys.argv[2],sys.argv[3])
	

