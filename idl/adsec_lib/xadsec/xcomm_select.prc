HEADER
; IDL Visual Widget Builder Resource file. Version 1
; Generated on:	03/10/2001 23:14.14
VERSION 1
END

WID_BASE_0 BASE 5 5 268 142
ONPOSTCREATE "xcs_on_creation_root_base"
TLB
CAPTION "Communication type"
XPAD = 3
YPAD = 3
SPACE = 3
BEGIN
  COMM_TYPE_DL DROPLIST 13 21 147 34
  REALIZE "xcs_on_realize_comm_type"
  NUMITEMS = 3
  ITEM "Dummy"
  ITEM "PCIHotLink"
  ITEM "SwiftNet"
  ONSELECT "xcs_on_select_comm_type"
  END
  WID_LABEL_0 LABEL 13 3 211 24
  VALUE "Select the type of communication:"
  ALIGNLEFT
  END
  XCS_OK_BTN PUSHBUTTON 173 64 73 34
  VALUE "Ok"
  ALIGNCENTER
  ONACTIVATE "xcs_on_press_ok"
  END
END
