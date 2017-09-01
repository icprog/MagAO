#@Class: pisces_ccd
#
# Python PISCES CCD module

from ccd import *
from AdOpt import calib, cfg, fits_lib
import os.path, time, numpy
import socket

class pisces_ccd:

    def __init__(self, app):

	self._ip = '192.168.18.165'
	self._port = 9000
	self._socket = None
        self._host = 'obs4'
        self._user = 'LBTO'

        self.app = app

	self._debug=0
        self._unwrap_th = -1000
	self.connect( self._ip, self._port)

    def filtername(self, n):
        ff = ['2.14 um','H2, 2.12 um','open','dark','J','Ks','H','Br_gamma 2.16 um','[FeII] 1.64 um','2.086 um']
        return ff[n]

    def debug(self, debug):
        self._debug = debug

    def connect( self, host, port):

        self._socket = socket.socket( socket.AF_INET, socket.SOCK_STREAM)
        print 'Connecting to %s:%d' % (host, port)
        self._socket.connect((host, port))
        if self._socket:
            print 'Connected'
            self._socket.settimeout(30)

    def disconnect(self):
        self._socket.close()

    def request(self, req):
        try:
            if self._debug: print 'Command:'+req
            self._socket.send(req+'\r\n')
	    answ = ''
            while 1:
                if self._debug: print 'Answ:'+ answ
                answ += self._socket.recv(200)
                if answ.find('Pisces>')>0:
                    break
            if self._debug: print 'Answ:'+ answ
            return answ.replace('Pisces>', '')
        except:
            self._socket.close()
            self.connect( self._ip, self._port)

    def status( self):
        answ = self.request('status')
        return (answ.splitlines()[0]).split()[1].upper()

    def filter( self, getName=False):
        answ = self.request('filter')
        pos = answ.find(':')
        f = int(answ[pos+1:])
        if not getName:
           return f
        else:
           return self.filtername(f)
          

    def setfilter(self, filter):
        self.request('filter %d' % filter)

    def nextfile(self):
        answ = self.request('observe')
        pos = answ.find('Filename:')
        pos2 = answ.find(')', pos)
        f = answ[pos+10:pos2].replace('(','').replace(')','')
	return f

    def setexptime(self, exptime):
        self.request('exptime %5.2f' % exptime)

    def setnframes(self, nframes):
        self.request('frames %d' % nframes)        

    def get(self, s, m1, m2):
        pos1 = s.find(m1)
        pos2 = s.find(m2, pos1+len(m1))
        return s[pos1+len(m1):pos2].strip()

    def setdirectory( self, dir):
        self.request('directory %s' % dir)

    def directory(self):
        answ = self.request('directory')
        return answ.get("currently '", "'")

    def setprefix( self, prefix):
        self.request('prefix %s' % prefix)

    def prefix(self):
        answ = self.request('prefix')
        return self.get(answ, "currently '", "'")

    def coadds(self):
        answ = self.request('coadds')
        return self.get(answ, 'We will coadd', 'images')

    def setcoadds(self, coadds):
        self.request('coadds %d' % coadds)

    def init(self):
        self.activate()
        self.setcoadds(1)

    def activate( self):
        self.request('activate')

    def deactivate(self):
        self.request('deactivate')
    
    def allstatus(self):
        answ = self.request('status') 
        exptime = float(self.get(answ, 'ITIME', '\n'))
        nframes = int(self.get(answ, 'FRAMES', '\n'))
        filter  = self.get(answ, 'FILTERNAME', '\n')
        nextfile = self.get(answ, 'Filename: (', ')')
        totaltime = int(self.get(answ, 'OBSTIME', '\n'))
        #filter = self.filter(getName=True)  # The Filter string does not work
        status = self.get(answ,'STATUS','\n').upper()
        index = int(self.get(answ,'INDEX', '\n'))
        prefix = self.get(answ,'PREFIX','\n')
        nextfile = '%s%04d.fits' % (prefix, index)
        return {'exptime':exptime, 'nframes':nframes, 'filter':filter, 'nextfile':nextfile, 'totaltime':totaltime, 'status':status}


    def dx(self):         #R:  ccd dimension in pixels(from 80 to 1024 depending on ccd type)
        return 1024

    def dy(self):         #R:  ccd dimension in pixels(from 80 to 1024 depending on ccd type)
        return 1024

    def xbin(self):
        return 1

    def ybin(self):
        return 1

    def set_framerate(self, framerate):
        pass

    def image(self, pathOnly = False, block=True, timeout=60):
        filename = self.allstatus()['nextfile']
        path = '/mnt/newdata/'+filename
        answ = self.request('go')
        if answ.find('activate')>0:
            self.request('activate')
            self.request('go')

        if not block:
            return path

        now = time.time()
        ok = False
        while now-time.time() < timeout:
            time.sleep(0.3)
            if self.status() == 'IDLE':
                ok = True
                break
        if ok:
            if pathOnly:
                return path
            else:
                return fits_lib.readSimpleFits(path)
               

    def read_frames( self, filenames, getHeader= False):

        nframes = len(filenames) 
        cube = numpy.zeros( (nframes, self.dx(), self.dx()), dtype = numpy.int16)
        for i in range(nframes):
            data = fits_lib.readSimpleFits(filenames[i]).astype('float')
            data += (data< self._unwrap_th)*65536
            cube[i,:,:] = data

        if not getHeader:
            return cube
        else:
            hdr = fits_lib.readFitsHdr(filenames[0])
            return (cube, dict(hdr.items()))


    def get_frames(self, num, type = 'pixels', getHeader=False):


        if num == 1:
            return self.get_frame(type = type, getHeader = getHeader)

        self.setnframes(num)
        prefix = self.prefix()
        allstatus = self.allstatus()
        nextfile = allstatus['nextfile']
        totaltime = allstatus['totaltime']

        self.image( pathOnly=True, timeout = totaltime*1.5)
        cube = numpy.zeros( (num, self.dx(), self.dx()), dtype = numpy.int16)

        first = int(self.get(nextfile, prefix, '.fits'))
        for i in range(num):
            n =first+i
            path = '/mnt/newdata/%s%04d.fits' % (prefix, n)
            data = fits_lib.readSimpleFits(path).astype('float')
            data += (data< self._unwrap_th)*65536
            cube[i,:,:] = data

        if not getHeader:
            return cube
        else:
            hdr = fits_lib.readFitsHdr('/mnt'+nextfile)
            return (cube, dict(hdr.items()))


    #@Method: formatCcd
    #
    # Formats a ccd name from a few variants down to the standard names.
    #@

    def formatCcd(self):
        return 'PISCES'

    #@Method{API}: get_detector_string
    
    def get_detector_string(self):
        return self.formatCcd()

    #
    # Gets a single frame from PISCES.
    # A dark file can be optionally subtracted.
    #@

    def get_frame(self, timeout=1000,          #@P{timeout}: timeout in milliseconds
                        type = 'pixels',       #@P{type}: can be 'pixels', or 'background'. The last case is identical to 'pixels', except that background parameters are ignored.
                        getHeader = False,
                        subtractDark=False):     #@P{subtractDark}: the current slopecomputer background will be be subtracted if <type> if 'pixels', otherwise it has no effects.
                                               #@R: 2D numpy object, of type uint16 (for pixels) or float32 (for slopes)


        path = self.image(pathOnly = True)
        data = (fits_lib.readSimpleFits(path)).astype('float')
        data += (data< self._unwrap_th)*65536
        if not getHeader:
            return data
        else:
            hdr = fits_lib.readFitsHdr(path)
            return (data, dict(hdr.items()))






