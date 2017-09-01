#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'form1.ui'
#
# Created: Sat Apr 25 23:00:55 2009
#      by: The PyQt User Interface Compiler (pyuic) 3.16
#
# WARNING! All changes made in this file will be lost!


import sys
from qt import *
from AdOpt.widgets import LogWidget


class Form1(QDialog):
    def __init__(self,parent = None,name = None,modal = 0,fl = 0):
        QDialog.__init__(self,parent,name,modal,fl)

        self.textLabel1 = QLabel(self,"textLabel1")
        self.textLabel1.setGeometry(QRect(40,20,100,20))

        self.log = LogWidget.LogWidget( self, 'Log')
        self.log.setGeometry(QRect(50,50,700,300))

        self.setCaption("W restart")
        self.textLabel1.setText("Please wait...")

        self.resize(QSize(800,400).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.log.setFD( sys.stdin.fileno())

if __name__ == "__main__":
    a = QApplication(sys.argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
    w = Form1()
    a.setMainWidget(w)
    w.show()
    a.exec_loop()
