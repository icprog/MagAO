;!EDIT_INPUT = 500
!X.STYLE = 2+16
!Y.STYLE = 2+16
!Z.STYLE = 2+16
!P.CHARSIZE = 1.5
!EXCEPT = 2
device, RETAIN=2

;cd, CURRENT=root_dir
;root_dir = filepath(ROOT=root_dir, SUB=["..","adsec_lib"], "")
;if !version.os_family eq "Windows" then delim = ';' else delim = ':'
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB=["adsec","level1"], ""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB="adsec_comm", ""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB=["adsec","level2"], ""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB=["adsec","level3"], ""))
;!PATH = !PATH + delim + expand_path("+"+filepath(ROOT=root_dir, SUB="gui", ""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB=["gui","xdds_gui"], ""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB=["gui","xsl_comm"], ""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB="xadsec", ""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB="adsec_utilities", ""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB="adsec_scripts", ""))
;;;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB="adsec_wrapper", ""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB="adsec_wyko", ""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB=["adsec_utilities","tfl_gui_lib"], ""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB=["adsec_utilities","asma"], ""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB=["adsec_utilities","stress"], ""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB="test",""))
;!PATH = !PATH + delim + expand_path(filepath(ROOT=root_dir, SUB="calibration",""))

;delvar, root_dir

