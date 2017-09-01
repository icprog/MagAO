; $Id: check_dsp.pro,v 1.8 2007/03/26 17:43:38 labot Exp $
;+
; CHECK_DSP
;
; The function returns an error if there is any DSP in the
; DSP list that has a non-existing number or there is any
; DSP number repetition in the list.
;
; err = check_dsp(dsp_list [, /NOALL])
;
; err:      BYTE scalar. 1B in case of any wrong DSP, 0B otherwise.
;
; dsp_list: NOFLOAT (see test_type.pro) vector. List of DSP numbers.
;
; KEYWORDS
;   NOALL: if it is set, the sc.all_dsp code is not considered a
;          a valid DSP identifier
;
; HISTORY
;  ?? ??? ????: Written by A. Riccardi (riccardi@arcetri.astro.it) (AR)
;  26 Feb 2004: AR: Help added
;
;  08 Arp 2004
;    *Task of the function is now performed thru check_list
;
;  05 Aug 2004: AR
;    fixed bug related to the NOALL keyword
;   02 Nov 2004, MX
;     Adsec.all changed in sc.all.
;-

function check_dsp, dsp_list, NOALL=no_all_dsp

    ; returns 1B if some dsp is not valid. 0B otherwise.
    @adsec_common

    return, check_list(dsp_list, adsec.n_dsp, NOALL=no_all_dsp, /GREAT)
end
