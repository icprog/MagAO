# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'pupilshift.ui'
#
# Created: Sun Nov 13 11:48:59 2016
#      by: The PyQt User Interface Compiler (pyuic) 3.16
#
# WARNING! All changes made in this file will be lost!


from qt import *


class PupilShift(QDialog):
    def __init__(self,parent = None,name = None,modal = 0,fl = 0):
        QDialog.__init__(self,parent,name,modal,fl)

        if not name:
            self.setName("PupilShift")


        PupilShiftLayout = QVBoxLayout(self,11,6,"PupilShiftLayout")

        self.textLabel2 = QLabel(self,"textLabel2")
        textLabel2_font = QFont(self.textLabel2.font())
        textLabel2_font.setPointSize(25)
        textLabel2_font.setBold(1)
        self.textLabel2.setFont(textLabel2_font)
        PupilShiftLayout.addWidget(self.textLabel2)
        spacer6 = QSpacerItem(101,31,QSizePolicy.Expanding,QSizePolicy.Minimum)
        PupilShiftLayout.addItem(spacer6)

        layout32 = QHBoxLayout(None,0,6,"layout32")

        self.textLabel1 = QLabel(self,"textLabel1")
        layout32.addWidget(self.textLabel1)

        self.spinPixels = QSpinBox(self,"spinPixels")
        self.spinPixels.setMaxValue(20)
        self.spinPixels.setMinValue(1)
        layout32.addWidget(self.spinPixels)
        PupilShiftLayout.addLayout(layout32)

        layout1 = QHBoxLayout(None,0,6,"layout1")
        spacer1 = QSpacerItem(41,31,QSizePolicy.Expanding,QSizePolicy.Minimum)
        layout1.addItem(spacer1)

        self.buttonUp = QPushButton(self,"buttonUp")
        self.buttonUp.setMaximumSize(QSize(50,32767))
        layout1.addWidget(self.buttonUp)
        spacer1_2 = QSpacerItem(41,31,QSizePolicy.Expanding,QSizePolicy.Minimum)
        layout1.addItem(spacer1_2)
        PupilShiftLayout.addLayout(layout1)

        layout2 = QHBoxLayout(None,0,6,"layout2")

        self.buttonLeft = QPushButton(self,"buttonLeft")
        self.buttonLeft.setMaximumSize(QSize(50,32767))
        layout2.addWidget(self.buttonLeft)
        spacer1_2_2 = QSpacerItem(41,31,QSizePolicy.Expanding,QSizePolicy.Minimum)
        layout2.addItem(spacer1_2_2)

        self.buttonRight = QPushButton(self,"buttonRight")
        self.buttonRight.setMaximumSize(QSize(50,32767))
        layout2.addWidget(self.buttonRight)
        PupilShiftLayout.addLayout(layout2)

        layout6 = QHBoxLayout(None,0,6,"layout6")
        spacer1_2_2_2 = QSpacerItem(46,31,QSizePolicy.Expanding,QSizePolicy.Minimum)
        layout6.addItem(spacer1_2_2_2)

        self.buttonDown = QPushButton(self,"buttonDown")
        self.buttonDown.setMaximumSize(QSize(50,32767))
        layout6.addWidget(self.buttonDown)
        spacer1_2_2_3 = QSpacerItem(45,31,QSizePolicy.Expanding,QSizePolicy.Minimum)
        layout6.addItem(spacer1_2_2_3)
        PupilShiftLayout.addLayout(layout6)

        self.languageChange()

        self.resize(QSize(213,247).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.connect(self.buttonUp,SIGNAL("clicked()"),self.buttonUp_clicked)
        self.connect(self.buttonLeft,SIGNAL("clicked()"),self.buttonLeft_clicked)
        self.connect(self.buttonRight,SIGNAL("clicked()"),self.buttonRight_clicked)
        self.connect(self.buttonDown,SIGNAL("clicked()"),self.buttonDown_clicked)


    def languageChange(self):
        self.setCaption(self.__tr("Pupil shift"))
        self.textLabel2.setText(self.__tr("Pupil shift"))
        self.textLabel1.setText(self.__tr("How many pixels:"))
        self.buttonUp.setText(self.__tr("Up"))
        self.buttonLeft.setText(self.__tr("Left"))
        self.buttonRight.setText(self.__tr("Right"))
        self.buttonDown.setText(self.__tr("Down"))


    def buttonUp_clicked(self):
        print "PupilShift.buttonUp_clicked(): Not implemented yet"

    def buttonLeft_clicked(self):
        print "PupilShift.buttonLeft_clicked(): Not implemented yet"

    def buttonRight_clicked(self):
        print "PupilShift.buttonRight_clicked(): Not implemented yet"

    def buttonDown_clicked(self):
        print "PupilShift.buttonDown_clicked(): Not implemented yet"

    def __tr(self,s,c = None):
        return qApp.translate("PupilShift",s,c)
