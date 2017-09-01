; $Id: sync_dsp.pro,v 1.8 2007/03/26 17:39:25 labot Exp $

;+
;   NAME
;    SYNC_DSP
;   
;   PURPOSE:
;    Syncronize DSP in single and multi crate adaptive secondary configuration. 
;
;   USAGE:
;    err = sync_dsp(N_CYCLES=ncycles, CHECK_ONLY=check_only)
;   
;   INPUT:
;    ncycles: number of cycles in single crate sync loop
;    check_only: check only the DSP syncronization in single crate configuration
;
;   OUTPUT:
;    err: error code  
;
;   HISTORY
;
;    Written by A. Riccardi (AR)
;    Osservatorio Asrofisico di Arcetri, ITALY
;    <riccardi@arcetri.astro.it>
;
;   13 Mar 2002, AR
;    Bug fixed in the CONT keyword setting of the MESSAGE call.
;
;   28 May 2004, AR
;    Modifications to work with P45. Warning, it cannot work on LBT672!
;
;   04 Aug 2004, Marco Xompero (MX)
;    Dummy communication compatibility added.
;
;   20 Jul 2006, AR
;    returns an error in case multiple crate system is present.
;    This case is not yet implemented.
;
;   20 Mar 2007, MX
;    Multi crate synchronization added.
;-
function sync_dsp, N_CYCLES = ncycle, CHECK_ONLY=check_only

    @adsec_common

    if adsec.n_crates eq 1 then begin
        first_dsp = 0
        last_dsp =  adsec.last_dsp

        if n_elements(ncycle) eq 0 then begin
            ncycle = 100
        endif else begin
            if test_type(ncycle, /INT,/LONG,/BYTE, DIM=dim) then begin
                message, "N_CYCLES must be integer", CONT=(sc.debug eq 0B)
                return, adsec_error.input_type
            endif
                if dim[0] ne 0 then begin
                message, "N_CYCLES must be a scalar", CONT=(sc.debug eq 0B)
                return, adsec_error.input_type
            endif
            if ncycle le 1 then begin
                message, "N_CYCLES must be greater than 1", CONT=(sc.debug eq 0B)
                return, adsec_error.input_type
            endif
        endelse

        n_dsp=last_dsp-first_dsp+1

        if not keyword_set(check_only) then begin
            err = write_same(first_dsp, last_dsp, dsp_map.global_counter, 0UL)
            if err ne adsec_error.ok then return, err

            buffer = lonarr(ncycle)
            for i_dsp=first_dsp,last_dsp-1 do begin
                for i=0,ncycle-1 do begin

                    err = read_seq(i_dsp, i_dsp+1B, dsp_map.global_counter, 1L, buf, /LONG)
                    if err ne adsec_error.ok then return, err

                    buffer[i]=buf[1]-buf[0]
                endfor

                hist = histogram(buffer, omax=omax, omin=omin, bin=1)
                max_hist = max(hist, h)
                dtime = long(round(omin+h))
                if -dtime+1 ne 1 then begin
                   err = write_same(i_dsp+1B, i_dsp+1B, dsp_map.step_global_counter, -dtime+1)
                   if err ne adsec_error.ok then return, err
               endif
            endfor
        endif

        sync_failed = 0B
        buffer = lonarr(ncycle)
        for i_dsp=first_dsp,last_dsp-1 do begin
            for i=0,ncycle-1 do begin

                err = read_seq(i_dsp, i_dsp+1B, dsp_map.global_counter, 1L, buf, /LONG)
                if err ne adsec_error.ok then return, err

                buffer[i]=buf[1]-buf[0]
            endfor

            hist = histogram(buffer, omax=omax, omin=omin, bin=1)
            max_hist = max(hist, h)
            dtime = long(round(omin+h))
            if dtime ne 0 then begin
                ;window, /FREE
                ;plot, indgen(omax-omin+1)+omin, hist, PSYM=10, TIT="DSP#"+strtrim(i_dsp+1)
                message, "The syncronization procedure failed for DSP #" $
                             +strtrim(i_dsp+1,2)+" ("+strtrim(dtime,2)+")", /INFO
                sync_failed = 1B
            endif
        endfor

        if (sync_failed and sc.host_comm ne 'Dummy') then return, adsec_error.sync_dsp_fail else return, adsec_error.ok

    endif else begin

        ;Disable interrupt routine on DSP boards.
        err = manage_interrupt(/DISABLE)
        if err ne adsec_error.ok then return, err

        ;Write fixed number into GC location.
        err = write_same_dsp(sc.all, dsp_map.global_counter, '12345'xul, /CHECK)
        if err ne adsec_error.ok then return, err

        err = get_global_counter(sc.all, gc)
        if err ne adsec_error.ok then return, err

        if (total(abs(gc-gc[0])) ne 0  and sc.host_comm ne 'Dummy') then begin
            
            message, 'WARNING: Unable to synchronize the DSP boards.', CONT = (sc.debug eq 0)
            return, adsec_error.sync_dsp_fail 

        endif
        
        
        ;Enable the interrupt routine on DSP boards.
        err = manage_interrupt(/ENABLE)
        if err ne adsec_error.ok then return, err

    endelse
    
    return, adsec_error.ok

end




