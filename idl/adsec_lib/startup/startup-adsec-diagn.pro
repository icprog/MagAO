@general_setup
@adsec_common
@ao_define
idl_error_status=0
if getenv('ADOPT_SIDE') eq 'L' then !AO_CONST.telescope_side='left'
if getenv('ADOPT_SIDE') eq 'R' then !AO_CONST.telescope_side='right'
if !version.os eq 'Win32' then !AO_CONST.unit= '672a' else !AO_CONST.unit=file_readlink(getenv('ADOPT_ROOT')+'/conf/adsec/current')
if !version.os eq 'Win32' then !AO_CONST.conf_file = 'conf\adsec\current\processConf\adsec.conf' else !AO_CONST.conf_file = 'conf/adsec/current/processConf/adsec_diagn.conf'

!AO_CONST.side_master=0B
!AO_CONST.dll_file = 'idl_wraplib.so'
err = idl_setup_wrap(!AO_CONST.conf_file)
if err ne 0 then !AO_CONST.msgd_link = 0B else !AO_CONST.msgd_link = 1B
tmp = idl_log_wrap()
!AO_STATUS.FSM_STATE  = 'PowerOff'
!AO_CONST.LOG_LEV_ERROR = tmp.LOG_LEV_ERROR  
!AO_CONST.LOG_LEV_FATAL = tmp.LOG_LEV_FATAL  
!AO_CONST.LOG_LEV_DEBUG = tmp.LOG_LEV_DEBUG  
!AO_CONST.LOG_LEV_WARNING = tmp.LOG_LEV_WARNING  
!AO_CONST.LOG_LEV_INFO  = tmp.LOG_LEV_INFO   
!AO_CONST.LOG_LEV_TRACE = tmp.LOG_LEV_TRACE  
!AO_CONST.LOG_LEV_DISABLE = tmp.LOG_LEV_DISABLE  
errcode=init_adsec()
