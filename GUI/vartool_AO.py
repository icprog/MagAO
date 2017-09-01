#!/usr/bin/env python
#
#+File: vartool_AO
#
# Vartool window to be integrated into the main AOGUI



# Imported modules

import sys, string, time, os, types
from qt import *
from qttable import *

from AdOpt import AOConstants, Logger
from AdOpt.wrappers import msglib

## Some definitions:

INT_VARIABLE   = AOConstants.AOVarCode['INT_VARIABLE']
REAL_VARIABLE  = AOConstants.AOVarCode['REAL_VARIABLE']
CHAR_VARIABLE  = AOConstants.AOVarCode['CHAR_VARIABLE']
BIT8_VARIABLE  = AOConstants.AOVarCode['BIT8_VARIABLE']
BIT16_VARIABLE  = AOConstants.AOVarCode['BIT16_VARIABLE']
BIT32_VARIABLE  = AOConstants.AOVarCode['BIT32_VARIABLE']
BIT64_VARIABLE  = AOConstants.AOVarCode['BIT64_VARIABLE']

VarLabels = ['Name', 'Type', 'Value', 'Watch?', '', 'Owner', 'Log?', 'Logfile', 'LastUpdated']

LabelIndex={}
for i in VarLabels:
    LabelIndex[ i] = VarLabels.index(i)


## +Class
##
## QReadOnlyTable  QTable derived class for a read-only table
##
## The createEditor() method is reimplemented to prevent
## in-place editing of the table contents

class QReadOnlyTable(QTable):

    def __init__(self, row, col, parent):
        QTable.__init__(self, row, col, parent) 

    def createEditor( self, row, col, f):
        return None


## +Class
##
## IntervalsDlg
##
## Dialog class to ask the definition of value intervals for a variable
## It has a combobox with the variable list taken from VarList, and three
## edit boxes kept in a Python dictionary indexed with the color keywords.

class IntervalsDlg( QDialog):

    def __init__(self, parent, mainwnd):
        QDialog.__init__(self, parent, '', 1)
        self.mainwnd = mainwnd
        self.resize(300,300)
        self.setCaption("New intervals")

        self.gridLayout = QGridLayout( self, 5, 2, 5)

        self.intervals = {}

        self.label1 = QLabel( "Variable:", self)
        self.gridLayout.addWidget( self.label1, 0, 0)

        self.list = QComboBox( 0, self)
        self.gridLayout.addWidget( self.list, 0, 1)
        self.connect( self.list, SIGNAL("activated(int)"), self.slotComboBox)
        for i in range( mainwnd.VarList.numRows()):
            self.list.insertItem(  mainwnd.VarList.text( i, LabelIndex['Name']).latin1() + ' (' + mainwnd.VarList.text( i, LabelIndex['Type']).latin1() + ')')

        self.label2 = QLabel("Green intervals:", self)
        self.gridLayout.addWidget( self.label2, 1, 0)
    
        self.label3 = QLabel("Yellow intervals:", self)
        self.gridLayout.addWidget( self.label3, 2, 0)
    
        self.label4 = QLabel("Red intervals:", self)
        self.gridLayout.addWidget( self.label4, 3, 0)

        self.intervals['green'] = QLineEdit(self)
        self.gridLayout.addWidget( self.intervals['green'], 1, 1)

        self.intervals['yellow'] = QLineEdit(self)
        self.gridLayout.addWidget( self.intervals['yellow'], 2, 1)

        self.intervals['red'] = QLineEdit(self)
        self.gridLayout.addWidget( self.intervals['red'], 3, 1)

        self.okbutton = QPushButton("OK", self)
        self.gridLayout.addWidget( self.okbutton, 4, 0)
        self.connect( self.okbutton, SIGNAL("clicked()"), self.slotOkButton)

        self.cancelbutton = QPushButton("Cancel", self)
        self.gridLayout.addWidget( self.cancelbutton, 4, 1)
        self.connect( self.cancelbutton, SIGNAL("clicked()"), self.slotCancelButton)

        ## Show the initial values

        self.slotComboBox(0)


    def slotComboBox(self,num):
        field = self.list.currentText().latin1()
        varname, dummy = string.split( field, ' (')
        
        if not self.mainwnd.Validators.has_key(varname):
            self.intervals['green'].setText("")
            self.intervals['yellow'].setText("")
            self.intervals['red'].setText("")
            return

        for validator in self.mainwnd.Validators[varname]:
            name = validator.name()
            txt = self.intervals[name].text().latin1()
            txt = txt + str( validator.bottom()) + " " + str(validator.top())+","
            self.intervals[name].setText(txt)
        
    def slotOkButton(self):
        self.varname = self.list.currentText().latin1()
        self.accept()

    def slotCancelButton(self):
        self.reject()


## +Class
##
## VarToolMain Main window for the Variable tool.
##
## This is the main window of this program. Almost all the functionality is built
## into this class

class vartool_AO( QDialog):

    # +Method
    #
    # __init__  window constructor
    #
    # This constructor initializes the window with all the required widgets,
    # setups some data structures and connects to the MsgD-RTDB server
    
    def __init__(self, AOwindow):
        QDialog.__init__(self)
        self.name = "Variable inspector"

        ## Load pixmaps

        self.pixmap = {}
        self.pixmap['green'] = QPixmap(os.getenv('ADOPT_ROOT')+"/images/green15.png")
        self.pixmap['red'] = QPixmap(os.getenv('ADOPT_ROOT')+"/images/red15.png")
        self.pixmap['yellow'] = QPixmap(os.getenv('ADOPT_ROOT')+"/images/yellow15.png")
        self.pixmap['empty'] = QPixmap(os.getenv('ADOPT_ROOT')+"/images/empty15.png")

        # Flag for quick redraw of the list

        self.varlisting = 0
        self.vardumping = 0

        ## Draw the widgets

        self.gridLayout = QGridLayout( self, 17, 7, 6)

        self.label1= QLabel("Variable list", self)
        self.gridLayout.addWidget( self.label1, 0, 0)

        self.VarList = QReadOnlyTable( 0, 9, self)
        self.VarList.setLeftMargin(0)

        for i in LabelIndex.keys():
            self.VarList.horizontalHeader().setLabel( LabelIndex[i], i)

        self.VarList.setColumnStretchable( LabelIndex['Logfile'], 1)

        self.gridLayout.addMultiCellWidget( self.VarList, 1, 13, 0, 1)
        self.connect( self.VarList, SIGNAL("clicked( int, int, int, const QPoint &)"), self.slotVarListClicked)
        self.connect( self.VarList.horizontalHeader(), SIGNAL("clicked(int)"), self.slotVarListHeaderClicked)
        self.sortOrder =1

        self.label3 = QLabel("Filter (with wildcard)", self)
        self.gridLayout.addWidget( self.label3, 14, 0)

        self.VarFilter = QLineEdit(self)
        self.gridLayout.addWidget( self.VarFilter, 15, 0)

        self.RefreshButton = QPushButton("Refresh", self)
        self.gridLayout.addWidget( self.RefreshButton, 15, 1)
        self.connect( self.RefreshButton, SIGNAL("clicked()"), self.slotRefreshButton)

        self.label4 = QLabel("Variable name", self)
        self.gridLayout.addMultiCellWidget( self.label4, 1, 1, 3, 6)

        self.label5 = QLabel("Variable value", self)
        self.gridLayout.addMultiCellWidget( self.label5, 3, 3, 3, 6)

        self.VarName = QLineEdit(self)
        self.gridLayout.addMultiCellWidget( self.VarName, 2, 2, 3, 6)

        self.VarValue = QLineEdit(self)
        self.gridLayout.addMultiCellWidget( self.VarValue, 4, 4, 3, 6)

        self.AddButton = QPushButton("Create", self)
        self.gridLayout.addWidget( self.AddButton, 9, 3)
        self.connect( self.AddButton, SIGNAL("clicked()"), self.slotAddButton)

        self.DelButton = QPushButton("Delete", self)
        self.gridLayout.addWidget( self.DelButton, 9, 6)
        self.connect( self.DelButton, SIGNAL("clicked()"), self.slotDelButton)

        self.ChangeButton = QPushButton("Change", self)
        self.gridLayout.addMultiCellWidget( self.ChangeButton, 9, 9, 4, 5)
        self.connect( self.ChangeButton, SIGNAL("clicked()"), self.slotChangeButton)

        self.label6 = QLabel("Variable type", self)
        self.gridLayout.addMultiCellWidget( self.label6, 5, 5, 3, 4)

        self.label7 = QLabel("Variable ownership", self)
        self.gridLayout.addMultiCellWidget( self.label7, 5, 5, 5, 6)

        self.TypeGroup = QButtonGroup()
        self.OwnerGroup = QButtonGroup()

        self.button1 = QRadioButton( "Integer", self)
        self.gridLayout.addMultiCellWidget( self.button1, 6, 6, 3, 4)
        self.TypeGroup.insert( self.button1, INT_VARIABLE)

        self.button2 = QRadioButton( "Float", self)
        self.gridLayout.addMultiCellWidget( self.button2, 7, 7, 3, 4)
        self.TypeGroup.insert( self.button2, REAL_VARIABLE)

        self.button3 = QRadioButton( "String", self)
        self.gridLayout.addMultiCellWidget( self.button3, 8, 8, 3, 4)
        self.TypeGroup.insert( self.button3, CHAR_VARIABLE)

        self.button4 = QRadioButton( "Private", self)
        self.gridLayout.addMultiCellWidget( self.button4, 6, 6, 5, 6)
        self.OwnerGroup.insert( self.button4, 0)
        self.button5 = QRadioButton( "Global", self)
        self.gridLayout.addMultiCellWidget( self.button5, 7, 7, 5, 6)
        self.OwnerGroup.insert( self.button5, 1)

        self.WatchCheck = QCheckBox("Watch this variable(s)", self)
        self.gridLayout.addMultiCellWidget( self.WatchCheck, 10, 10, 3, 6)
        self.connect( self.WatchCheck, SIGNAL("toggled(bool)"), self.slotWatchCheck)

        self.LogCheck = QCheckBox("Log this variable(s)", self)
        self.gridLayout.addMultiCellWidget( self.LogCheck, 11, 11, 3, 6)
        self.connect( self.LogCheck, SIGNAL("toggled(bool)"), self.slotLogCheck)

        self.LogButton = QPushButton("Set log file", self)
        self.gridLayout.addMultiCellWidget( self.LogButton, 12, 12, 3, 6)
        self.connect( self.LogButton, SIGNAL("clicked()"), self.slotLogButton)

        self.IntervalsButton = QPushButton("Set intervals", self)
        self.gridLayout.addMultiCellWidget( self.IntervalsButton, 13, 13, 3, 6)
        self.connect( self.IntervalsButton, SIGNAL("clicked()"), self.slotIntervalsButton)

        self.DumpButton = QPushButton("Dump variables", self)
        self.gridLayout.addMultiCellWidget( self.DumpButton, 15, 15, 5, 6)
        self.connect( self.DumpButton, SIGNAL("clicked()"), self.slotDumpButton)

        ## Prepare an empty dictionary of watched variables

        self.Watched = {}   

        ## Prepare an empty dictionary of Logger objects (one for each logged variable)

        self.Loggers = {}
    
        ## Prepare an empty dictionary of Validator objects

        self.Validators = {}


        ## Connect() to the VARREPLY slot - the only one that we use

        QObject.connect( AOwindow, PYSIGNAL( "%d" % AOConstants.AOMsgCode['VARREPLY']), self.slotNotify)
        QObject.connect( AOwindow, PYSIGNAL( "%d" % AOConstants.AOMsgCode['VAREND']), self.slotNotify)
        QObject.connect( AOwindow, PYSIGNAL( "%d" % AOConstants.AOMsgCode['TERMINATE']), self.close)
    
        ## Start with a complete variable list
        
        self.VarFilter.setText("*")
        self.slotRefreshButton()

    ## +Method
    ##
    ## readjust       readjust table widths to fit contents  
    ##
    ## Since the adjusting is a slow operation, it has been put into a separate function
    ## that functions may or not may call after table values have changed.

    def readjust(self):
        for i in LabelIndex.keys():
            self.VarList.adjustColumn( LabelIndex[i])
    

    ## +Method
    ##
    ## slotNotify   handler for socket messages
    ##
    ## This method is the network event handler for the whole application. It's called by QSocketNotifier when
    ## some data is waiting in the server-connected socked. Thus WaitMessage() is guaranteed to return some
    ## useful data. Different actions are taken based on the message contents.

    def slotNotify(self, args):
        (message) = args
        status = message['status']
        code = message['code']

        if status != "NO_ERROR" and status != "OK":
            print "Error: Status is "+status
            return

        # When a VAREND arrives, display all variables in one shot
        if code == AOConstants.AOMsgCode['VAREND']:

            if self.vardumping:
                keys = self.vardump.keys()
                keys.sort()
                for key in keys:
                    self.dumpfile.write( self.vardump[key])
                self.dumpfile.close()
                self.vardumping = 0
                QMessageBox.information(self, 'Dump saved', 'Dump saved')

            if self.varlisting:
                self.VarList.setNumRows( len(self.varlist))
                count=0
                ss = self.varlist.keys()
                ss.sort()
                for varname in ss:
                    for i in self.varlist[varname].keys():  
                        formattedText = self.__format(self.varlist[varname][i])
                        self.VarList.setText(count, LabelIndex[i], formattedText)

                    self.Loggers[self.varlist[varname]['Name']] = Logger.Logger()
                    self.Watched[self.varlist[varname]['Name']]=0
                    count = count+1

                self.readjust()
            
        elif code == AOConstants.AOMsgCode['VARREPLY'] or code == AOConstants.AOMsgCode['VARCHANGD']:
            varname = message['name']

            pos = self.findVarListElement( varname)

            # Special varlist handle
            if self.varlisting:
                if not self.varlist.has_key(varname):
                    type = {INT_VARIABLE:'int',REAL_VARIABLE:'real',CHAR_VARIABLE:'string'}[ message['type']]
                    owner = message['owner'] or 'global'
                    self.varlist[varname] = {'Name':varname, 'Type':type, 'Value':self.__format(message['value']), 'Watch?':'N', 'Owner':owner, 'Log?':'N','Logfile':'','LastUpdated': self.__formatTime(message['mtime'])}
                    return

            # Special vardump handle
            if self.vardumping:
                str = '%-40s %s\n' % (varname, self.__format(message['value']))
                self.vardump[varname] = str
                
            # Add variable to varlist if needed

            elif pos <0:
                pos = self.VarList.numRows()
                self.VarList.setNumRows( pos+1)
                type = {INT_VARIABLE:'int',REAL_VARIABLE:'real',CHAR_VARIABLE:'string'}[ message['type']]
                owner = message['owner'] or 'global'
                #texts = {'Name':varname, 'Type':type, 'Value':self.__format(message['value']), 'Watch?':'N', '':None, 'Owner':owner, 'Log?':'N','Logfile':''}
                texts = {'Name':varname, 'Type':type, 'Value':self.__format(message['value']), 'Watch?':'N', 'Owner':owner, 'Log?':'N','Logfile':'','LastUpdated': self.__formatTime(message['mtime'])}
                #if type == 'real':
    			#    text['Value'] = "%5.2f" % message['value']
                #    self.varlist.append(texts)
                for i in texts.keys():
                    formattedText = self.__format(texts[i])
                    self.VarList.setText( pos, LabelIndex[i], formattedText)

                ## Create some objects related to this variable

                self.Loggers[varname] = Logger.Logger()
                self.Watched[varname]=0
                return
                
            ## Refresh the detail view

            if varname == self.VarName.text().latin1():
                self.VarValue.setText( self.__format(message['value']))
                self.TypeGroup.setButton( message['type'])  
                self.WatchCheck.setChecked( self.Watched[varname])

                if message['owner'] == '':
                    self.OwnerGroup.setButton(1)
                else:
                    self.OwnerGroup.setButton(0)
                
                if self.Loggers[varname].GetOutputFile() != '':
                    self.LogCheck.setEnabled(1)
                else:
                    self.LogCheck.setEnabled(0)


            ## Update the shown value
            # Check if is a float number and cut decimal digits
            self.VarList.setText( pos, LabelIndex['Value'], self.__format(message['value']))
            self.VarList.setText( pos, LabelIndex['LastUpdated'], self.__formatTime(message['mtime']))
            item = self.VarList.item( pos, LabelIndex[''])
            if item:
                item.setPixmap( self.pixmap['empty'])

            if self.varlisting == 0:
                self.VarList.adjustColumn( LabelIndex['Value'])

            ## Take care of writing the log files

            self.LogMessage( message)

            ## Validate the variable value

            self.Validate( message['name'], message['value'], pos)

        else:   
            print "Got message: "+str(message['code'])+"  -  "+message['body'] 
            

    ## +Method: LogMessage
    ##
    ## This function takes a VARREPLY message dictionary, and checks if any
    ## loggers is interested in the new value. If so, sends a "name: value" string
    ## to the logger (who will tipically write this to a file with a timestamp)

    def LogMessage( self, message):
        varname = message['name']
        if self.Loggers.has_key(varname):
            line = varname + ": " + self.__format(message['value'])
            self.Loggers[varname].LogString( line)


    ## +Method: Validate
    ##
    ## This function takes a variable name and value, and checks the value
    ## against the set of defined validators for the variable
    ## It then shows the corresponding light on the VarList table

    def Validate( self, varname, value, pos):
        if not self.Validators.has_key(varname):
            return

        if len(self.Validators[varname]) < 1:
            return

        strvalue = self.__format(value)
        result=''
        for validator in self.Validators[varname]:
            state = validator.validate( strvalue,0)
            if (state[0] == QValidator.Acceptable):
                result = validator.name()
                break

        if result=='':
            return

        item = self.VarList.item( pos, LabelIndex[''])
        item.setPixmap( self.pixmap[ result])   

    
    ## +Method: findVarListElement
    ##
    ## This function searches for a text string into the VarList table.
    ## Returns the (zero-based) index of the first matching element, or -1 if not found

    def findVarListElement(self, text):
        for i in range( self.VarList.numRows()):
            if self.VarList.text( i, LabelIndex['Name']).latin1() == text:
                return i

        return -1

    ## +Method: slotAddButton
    ##
    ## Handler for the Create button. Sends a message to the MsgD-RTDB asking the
    ## creation of a new variable, with the properties specified by the current
    ## state of various widgets: name, value, type. 
    ## For integer and real values, arrays of comma-separated values are accepted.
    ## Optional parenthesis () around the array (in Python tuple style) are permitted
    ## After a successful creation, two more messages are sent to the MsgD-RTDB to set
    ## the variable's value and to get it back (so the VARREPLY message is dispatched to
    ## the handler routine, that will visualize it)

    def slotAddButton(self):

        varname, vartype, own, length, values = self.GetCurrentVarParameters()
    
        status =  msglib.CreateVar( varname, vartype, own, length)
        if status < 0:
            print "Error in CreateVar(): ", status
        else:
            msglib.SetVar( varname, vartype, length, values)
            msglib.GetVar( varname)

    ## +Method: slotChangeButton
    ##
    ## Handler for the Change button. Sends a message to the MsgD-RTDB to change the value
    ## of a variable. After this, a GETVAR message is sent to display the variable's new value

    def slotChangeButton(self):

        varname, vartype, own, length, values = self.GetCurrentVarParameters()

        status = msglib.SetVar( varname, vartype, length, values)
        if status < 0:
            print "Error in SetVar(): ", status

        msglib.GetVar(varname)

    ## +Method: GetCurrentVarParameters
    ##
    ## This function gets the current variable name, value etc. from the window widgets,
    ## and returns a tuple with the following elements:
    ## (varname,    = variable name
    ##  vartype,    = variable type
    ##      own,    = type of ownership ('p' for private, 'g' for global)
    ##   length,    = length of the array (# of elements for int and real, # of characters for string)
    ##   values)    = tuple of elements (for int and real), or character string

    def GetCurrentVarParameters(self):
        varname = self.VarName.text().latin1()
        varvalue = self.VarValue.text().latin1()
        vartype = self.TypeGroup.id( self.TypeGroup.selected())
        varowner = self.OwnerGroup.id( self.OwnerGroup.selected())
        own = ('p','g')[varowner]
        length=0
        values=();

        if vartype != CHAR_VARIABLE:
            if varvalue[0] == '(':
                varvalue = varvalue[1:]
            if varvalue[-1] == ')':
                varvalue = varvalue[:-1]    

            values = string.split(varvalue,',')

            for i in range(len(values)):
                if vartype == INT_VARIABLE:
                    values[i] = int(values[i])
                if vartype == REAL_VARIABLE:
                    values[i] = float(values[i])

            length = len(values)
        else:
            values = varvalue
            length = len( values)

        result = ( varname, vartype, own, length, values)
        return result
    


    ## +Method: slotDelButton
    ##
    ## Handler for the Delete button. Not currently used, since the MsgD-RTDB does not
    ## support the VARDEL message yet.

    def slotDelButton(self):
        varname = self.VarName.text().latin1()
        msglib.DelVar(varname,None,None)

    ## +Method: slotRefreshButton
    ##
    ## Handler for the Refresh button. Clears the VarList table and sends a message to the
    ## MsgD-RTDB asking for a full variable list. The slotNotify handler routine will
    ## take care of displaying the variables.

    def slotRefreshButton(self):
        self.VarList.setNumRows(0)
        regexp = self.VarFilter.text().latin1()
        msglib.SendMessage("", AOConstants.AOMsgCode['VARLIST'], regexp)         
        self.varlisting = 1
        self.varlist= {}


    ## +Method: slotLogCheck
    ##
    ## Handler for the "Log this variable" checkbox. Activates or deactivates the loggers
    ## corresponding to any selected variable in the VarList table. The VarList information
    ## is changed to reflect the different state of the loggers.

    def slotLogCheck( self, state):
        for i in range( self.VarList.numRows()):
            if self.VarList.isRowSelected(i):
                varname = self.VarList.text( i, LabelIndex['Name']).latin1()
                self.Loggers[varname].SetActive(state)
                flag = 'N'
                if state:
                    flag = 'Y'
                self.VarList.setText( i, LabelIndex['Log?'], flag)

    ##@Method: slotDumpButton
    ##
    ## Asks for a filename and dumps all the MsgD-RTDB variables into a text file
    ##@

    def slotDumpButton(self):

        autofilename = time.strftime('dump_%Y%m%d-%H.%M.txt', time.localtime())

        filename = QFileDialog.getSaveFileName( autofilename, "", self)
        if filename.isEmpty():
            return

        filename = filename.latin1()
        self.dumpfile = file(filename, 'w')

        msglib.SendMessage("", AOConstants.AOMsgCode['VARLIST'], "*")         
        self.vardumping=1
        self.vardump={}



    ## +Method: slotLogButton
    ##
    ## Handler for the "Set log file" button. Opens a file dialog to allow the user
    ## to specify a file name for the output file. If a valid filename is entered,
    ## the interested logger is notified, the GetVar table is update with the
    ## file name and the "Log this variable" checkbox is activated

    def slotLogButton( self):
        filename = QFileDialog.getSaveFileName( QString.null, "", self)
        if filename.isEmpty():
            return

        filename = filename.latin1()

        varname = self.VarName.text().latin1()
        if not self.Loggers.has_key(varname):
            self.Loggers[varname] = Logger()

        self.Loggers[varname].SetOutputFile( filename)
        pos = self.findVarListElement( varname)
        if pos >=0:
            self.VarList.setText( pos, LabelIndex['Logfile'], filename)
            self.LogCheck.setEnabled(1)
            self.timer.start( 500, 1)

    ## +Method: slotWatchCheck
    ##
    ## Handler for the "Watch this variable" checkbox. Activates or deactivates the watch state for
    ## any selected variables in the VarList table. For each variable, a VARNOTIF or VARNONOTF is sent
    ## to the MsgD-RTDB. In case of a new watched variable, a GETVAR message is sent as well, to get
    ## the current state of the variable.
    
    def slotWatchCheck(self, state):
        for i in range( self.VarList.numRows()):
            if self.VarList.isRowSelected(i):
                varname = self.VarList.text( i, LabelIndex['Name']).latin1()
                prev_state = self.Watched[varname]
                self.Watched[varname] = state           ## Create the key setting its value
                if prev_state != state:
                    if state:
                        self.VarList.setText( i, LabelIndex['Watch?'], 'Y')
                        msglib.SendMessage("", AOConstants.AOMsgCode['VARNOTIF'], varname)
                        msglib.GetVar( varname)
                    else:
                        self.VarList.setText( i, LabelIndex['Watch?'], 'N')
                        msglib.SendMessage("", AOConstants.AOMsgCode['VARNONOTIF'], varname)
    
                
                    
    ## +Method: slotVarListClicked
    ##
    ## Handler for mouse clicks inside the VarList table. When a variable is clicked,
    ## it's selected and copied to the detail section, and a GETVAR message is sent to actually
    ## fill the detail list. The complex selection code is a workaround for the missing
    ## QTable::setFocusStyle() function, that seems to be present only in QT > 3.0.0

    def slotVarListClicked( self, row, col, button, point):
        varname = self.VarList.text( row, LabelIndex['Name']).latin1()
        self.VarList.clearSelection()
        sel = QTableSelection()             # Makes a row-wide selection
        sel.init( row,0)
        sel.expandTo( row, len(LabelIndex.keys())-1)
        self.VarList.addSelection( sel)

        self.VarName.setText( varname)
        msglib.GetVar( varname)

    ## +Method: slotVarListHeaderClicked
    ##
    ## Handler for mouse clicks on the VarList header. When a header is clicked, the table
    ## is sorted according to the chosen column. The sort order alternates between
    ## ascending and descending each time.

    def slotVarListHeaderClicked( self, col):
        self.VarList.sortColumn( col, self.sortOrder, 1)
        self.sortOrder = 1 - self.sortOrder

    ## +Method: slotIntervalsButton
    ##
    ## Handler for the "Set Intervals" button. Opens the Intervals dialog and lets the user set
    ## new alarm intervals for the variables

    def slotIntervalsButton(self):
        dlg = IntervalsDlg(self, self)
    
        if dlg.exec_loop() != QDialog.Accepted:
            return

        selected = dlg.varname
        varname, type = string.split(selected, ' (')
        type = type[:-1]

        self.Validators[ varname] = []

        for color in ('green','yellow','red'):
            intervals = string.split( dlg.intervals[color].text().latin1(), ',')

            for interval in intervals:
                bottomup = string.split( interval)
                if len(bottomup) != 2:
                    continue

                bottom, top = string.split( interval)
    
                if type == 'int':
                    self.Validators[ varname].append( QIntValidator( int(bottom), int(top), self))
                if type == 'real': 
                    self.Validators[ varname].append( QDoubleValidator( float(bottom), float(top), 8, self))
    
                self.Validators[ varname][-1].setName(color)
    
        
        msglib.GetVar( varname)
       
       
    def __format(self, value):
        if type(value) == types.TupleType:
            return '(' + ','.join([ self.__format(v) for v in value]) + ')'

        if type(value) == types.FloatType:
            return self.__formatFloat(value)
        else:
            return str(value)
        
        
    def __formatFloat(self, value, decDigits = 3):
        format = "%." + str(decDigits) + "f"
        text = str(format % float(value))
        return text

    def __formatTime(self, value):
        return time.strftime('%H:%M:%S', time.gmtime(value))



def slotNotify( fd):
    message = msglib.WaitMessage()
    app.emit(PYSIGNAL("%d" % message['code']), (message,) )

    if message['code'] == AOConstants.AOMsgCode['VARREPLY'] or message['code'] == AOConstants.AOMsgCode['VARCHANGD']:
        app.emit(PYSIGNAL("%s" % message['name']), (message,) )

    
    

if __name__ == "__main__":

    app = QApplication(sys.argv)    
    name = "VAR"

    # Connect to MsgD-RTDB server
    counter=0
    while counter <10:
        ServerFD = msglib.ConnectToServer( "%s%d" % (name, counter) )
        if ServerFD >=0:
            break
        counter = counter+1

    if counter == 10:
        errstring = msglib.GetErrorString( ServerFD)
        QMessageBox.warning( app.mainWidget(), "Adopt GUI error", "Error "+str(ServerFD)+" connecting to MsgD-RTDB server: "+errstring +"\n\n")
        exit

    y = vartool_AO(app)

    # Setup socket notification
    notify = QSocketNotifier( ServerFD, QSocketNotifier.Read, y)
    notify.setEnabled(1)
    app.connect( notify, SIGNAL("activated(int)"), slotNotify)



    app.setMainWidget(y)
    y.show()

    app.exec_loop()
    
