;$Id: test_skip_frame.pro,v 1.13 2010/01/11 10:16:31 marco Exp $$
;+
; NAME:
;   TEST SKIP_FRAME
;
; PURPOSE:
;   Test routine to discover frame skipping
;
; USAGE:
;   err = test_skip_frame(bit)
;
; INPUT:
;   None.
;
; OUTPUT:
;   err: Error code.
;   bit: Bit mask [0: one mode out of ranges, 1: one force out of ranges, 2:  one command out of ranges ] x N_ACTUATORS
;
; KEYWORDS:
;   DSP: test skip frame bits even in DSP memory addresses.
;   NOVERB: un-verbose mode
;   SKIPFLAG: test an error presence. NB: it doesn't reset the error counters!!
;   OLDSKIP, OLDPEND:   input numbers
;   RESET:  remove all thresholds.
;
; HISTORY
;
;   created by Marco Xompero on 8 Feb 2007
;   Osservatorio Astrofisico di Arcetri, ITALY
;   <marco@arcetri.astro.it>
;   20 Feb 2007
;     DSP keyword added. Board lists generated fixed.
;   14 Mar 2007
;     FL checks added.
;   19 Sep 2007
;     SkipFC and PendingFC check added. RESET keyword added.
;   14 Aug 2008 A. Riccardi (AR)
;     added keywords: 
;-
Function test_skip_frame, bit, DSP=dsp, NOVERB=noverb, SKIPFLAG = skipflag, OLDSKIP=oldsafe, OLDPEND = oldpend $
                        , MIRROR_CNT=mirrc, PENDING_SKIP_FRAME_CNT=pendc, SAFE_SKIP_FRAME_CNT=safec $
                        , FL_CRC_ERR=fl_crc, FL_TIMEOUT=fl_tim, WFS_CNT = wfs_cnt, SHOWPBS=showpbs, DEBUG=debug

    @adsec_common
    
    if keyword_set(DSP) then begin
        err = read_seq_dsp(sc.all, rtr_map.start_RTR+1L, 1l, mck, /UL)
        if err ne adsec_error.ok then return, err

        mck = mck and 1
        if total(mck gt 0) ne 0 then begin
            idm = where(mck, cm)
             if ~keyword_set(NOVERB) then print, "#DSP with modes out of range: "+string(cm, FORMAT='(I3.3)')
             if ~keyword_set(NOVERB) then print, "ID: ",strtrim(idm,2)
        endif
        err = read_seq_dsp(sc.all, rtr_map.start_MM+1L, 1l, fck, /UL)
        if err ne adsec_error.ok then return, err
        
        fck = fck and 1
        if total(fck gt 0) ne 0 then begin
            idf = where(fck, cf)
             if ~keyword_set(NOVERB) then print, "#DSP with commands out of range: "+string(cf, FORMAT='(I3.3)')
             if ~keyword_set(NOVERB) then print, "ID: ",strtrim(idf,2)
        endif
        
        err = read_seq_dsp(sc.all, rtr_map.start_FF+1l, 1l, cck, /UL)
        if err ne adsec_error.ok then return, err
        
        cck = cck and 1
        if total(cck gt 0) ne 0 then begin
            idc = where(cck, cc)
            if ~keyword_set(NOVERB) then print, "#DSP with forces out of range: "+string(cc, FORMAT='(I3.3)')
            if ~keyword_set(NOVERB) then print, "ID: ",strtrim(idc,2)
        endif
        
        bit = [mck, fck, cck]
    endif
    
    err = read_seq_dsp(0, switch_map.WFSGlobalTimeout,1L , wfsc, /SWITCH, /ULONG)
    if err ne adsec_error.ok then return, err
    if ~keyword_set(NOVERB) then print, FORMAT='(%"WFSGlobalTimeout: %8.8X \(%10.10I\)")', wfsc, wfsc
    err = read_seq_dsp(0, switch_map.WFSGlobalTimeoutCnt,1L , wfsc_cnt, /SWITCH, /ULONG)
    if err ne adsec_error.ok then return, err
    if ~keyword_set(NOVERB) then print, FORMAT='(%"WFSGlobalTimeoutCnt: %8.8X \(%10.10I\)")', wfsc_cnt, wfsc_cnt
    err = read_seq_dsp(0, switch_map.MirrFramesCounter,1L , mirrc, /SWITCH, /ULONG)
    if err ne adsec_error.ok then return, err
    if ~keyword_set(NOVERB) then print, FORMAT='(%"MirrFramesCounter: %8.8X \(%10.10I\)")', mirrc, mirrc
    err = read_seq_dsp(0, switch_map.PendingSkipFrameCnt,1L , pendc, /SWITCH, /ULONG)
    if err ne adsec_error.ok then return, err
    if ~keyword_set(NOVERB) then print, FORMAT='(%"PendingSkipFrameCnt: %8.8X \(%10.10I\)")', pendc, pendc
    err = read_seq_dsp(0, switch_map.SafeSkipFrameCnt,1L , safec, /SWITCH, /ULONG)
    if err ne adsec_error.ok then return, err
    if ~keyword_set(NOVERB) then print, FORMAT='(%"SafeSkipFrameCnt: %8.8X \(%10.10I\)")', safec, safec
    err = read_seq_dsp(0, switch_map.NumFlCrcErr,1L , fl_crc, /SWITCH, /ULONG)
    if err ne adsec_error.ok then return, err
    if ~keyword_set(NOVERB) then print, FORMAT='(%"NumCrcErr: %8.8X \(%10.10I\)")', fl_crc, fl_crc
    err = read_seq_dsp(0, switch_map.NumFlTimeout,1L , fl_tim, /SWITCH, /ULONG)
    if err ne adsec_error.ok then return, err
    if ~keyword_set(NOVERB) then print, FORMAT='(%"NumFlTimeout: %8.8X \(%10.10I\)")', fl_tim, fl_tim
    err = read_seq_dsp(sc.all, rtr_map.SWITCH_SCSTARTRTR,4l,wfs_tt, /swit,/ulong)
    if err ne adsec_error.ok then return, err
    if ~keyword_set(NOVERB) then print, FORMAT='(%"WFSFramesCounter: %8.8X \(%10.10I\)")', wfs_tt[0], wfs_tt[0]
    wfs_cnt = wfs_tt[0]
    err = read_seq_dsp(sc.all, switch_map.mirrorframerate,1l,ftt, /swit,/ulong)
    if err ne adsec_error.ok then return, err
    if ~keyword_set(NOVERB) then print, FORMAT='(%"MirrFrameRate: %8.8X \(%10.10I\)")', ftt[0], ftt[0]

    if keyword_set(debug) then begin
        err = read_seq_dsp(0, 'fff0'xl, 8l, bb, /sw)
        if err ne adsec_error.ok then return, err
        if ~keyword_set(NOVERB) then print, FORMAT='(%"FFF0: %8.8X \(%10.10I\)")', bb[0], bb[0]
        if ~keyword_set(NOVERB) then print, FORMAT='(%"FFF1: %8.8X \(%10.10I\)")', bb[1], bb[1]
        if ~keyword_set(NOVERB) then print, FORMAT='(%"FFF2: %8.8X \(%10.10I\)")', bb[2], bb[2]
        if ~keyword_set(NOVERB) then print, FORMAT='(%"FFF3: %8.8X \(%10.10I\)")', bb[3], bb[3]
        if ~keyword_set(NOVERB) then print, FORMAT='(%"FFF4: %8.8X \(%10.10I\)")', bb[4], bb[4]
        if ~keyword_set(NOVERB) then print, FORMAT='(%"FFF5: %8.8X \(%10.10I\)")', bb[5], bb[5]
        if ~keyword_set(NOVERB) then print, FORMAT='(%"FFF6: %8.8X \(%10.10I\)")', bb[6], bb[6]
        if ~keyword_set(NOVERB) then print, FORMAT='(%"FFF7: %8.8X \(%10.10I\)")', bb[7], bb[7]
    endif

    
     ;wait, 5
     ;err = read_seq_dsp(0, switch_map.MirrFramesCounter,1L , mirrc2, /SWITCH, /ULONG)
     ;err = read_seq_dsp(sc.all, rtr_map.SWITCH_SCSTARTRTR,4l,wfs_tt2, /swit,/ulong)
     ;print, (mirrc2-mirrc)/5.
     ;print, (wfs_tt2[0]-wfs_tt[0])/5.
    err = read_seq_dsp(0, rtr_map.param_selector,1l,pbs, /ulong)
    if err ne adsec_error.ok then return, err
    if keyword_set(showpbs) then print, "Param Selector: "+conv2hex(pbs)
    if ~keyword_set(NOVERB) then print, FORMAT='(%"ParamSelector: %8.8X \(%10.10I\)")', pbs, pbs
    if (pbs and 2L^16) gt 0 then print, "NAN on slopes found: frame changed from WFS frame to OVS frame"

    skipflag = (pendc gt 0) or (safec gt 0) or (fl_crc gt 0) or (fl_tim gt 0)

    if n_elements(oldsafe) gt 0 then begin
        if safec ne oldsafe then begin
            message, "SAFE PROTECTION ACTIVATED. FRAME SKIPPED!!!!!", /INFO
            return, adsec_error.IDL_SAFE_SKIP_ACTIVE
        endif
    endif
    oldsafe = safec

    if n_elements(oldpend) gt 0 then begin
        if pendc ne oldpend then begin
            message, "SAFE PROTECTION ACTIVATED. FRAME SKIPPED!!!!!", /INFO
            return, adsec_error.IDL_SAFE_SKIP_ACTIVE
        endif
    endif
    oldpend = pendc

    return, adsec_error.ok
End
