# 
# tree_gui is a generic class implementing  a gui to display
# the properties of a tree object.
# A tree is composed of leaf objects each having a single parent
# and 0 or more childs. A leaf must have a widget member in order to be 
# displayed by a tree_gui.
# The tree structure is displayed in a listView object on the left
# side of the dialog. The properties of the selected object are 
# displayed in the right part of the panel
# 
# History:
# 8 Jul 2005 lorenzo busoni <lbusoni@arcetri.astro.it>
#
#

from qt import *

        
class tree_gui(QDialog):
    def __init__(self, tree=None, parent = None, name = None, \
            caption=None, fl = 0):
        QDialog.__init__(self,parent,name,0,fl)

        if not name:
            self.setName("A Tree")
        if not caption:
            self.setCaption("A Tree")
         
        self.t = tree
        if not self.t:
            self.t = []

        # a layout with 2 horizontal blocks
        self.gridLayout = QGridLayout( self, 2, 1, 5, -1, 'gridLayout')
        self.gridLayout.setRowStretch( 1, 0)
        
        # add a vboxlayout to the bottom one
        self.bottomLayout  = QVBoxLayout()
        self.gridLayout.addLayout(self.bottomLayout, 1, 0)
        
        # split the remaining into 2 vertical blocks
        self.body      =  QGridLayout( self, 1, 2, 0, -1, 'body')
        self.gridLayout.addItem(self.body, 0, 0)

        # add an extension button to the bottombox
        self.ButtonView = QPushButton("Full View", self, "ButtonView")
        self.ButtonView.setToggleButton(1)
        self.bottomLayout.addWidget(self.ButtonView,0, True)
        # add a Done button to the bottombox
        self.ButtonDone = QPushButton("Done", self, "ButtonDone")
        self.bottomLayout.addWidget(self.ButtonDone,0, True)

        # create a WidgetStack for the extended view
        self.widgetStack1 = QWidgetStack(self,"widgetStack1")
         #self.body.addWidget(self.widgetStack1, 0, 1)

        # add a ListView to the left column of the body. Sorting is disabled 
        self.listView1 = QListView(self,"listView1")
        self.listView1.addColumn("")
        self.listView1.setSorting(-1)
        self.listView1.clear()
        self.body.addWidget(self.listView1, 0, 0)

        # populate listView
        for b in self.t:
            if b.parent: 
                attachto = b.parent.listViewItem
            else:
                attachto = self.listView1
            b.listViewItem = QListViewItem(attachto)
            b.listViewItem.setOpen(1)
            b.listViewItem.setText(0,b.label)
            b.widgetid = self.widgetStack1.addWidget(b.widget,-1)
        

        self.setOrientation( Qt.Horizontal)
        self.setExtension(self.widgetStack1)

        self.connect(self.ButtonView, SIGNAL("toggled(bool)"), self.TogView)

        self.connect(self.ButtonDone,SIGNAL("clicked()"),self.close)
        self.connect(self.listView1,SIGNAL("selectionChanged()"),self.giraPagina)

    def TogView(self):
        if self.ButtonView.isOn():
            self.ButtonView.setText("List View")
            self.showExtension(1)
        else:
            self.ButtonView.setText("Full View")
            self.showExtension(0)


    def giraPagina(self):
        e = self.listView1.currentItem()
        for b in self.t:
            if b.listViewItem == e:
                print b.label
                self.widgetStack1.raiseWidget(b.widget) 
        

if __name__ == "__main__":
    import sys
    app = QApplication(sys.argv)
    f = tree_gui()
    f.show()
    app.setMainWidget(f)
    app.exec_loop()

