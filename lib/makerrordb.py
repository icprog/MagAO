#!/usr/bin/env python
#@File: makerrordb.py

# Generates an include file  for macro definition and a c file for
# the creation of the error table structure from the central error database 
# file (usually: errordb.txt)
#
# Oct 2008: Can also generate an equivalent python file. - A. Puglisi
# Feb 2009: Python version revised - L.Fini
#
#@

import re,sys,string,os

m_com=re.compile("[#;]")        # Matches comment lines
m_blk=re.compile("\s*$")        # Matches blank lines
m_def=re.compile("[A-Za-z]")    # Matches define lines
m_cnt=re.compile("\s+(\S.+)$")  # Matches comment continuation lines
m_splt=re.compile("\s+")        # Set splitting reg.expr.
m_bck=re.compile("\s+")         # matches block separator
noutp=0

def main():
  global outc,outh,outpy,noutp,pyfile,outpro,pydef
  prog="makerrordb"
  vers="2.4"

  if len(sys.argv)<2 or len(sys.argv)>4:
    print """
Usage: makerrordb.py file.txt type [-py <pyname>] [-pydef <pyname>]

The utility generates file.h and file.c and file.pro and optionally pyname.py
All variables and types use the <type> prefix
If -py is used, the file will contain a standard a Code/Type dictionary.
If -pydef is used, the file will contain Python variables similar to C defines.


"""
    sys.exit(0)


  nsplit=string.split(sys.argv[1],'.')
  hfile = nsplit[0]+'.h'
  cfile = nsplit[0]+'.c'
  profile = nsplit[0]+'.pro'
  pyfile = nsplit[0]+'.py'
  define = nsplit[0]+'_H'
  fileCode = nsplit[0]
  pydef = False

  inp=open(sys.argv[1])    # Open input text file

  outh=open(hfile,'w')        # Open output .h file
  outc=open(cfile,'w')        # Open output .c file
  outpro=open(profile,'w')      # Open output .pro file
  if len(sys.argv)==4 and sys.argv[2] == '-py':
    pyfile = sys.argv[3]
    outpy = open(pyfile+'.py','w')  # Open output .py file
    if pyfile[-4:] == 'Code':
        pyfile = pyfile[:-4]    # Normalize dictionary name (will append Code/Type)
  elif len(sys.argv)==4 and sys.argv[2] == '-pydef':
    pyfile = sys.argv[3]
    outpy = open(pyfile+'.py','w')  # Open output .py file
    pydef = True
  else:
    outpy = None

  inp.readline()     # Discard first line

  printcomment("@File: "+hfile, outh)
  printcomment("@File: "+cfile, outc)
  printcomment("@File: "+pyfile, outpy, '#')

  while 1:                  # Jump over file header
    line=inp.readline()
    if m_bck.match(line):
      break
  
  printall("\n");
  printall(" *********      DO NOT EDIT       DO NOT EDIT      DO NOT EDIT    *********", comment = True)
  printall(" Automatically generated from: "+sys.argv[1]+" by "+prog+" Version: "+vers, comment = True)
  printall(" **************************************************************************\n", comment = True)

  print >>outc, "//@"
  print >>outc, "\nstatic DbTable "+fileCode+"Table[] = {"
  print >>outh, "#ifndef "+define
  print >>outh, "#define "+define
  print >>outh, ""
  print >>outh, "// AO"+fileCode+"_START //"

  if outpy and (not pydef):
     print >>outpy, "%sCode = {}" % pyfile
     print >>outpy, "%sType = {}" % pyfile
     print >>outpy, "%sDesc = {}" % pyfile

  lines=[]
  for line in inp:
    if line.find('#')>=0:
        line = line[:line.find('#')]
    if not line:
        continue
    line=line.rstrip()
    mtch = m_cnt.match(line)  # Continuation lines are treated separately
    if mtch:
      if not lines:
        print "Continuation line doesn't follow definition: >",line
      else:
        lines.append(" "+mtch.group(1))
      continue

    if lines:                 # If code line possibly with continuation 
      printcode(lines)        # is pending, output the code
      lines=[]

    if m_com.match(line):
      printcomment(line[1:], outh)
    elif m_blk.match(line):
      printcomment("", outh)
    elif m_def.match(line):
      lines=[line]
    elif m_cnt.match(line):
      lines.append(line)
    else:
      print "Illegal line: >",line

  if lines: printcode(lines)   # Output remaining pending lines

  print >>outc, "};"
  print >>outh, "// AO"+fileCode+"_STOP //"

  print >>outc, "int "+fileCode+"TabLen = sizeof("+fileCode+"Table)/sizeof(DbTable);"

  print >>outh, "#endif /* "+define+" */"
  print >>outh, ""  # Avoid no-newline warning

  outh.close()
  outc.close()
  outpro.close()
  if outpy:
    outpy.close()
  inp.close()

  if noutp == 0:
    print "Warning no code lines have been generated!!!"
    sys.exit(1)
  sys.exit(0)

def printcode(lines):
  "Output a line of code"
  global outc,outh,outpy,noutp,pyfile,pydef
  a=m_splt.split(lines[0])
  noutp +=1
                                          # First do .h file
  print >>outh, "#define %30s %10s" %(a[0],a[1]),
  if len(a)>2:
    print >>outh, '//',string.join(a[2:])
  else:
    print >>outh
  if len(lines)>1:
    for l in lines[1:]:
      print >>outh, '                                                  //',l
  
                                          # then do .c file
  aux = '  {(int)%30s, (char *)"%s" ,(char *)"' %(a[0],a[0])
  if len(a)>2:
    aux += string.join(a[2:])
  if len(lines)>1:
    for l in lines[1:]:
      aux += l
  print >>outc, aux+'"},'
                                          # then do .pro file
  type='int';
  aux = '  %30s int %s ;' %(a[0],a[1])
  if len(a)>2:
    aux += string.join(a[2:])
  if len(lines)>1:
    for l in lines[1:]:
      aux += l
  print >>outpro, aux

                                         # Now do .py file
  if outpy:
     if pydef:
         print >>outpy,"%s = %s" % (a[0], a[1])
     else:
         print >>outpy,"%sCode['%s'] = %s" % (pyfile,a[0],a[1])
         print >>outpy,"%sType[%s] = '%s'" % (pyfile,a[1],a[0])
         aux = "%sDesc[%s] = " % (pyfile, a[1])
         if len(a)>2:
            aux+='"""'+string.join(a[2:])
         else:
            aux+='"""No definition available'
         print >>outpy, aux,
         if len(lines)>1:
            for l in lines[1:]:
               print >>outpy,l,
         print >>outpy,'"""'

def printcomment(line, out, prefix = '//'):
  "prints a comment or blank line"
  if line:
    if line[0] == '-': return   # Do not output '#- ...' comments
    cmt = prefix+line
  else:
    cmt=""
  if out:
    print >>out, cmt

def printall(line, comment = True):
  global outc,outh,outpy,outpro
  if comment:
    printcomment(line, outc)
    printcomment(line, outh)
    printcomment(line, outpy, '#')
    printcomment(line, outpro, ';')

  else:
    print >>outh, line
    print >>outc, line
    print >>outpro, line
    if outpy:
       print >>outpy, line


if __name__ == '__main__':
  main()
