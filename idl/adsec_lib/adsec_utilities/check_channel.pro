; $Id: check_channel.pro,v 1.8 2007/03/26 17:43:38 labot Exp $

;+
; CHECK_CHANNEL
;
; The function returns an error if there is any actuator number
; (aka channel) in the actuator list that corresponds to a
; non-existing actuator or there is any actuator number repetition
; in the list.
;
; err = check_channel(actuator_list [, /NOALL])
;
; err:           BYTE scalar. 1B in case of any wrong channel, 0B otherwise.
;
; actuator_list: NOFLOAT (see test_type.pro) vector. List of channel numbers.
;
; KEYWORDS
;   NOALL: if it is set, the sc.all_actuators code is not considered a
;          a valid actuator identifier
;
; HISTORY
;  ?? ??? ????: Written by A. Riccardi (riccardi@arcetri.astro.it) (AR)
;  26 Feb 2004: AR:
;               *actuator_list can be NOFLOAT now.
;               *Help added
;
;  08 Apr 2004: AR
;    *Task of the function is now performed thru check_list
;
;  05 Aug 2004: AR
;    fixed bug related to the NOALL keyword
;  02 Nov 2004, MX
;    Adsec.all changed in sc.all.
;-
function check_channel, channel, NOALL=no_all_channel

    ; returns 1B if some channel is not valid. 0B otherwise.
    @adsec_common

    return, check_list(channel, adsec.n_actuators, NOALL=no_all_channel, /GREAT)
end
