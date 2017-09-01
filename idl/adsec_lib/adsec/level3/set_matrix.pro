;$Id: set_matrix.pro,v 1.6 2006/01/27 15:16:22 labot Exp $$
;+
;set the matrices of the delay lines
; HISTORY:
;   03 Aug 2005 Marco Xompero (MX)
;    adsec structure converted to rtr structure.
;   29 Aug 2005 Armando Riccardi (AR)
;    neglectd B0 matrix on block A added.
;   07 Oct 2005, MX
;     RTR matrices now managed by pointers.
;   29 Nov 2005, MX
;     Function now permits managing single block matrices changes.
;   Jan 2006, DZ
;     Updated the procedure with the new rtr fields.
;-


Function set_matrix, AONLY=aonly, BONLY=bonly
@adsec_common

if ~keyword_set(aonly) and ~keyword_set(bonly) then begin
    aonly = 1
    bonly = 1
endif

if keyword_set(aonly) then begin
    disp_mess, 'Sending default matrix B0 block A  ...',/APPEND
    err = set_b0_matrix(*rtr.b0_a_matrix)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on writing.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND

    disp_mess, 'Sending default matrix B_delay block A  ...',/APPEND
    err = set_b_delay_matrix(*rtr.b_delay_a_matrix)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on writing.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND
endif

if keyword_set(bonly) then begin
    disp_mess, 'Sending default matrix B0 block B  ...',/APPEND
    err = set_b0_matrix(*rtr.b0_b_matrix, /block)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on writing.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND
    
    disp_mess, 'Sending default matrix B_delay block B  ...',/APPEND
    err = set_b_delay_matrix(*rtr.b_delay_b_matrix, /block)
    if err ne adsec_error.ok then begin
        disp_mess, '... error on writing.', /APPEND
        return, err
    endif
    disp_mess, '... done.', /APPEND
endif

disp_mess, 'Sending default matrix A_delay   ...',/APPEND
err = set_a_delay_matrix(*rtr.a_delay_matrix)
if err ne adsec_error.ok then begin
    disp_mess, '... error on writing.', /APPEND
    return, err
endif
disp_mess, '... done.', /APPEND

return, adsec_error.ok
end
