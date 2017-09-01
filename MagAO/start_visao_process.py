#!/usr/bin/env python

import signal, subprocess, shlex, sys, os

subp = subprocess.Popen

def set_proc_name(newname):
   from ctypes import cdll, byref, create_string_buffer
   libc = cdll.LoadLibrary('libc.so.6')
   buff = create_string_buffer(len(newname)+1)
   buff.value = newname
   libc.prctl(15, byref(buff), 0, 0, 0)

def handler(signal, frame):
   global subp
   subp.send_signal(signal)

def start_visao_process(proc_name):
   global subp
   sshcom = 'source ~/Source/visao/visao_environment.sh; cd $VISAO_ROOT/bin; ./start_process.py ' + proc_name
   args = ['ssh', '-t', '-t', 'aosup@visaosup.visao', sshcom]
   print args
   subp = subprocess.Popen(args)
   
def main():
   global subp

   exec_name = os.path.basename(sys.argv[0])
   proc_name = sys.argv[2]
   
   set_proc_name(exec_name)
   
   signal.signal(signal.SIGINT, handler)
   signal.signal(signal.SIGTERM, handler)
   
   start_visao_process(proc_name)

   #subp.wait()


if __name__ == '__main__': main()

