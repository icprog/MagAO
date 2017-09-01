; $Id: analize_cloop.pro,v 1.3 2007/02/07 18:06:07 marco Exp $
;
;+
; NAME:
;       ANALIZE_CLOOP
;
; PURPOSE:
;       The function analyses the closed loop data from a file, if no data or file is set the function
;        loads and analyses the diagnostic closed loop data from sdram.  
;
; CATEGORY:
;       General utilities.
;
; CALLING SEQUENCE:
;   error = analize_cloop(DATA=data, FILE=file, STARTFC=start, ENDFC=endf, $
;                           DOSAVE=dosaveq,START_POS=start_pos, END_POS=end_pos)
;
;
; INPUTS:
;       None
;
; OUTPUTS:
;       error: error code.
; KEYWORDS:
;   
;   DATA        = closed loop data structure to analyse.
;   FILE        = file .sav contains thr closed loop data structure.
;   STARTFC     = the first frame counter for the closed loop data analysis.        
;   ENDFC       = the last frame counter for the closed loop data analysis.
;   DOSAVE      = If set function saves the closed loop data.
;   START_POS   = the frame from which to cut the closed loop data.
;   END_POS     = the last frame from which to cut the closed loop dat.
;
;
; COMMON BLOCKS:
;       common adsec_common_block
; HISTORY
;   March 2006 Marco Xompero(MX)
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;
;-

Function cut_diag_frame, startf, endf, data
    
    names = tag_names(data)
    nt = n_tags(data)
    if nt eq 0 then begin
        print, "No structure in input. Returning."
        return, -1
    endif
    dummy = create_struct(names[0], (data.(0))[startf:endf])
    for i=1, nt-1 do begin
        st = n_elements(size(data.(i), /DIM))
        if st eq 1 then dummy = create_struct(names[i], (data.(i))[startf:endf], dummy) $
                   else dummy = create_struct(names[i], (data.(i))[*,startf:endf], dummy) 
    endfor
    return, dummy

End



Function analize_cloop, DATA=data, FILE=file, STARTFC=start, ENDFC=endf, DOSAVE=dosaveq,START_POS=start_pos, END_POS=end_pos

    @adsec_common
    dosaveq = 0

    if keyword_set(FILE) then restore, file, /ver
    ;data = cloop_3v
    

    if n_elements(DATA) eq 0 then begin

        dosaveq = 1
        
        ;ask for the num of frames to acquire
        answ = textbox(TITLE="QUESTION", LABEL="How many frames to capture?")
        if answ eq "" then return, adsec_error.ok
        n_frames = fix(answ)
        if n_frames gt (dsp_datasheet.sdram_mem_size/adsec.n_dsp_per_board/rtr.diag_len) or $
                    n_frames lt 0 then begin
            print, "Wrong value on input. No action performed."            
            return, adsec_error.input_type
        endif

        ;ask for the start point
        answ = dialog_message("History from current position ('No' means from the 0x000000 SDRAM address.)", /QUESTION)

    
        if answ eq "Yes" then begin

            sync=1 

        endif else begin

            ;ask for the start point
            answ = textbox(LABEL="Enter the number of frames to skip ('0' means no frames)", TITLE='QUESTION')
            nskip = fix(answ)
            
            if nskip gt (dsp_datasheet.sdram_mem_size/adsec.n_dsp_per_board/rtr.diag_len) or $
                        nskip lt 0 then begin
                print, "Wrong value on input. No action performed."            
                return, adsec_error.input_type
            endif

        endelse

        err = load_diag_data(n_frames, data, READFC=sync, NOFA=sync, SKIP=nskip)

    endif 

    n_frames  = n_elements(data.wfs_frames_counter)

    if n_elements(startfc) eq 0 then startfc = data.wfs_frames_counter[0]
    if n_elements(endfc) eq 0 then endfc = data.wfs_frames_counter[n_frames-1]

;    start_pos = where(data.wfs_frames_counter eq startfc, cs)
;    if cs eq 1 then start_pos=start_pos[0]
;    end_pos = where(data.wfs_frames_counter eq endfc, ce)
;    if ce eq 1 then end_pos=end_pos[0]
;
;    if (cs eq 0) or (ce eq 0) then begin
;        print, "Wrong range submitted. Returning."
;        return, adsec_error.input_type
;    endif


;    if (cs gt 1) or (ce gt 1) then begin
;        print, "Frames counter not unique. Performing the largest range cut."
;        start_pos = start_pos[0]
;        end_pos = end_pos[ce-1]
;    endif

    
    if n_elements(start_pos) eq 0 then start_pos = 0
    if n_elements(end_pos) eq 0 then end_pos = n_frames-1

    data_cut = cut_diag_frame(start_pos, end_pos, data)

    n_frames  = n_elements(data_cut.wfs_frames_counter)

    time = lindgen(n_frames)
    colors = comp_colors(adsec.n_actuators)

    
    ;data analisys
    mir_pos = data_cut.dist_average
    mir_cur = data_cut.curr_average

    ;modal coeff time history
    dmodes = data_cut.modes / sqrt(n_elements(adsec.act_w_cl))
    m_dmodes2 = sqrt(rebin((dmodes[0:n_elements(adsec.true_act)-1, *])^2, 1, n_frames))

    yr = [min(dmodes[adsec.act_w_cl, *]), max(dmodes[adsec.act_w_cl, *])]*1d6
    wplot, time, dmodes[0,*]*1d6, /new, yr = yr, TITLE="MODES APPLIED", xs=17, psym=-4
    for i=0, adsec.n_actuators-1 do woplot, time, dmodes[i,*]*1d6, col=colors[i], psym=-4
    wplot, time, m_dmodes2*1d6,  /new, /ynoz,  TITLE="SQRT(AVE(MODES^2))", xs=17, psym=-4

    ;command time history
    dcom = data_cut.new_delta_command
    m_dcom = rebin(dcom[adsec.act_w_cl, *], 1, n_frames)
    yr = [min(m_dcom[adsec.act_w_cl]), max(m_dcom[adsec.act_w_cl])]*1d6
    wplot, time, m_dcom*1d6, TITLE="MEAN COMMAND WRT BIAS COMMAND", /NEW, psym=4, yr=yr
    
    ;command time history
    ave_pos = data_cut.dist_average
    m_ave_pos = rebin(ave_pos[adsec.act_w_cl, *], 1, n_frames)
    yr = [min(m_ave_pos), max(m_ave_pos)]*1d6
    wplot, time, m_ave_pos*1d6, TITLE="MEAN CAPSENS POSITION", /NEW, psym=4, yr=yr

    ;frames counter
    fc = data_cut.wfs_frames_counter
    wplot, time, fc, /ynoz, TITLE="FRAMES COUNTER", /NEW

    ;saving the data
    if dosaveq then begin

        answ = dialog_message("Do you want to save analized data?", /QUESTION)
        if answ eq "Yes" then begin
            save, file=dialog_pickfile(), data, data_cut
        endif

    endif
    
    

End
