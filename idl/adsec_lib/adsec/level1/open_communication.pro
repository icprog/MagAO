;$Id: open_communication.pro,v 1.7 2008/02/05 10:05:42 labot Exp $
;
;+
;   NAME:
;    OPEN_COMMUNICATION
;
;   PURPOSE:
;    Initialize a communication with the low level controller of the Adaptive Secondary.
;
;   USAGE:
;    err=open_communication()
;
;   OUTPUT:
;    Error Code.
;
;   HISTORY:
;
;    13 Jul 2004: written by A. Riccardi (AR), riccardi@arcetri.astro.it
;
;    08 Feb 2005: Marco Xompero (MX). marco@arcetri.astro.it
;                   New communication names choice added. Help written.
;    02 Nov 2005: Daniela Zanotti(DZ) 
;                 Insert keyword NO ASK to skip the question for 
;                 the comunication.       
;    07 Feb 2007: MX
;                 Obsolete features removed
;-
;
function open_communication, NO_ASK=no_ask

    @adsec_common

    ;if sc.host_comm eq "Dummy" then begin
    ;   print, "DUMMY COMMUNICATION enabled"
    ;   return, adsec_error.ok
    ;endif

    if sc.host_comm eq "Dummy" then no_ask=1B
    if keyword_set(no_ask) then  answ='yes' else begin
                answ = dialog_message(["Do you want to initialize the", $
                           sc.host_comm+" communication?"], /QUESTION)
    endelse
    
    if strlowcase(answ) eq "yes" then begin

        err = open_superv()
        if err ne adsec_error.ok then return, err
        
    endif

    return, adsec_error.ok
end
