;$Id: get_fast_diagnostic.pro,v 1.11 2007/11/05 10:05:11 marco Exp $
;+
;   NAME:
;    GET_FAST_DIAGNOSTIC
;
;   PURPOSE:
;    Get the data from fast_diagnostic data
;
;   USAGE:
;    err = get_fast_diagnostic(str, TIMEOUT=timeout)
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    str : structure with the diagnostic fields.
;
;   KEYWORD:
;    TIMEOUT:   timeout in seconds. if not set, default is 0.1s
;
;    EMPTY:     if it is set, no diagnostic is read and an empty structure is returned.
;
;   HISTORY:
;    Created by Marco Xompero on 03 Aug 2005
;    marco@arcetri.astro.it
;   04 Aug 2005
;    Some parameter is now written in the rtr configuration file. Help written.
;   04 Aug 2005, A. Riccardi
;    EMPTY keyword added.
;   29 Aug 2005, AR
;    Timeout set in second. Default value changed.
;   20 Jun 2006, MX
;    Rematched the PB structure. Dummy case now managed.
;   06 Mar 2007, MX
;    Start modification to get data from FAST-DIAGNOSTIC Supervisor Module. Dummy mode fixed
;   5 Jun 2007, MX
;    Data structure updated.
;   4 Nov 2007, MX
;    NAN value now tested and nulled.
;-

Function get_fast_diagnostic, diag_struct, TIMEOUT=timeout_in_seconds, EMPTY=empty_struc

    @adsec_common

    diag_struct = {                                    $                                                                 $
        Frame_Number      : 0UL,                       $                                         $
        DistAverage       : fltarr(adsec.n_actuators),  $
        CurrAverage       : fltarr(adsec.n_actuators),  $
        DistAverageDouble : dblarr(adsec.n_actuators), $
        CurrAverageDouble : dblarr(adsec.n_actuators), $
        Modes             : fltarr(adsec.n_actuators),  $
        NewDeltaCommand   : fltarr(adsec.n_actuators),  $
        FFCommand         : fltarr(adsec.n_actuators),  $
        IntControlCurrent : fltarr(adsec.n_actuators),  $
        FFPureCurrent     : fltarr(adsec.n_actuators),  $
        DistRMS           : dblarr(adsec.n_actuators), $
        CurrRMS           : dblarr(adsec.n_actuators)  $
    }

    if keyword_set(empty_struc) then return, adsec_error.ok

    ;reading shared memory
    if (sc.host_comm eq "Dummy") then begin
        common gfd_seed_block, gfd_seed
        diag_struct.Frame_Number      = round(randomu(gfd_seed) * 2ul^31)
        diag_struct.DistAverage       = randomu(gfd_seed, adsec.n_actuators) * 100e-6
        diag_struct.CurrAverage       = randomu(gfd_seed, adsec.n_actuators) * 1e-1
        diag_struct.DistAverageDouble = randomu(gfd_seed, adsec.n_actuators) * 1e-1
        diag_struct.CurrAverageDouble = randomu(gfd_seed, adsec.n_actuators) * 100e-6
        diag_struct.Modes             = randomu(gfd_seed, adsec.n_actuators) * 1e-6
        diag_struct.NewDeltaCommand   = randomu(gfd_seed, adsec.n_actuators) * 1e-6
        diag_struct.FFCommand         = randomu(gfd_seed, adsec.n_actuators) * 1e-6
        diag_struct.IntControlCurrent = randomu(gfd_seed, adsec.n_actuators) * 1e-1
        diag_struct.FFPureCurrent     = randomu(gfd_seed, adsec.n_actuators) * 1e-1
        diag_struct.DistRMS           = randomu(gfd_seed, adsec.n_actuators) * 1e-8
        diag_struct.CurrRMS           = randomu(gfd_seed, adsec.n_actuators) * 1e-3

    endif else begin

        err =  getdiagnvalue(process_list.fastdiagn.msgd_name, 'Ch* Modes* MirrorFrame*', -1, -1, val, time=2)
        if err lt adsec_error.ok then begin
            message, "Error retrieving fast data from fast diagnostic...", CONT=(sc.debug eq 0B)
            return, err
        endif
        
        dummy = val.last
        id_nan = where(finite(dummy, /NAN) eq 1, cnan)
        if cnan gt 0 then dummy[id_nan] = 0.0

        ca = adsec.n_actuators
        diag_struct.CurrAverage       = dummy(0:ca-1)
        diag_struct.CurrRMS           = dummy(ca:2*ca-1)
        diag_struct.DistAverage       = dummy(2*ca:3*ca-1)
        diag_struct.DistRMS           = dummy(3*ca:4*ca-1)
        diag_struct.FFCommand         = dummy(4*ca:5*ca-1)
        diag_struct.FFPureCurrent     = dummy(5*ca:6*ca-1)
        diag_struct.IntControlCurrent = dummy(6*ca:7*ca-1)
        diag_struct.NewDeltaCommand   = dummy(7*ca:8*ca-1)
        diag_struct.Frame_Number      = dummy(8*ca)
        diag_struct.Modes             = dummy(8*ca+1:9*ca)


    endelse

    return, adsec_error.ok

End
