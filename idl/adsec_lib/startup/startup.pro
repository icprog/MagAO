@general_setup
@adsec_common
@ao_define
idl_error_status=0
if getenv('ADOPT_SIDE') eq 'L' then !AO_CONST.telescope_side='left'
if getenv('ADOPT_SIDE') eq 'R' then !AO_CONST.telescope_side='right'
if !version.os eq 'Win32' then !AO_CONST.unit= '672a' else !AO_CONST.unit=file_readlink(getenv('ADOPT_ROOT')+'/conf/adsec/current')
if !version.os eq 'Win32' then !AO_CONST.conf_file = 'conf\adsec\current\processConf\adsec.conf' else !AO_CONST.conf_file = 'conf/adsec/current/processConf/adsec.conf'
!AO_CONST.side_master=1B
!AO_CONST.conf_file = 'Dummy'
!AO_CONST.dll_file = 'no_dll'
!AO_CONST.msgd_link = 0B
errcode=init_adsec()
