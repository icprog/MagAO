#
#@File: aostest.py
#
# Test many features of AOS
#
# This program starts by setting a number of variables for the AOS,
# then enters an infinite loop in which it periodically updates 
# a 256x256 raw image coded as an 8 byte array into an RTDB variable
#
# Note as of September 2010 this procedure is obsolete. It has been
# replaced by a dedicated thrdtest procedure in the AOS development
# source tree
#@

from AdOpt import thAOApp, AOVar, AOMsgCode, AOSCode, AOGlobals
import time
import sys
import array
import Image
import random

CurSide=''

def help():
  print "\nProgram aostest.    L. Fini, June 2009\n"
  print "Test many features of AOS\n"
  print "Usage: python aostest.py [-v] [-e] [-i] [-m IP:port] left|right\n"
  print "  -v: test only AOS variables"
  print "  -e: test only error messages"
  print "  -i: test only image update"
  print "  -m: specify MsgD IP:port (e.g.: 192.168.3.1:9753)\n"
  print "If no option is specified all three steps are performed in sequence\n"
  print "This procedure starts by setting a number of variables for the AOS,"
  print "then sends some info, warning and error messages, then enters an"
  print "infinite loop in which it periodically updates a 256x256 raw image"
  print "coded as an 8 byte array into an RTDB variable\n"
  print "After the last step, he program must be stopped by typing CTRL-C\n"

def term_handler():
  sys.exit()

class RandomPosition:
  def __init__(self,min,max):
    self.min=min
    self.max=max
    self.dd=max-min
    self.speedx=0.1
    self.speedy=0.2
    self.x=int(random.randint(self.min,self.max))
    self.y=int(random.randint(self.min,self.max))
    self.n=0
  def new(self):
    if self.n>=10:
      self.speedx=randint(-20,20)*0.001
      self.speedy=randint(-20,20)*0.001
      self.n=0
    self.x+=int(self.speedx*self.dd)
    self.y+=int(self.speedy*self.dd)
    if self.x<self.min: 
      self.x=2*self.min-self.x
      self.speedx=-self.speedx
    elif self.x>self.max:
      self.x=2*self.max-self.x
      self.speedx=-self.speedx
    if self.y<self.min: 
      self.y=2*self.min-self.y
      self.speedy=-self.speedy
    elif self.y>self.max:
      self.y=2*self.max-self.y
      self.speedy=-self.speedy
    return (self.x,self.y)
  
def crea(name,value):
  global app
  varName=name
  pos=name.find('!')
  if pos>=0: varName[pos]=CurSide

  if (type(value) is list) or (type(value) is tuple):
    c=value[0]
  else:
    c=value
  if type(c) is int or type(c) is long:
    vtype='INT_VARIABLE'
  elif type(c) is float:
    vtype='REAL_VARIABLE'
  elif type(c) is str:
    vtype='CHAR_VARIABLE'
  else:
    print "Variable type error.",type(c),"not supported"
    return None

  var=AOVar.AOVar(name=varName,tipo=vtype,value=value)

  print "Creating variable",var.name,":",value
  app.CreaVar(var)
  return var;

def set(var,value):
  global app
  print "Setting variable",var.name,":",value
  app.SetVar(var,value)

def sleep(tm):
  print "Sleeping",tm,"sec"
  time.sleep(tm)
  
def playWithAOS(app,arg):
  global wfs_pyramid_pos,pyp
  pyp=RandomPosition(10,240)

  aos_servstat=crea("AOARB.!.ServStat",0)

  print "Setting initial values for all variables. Because AO.ServStat is 0" 
  print "No changes will be seen in Data Dictionary" 

  aoarb_correctedmodes=crea("AOARB.!.CORRECTEDMODES", 0)
  aoarb_fsm_state=     crea("AOARB.!.FSM_STATE","Ready")
  aoarb_idl_stat=      crea("AOARB.!.IDL_STAT",1)
  aoarb_lab_mode=      crea("AOARB.!.LAB_MODE",0)
  aoarb_loopon=        crea("AOARB.!.LOOPON",0)
  aoarb_mode=          crea("AOARB.!.MODE","")
  aoarb_msg=           crea("AOARB.!.MSG","Initializing AO variables")
  aoarb_ofl_enabled=   crea("AOARB.!.OFL_ENABLED",0)
  aoarb_strehl=        crea("AOARB.!.STREHL",0.0)
  aoarb_wfs_source=    crea("AOARB.!.WFS_SOURCE",AO_WFS_FLAO)

  adsec_coil_status=   crea("AOARB.!.ADSEC.COIL_STATUS",1)
  adsec_fsm_state=     crea("AOARB.!.ADSEC.FSM_STATE","Ready")
  adsec_health=        crea("AOARB.!.ADSEC.HEALTH",1)
  adsec_led=           crea("AOARB.!.ADSEC.LED",2)
  adsec_msg=           crea("AOARB.!.ADSEC.MSG","Initializing AdSec variables")
  adsec_pwr_status=    crea("AOARB.!.ADSEC.PWR_STATUS",1)
  adsec_shape=         crea("AOARB.!.ADSEC.SHAPE","Default")
  adsec_tss_status=    crea("AOARB.!.ADSEC.TSS_STATUS",0)

  wfs_ccdbin=          crea("wfsarb.!.CCDBIN@M_FLAOWFS",2)
  wfs_ccdfreq=         crea("wfsarb.!.CCDFREQ@M_FLAOWFS",547.)
  wfs_counts=          crea("wfsarb.!.COUNTS@M_FLAOWFS",1456)
  wfs_fsm_state=       crea("wfsarb.!.FSM_STATE@M_FLAOWFS","Ready")
  wfs_filter1=         crea("wfsarb.!.FILTER1@M_FLAOWFS","Red")
  wfs_health=          crea("wfsarb.!.HEALTH@M_FLAOWFS",1)
  wfs_led=             crea("wfsarb.!.LED@M_FLAOWFS",1)
  wfs_mod_ampl=        crea("wfsarb.!.MOD_AMPL@M_FLAOWFS",1.5)
  wfs_no_subaps=       crea("wfsarb.!.NO_SUBAPS@M_FLAOWFS",20)
  wfs_pyramid_pos=     crea("wfsarb.!.PYRAMID_POS@M_FLAOWFS",pyp.new())
  wfs_tv_binning=      crea("wfsarb.!.TV_BINNING@M_FLAOWFS",4)
  wfs_tv_exptime=      crea("wfsarb.!.TV_EXPTIME@M_FLAOWFS",0.02)
  wfs_tv_filter2=      crea("wfsarb.!.TV_FILTER2@M_FLAOWFS","Filter_2")

  print "Now AO.ServStat is increased ..."

  set(aos_servstat,7)   # Set operating status

  print "Now return parameters variables are created"

def sendErrors(app):
  print "Sending an INFO message"
  app.SendMsg(app.destination,AOMsgCode['AOS_HOUSKEEP'],"Test info message",payload=AOSCode['AOS_INFO'])
  sleep(3)
  print "Sending a WARNING message"
  app.SendMsg(app.destination,AOMsgCode['AOS_HOUSKEEP'],"Test warning message",payload=AOSCode['AOS_WARNING'])
  sleep(3)
  print "Sending an ERROR message"
  app.SendMsg(app.destination,AOMsgCode['AOS_HOUSKEEP'],"Test error message",payload=AOSCode['AOS_ERROR'])
  sleep(3)
  

def imgAsString(file):
   if file:
      image=Image.open(file)
      r,c=image.size
      size=array.array('l',(r,c))
      data=size.tostring()+image.tostring()
   else:
      data=array.array('l',(0,0)).tostring()
   return data

def imgAsList(file):
   data=imgAsString(file)
   return array.array('B',data).tolist()

def updateImage(app,arg, cycles=0):
  global wfs_pyramid_pos
  if cycles>0:
    decr=1
  else:
    decr=0
  varName=arg+'.WFS.TV_IMAGE'
  data5=imgAsList(None)
  data0=imgAsList("file0.png")
  data1=imgAsList("file1.png")
  data2=imgAsList("file2.png")
  data3=imgAsList("file3.png")
  data4=imgAsList("file4.png")
  img=AOVar.AObit8(varName,data5)
  app.CreaVar(img)
  while not app.TimeToDie():
    cycles -= decr
    if cycles<0: break
    app.SetVar(img,data0)
    set(wfs_pyramid_pos,pyp.new())
    time.sleep(1.0)
    app.SetVar(img,data1)
    if app.TimeToDie(): break;
    set(wfs_pyramid_pos,pyp.new())
    time.sleep(1.0)
    app.SetVar(img,data2)
    set(wfs_pyramid_pos,pyp.new())
    if app.TimeToDie(): break;
    time.sleep(1.0)
    app.SetVar(img,data3)
    set(wfs_pyramid_pos,pyp.new())
    if app.TimeToDie(): break;
    time.sleep(1.0)
    app.SetVar(img,data4)
    set(wfs_pyramid_pos,pyp.new())
    if app.TimeToDie(): break;
    time.sleep(1.0)
    app.SetVar(img,data5)
    set(wfs_pyramid_pos,pyp.new())
    if app.TimeToDie(): break;
    time.sleep(3.0)


def main():
  global CurSide,app
  if len(sys.argv) < 2:
    help()
    sys.exit()
  sys.argv.pop(0)    # Remove command name

  dovars=True
  doerrs=True
  doimg=True

  wantvars=False
  wanterrs=False
  wantimg=False
  arg=''

  addr=None

  while sys.argv:
    arg=sys.argv.pop(0)
    if arg == '-v': wantvars=True
    elif arg == '-e': wanterrs=True
    elif arg == '-i': wantimg=True
    elif arg == '-m': addr=sys.argv.pop(0)
    
      
  arg=arg[0].upper()
  if arg!='L' and arg!='R':
    help()
    sys.exit() 

  CurSide=arg

  myname="AOARB.%c"% CurSide

  if(wantvars or wanterrs or wantimg):
    dovars=wantvars
    doerrs=wanterrs
    dovimg=wantimg
  try:
    app=thAOApp(myname,id=(-1),ip=addr)
  except:
    print "Cannot connect to MsgD"
    sys.exit()

  app.destination="AOS.%c" % CurSide

  if dovars:
    print "\nThe program will now exercise the Variables related to AOS"
    playWithAOS(app,arg)

  if doerrs:
    print "\nThe program will now exercise error message display in AOS"
    sendErrors(app)

  if doimg:
    print "\nThe program will do a few cycles updating the CCD image and then stop."
    updateImage(app,arg,6)
  

if __name__=='__main__': main()
