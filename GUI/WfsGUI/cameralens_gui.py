# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'QtDesigner/cameralens_gui.ui'
#
# Created: Wed Jan 23 12:24:53 2008
#      by: The PyQt User Interface Compiler (pyuic) 3.13
#
# WARNING! All changes made in this file will be lost!


from qt import *


class CameraLensGui(QDialog):
    def __init__(self,parent = None,name = None,modal = 0,fl = 0):
        QDialog.__init__(self,parent,name,modal,fl)

        if not name:
            self.setName("CameraLensGui")

        self.setMinimumSize(QSize(380,230))
        self.setMaximumSize(QSize(380,230))

        CameraLensGuiLayout = QVBoxLayout(self,11,6,"CameraLensGuiLayout")

        self.statusBox = QGroupBox(self,"statusBox")
        self.statusBox.setSizePolicy(QSizePolicy( QSizePolicy.Expanding,QSizePolicy.Preferred,0,0,self.statusBox.sizePolicy().hasHeightForWidth()))
        self.statusBox.setColumnLayout(0,Qt.Vertical)
        self.statusBox.layout().setSpacing(6)
        self.statusBox.layout().setMargin(11)
        statusBoxLayout = QHBoxLayout(self.statusBox.layout())
        statusBoxLayout.setAlignment(Qt.AlignTop)

        self.ctrlStatusLabel = QLabel(self.statusBox,"ctrlStatusLabel")
        self.ctrlStatusLabel.setSizePolicy(QSizePolicy( QSizePolicy.Expanding,QSizePolicy.Preferred,0,0,self.ctrlStatusLabel.sizePolicy().hasHeightForWidth()))
        statusBoxLayout.addWidget(self.ctrlStatusLabel)

        #self.pushButton1 = QPushButton(self.statusBox,"status picture")
        #statusBoxLayout.addWidget(self.pushButton1)
        CameraLensGuiLayout.addWidget(self.statusBox)

        self.posXBox = QGroupBox(self,"posXBox")
        self.posXBox.setSizePolicy(QSizePolicy( QSizePolicy.Expanding,QSizePolicy.Preferred,0,0,self.posXBox.sizePolicy().hasHeightForWidth()))
        self.posXBox.setColumnLayout(0,Qt.Vertical)
        self.posXBox.layout().setSpacing(6)
        self.posXBox.layout().setMargin(11)
        posXBoxLayout = QHBoxLayout(self.posXBox.layout())
        posXBoxLayout.setAlignment(Qt.AlignTop)

        self.posXreqEdit = QLineEdit(self.posXBox,"posXreqEdit")
        self.posXreqEdit.setSizePolicy(QSizePolicy(QSizePolicy.Fixed,QSizePolicy.Fixed,0,0,self.posXreqEdit.sizePolicy().hasHeightForWidth()))
        self.posXreqEdit.setMaximumSize(QSize(50,32767))
        posXBoxLayout.addWidget(self.posXreqEdit)

        self.posXcurLabel = QLabel(self.posXBox,"posXcurLabel")
        self.posXcurLabel.setMinimumSize(QSize(60,0))
        posXBoxLayout.addWidget(self.posXcurLabel)

        self.posXunit = QLabel(self.posXBox,"posXunit")
        posXBoxLayout.addWidget(self.posXunit)

        self.sliderPosX = QSlider(self.posXBox,"sliderPosX")
        self.sliderPosX.setSizePolicy(QSizePolicy( QSizePolicy.Expanding,QSizePolicy.Fixed,0,0,self.sliderPosX.sizePolicy().hasHeightForWidth()))
        self.sliderPosX.setMinimumSize(QSize(100,0))
        self.sliderPosX.setMinValue(0)
        self.sliderPosX.setMaxValue(120)
        self.sliderPosX.setLineStep(1)
        self.sliderPosX.setPageStep(10)
        self.sliderPosX.setValue(0)
        self.sliderPosX.setOrientation(QSlider.Horizontal)
        posXBoxLayout.addWidget(self.sliderPosX)
        CameraLensGuiLayout.addWidget(self.posXBox)

        self.posYBox = QGroupBox(self,"posYBox")
        self.posYBox.setSizePolicy(QSizePolicy( QSizePolicy.Expanding,QSizePolicy.Preferred,0,0,self.posYBox.sizePolicy().hasHeightForWidth()))
        self.posYBox.setColumnLayout(0,Qt.Vertical)
        self.posYBox.layout().setSpacing(6)
        self.posYBox.layout().setMargin(11)
        posYBoxLayout = QHBoxLayout(self.posYBox.layout())
        posYBoxLayout.setAlignment(Qt.AlignTop)

        self.posYreqEdit = QLineEdit(self.posYBox,"posYreqEdit")
        self.posYreqEdit.setSizePolicy(QSizePolicy(QSizePolicy.Fixed,QSizePolicy.Fixed,0,0,self.posYreqEdit.sizePolicy().hasHeightForWidth()))
        self.posYreqEdit.setMaximumSize(QSize(50,32767))
        posYBoxLayout.addWidget(self.posYreqEdit)

        self.posYcurLabel = QLabel(self.posYBox,"posYcurLabel")
        self.posYcurLabel.setMinimumSize(QSize(60,0))
        posYBoxLayout.addWidget(self.posYcurLabel)

        self.posYunit = QLabel(self.posYBox,"posYunit")
        posYBoxLayout.addWidget(self.posYunit)

        self.sliderPosY = QSlider(self.posYBox,"sliderPosY")
        self.sliderPosY.setSizePolicy(QSizePolicy( QSizePolicy.Expanding,QSizePolicy.Fixed,0,0,self.sliderPosY.sizePolicy().hasHeightForWidth()))
        self.sliderPosY.setMinimumSize(QSize(100,0))
        self.sliderPosY.setMinValue(0)
        self.sliderPosY.setMaxValue(120)
        self.sliderPosY.setValue(0)
        self.sliderPosY.setOrientation(QSlider.Horizontal)
        posYBoxLayout.addWidget(self.sliderPosY)
        CameraLensGuiLayout.addWidget(self.posYBox)

        self.languageChange()

        self.resize(QSize(380,230).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.connect(self.posXreqEdit,SIGNAL("returnPressed()"),self.posXreqEdit_returnPressed)
        self.connect(self.posYreqEdit,SIGNAL("returnPressed()"),self.posYreqEdit_returnPressed)
        self.connect(self.sliderPosX,SIGNAL("sliderMoved(int)"),self.sliderPosX_sliderMoved)
        self.connect(self.sliderPosY,SIGNAL("sliderMoved(int)"),self.sliderPosY_sliderMoved)


    def languageChange(self):
        self.setCaption(self.__tr("CameraLensCtrl"))
        self.statusBox.setTitle(self.__tr("Status"))
        self.ctrlStatusLabel.setText(self.__tr("unknown"))
        #self.pushButton1.setText(self.__tr("pushButton1"))
        self.posXBox.setTitle(self.__tr("Position Y"))
        self.posXreqEdit.setText(QString.null)
        self.posXcurLabel.setText(self.__tr("unknown"))
        self.posXunit.setText(self.__tr("um"))
        self.posYBox.setTitle(self.__tr("Position X"))
        self.posYreqEdit.setText(QString.null)
        self.posYcurLabel.setText(self.__tr("unknown"))
        self.posYunit.setText(self.__tr("um"))


    def posXreqEdit_returnPressed(self):
        print "CameraLensGui.posXreqEdit_returnPressed(): Not implemented yet"

    def posYreqEdit_returnPressed(self):
        print "CameraLensGui.posYreqEdit_returnPressed(): Not implemented yet"

    def sliderPosX_sliderMoved(self,a0):
        print "CameraLensGui.sliderPosX_sliderMoved(int): Not implemented yet"

    def sliderPosY_sliderMoved(self,a0):
        print "CameraLensGui.sliderPosY_sliderMoved(int): Not implemented yet"

    def __tr(self,s,c = None):
        return qApp.translate("CameraLensGui",s,c)
