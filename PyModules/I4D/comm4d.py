# aggiornare un file contenente la lista delle directory create, cosi fa il produce da quel file
#MessageBox(text, caption='capt')
#TextBox(text, caption='capt')

import sys
import os
import time
import shutil
from datetime import date
import time, os, os.path, gc
import glob
from I4D import Constant4d
simulator_mode=Constant4d.simulator
#if simulator_mode != 5:
from Scripting.App import * 
from Scripting.Modify import *
from Scripting.Data import *
from Scripting.Config import *
from Scripting.Measurements import *
from Scripting.Show import TextBox
from Scripting.Show import *
from PhazeMonkey.Util import FastAcq
from I4D import FastAcqProcess3
from I4D import FastAcqMeasure3
from Commons import Constants

disk=Constant4d.disk

#error='4derror'
imgdir=Constant4d.imgdir
datadir=Constant4d.datadir
logdir=Constant4d.logdir
#print 'Data dir= '+datadir
imagenames=Constant4d.imagenames
flatname=Constant4d.flat
sysdir="C:\Program Files\4Sight1.8"

meas_date=time.strftime('%Y%m%d',time.gmtime())
logdir=logdir+'/'+meas_date+'/'
fout=logdir+Constant4d.logfile
fin=datadir+'/'+Constant4d.exchange_file
imgdir=Constant4d.imgdir
if os.path.exists(logdir) == False:
	os.mkdir(logdir)

if os.path.exists(imgdir) == False:
	os.mkdir(imgdir)

def log(dataw):
	print 'Logging Informations'
	now=time.strftime("%a, %d %b %Y %H:%M:%S", time.gmtime())
	f=open(fout,'a+')
	dataw='4D LOG   '+now+'    --->   '+dataw+'\n'
	f.write(dataw)
	#TextBox(dataw, caption='4D acquisition Log')
	f.close()

def startlog(proc):
	s=proc+'\t\tSTARTED'
	if simulator_mode == 1:
		s=s+' in simulator mode'
	
	#log(s)

def stoplog(proc):
	s=proc+'\t\tFINISHED'
	#log(s)


def init():
	if simulator_mode == 0:
		FastAcqMeasure3.setTrigger(1)

	print 'Trigger On'
	s='External Trigger enabled'
	log(s)

def waittrigger():
	time0=time.clock()
	ret=FastAcqMeasure3.acquire(0,2,'dummy',imagenames)
	ret=FastAcqMeasure3.acquire(0,1,'dummy',imagenames)
	nn=len(ret[0])
	time1=time.clock()
	print (time1-time0)

def flat(n=10):
	folder=flatname
	print datadir+'/'+folder+'/'
	if os.path.exists(datadir+'/'+folder+'/') == True:
		rmtree(datadir+'/'+folder+'/')
		os.chdir(datadir)
		os.rmdir(datadir+'/'+folder+'/')
		print 'folder deleted'

	print folder
	capture(n, folder)
	produce(folder)
	print 'Images captured and produced'

def capture(n, folder='acq'):
	copy=(folder == 'acq')
	print copy
	proc='CAPTURE'
	#sysdir=os.getcwd()
	print 'Capturing started'
#	Config.SetSampling(Constant4d.sampling)
	now=str(date.today())
	namefile=imagenames
	myfold=folder
	print 'Destination folder= '+myfold
	print 'Acquiring '+str(n)+' images'
	startlog(proc)

	if simulator_mode == 0:
		ret=FastAcqMeasure3.acquire(0,n,myfold,namefile)
	
	else:
		sim_capture(n,myfold, namefile)

	print str(n)+' images captured'
	ldir=imgdir+'/'+myfold+'/raw'
	os.chdir(ldir)
	list=os.listdir(ldir)
	nacq=len(list)
	#last='_%(c)04d.dat'%{'c':nacq-1}
	#info0, info1=os.stat(ldir+'/'+namefile+'_0000.dat'), os.stat(ldir+'/'+namefile+last)
	#dt=float(info1.st_ctime-info0.st_ctime)  #was ctime
	frinfo=ret[0]		
	print frinfo
	if simulator_mode == 0:
		bad=0
		missed=0
		tt=[]
		deltat=[]
		i=-1
		for frame in frinfo:
			i=i+1
			tt.append(frame[0])
			if i > 0:
				deltat.append(tt[i]-tt[i-1])

			bad=bad+frame[4]
			missed=missed+frame[6]
	
		freq = 1/((sum(deltat)/(nacq-1))/1000000.)
		

	else:
		freq=25
		bad=0
		missed=0
	
	freqs='%(c).2f'%{'c':freq}
	bads='%(c).2f'%{'c':bad}
	misseds='%(c).2f'%{'c':missed}
	os.chdir(sysdir)
	nfiles=len(os.listdir(ldir))
	
	
	print 'Capturing completed'
	out_fold=''
	if copy == True:
	
		ff=open(fin)
		tt=ff.read()
		tt=tt.strip()
		ff.close()
		fromfolder=imgdir+'/'+myfold+'/raw'
		dest=datadir+'/'+tt
		os.mkdir(dest)
		dest=dest+'/raw'
		os.mkdir(dest)
		list=os.listdir(fromfolder)
		os.chdir(fromfolder)
		for f in list:
			shutil.move(f, dest)
	
		os.chdir(imgdir+'/'+myfold)
		os.rmdir(fromfolder)
		os.chdir(imgdir)
		os.rmdir(imgdir+'/'+myfold)
		os.unlink(fin)
		out_fold=tt
	else:
		move(myfold)
		out_fold=myfold


	if bad == 0 and missed == 0: 
		print 'Frames successfully captured, trigger frequency=  '+freqs+' Num of BAD= '+bads+' Num of MISSED= '+misseds
		
		data=proc+'\tOK\t\t'+str(nacq)+'/'+str(n)+'\t\tImages Captured;\t\tFreq= '+freqs+' Num of BAD= '+bads+' Num of MISSED= '+misseds+'\t\tFolder= '+folder
		log(data)
	else: 
		print 'Acquisition failed'
		dataw=proc+'\tERR\t\tAcquisition Failed, '+str(nacq)+'/'+str(n)+' images captured'
		log(dataw)
		raise error

	stoplog(proc)
	os.chdir(sysdir)
	return out_fold
	
	
	

def process(fold):#(n, namefile, folder)
	proc=('PROCESS')
	sysdir=os.getcwd()
	print 'Processing started'
	namefile=imagenames
	ldir=datadir+'/'+fold+'/raw'
	#os.chdir(ldir)
	list=os.listdir(ldir)
	n=len(list)
	startlog(proc)
	if simulator_mode == 0:
		FastAcqMeasure3.acquire(1,n,fold,namefile)
	else:
		sim_process(n,fold, namefile)
	
	nfiles=len(os.listdir(ldir))
	print 'Processing completed'
	print str(nfiles)+' files processed'
	if nfiles == n:
		dataw=proc+'\tOK:\t\t'+str(nfiles)+'/'+str(n)+'\t\tFiles processed.\t\tDestination Folder='+ldir
		log(dataw)
	else:
		dataw=proc+'\tERR:\t\t'+str(nfiles)+'/'+str(n)+'\t\tFiles processed.\t\tDestination Folder='+ldir
		log(dataw)
	stoplog(proc)
	#os.chdir(sysdir)
	
def reduce(folder):#(n, namefile, folder)
	proc='REDUCE'
	print 'Data Reduction started'
	namefile=imagenames
	want_frames=0								#put 1 instead of 0, or cancel '_mod' and 0 to restore previous code
	#now=str(date.today())
	#fold=now+'_'+folder
	ldir=datadir+'/'+folder+'/calibrated'
	#os.chdir(ldir)
	list=os.listdir(ldir)
	n=len(list)
	startlog(proc)
	if simulator_mode == 0:
		FastAcqProcess3.datToH5(folder,namefile , n,want_frames)	
	else:
		sim_reduce(n,folder, namefile)
	
	ldir=datadir+'/'+folder+'/hdf5'
	nfiles=len(os.listdir(ldir))/(want_frames+1)
	print str(nfiles)
	if nfiles == n: 
		print 'Data reduction completed, folder "'+folder+'.HDF5" created'
		dataw=proc+'\tOK:\t\t'+str(nfiles)+'/'+str(n)+'\t\tFiles reduced.\t\tDestination Folder='+ldir
		log(dataw)
		calfolder=datadir+'/'+folder+'/calibrated'
		print calfolder
		ll=os.listdir(calfolder)
		for i in ll:
			os.remove(calfolder+'/'+i)
		#os.chdir(datadir+'/'+folder)
		os.rmdir(calfolder)
		print 'Folder'+calfolder+' deleted from disk'
	else:
		print 'Reduction failed'
		dataw=proc+'\tERR:\t\t'+str(nfiles)+'/'+str(n)+'\t\tFiles processed.\t\tDestination Folder='+ldir
		log(dataw)
		raise error
	stoplog(proc)
	
def move(fold):#(fold,dest)
	proc='MOVE'
	dest=fold
	print 'Files transfer started'
	#sysdir=os.getcwd()
	#print sysdir
	startlog(proc)
	fromfolder=imgdir+'/'+fold+'/raw'
	dest=datadir+'/'+dest
	os.mkdir(dest)
	dest=dest+'/raw'
	os.mkdir(dest)
	#os.chdir(imgdir+fold)
	list=os.listdir(fromfolder)
	n1=len(list)
	win=CreateProgressWindow(title='Move', message='Moving files', maximum=n1)
	os.chdir(fromfolder)
	i=0
	for f in list:
		shutil.move(f, dest)
		bar=ProgressWindow.Update(win,i)
		i=i+1
		#os.remove(f)
	list2=os.listdir(dest)
	n2=len(list2)
	if n1 == n2:
		print 'Files transfer successfully completed, '+str(n2)+'/'+str(n1)+' Images copied'
		dataw='MOVE\tOK:\t\t'+str(n2)+'/'+str(n1)+'\t\tImages copied, from Folder='+imgdir+'/'+fold+', to Folder '+dest
		log(dataw)
	else:
		print 'Files transfer failed, '+str(n2)+'/'+str(n1)+' Images copied'
		dataw='MOVE\tERR:\t\t'+str(n2)+'/'+str(n1)+'\t\tImages copied, from Folder='+imgdir+'/'+fold+', to Folder '+dest
		log(dataw)

	os.chdir(imgdir+'/'+fold)
	os.rmdir(fromfolder)
	os.chdir(imgdir)
	os.rmdir(imgdir+'/'+fold)
	ProgressWindow.Close(win)
	stoplog(proc)
	os.chdir(sysdir)
		
def produce(folder):
	proc=('PRODUCE')
	print 'Producing started'
	startlog(proc)
	namefile=imagenames
	process(folder)
	reduce(folder)
	print 'Producing completed'
	#dataw='PRODUCE\tOK:\t\t'+str(n)+'/'+str(n)+'\t\tImages produced'
	#log(dataw)
	stoplog(proc)

def image(n,folder):
	proc=('IMAGE')
	print 'Imaging started'
	startlog(proc)
	namefile=imagenames
	capture(n,folder)
	move(folder)
	process(n,folder)
	reduce(n,folder)
	print 'Imaging completed'
	dataw='IMAGE\tOK:\t\t'+str(n)+'/'+str(n)+'\t\tImages acquired'
	log(dataw)
	os.chdir(sysdir)
	stoplog(proc)

def multiproduce(folder):#accepts as inputs both a folder containing several subfolders and a given array of folders
	if len(folder) == 1:
		ll=os.listdir(datadir+'/'+folder[0])
		for i in range(len(ll)):
			ll[i]=folder[0]+ll[i]
		print ll
	else:
		print 'opz 1'
		ll=folder
		
	ndir=len(ll)
	j=0
	nfile=0
	for i in ll:
		nfile=nfile+len(os.listdir(datadir+'/'+i+'/raw'))
	
	tottime=(0.9*nfile)/60.
	print 'Producing started. The process will require %(c).1f minutes'%{'c':tottime}
	win=CreateProgressWindow(title='Running 4SightScript', message='Producing Started,   %(c).1f minutes'%{'c':tottime}+' remaining', maximum=tottime*100)
	
	print 'Total files number= '+str(nfile)+', total processing time=%(c).2f minutes'%{'c':tottime}
	for i in ll:
		j=j+1
		print 'Producing Folder '+i+', '+str(j)+'/'+str(ndir)
		n=len(os.listdir(datadir+'/'+i+'/raw/'))
		print 'file2 analize='+str(n)
		remtime=(tottime-(0.9*n)/60.)
		print remtime
		bar=ProgressWindow.Update(win,remtime*100,new_message='Producing folder '+i+'  %(c).1f minutes'%{'c':tottime}+' remaining')
		
		print 'Remaining processing time= %(c).2f minutes'%{'c':remtime}
		produce(n, i)
	ProgressWindow.Close(win)
	print 'MultiProducing completed'



def sim_capture(n,folder, name):
	filefolder=imgdir+'/'+folder
	print filefolder
	if os.path.exists(filefolder) == False:
		os.mkdir(filefolder)
	
	filefolder=imgdir+'/'+folder+'/raw'
	print filefolder
	if os.path.exists(filefolder) == False:
		os.mkdir(filefolder)

	for i in range(n):
		num='_%(c)04d'%{'c':i}
		namefile=filefolder+'/'+name+num+'.dat'
		f=open(namefile,'a+')
		dataw='La risposta '
		f.write(dataw)
		f.close()


def sim_process(n, folder, name):
	filefolder=datadir+'/'+folder
	if os.path.exists(filefolder) == False:
		os.mkdir(filefolder)

	filefolder=filefolder+'/calibrated'
	if os.path.exists(filefolder) == False:
		os.mkdir(filefolder)

	for i in range(n):
		num='_%(c)04d'%{'c':i}
		namefile=filefolder+'/'+name+num+'.dat'
		f=open(namefile,'a+')
		dataw='e dentro di te '
		f.write(dataw)
		f.close()

def sim_reduce(n,folder, name):
	filefolder=datadir+'/'+folder
	if os.path.exists(filefolder) == False:
		os.mkdir(filefolder)
	for i in range(n):
		num='_%(c)04d'%{'c':i}
		namefile=filefolder+'/'+name+num+'.hdf5'
		f=open(namefile,'a+')
		dataw='e pero e sbagliata... '
		f.write(dataw)
		f.close()
def rmtree(path):
	for root, dirs, files in os.walk(path, topdown=False):
		for name in files:
			os.remove(os.path.join(root, name))
		for name in dirs:
			os.rmdir(os.path.join(root, name))
	
		


#def mkfold(mod1, mod2, ext):
	#this function formats the folder name in the form
	

#def use4d(n, namefile,folder,type):	#type:capture, process, reduce, produce, image
#	if type='capture':
#		capture(n, namefile, folder)
#	if type='process':
#		process(n, namefile, folder)
#	if type='reduce':
#		
#	if type='produce':
#		
#	if type='image':
	

