#!/usr/bin/env python2
#
#@File: BCUfunctions_obsoleted.py
#
# This file contains old functions previously part of BCUfunctions
# but not used anymore


#+Function: set_oko
#
# Sends a message to the BCUCtrl program to move the OKO mirror
#
# Input: oko_pos: list[37] of actuator positions
#-

def set_oko(oko_pos, reply=0):

    oko = [0] * 38

    oko[0] = 200
    for i in range(37):
        oko[i+1] = oko_pos[i]+1

    okoarray = array.array('L')
    okoarray.fromlist(oko)

    print "set_oko: sending command"

    if reply==0:
        ll.SendBinaryMessage("BCUCTRL", "SETOKO", okoarray)
    else:
        ll.SendBinaryMessage("BCUCTRL", "SETOKO", okoarray, 1)

#+Function: meas_intmat_fast
#
# Measures a zonal interaction matrix
#
# n_frames: number of frames to average for each actuator

def meas_intmat_fast( n_frames):

    ref_pos = 100
    act_pos = 180

    # Get the reference frame

    act = [ref_pos] * 37
    set_oko(act)

    cur = getCurrentFrameNumber()
    slopes = getSlopes( cur, cur+n_frames)
    nomefile = "/home/adopt/scripts/intmat/slopes_flat"
    saveFrame( slopes, nomefile)

    for a in range(37):

        print "actuator "+str(a)

        act = [ref_pos] * 37
        act[a] = act_pos
        set_oko(act)

        cur = getCurrentFrameNumber()
        slopes = getSlopes( cur, cur+n_frames)
        nomefile = "/home/adopt/scripts/intmat/slopes_act"+str(a)
        saveFrame( slopes, nomefile)


#+Function: flatOKO
#
# Apply the Wyko flat to the OKO mirror
#
# Returns the act[] vector with the flat commands
#
# okonum is 0 for the "new" oko, and 1 for the "old" oko
#
# If set is 0, does not set the OKO, but returns the command list

def flatOKO( okonum=0, set=1):

# Previous flat
#   act = [ 96, 88, 99, 113, 98, 108, 95, 103, 98, 103, 120, 96, 114, 106, 101, 89, 89, 95, 95, 95, 86, 77, 42, 45, 34, 15, 43, 75, 89, 90, 85, 76, 69, 81, 77, 92, 96]

#   act = [ 96,88,100,114,98,108,95,108,99, 104,122,95,115,113,103,88,86,93, 96,96,90,80,48,46,36,21,51, 87,99,101,87,81,66,81,77,95, 98]

#   act =  [97,85,100,113,100,106,96,118,101, 106,112,95,113,119,104,84,82,90, 98,116,118,109,88,76 ,65,62,79, 109,111,107,89,83,68,79,88,102, 113]

#   act = [ 97,86,98,112,98,108,96,113,98, 108,117,98,114,117,101,84,83,89, 95,112,112,103,74,68,57,46,69, 105,111,107,89,83,62,74,81,96, 108]

#   if oko_num == 1:
        # Vecchio oko
#       act = [ 91,86,98,104,84,101,94,121,102,104,113,89,105,123,96,73,76,88,99,116,115,102,70,63,54,51,80,118,112,111,89,85,61,70,80,101,118] 

#   Flat fatto col fisba
#   act = [89,96,109,112,109,107,94,104,103,111,126,104,126,116,112,73,74,90,87,113,117,88,63,73,55,60,62,103,106,94,82,76,64,77,78,29,128]

#   Vecchio flat di chiusura
#   act = [ 48, 102, 125,  78,  62,  40,  70, 154, 162, 180, 180, 180, 161, 175,  97, 180, 180, 180, 121, 180, 180, 180,  56, 167, 133,  68, 180, 180,  94, 180, 140,  20,  20, 150,  20, 180, 120]

    act = [ 29,  74, 192,  78,  58,  54,  90, 137, 241, 148, 178, 167, 189, 178, 118, 169, 136, 128, 113, 101, 138,  91,  93, 105, 159, 132, 124, 172, 151,  87, 111,  85,  38,   2,  37, 96,  63]


#   fitsobj = pyfits.open('../config/commands/current_flat.fits')
#   act = fitsobj[0].data
#   fitsobj.close()
#   act_flat = act[0,:]

    act_flat = act

    print act_flat
#   if oko_num == 0:
        # Nuovo oko (turbolentatore facente veci DM)
#       act = [ 127, 151, 129, 126, 120, 130, 133, 109, 103, 56, 68, 26, 77, 72, 110, 97, 140, 145, 144, 52, 83, 87, 136, 105, 120, 132, 77, 36, 18, 23, 50, 61, 64, 42, 15, 47, 40]

#   if set != 0:
    set_oko(act_flat)

    return act_flat


#+Function: setOldOko
#
# Sets the old oko position via the local oko daemon
#-

def setOldOko(list):

    pipe = os.popen("telnet lolcahost 3037", "w")
    for n in range(len(list)):
        command = str(n)+" "+str(list[n])+"\n"
        print command
        pipe.write(command)

    pipe.close()


#+Function: findPupil
#
# finds a pupil inside a quadrant
#
# returns a tuple with:
# - pupil center x coordinate
# - pupil center y coordinate
# - pupil radius

def findPupil(quadrant):

    shape = quadrant.shape
    dx = shape[0]

    avg_pixel = quadrant.mean()

    mask = where(quadrant > avg_pixel, 1, 0)
    
    center_x = (nonzero(mask) % dx).mean()
    center_y = (nonzero(mask) / dx).mean()

    radius=1

    return (center_x, center_y, radius)

#+Function: generateOKO
#
# Generates a REC matrix, a TimeFilter matrix and a VoutVector0 matrix
# to set the OKO to a certain actuator pattern
#
# Input: actuator position array (38 numbers, 0 = BIAS), in the 0-200 range
#        ccd dimension
#
# Output: tuple of three matrices. All data must be converted to 32bit floats before
#         being sent to BCU
#-

def generateOKO( act_comm):

    if len(act_comm) != 38:
        print "ERROR: actuator list must be 38 elements long ("+str(len(act_comm))+" found)"
        return

    # List of Vout indexes for each OKO channel (0 = bias, then 37 actuators)
    OKOtoBCU = [ 1, 12, 20, 11, 9, 3, 6, 14, 21, 17, 10, 13, 0, 5, 4, 2, 7, 8,
                     15, 16, 36, 34, 35, 38, 37, 23, 25, 18, 22, 24, 28, 29, 26,
                     27, 30, 31, 32, 33]

    # REC table is all zero!
    rec_table = [0] * (40 * (ccd_dx/2) * (ccd_dx/2) * 2)

    # TimeFilter table is a diagonal matrix
    timefilter_table = [0] * (40 * 40)

    for act in range(38):
        timefilter_table[ OKOtoBCU[ act]] = act_comm[ act]

    # VoutVector0 and 1 are a series of 1

    VoutVector0 = [1] * 40


    return ( rec_table, timefilter_table, VoutVector0)



def searchOffset():

    offsetx = {}
    offsety = {}
    
    # default values

    offsetx['b'] = 6
    offsety['b'] = 0
    
    offsetx['c'] = -1
    offsety['c'] = 7 
    
    offsetx['d'] = 5
    offsety['d'] = 6
    
    print "Enter quadrant letter, followed by axis, followed by offset."
    print "Example: ax3   or   by-2"
    
    while(1):
        line = sys.__stdin__.readline()
        quadrant = line[0]
        axis = line[1]
        number = int(line[2:])

        if axis == 'x':
            offsetx[quadrant] = number
        else:
            if axis == 'y':
                offsety[quadrant] = number

        print "Offset x: "+str(offsetx['b'])+" "+str(offsetx['c'])+" "+str(offsetx['d'])
        print "Offset y:"+str(offsety['b'])+" "+str(offsety['c'])+" "+str(offsety['d'])

        nomefile = createBCUTables('cloop', 'search', offsetx, offsety, 80)

        ll.SendBinaryMessage("BCUCTRL", "SETLUT", nomefile)


# OLD VERSION OF THE GENERATE_LUT FUNCTION:
#
#+Function: generateLUT_old
#   
# Generates the SciMeasure -> DSP lookup table
#
# CCD Dimension must be a multiple of 4 !!
#   
# This function generates the lookup table that is used
# by the BCU to place pixel data in the right order
# for the slope calculation.
#
# Pixels must end in this position (DSP ordering):
#
# (table1)
#
# A1    0x0034
# B1    0x0035
# A2    0x0036
# B2    0x0037
# C1    0x0038
# C2    0x0039
# D1    0x003A
# D2    0x003B
# A3    0x003C
# B3    0x003D
# ...  ....
#
# And the table, assuming a 40 pixels wide quadrant, will look like this:
#
# (table2)
#
# 0: (A1)    0x0034
# 1: (B40)   0x...
# 2: (C1600) 0x....
# 3: (D1561) 0x....
# 4: (A2)    0x0036
#
# If f() is the raster->scimeasure transfer function, then
#
# table2 = f(table2)
#
# After this, every 8 pixels the # of slopes ready to be calculated must be inserted
#
#
# Algorithm used:
#
# - generate a table that fills each pixel with its target address for slope calculation
#   (in CCD order)
# - move each quadrant by the specified offset, blanking them if they go out of the CCD border
# - mark pixels from the outer row & column if ccd_dx is not a multiple of 4 (for 3x3 binning)
# - scramble pixels into EEV39 ordering
# - remove marked pixels (it should preserve the EEV39 ordering)
# - insert slope counter every 8 pixels

# DSP start address for pixel reordering

# This has to be tested
#REJECT_PIXEL = 0x8000

REJECT_PIXEL = 0x0034

# this one for pixels that must be removed after converting to the EEV39 ordering
REMOVE_PIXEL = -1
 
def generateLUT_old( ccd_dx, offsetx, offsety):

    # Generate the dsp-ordered sequence (table1)

    not_four_multiple =0

    if ccd_dx % 2 != 0:
        print "ERROR: CCD dimension must be a multiple of 2"
        return

    # if not a multiple of four, remember it 
    if ccd_dx % 4 != 0:
        not_four_multiple =1

    numpixels = ccd_dx * ccd_dx
    input_quadrant_dx = ccd_dx/2
    
    # We may have a different output dx
    if not_four_multiple:
        output_quadrant_dx = input_quadrant_dx-1
    else:
        output_quadrant_dx = input_quadrant_dx

    table1 = [REJECT_PIXEL] * numpixels

    counter = DSP_LUT_ADDRESS

    # Write into the table the output position of each pixel
    # The X loop only considers even pixels (odd pixels are filled at the same time)

    for y in range(output_quadrant_dx):
        for x in range(0, output_quadrant_dx, 2):

            a1 = counter
            b1 = counter+1
            a2 = counter+2
            b2 = counter+3
            c1 = counter+5
            d1 = counter+4
            c2 = counter+7
            d2 = counter+6

            table1[ y*ccd_dx +x] = a1
            table1[ y*ccd_dx +x +input_quadrant_dx] = b1
            table1[ y*ccd_dx +x +1] = a2
            table1[ y*ccd_dx +x +input_quadrant_dx +1] = b2
            table1[ (y+input_quadrant_dx)*ccd_dx +x] = c1
            table1[ (y+input_quadrant_dx)*ccd_dx +x +input_quadrant_dx] = d1
            table1[ (y+input_quadrant_dx)*ccd_dx +x +1] = c2
            table1[ (y+input_quadrant_dx)*ccd_dx +x +input_quadrant_dx +1] = d2

            counter = counter+8

    # Quadrant counters in the order A1, B1, C1, D1, A2, B2, C2, D2
    counters = [ 0, 1, 4, 5, 2, 3, 6, 7]

    for c in range(len(counters)):
        counters[c] = counters[c] + DSP_LUT_ADDRESS

    pixelmask = [1] * numpixels

#   even=1
#   whichcounter=0
#   pixelcounter=0
#   for y in range(ccd_dx):
#       for x in range(ccd_dx):
#           if pixelmask[ y*ccd_dx+x] ==1:

#               whichcounter=0

#               ## Choose the right counter accoding to the current quadrant
#               if (x<quadrant_dx) and (y<quadrant_dx):
#                   whichcounter=0
#                   if even==0:
#                       whichcounter = whichcounter+4
#                   table1[ y*ccd_dx +x] = counters[whichcounter]       # A1 - A2
#
#               if (x>=quadrant_dx) and (y<quadrant_dx):
#                   whichcounter=1
#                   if even==0:
#                       whichcounter = whichcounter+4
#                   table1[ y*ccd_dx +x] = counters[whichcounter]  # B1 - B2
#
#               if (x<quadrant_dx) and (y>=quadrant_dx):
#                   whichounter=2
#                   if even==0:
#                       whichcounter = whichcounter+4
#                   table1[ y*ccd_dx +x] = counters[whichcounter]   # C1 - C2
#
#               if (x>=quadrant_dx) and (y>=quadrant_dx):
#                   whichcounter=3
#                   if even==0:
#                       whichcounter = whichcounter+4
#                   table1[ y*ccd_dx +x] = counters[whichcounter]  # D1 - D2
#           
#               counters[whichcounter] = counters[whichcounter] +8
#               even = 1-even
#
#           else:
#               table1[ y*ccd_dx +x] = REJECT_PIXEL
#
    # Comment out the full-aperture mask
    # The X loop only considers even pixels (odd pixels are filled at the same time)

#   offsetx = {}
#   offsety = {}

#   offsetx['b'] = +5
#   offsety['b'] = 10


#   offsetx['c'] = -1
#   offsety['c'] = 5 


#   offsetx['d'] = 4
#   offsety['d'] = 5 

    # Move the quadrants by the specified offsets. Pixels that find themselves outside the CCD
    # will be marked as 0x0034 - i.e. empty

    print "Table1[0]", table1[0]
    print "Table1[0]", table1[1]

    temp_table = [0] * numpixels

    for y in range(input_quadrant_dx):
        for x in range(input_quadrant_dx):
            temp_table[ y*ccd_dx +x] = table1[ y*ccd_dx +x]         # A1    

            x_b = x+offsetx['b']
            y_b = y+offsety['b']
            if (x_b>=0) and (x_b<input_quadrant_dx) and (y_b>=0) and (y_b<input_quadrant_dx):
                temp_table[ y*ccd_dx +x +input_quadrant_dx] = table1[ y_b * ccd_dx +x_b +input_quadrant_dx] # B1
            else:
                temp_table[ y*ccd_dx +x +input_quadrant_dx] = REJECT_PIXEL

            x_c = x+offsetx['c']
            y_c = y+offsety['c']
            if (x_c>=0) and (x_c<input_quadrant_dx) and (y_c>=0) and (y_c<input_quadrant_dx):
                temp_table[ (y+input_quadrant_dx)*ccd_dx +x] = table1[ (y_c+input_quadrant_dx)*ccd_dx +x_c]             # C1
            else:
                temp_table[ (y+input_quadrant_dx)*ccd_dx +x] = REJECT_PIXEL

            x_d = x+offsetx['d']
            y_d = y+offsety['d']
            if (x_d>=0) and (x_d<input_quadrant_dx) and (y_d>=0) and (y_d<input_quadrant_dx):
                temp_table[ (y+input_quadrant_dx)*ccd_dx +x +input_quadrant_dx] = table1[ (y_d+input_quadrant_dx)*ccd_dx + x_d + input_quadrant_dx] # D1
            else:
                temp_table[ (y+input_quadrant_dx)*ccd_dx +x +input_quadrant_dx] = REJECT_PIXEL

            counter = counter+8

    table1 = temp_table

    print "Table1[0]", table1[0]
    print "Table1[1]", table1[1]

    # Now, this table must be converted into scimeasure ordering
    table2 = FrameToEEV39(table1)

    print "Table2[0]", table2[0]
    print "Table2[1]", table2[1]

#   A test with printframe() seems to give good results
#   printframe( table1, ccd_dx)

#   printframe( table1, ccd_dx)
#   print ""
#   printframe( table2, ccd_dx)



    # Fix the pixel number if needed (this is only to reach a multiple of 8)
    if not_four_multiple:
        table2.append(REJECT_PIXEL)
        table2.append(REJECT_PIXEL)
        table2.append(REJECT_PIXEL)
        table2.append(REJECT_PIXEL)

    # Now insert the slope counter every 8 pixels
    # Work backwards, otherwise the indexes are not 8-ordered anymore!

    # For odd numbers of pixels, insert the # of slopes to be calculated
    # in the next-to-last position instead of the last
    table_len = len(table2)
    for pos in range( table_len-8, 0, -8):
        if not_four_multiple and pos == table_len-8:
            table2.insert(pos, output_quadrant_dx*output_quadrant_dx/2)
        else:
            table2.insert( pos, 0)

    # Final number of slopes to be computed

    if not not_four_multiple:
        table2.append( output_quadrant_dx * output_quadrant_dx /2)

    # even number of elements for table2

    if len(table2) % 2 != 0:
        table2.append(0)

    return table2


