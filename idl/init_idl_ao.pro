
print, 'Running the init IDL AO file...'
print, 'Setting the AO library paths...'

if !version.os_family eq "Windows" then delim = ';' else delim = ':'
root_dir = GETENV('ADOPT_ROOT')
!PATH = EXPAND_PATH('<IDL_DEFAULT_PATH>')+delim+EXPAND_PATH('+'+filepath(ROOT=root_dir, SUB=['idl'],'')   )
delvar, root_dir


device, retain=2
print, 'DLM path is : ', !DLM_PATH
print, '...user dlm paths set.'


