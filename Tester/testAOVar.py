#
# @File: testAOVar.py
#
# Simple test program for Python class AOVar
#
# Creates some variables of different types
#@

from AdOpt import thAOApp, AOVar

imgI=AOVar.AOVar(name='TESTvarINT',tipo='INT_VARIABLE',value=[1,2,10001,100002])
imgR=AOVar.AOVar(name='TESTvarREAL',tipo='REAL_VARIABLE',value=[1.0,2.0,0.33])
img8=AOVar.AOVar(name='TESTvarBIT8',tipo='BIT8_VARIABLE',value=[1,2,0xff])
img16=AOVar.AOVar(name='TESTvarBIT16',tipo='BIT16_VARIABLE',value=[1,2,0x1001])
img32=AOVar.AOVar(name='TESTvarBIT32',tipo='BIT32_VARIABLE',value=[1L,2L,0x100002L])
img64=AOVar.AOVar(name='TESTvarBIT64',tipo='BIT64_VARIABLE',value=[1L,2L,0x100002L])
app=thAOApp('TestVar')
app.CreaVar(imgI)
app.CreaVar(imgR)
app.CreaVar(img8)
app.CreaVar(img16)
app.CreaVar(img32)
app.CreaVar(img64)

