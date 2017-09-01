Pro full_compare_flash, DOWNLOADED=downloaded, SAVEF=savef

    @adsec_common
    ;path = "/home/adopt/flash_releases/Pack-post-acceptance/"
    path = "/home/adopt/flash_releases/flao-LBTa-acceptance/"
    
    
    if keyword_set(DOWNLOADED) then begin
    file_list = {                                        $
                accel:       "AccelLogic_2_01.rbf.downloaded"              ,$
                accel_nios:  "DSPNios_4_00_0004_acc.bin.downloaded"        ,$
                siggen:      "SigGenLogic_5_00.rbf.downloaded"             ,$
                siggen_nios: "DSPNios_4_00_0002_sg.bin.downloaded"         ,$
                swb:         "BCULogic_12_22_swb.rbf.downloaded"           ,$
                swb_nios:    "BCUNios_5_00_8042_swb.bin.downloaded"        ,$
                crb:         "BCULogic_12_35_crb.rbf.downloaded"           ,$
                crb_nios:    "BCUNios_5_01_0042_sc&crb.bin.downloaded"     ,$
                dsp:         "DSPLogic_6_04.rbf.downloaded"                ,$
                dsp_nios:    "DSPNios_4_00_0001_dsp.bin.downloaded"         $
    }
    endif else begin

    file_list = {                                        $
                accel:       "AccelLogic_2_01.rbf"              ,$
                accel_nios:  "DSPNios_4_00_0004_acc.bin"        ,$
                siggen:      "SigGenLogic_5_00.rbf"             ,$
                siggen_nios: "DSPNios_4_00_0002_sg.bin"         ,$
                swb:         "BCULogic_12_22_swb.rbf"           ,$
                swb_nios:    "BCUNios_5_00_8042_swb.bin"        ,$
                crb:         "BCULogic_12_35_crb.rbf"           ,$
                crb_nios:    "BCUNios_5_01_0042_sc&crb.bin"     ,$
                dsp:         "DSPLogic_6_04.rbf"                ,$
                dsp_nios:    "DSPNios_4_00_0001_dsp.bin"         $
    }
    
    endelse
    ;ACCELEROMETERs
    for i=1,5,2 do begin

        print, FORMAT='($,"COMPARING ACCELEROMETER USER LOGIC # " ,I1 ,"   ---->   ")', i
        err = compare_flash(i,path+file_list.accel, /USER_LOGIC, /SIGGEN, SAV=keyword_set(SAVEF))

        print,FORMAT='($, "COMPARING ACCELEROMETER USER NIOS # " ,I1  , "   ---->   ")',i
        err = compare_flash(i,path+file_list.accel_nios, /USER_PROGRAM, /SIGGEN, SAV=keyword_set(SAVEF))

    endfor

    ;SIGGENs
    for i=0,4,2 do begin

        print, FORMAT='($,"COMPARING SIGGEN USER LOGIC # " ,I1  , "   ---->   ")',i
        err = compare_flash(i,path+file_list.siggen, /USER_LOGIC, /SIGGEN, SAV=keyword_set(SAVEF))

        print, FORMAT='($,"COMPARING SIGGEN USER NIOS # " ,I1  , "   ---->   ")',i
        err = compare_flash(i,path+file_list.siggen_nios, /USER_PROGRAM, /SIGGEN, SAV=keyword_set(SAVEF))

    endfor

    ;SWITCHBCU
    print, FORMAT='($,"COMPARING SWITCH BCU USER LOGIC   ---->   ")'
    err = compare_flash(i,path+file_list.swb, /USER_LOGIC, /SWI, SAV=keyword_set(SAVEF))
    
    print, FORMAT='($,"COMPARING SWITCH BCU USER NIOS    ---->   ")'
    err = compare_flash(i,path+file_list.swb_nios, /USER_PROGRAM, /SWI, SAV=keyword_set(SAVEF))

    ;CRATEBCUs
    for i=0,5 do begin

        print, FORMAT='($,"COMPARING CRATE BCU USER LOGIC # " ,I1  , "   ---->   ")'
        err = compare_flash(i,path+file_list.crb, /USER_LOGIC, /BCU, SAV=keyword_set(SAVEF))

        print, FORMAT='($,"COMPARING CRATE BCU USER NIOS # " ,I1  , "   ---->   ")'
        err = compare_flash(i,path+file_list.crb_nios, /USER_PROGRAM, /BCU, SAV=keyword_set(SAVEF))

    endfor
    
    ;DSPBOARDs
    for i=0, 83 do begin

        print, FORMAT='($,"COMPARING ALL DSP USER LOGIC  ---->   ")'
        err = compare_flash(i, path+file_list.dsp, /USER_LOGIC, SAV=keyword_set(SAVEF))
        
        print, FORMAT='($,"COMPARING ALL DSP USER NIOS   ---->   ")'
        err = compare_flash(i, path+file_list.dsp_nios, /USER_PROGRAM, SAV=keyword_set(SAVEF))

    endfor

End
