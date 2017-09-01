; $Id: proc_sysdown.pro,v 1.8 2007/09/19 10:35:26 labot Exp $
;+
; NAME:
;   PROC_SYSDOWN
;
; PURPOSE:
;   Set the mirror in the REST IN PEACE state and stop the system
;
; CATEGORY:
;   Adsec Widget Function
;
; CALLING SEQUENCE:
;
;   Err = PROC_SYSDOWN ()
;
; OUTPUTS:
;   Error code (see INIT_ERROR_CODES.pro)
;
;
; COMMON BLOCKS:
;   Adsec std common blocks (see ADSEC_COMMON.pro )
;
; EXAMPLE:
;
;   To shut down the system just write:
;
;     err = PROC_SYSDOWN()
;
;   HISTORY:
;    Created by A.Riccardi (AR) on 25/12/0000
;    04 Feb 2004 M.Xompero (MX)
;      Bias magnet supported.
;    17 Feb 2004 MX
;      New adsec.secure_factor variable used.
;    28 May 2004, AR
;      modifications to match LBT formats
;    02 Nov 2004, MX
;      Adsec.all changed in sc.all.
;    17 July 2007, DZ
;      Introduce curr2apply instead of sys_status.current and added a write bias_current
;       to zero.        
;
;-
function proc_sysdown

    @adsec_common
    
    time_ps = time_preshaper(/FF)

    answer = dialog_message("Do you want to put the mirror in the rest state?" $
                , /QUESTION)

    if strlowcase(answer) eq "yes" then begin
        disp_mess, "Stopping the control for all the actruators..."
        err = stop_control(sc.all_actuators)
        if (err ne adsec_error.ok) then begin
            disp_mess, "Error stopping the control (err code "+strtrim(err, 2)+").", $
                       /APPEND
            return, err
        endif
        disp_mess, "... done.", /APPEND

        disp_mess, "Clearing the feedforward currents..."
        err = clear_ff()
        if (err ne adsec_error.ok) then begin
            disp_mess, "Error clearing the feedforward currents (err code "+strtrim(err, 2)+").", $
                       /APPEND
            return, err
        endif
        disp_mess, "... done.", /APPEND

       update_panels

        disp_mess, "Ramping the bias current to zero...", /APPEND
        if (total(sys_status.current ne 0) gt 0) then begin
            start_curr = (adsec.curr4bias_mag+adsec.secure_factor)*adsec.weight_curr
            nstep = 50
            nstep_dec = 5
            tot_time = 2.0 ; [s]
            curr_step = start_curr/nstep
            dtime = (tot_time/nstep) > 0.01

         err = clear_ff()
            if err ne adsec_error.ok then begin
                disp_mess, "Error resetting the ff current (err code "+strtrim(err, 2)+").", $
                           /APPEND
                return, err
            endif

            for i=long(nstep),0L,-1L do begin
                ;sys_status.current = i*curr_step
                curr2apply= i*curr_step
                ;err = write_bias_curr(sc.all_actuators, sys_status.current)
                err = write_bias_curr(sc.all_actuators, curr2apply)
                if err ne adsec_error.ok then begin
                    disp_mess, "Error setting the bias current (err code "+strtrim(err, 2)+").", $
                               /APPEND
                    return, err
                endif
                if (i mod nstep_dec) eq 0 then update_panels
                wait, dtime
            endfor
        endif
        
        err = write_bias_curr(sc.all_actuators, 0.0)
            if err ne adsec_error.ok then begin
                disp_mess, "Error setting the bias current (err code "+strtrim(err, 2)+").", $
                               /APPEND
                return, err
            endif
        wait, time_ps

        disp_mess, "... done.", /APPEND
        update_panels
        disp_mess, "All done.", /APPEND

    endif
    ; return value for future implementations
    return, adsec_error.ok

end
