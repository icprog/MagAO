Pro write_fast_conf_file, diag_name, adsec_save, filename

    
 ;   if (file_info(filename)).exists then file_move, filename, filename+'.backup'
  ;  openw, unit, filename, /GET     

    ;*****************************************************************************************************
    ;INITIAL COMMENTS
    ;*****************************************************************************************************
    ; "# config file for parameters settings of fastdiagn"
    ; "# Each entry must be composed of 11 fields:"
    ; "# Fields 0-2 identify a filter that matches a set of DiagnVars"
    ; "# Fields 3-10 are used to specify parameters to be set for the DiagnVars matched by the filter."
    ; "#"
    ; "# Examples of filters:"
    ; "# Matching all variables:                                   *           All     All"
    ; "# Matching all DistAve vars:                                DistAve     All     All"
    ; "# Matching all variables with index 13:                     *           13      13"
    ; "# Matching first 100 Modes:                                 Modes       All     99"
    ; "#"
    ; "# [Alarm/Warn][Min/Max] specify alarm and warning ranges. Use SI units."
    ; "#"
    ; "# RunningMeanLen specify the length in seconds of the running mean buffer over which the average value is computed."
    ; "# It defaults to 1s."
    ; "# "
    ; "# CAF specify the number of Consecutive Allowed Faults that can be detected before triggering a warning/alarm."
    ; "# This can be useful in case of faulty sensors that sometimes output a very wrong value. It defaults to 0."
    ; "#"
    ; "# Set Enabled to "disabled" if you don't want to monitor the matched variables. Defaults is "enabled"."
    ; "#"
    ; "# Set Slow to "slow" if the matched variables are heavy to be computed. The update of the "slow" variables is done in a "
    ; "# dedicated thread at a reduced rate with respect to "fast" variables. Default is "fast"   "
    ; "#"
    ; "# Entry will be applied in order, from top to bottom. "
    ; "#"
    ; "# FamilyName     From    To     AlarmMin-[m]    WarnMin-[m]    WarnMax-[m]     AlarmMax-[m]   RunningMeanLen-[s]  CAF  Enabled-Slow/Fast"

    ;*****************************************************************************************************
    ;DIAGNAPP VARIABLES
    ;*****************************************************************************************************
    err=setdiagnparam(diag_name,"DiagnApp* ", -1, -1, $
                        ALARM_MIN=0.0, WARNING_MIN=0.0, WARNING_MAX=!VALUES.F_INFINITY, ALARM_MAX=!VALUES.F_INFINITY $
                        , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
    if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err

    ;*****************************************************************************************************
    ;COUNTER VARIABLES
    ;*****************************************************************************************************
    err=setdiagnparam(diag_name,"WFSFrameCounter ", -1, -1, $
                        ALARM_MIN=0.0, WARNING_MIN=0.0, WARNING_MAX=!VALUES.F_INFINITY, ALARM_MAX=!VALUES.F_INFINITY $
                        , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
    if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
    err=setdiagnparam(diag_name,"MirrorFrameCounter ", -1, -1, $
                        ALARM_MIN=0.0, WARNING_MIN=0.0, WARNING_MAX=!VALUES.F_INFINITY, ALARM_MAX=!VALUES.F_INFINITY $
                        , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
    if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
    err=setdiagnparam(diag_name,"CommandHistoryPtr ", -1, -1, $
                        ALARM_MIN=0.0, WARNING_MIN=0.0, WARNING_MAX=!VALUES.F_INFINITY, ALARM_MAX=!VALUES.F_INFINITY $
                        , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
    if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
    
    ;*****************************************************************************************************
    ;PARAM BLOCK SELECTOR VARIABLES:
    ;each entry is a bit of ParamBlockSelector word (identical in each header/footer)
    ;*****************************************************************************************************
    err=setdiagnparam(diag_name,"ParamBlock* ", -1, -1, $
                        ALARM_MIN=-1, WARNING_MIN=-1, WARNING_MAX=2, ALARM_MAX=2 $
                        , MEAN_PERIOD=0, CONS_ALL=1, /ENA)
    if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err

    ;*****************************************************************************************************
    ;SWITCHBCU related entries 
    ;*****************************************************************************************************
    err=setdiagnparam(diag_name,"Slopes ", -1, -1, $
                        ALARM_MIN=-!VALUES.F_INFINITY, WARNING_MIN=-!VALUES.F_INFINITY, WARNING_MAX=!VALUES.F_INFINITY, ALARM_MAX=!VALUES.F_INFINITY $
                        , MEAN_PERIOD=0.1, CONS_ALL=0, /ENA)
    if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
    err=setdiagnparam(diag_name,"Switch*", -1, -1, $
                        ALARM_MIN=0.0, WARNING_MIN=0.0, WARNING_MAX=!VALUES.F_INFINITY, ALARM_MAX=!VALUES.F_INFINITY $
                        , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
    if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err

    ;*****************************************************************************************************
    ;CRATEBCU related entries 
    ;*****************************************************************************************************
    for i=0, n_elements(adsec_save.true_act)-1 do begin
        act_i = (indgen(672))[adsec_save.true_act[i]]

        err=setdiagnparam(diag_name,"ChDistAverage", act_i, act_i, $
                          ALARM_MIN=adsec_save.min_lin_dist[act_i], WARNING_MIN=adsec_save.min_lin_dist[act_i]+1e-6 $
                          , WARNING_MAX=125e-6, ALARM_MAX=135e-6 $
                          , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
        if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
        err=setdiagnparam(diag_name,"ChCurrAverage", act_i, act_i, $
                          ALARM_MIN=-0.79, WARNING_MIN=-0.7, WARNING_MAX=0.75, ALARM_MAX=0.79 $
                          , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
        if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
        err=setdiagnparam(diag_name,"ChNewDeltaCommand", act_i, act_i, $
                          ALARM_MIN=-0.69, WARNING_MIN=-0.3, WARNING_MAX=0.3, ALARM_MAX=0.69 $
                          , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
        if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
        err=setdiagnparam(diag_name,"ChFFCommand", act_i,act_i, $
                          ALARM_MIN=-0.69, WARNING_MIN=-0.3, WARNING_MAX=0.3, ALARM_MAX=0.69 $
                          , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
        if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
        err=setdiagnparam(diag_name,"ChIntControlCurrent", act_i, act_i, $
                          ALARM_MIN=-0.69, WARNING_MIN=-0.3, WARNING_MAX=0.3, ALARM_MAX=0.69 $
                          , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
        if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
        err=setdiagnparam(diag_name,"ChFFPureCurrent", act_i, act_i, $
                          ALARM_MIN=-0.69, WARNING_MIN=-0.3, WARNING_MAX=0.3, ALARM_MAX=0.69 $
                          , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
        if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
        err=setdiagnparam(diag_name,"Modes", act_i, act_i, $
                        ALARM_MIN=-1e-3, WARNING_MIN=-1e-3, WARNING_MAX=1e-3, ALARM_MAX=1e-3 $
                          , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
        if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
        err=setdiagnparam(diag_name,"ChDistRMS", act_i, act_i, $
                          ALARM_MIN=-2e-14, WARNING_MIN=-2e-14, WARNING_MAX=30e-9, ALARM_MAX=100e-9 $
                          , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
        if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
        err=setdiagnparam(diag_name,"ChCurrRMS ", act_i, act_i, $
                          ALARM_MIN=-2e-14, WARNING_MIN=-2e-14, WARNING_MAX=0.5, ALARM_MAX=0.5 $
                          , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
        if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
    endfor

    ;*****************************************************************************************************
    ;Particular settings
    ;*****************************************************************************************************
    ;PARAM SELECTOR
    err=setdiagnparam(diag_name,"ParamBlockSlopeLinearizeMethod", -1, -1, $
                        ALARM_MIN=-1, WARNING_MIN=-1, WARNING_MAX=8, ALARM_MAX=8 $
                        , MEAN_PERIOD=0, CONS_ALL=1, /ENA)
    if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err

    ;SWITCH BCU
    err=setdiagnparam(diag_name,"SwitchAccelerometerCoeff", -1, -1, $
                        ALARM_MIN=-!VALUES.F_INFINITY, WARNING_MIN=-!VALUES.F_INFINITY, WARNING_MAX=!VALUES.F_INFINITY, ALARM_MAX=!VALUES.F_INFINITY $
                        , MEAN_PERIOD=0.1, CONS_ALL=0, /ENA)
    if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err

    ;ACTUATORS WITH CAPACITIVE SENSOR NOT PROPERLY WORKING
    if adsec_save.act_wo_pos[0] gt 0 then begin
        for i=0, n_elements(adsec_save.act_wo_pos)-1 do begin
            act_i = adsec_save.act_wo_pos[i]
            err=setdiagnparam(diag_name,"ChDistAverage", act_i, act_i, ENA=0)
            if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
            err=setdiagnparam(diag_name,"ChDistRMS", act_i, act_i, ENA=0)
            if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
        endfor
    endif
    ;ACTUATORS WITH CURRENT DRIVER NOT PROPERLY WORKING
    if adsec_save.act_wo_curr[0] gt 0 then begin
        for i=0, n_elements(adsec_save.act_wo_curr)-1 do begin
            act_i = adsec_save.act_wo_curr[i]
            err=setdiagnparam(diag_name,"ChCurrAverage", act_i, act_i, ENA=0)
            if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
            err=setdiagnparam(diag_name,"ChCurrRMS", act_i, act_i, ENA=0)
            if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
        endfor
    endif

    return
            

End


Pro write_slow_conf_file, diag_name, adsec_save, filename

    
    if (file_info(filename)).exists then file_move, filename, filename+'.backup'
    openw, unit, filename, /GET     

    ;*****************************************************************************************************
    ;INITIAL COMMENTS
    ;*****************************************************************************************************
    ; "# config file for parameters settings of fastdiagn"
    ; "# Each entry must be composed of 11 fields:"
    ; "# Fields 0-2 identify a filter that matches a set of DiagnVars"
    ; "# Fields 3-10 are used to specify parameters to be set for the DiagnVars matched by the filter."
    ; "#"
    ; "# Examples of filters:"
    ; "# Matching all variables:                                   *           All     All"
    ; "# Matching all DistAve vars:                                DistAve     All     All"
    ; "# Matching all variables with index 13:                     *           13      13"
    ; "# Matching first 100 Modes:                                 Modes       All     99"
    ; "#"
    ; "# [Alarm/Warn][Min/Max] specify alarm and warning ranges. Use SI units."
    ; "#"
    ; "# RunningMeanLen specify the length in seconds of the running mean buffer over which the average value is computed."
    ; "# It defaults to 1s."
    ; "# "
    ; "# CAF specify the number of Consecutive Allowed Faults that can be detected before triggering a warning/alarm."
    ; "# This can be useful in case of faulty sensors that sometimes output a very wrong value. It defaults to 0."
    ; "#"
    ; "# Set Enabled to "disabled" if you don't want to monitor the matched variables. Defaults is "enabled"."
    ; "#"
    ; "# Set Slow to "slow" if the matched variables are heavy to be computed. The update of the "slow" variables is done in a "
    ; "# dedicated thread at a reduced rate with respect to "fast" variables. Default is "fast"   "
    ; "#"
    ; "# Entry will be applied in order, from top to bottom. "
    ; "#"
    ; "# FamilyName     From    To     AlarmMin-[m]    WarnMin-[m]    WarnMax-[m]     AlarmMax-[m]   RunningMeanLen-[s]  CAF  Enabled-Slow/Fast"

    ;*****************************************************************************************************
    ;DIAGNAPP VARIABLES
    ;*****************************************************************************************************
    err=setdiagnparam(diag_name,"DiagnApp* ", -1, -1, $
                        ALARM_MIN=0.0, WARNING_MIN=0.0, WARNING_MAX=!VALUES.F_INFINITY, ALARM_MAX=!VALUES.F_INFINITY $
                        , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
    if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err

    ;*****************************************************************************************************
    ;BCU NIOS FIXED AREA
    ;*****************************************************************************************************
    err=setdiagnparam(diag_name,"DiagnApp* ", -1, -1, $
                        ALARM_MIN=0.0, WARNING_MIN=0.0, WARNING_MAX=!VALUES.F_INFINITY, ALARM_MAX=!VALUES.F_INFINITY $
                        , MEAN_PERIOD=0, CONS_ALL=0, /ENA)
    if err lt 0 then log_message, "ERROR during configuration of: "+diag_name, ERR=err
    ;BCULocalCurrentThreshold             All     All     0.0         0.0         65        70        1e-4            0    ena     fast
    ;BCUVPSet                             All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;BCUTotalCurrentThresholdPos          All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;BCUTotalCurrentThresholdNeg          All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;BCUCrateID                           All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;BCUWhoAmI                            All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;BCUSoftwareRelease                   All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;BCULogicRelease                      All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;BCUIPAddress                         All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;BCUFramesCounter                     All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;BCUSerialNumber                      All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;BCUPowerBackplaneSerial              All     All     0.0         0.0         17        17        1e-4            0    ena     fast
    ;BCUEnableMasterDiagnostic            All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;BCUDecimationFactor                  All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;BCURemoteIPAddress                   All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;*****************************************************************************************************
    ;SWITCH NIOS FIXED AREA
    ;*****************************************************************************************************
;# Switch nios fixed area
  ;SwitchCrateID                        All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
  ;SwitchWhoAmI                         All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
  ;SwitchSoftwareRelease                All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
  ;SwitchLogicRelease                   All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
  ;SwitchIPAddress                      All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
  ;SwitchFramesCounter                  All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
  ;SwitchSerialNumber                   All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
  ;#SwitchPowerBackplaneSerial           All     All     0.0         0.0         17          17          1e-4            0    ena     fast
  ;SwitchEnableMasterDiagnostic         All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
  ;SwitchDecimationFactor               All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
  ;SwitchRemoteIPAddress                All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
    ;*****************************************************************************************************
    ;SWITCH NIOS FIXED AREA
    ;*****************************************************************************************************
;# DSP nios fixed area
;  DSPWhoAmI                            All     All     0.0         0.0         255       255       1e-4            0    ena     fast
;  DSPSoftwareRelease                   All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
;  DSPLogicRelease                      All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
;  DSPSerialNumber                      All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
;;  DSPDiagnosticRecordPtr               All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
;  DSPDiagnosticRecordLen               All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
;  DSPRdDiagnosticRecordPtr             All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
;;  DSPWrDiagnosticRecordPtr             All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
;  DSPADCspiOffset                      All     All     -Inf        -Inf        Inf       Inf       1e-4            0    ena     fast
;  DSPADCspiGain                        All     All     -Inf        -Inf        Inf       Inf       1e-4            0    ena     fast
;  DSPADCOffset                         All     All     -Inf        -Inf        Inf       Inf       1e-4            0    ena     fast
;  DSPADCGain                           All     All     -Inf        -Inf        Inf       Inf       1e-4            0    ena     fast
;  DSPDACOffset                         All     All     -Inf        -Inf        Inf       Inf       1e-4            0    ena     fast
;  DSPDACGain                           All     All     -Inf        -Inf        Inf       Inf       1e-4            0    ena     fast
;    ;*****************************************************************************************************
    ;SWITCH NIOS FIXED AREA
    ;*****************************************************************************************************
;# SGN nios fixed area
;  SGNWhoAmI                            All     All     0.0         0.0         255       255       1e-4            0    ena     fast
;  SGNSoftwareRelease                   All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
;  SGNLogicRelease                      All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast
;  SGNSerialNumber                      All     All     0.0         0.0         Inf       Inf       1e-4            0    ena     fast

    ;*****************************************************************************************************
    ;SWITCH NIOS FIXED AREA
    ;*****************************************************************************************************
;# SGN related vars ( 1 record per SGN, 1 SGN per CRATE = 6)
;  SGNStratixTemp                       All     All     -22         20          52          70          1e-4               1    ena     fast
;  SGNPowerTemp                         All     All     -22          20          52          70          1e-4               1    ena     fast
;  SGNDspsTemp                          All     All     -22          20          52          70          1e-4               1    ena     fast
;
;    ;*****************************************************************************************************
;    ;SWITCH NIOS FIXED AREA
    ;*****************************************************************************************************
;# DSP related vars ( 1 record per DSB, 14 DSB per CRATE = 84)
;  DSPStratixTemp                       All     All     -22         20          52          70          1e-4               1    ena     fast
;  DSPPowerTemp                         All     All     -22          20          60          70          1e-4               1    ena     fast
;  DSPDspsTemp                          All     All     -22         20          50          70          1e-4               1    ena     fast
;  DSPDriverTemp                        All     All     -22          20          60          80          1e-4               1    ena     fast
;    ;*****************************************************************************************************
;    ;SWITCH NIOS FIXED AREA
;    ;*****************************************************************************************************
;# DSP related vars ( 8 record per DSB, 14 DSB per CRATE = 672)
;  DSPCoilCurrent                       All     All     -3          -2.5        2.5         3           1e-4               1    ena     fast
;    ;*****************************************************************************************************
    ;SWITCH NIOS FIXED AREA
;    ;*****************************************************************************************************
;# SWITCH related vars ( 1 record per BCU, 1 CRATE = 1)
;  SwitchStratixTemp                    All     All     -22         -10         60          80          1e-4               1    ena     fast
;  SwitchPowerTemp                      All     All     -22         -10         50          60          1e-4               1    ena     fast
;    ;*****************************************************************************************************
;    ;SWITCH NIOS FIXED AREA
;    ;*****************************************************************************************************
;;# BCU related vars ( 1 record per BCU, 6 CRATES = 6)
;  BCUStratixTemp                       All     All     -22         -10         52          72          1e-4               3    ena     fast
;  BCUPowerTemp                         All     All     -22         -10         50          60          1e-4               3    ena     fast
;  BCUVoltageVCCL                       All     All     2.7         2.9         5           5.5         1e-4               1    ena     fast
;  BCUVoltageVCCA                       All     All     11          11.5        14.5        15          1e-4               1    ena     fast
;  BCUVoltageVSSA                       All     All    -15         -14.5       -11.5       -11          1e-4               1    ena     fast
;  BCUVoltageVCCP                       All     All     10          10.5        13          13.5        1e-4               1    ena     fast
;  BCUVoltageVSSP                       All     All    -13.5       -13         -10.5       -10          1e-4               1    ena     fast
;  BCUCurrentVCCL                       All     All    -1.0        -1.0         40          40          1e-4               1    ena     fast
;  BCUCurrentVCCA                       All     All    -1.0        -1.0         6           6           1e-4               1    ena     fast
;  BCUCurrentVSSA                       All     All    -6          -6           1           1           1e-4               1    ena     fast
;  BCUCurrentVCCP                       All     All    -5          -4           15          18          1e-4               1    ena     fast
;  BCUCurrentVSSP                       All     All    -15         -12          1           1           1e-4               1    ena     fast
;    ;*****************************************************************************************************
;;    ;SWITCH NIOS FIXED AREA
;;    ;*****************************************************************************************************
;;# Environment   ( 1 record per BCU, 6 CRATES = 6)
;;  BCUCoolerIn0Temp                     All     All     -25          -15          40          80        1e-4            1    ena     fast 
;;  BCUCoolerIn1Temp                     All     All     -25          -15          30          80        1e-4            1    ena     fast
;;  BCUCoolerOut0Temp                    All     All     -25          -15          inf         inf       1e-4            1    ena     fast
;;  BCUCoolerOut1Temp                    All     All     -25          -15          inf         inf       1e-4            1    ena     fast
;;  BCUCoolerPressure                    All     All     0           0           10          50          1e-4            1    dis     fast
;;  BCUHumidity                          All     All     0           0           80          95          1e-4            1    ena     fast
;;    ;*****************************************************************************************************
;;    ;SWITCH NIOS FIXED AREA
;;    ;*****************************************************************************************************
;;# This are only for MasterBCU ( 1 record per BCU, 1 Crate MasterBCU = 1)
;;  TotalCurrentVCCP                     All     All    -100          -100           100         100         1e-4               1    ena     fast
;;  #TotalCurrentVSSP                     All     All    -50           -50            1           1           1e-4               1    ena     fast
;;  TotalCurrentVSSP                     All     All    -150           -50            1           1           1e-4               1    ena     fast
;;  TotalCurrentVP                       All     All    -150          -150           150         150         1e-4               1    ena     fast
;;
;    ;*****************************************************************************************************
;    ;SWITCH NIOS FIXED AREA
;    ;*****************************************************************************************************
;# each entry here is a bit of BCU bck_digitalIO  ( 1 record per BCU, 6 CRATES = 6)
;  BCUDigitalIODriverEnabled            All     All    -1          -1           2           2          1e-4            1    ena     fast
;  BCUDigitalIOCrateIsMaster            All     All    -1          -1           2           2          1e-4            1    ena     fast
;  BCUDigitalIOBusSysFault              All     All    -1          -1           1           1          1e-4            1    ena     fast
;  BCUDigitalIOVccFault                 All     All    -1          -1           1           1          1e-4            1    ena     fast
;  BCUDigitalIOACPowerFault0            All     All    -1          -1           1           1          1e-4            1    ena     fast
;  BCUDigitalIOACPowerFault1            All     All    -1          -1           1           1          1e-4            1    ena     fast
;  BCUDigitalIOACPowerFault2            All     All    -1          -1           1           1          1e-4            1    ena     fast
;  BCUDigitalIOICDisconnected           All     All    -1          -1           2           2          1e-4            1    ena     fast
;  BCUDigitalIOOvercurrent              All     All    -1          -1           1           1          1e-4            1    ena     fast
;  BCUDigitalIOCrateID                  All     All    -1          -1           100         100        1e-4            1    ena     fast
;
;    ;*****************************************************************************************************
;    ;SWITCH NIOS FIXED AREA
;    ;*****************************************************************************************************
;# each entry here is a bit of BCU reset_status word ( 1 record per BCU, 6 CRATE = 6)
;  BCUResetStatusFPGAReset              All     All    -1          -1           2           2          1e-4            1    ena     fast
;  BCUResetStatusBUSReset               All     All    -1          -1           2           2          1e-4            1    ena     fast
;  BCUResetStatusDSPReset               All     All    -1          -1           2           2          1e-4            1    ena     fast
;  BCUResetStatusFLASHReset             All     All    -1          -1           2           2          1e-4            1    ena     fast
;  BCUResetStatusPCIReset               All     All    -1          -1           2           2          1e-4            1    ena     fast
;  BCUResetStatusBusDriverEnable        All     All    -1          -1           2           2          1e-4            1    ena     fast
;  BCUResetStatusBusDriverEnableStatus  All     All    -1          -1           2           2          1e-4            1    ena     fast
;  BCUResetStatusBusPowerFaultCtrl      All     All    -1          -1           2           2          1e-4            1    ena     fast
;  BCUResetStatusBusPowerFault          All     All    -1          -1           1           1          1e-4            1    ena     fast
;  BCUResetStatusSystemWatchdog         All     All    -1          -1           1           1          1e-4            1    ena     fast
;
;    ;*****************************************************************************************************
;    ;SWITCH NIOS FIXED AREA
;    ;*****************************************************************************************************
;# each entry here is a bit of the DSP driver_status word (  1 record per DSB, 14 DSB per CRATE = 84)
;  DSPDriverStatusFPGAReset             All     All    -1          -1           2           2          1e-4            1    ena     fast
;  DSPDriverStatusDSP0Reset             All     All    -1          -1           2           2          1e-4            1    ena     fast
;  DSPDriverStatusDSP1Reset             All     All    -1          -1           2           2          1e-4            1    ena     fast
;  DSPDriverStatusFLASHReset            All     All    -1          -1           2           2          1e-4            1    ena     fast
;  DSPDriverStatusBusPowerFaultCtrl     All     All    -1          -1           2           2          1e-4            1    ena     fast
;  DSPDriverStatusBusPowerFault         All     All    -1          -1           2           2          1e-4            1    ena     fast
;  DSPDriverStatusBusDriverEnable       All     All    -1          -1           2           2          1e-4            1    ena     fast
;  DSPDriverStatusEnableDSPWatchdog     All     All    -1          -1           2           2          1e-4            1    ena     fast
;  DSPDriverStatusDSP0WatchdogExpired   All     All    -1          -1           2           2          1e-4            1    ena     fast
;  DSPDriverStatusDSP1WatchdogExpired   All     All    -1          -1           2           2          1e-4            1    ena     fast
;    ;*****************************************************************************************************
;    ;SWITCH NIOS FIXED AREA
;    ;*****************************************************************************************************
;# Driver Enable bits are 672 
;  DSPDriverStatusDriverEnable          All     All    -1          -1           2           2          1e-4            1    ena     fast
;#
;# 
;    ;*****************************************************************************************************
;    ;SWITCH NIOS FIXED AREA
;    ;*****************************************************************************************************
;  ExternalTemperature                  All     All    -150        -150         100         100        1e-4            1    ena     fast
;  ExternalHumidity                     All     All    -10         -10          110         110        1e-4            1    ena     fast
;  DewPoint                             All     All    -273        -273         110         110        1e-4            1    ena     fast
;  WaterColdplateInlet                  All     All    -18         -0.1         inf         inf         1e-4            1    ena     fast
;  WaterColdplateOutlet                  All     All    -18         -0.1        inf         inf         1e-4            1    ena     fast
;  WaterMainInlet                       All     All    -18         -0.1         80          85         1e-4            2    ena     fast
;;  WaterMainOutlet                      All     All    -18         -0.1         inf         inf         1e-4            2    ena     fast
;  CheckDewPoint                        All     All    -1          2            100         100        1e-4            1    ena     fast
;  FluxRateIn                           All    All     5           5           20          20          1e-4           20    ena     fast
;
;
;# Variables that corresponds to broken sensors
    ;*****************************************************************************************************
    ;SWITCH NIOS FIXED AREA
    ;*****************************************************************************************************
;  #during fastlink test has not real value
;  DSPStratixTemp                       All     All     -22          20          52          70          5               10    ena     fast 
;  #the flux-meter need more time during startup
;;  BCUHumidity                          1     1     3           3           21          21          100           20    ena     fast
;  BCUHumidity                          2     2     -10          0           101         101          100           20    ena     fast
;  BCUHumidity                          5     5     -10          0           101         101          100           20    ena     fast
;
;  #SGNStratixTemp                       2       2       18          20          52          60          5               1    dis     fast
;    ;*****************************************************************************************************
;    ;SWITCH NIOS FIXED AREA
;    ;*****************************************************************************************************
;  DSPDriverTemp                        68      68      -inf         -inf          inf          inf         5               1    dis    fast
;  SGNDspsTemp                          0     0     18          20          52          70          5               1    dis     fast
;  SGNDspsTemp                          2     2     18          20          52          70          5               1    dis     fast
;  SGNDspsTemp                          4     4     18          20          52          70          5               1    dis     fast
;#DISABLE
;  BCUHumidity                          0     0     -Inf        -Inf        Inf         Inf          1e-4            1    dis     fast
;  BCUHumidity                          3     4     -Inf        -Inf        Inf         Inf          1e-4            1    dis     fast
 
End 

Pro config_diagnostics

    @adsec_common
    diag_name = process_list.fastdiagn.msgd_name
    adopt_root = getenv("ADOPT_ROOT")
    side = strlowcase(!AO_CONST.telescope_side)
    if !version.os_family eq "Windows" then delim = '\' else delim = '/'
    filename = adopt_root+delim+"conf"+delim+side+delim+'fastdiagn'+delim+"fastdiagn.param"
    adsec_save = adsec
    case diag_name of 
        "fastdiagn.L" : write_fast_conf_file, diag_name, adsec_save, filename
;        "HOUSEKPR00": write_slow_conf_file, adsec_save, filename
        else: log_message, "ERROR: No valid diagnostics selected.", ERR=adsec_error.input_type
    endcase

    return

End
