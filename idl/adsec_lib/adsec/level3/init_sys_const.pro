; $Id: init_sys_const.pro,v 1.13 2009/09/18 08:25:01 marco Exp $$
;
;+
;  NAME:
;   INIT_SYS_CONST
;
;  PURPOSE:
;   Load constant characteristics of the SW for various use in the SC common strucute.
;
;  USAGE:
;   err = INIT_SYS_CONST(filename)
;
;  INPUT:
;   filename(optional)  : filename of data to load. Default adsec_path.conf+'sys_const_LBT672.txt'.
;
;  OUTPUT:
;   error               : error code.
;
;  COMMON BLOCKS:
;   SC                  : secondary adaptive software system constants and 'sc' common block will be filled.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on 26 Oct 2004 by Marco Xompero (MX).
;   marco@arcetri.astro.it
;   01 Dec 2004: MX
;    default configuration filename changed
;   08 Feb 2005: MX
;    Communication name bug fixed.
;   04 Aug 2005, MX
;    New MsgdRTDB veriables name added.
;   March 2006, MX
;    Added no_ask keyword.
;   18 Apr 2007, MX
;    Added/changed parameters for programs path.
;   01 Oct 2008, MX
;    REG_NAME keyword added in order to directly pass the name for registration in MSGD-RTDB
;-

Function init_sys_const, filename, NO_ASK=no_ask, REG_NAME = reg_name


    @adsec_common

    if n_elements(filename) eq 0 then filename=filepath(ROOT=adsec_path.conf,SUB=['general'],'sys_const.txt')

    exist = file_search(filename)

    if exist eq "" then begin
        message, "Filter parameters file not found!!", CONT = (sc.debug eq 0)
        return, adsec_error.invalid_file
    endif else begin
        data = read_ascii_structure(filename)
    endelse
   
    dll_name = !ao_const.dll_file
    name = !ao_const.conf_file
    host_comm = name  
   
    ;NB the DSPBoard program is supposed to be the SAME for all DspBoards
    dsp_prog_path = filepath(prog_vers.crate[0].dspBoard[0].dsp, ROOT=adsec_path.prog, SUB=['dspBoard', 'dsp'])+path_sep()
    siggen_prog_path = filepath(prog_vers.crate[0].siggen[0].dsp, ROOT=adsec_path.prog, SUB=['sigGen', 'dsp'])+path_sep()
    id_acc=  where(prog_vers.crate[*].siggen[0].is_acc)
    if id_acc[0] eq -1 then begin
        message, "WARNING NO ACCELEROMETER FOUND!!!!", /INFO
        accel_prog_path = ""
    endif else begin
        accel_prog_path = filepath(prog_vers.crate[id_acc[0]].siggen[0].dsp, ROOT=adsec_path.prog, SUB=['accBoard', 'dsp'])+path_sep()
    endelse

    bcu_prog_path = filepath(prog_vers.crate[0].bcu.dsp, ROOT=adsec_path.prog, SUB=['crateBcu', 'dsp'])+path_sep()
    switch_prog_path = filepath(prog_vers.switchbcu.dsp, ROOT=adsec_path.prog, SUB=['switchBcu', 'dsp'])+path_sep()


n_act_names = 3                         ;; number of formats for the actuator names
                                        ;; 0 = internal numbering (0...35)
                                        ;; 1 = mechanical numbering (1...36)
                                        ;; 2 = RAA format
act_name = strarr(adsec.n_actuators, n_act_names)  ;; actuator name: "RAA"
                                        ;; R =ring number (0-9)
                                        ;; AA=act number in the ring (00-XX)




if adsec_shell.n_rings gt 10 then $
    ring_format_str='(I2.2)' $
else $
    ring_format_str='(I1.1)'
if adsec_shell.n_act_ring[adsec_shell.n_rings-1] gt 100 then $
    act_format_str='(I3.3)' $
else $
    act_format_str='(I2.2)'

act0 = 0
for nr=0,adsec_shell.n_rings-1 do begin

    act_name[act0,0] = strtrim(indgen(adsec_shell.n_act_ring[nr])+act0, 2)
    act_name[act0,1] = strtrim(indgen(adsec_shell.n_act_ring[nr])+act0, 2)
    act_name[act0,2] = string(nr,FORMAT=ring_format_str) $
      + string(indgen(adsec_shell.n_act_ring[nr]),FORMAT=act_format_str)

    act0 = act0+adsec_shell.n_act_ring[nr]
endfor

if adsec.dummy_act[0] ge 0 then begin
    act_name[act0:*,0] = strtrim(indgen(n_elements(adsec.dummy_act))+act0, 2)
    act_name[act0:*,1] = "XX"
    act_name[act0:*,2] = "XX"
endif
    


act_name[*,1:2] = act_name[adsec.act2mir_act,1:2]



    sc =                                       $
    {                                          $
        lf             :  [10B],               $   ; line feed
        cr             :  [13B],               $   ; carriage return
        ctrlc          :  [3B],                $   ; control-c
        true           :  1B,                  $   ; logical true
        false          :  0B,                  $   ; logical false
        ;;
        debug          :  data.debug,          $   ; 1B/0B run routines in debug mode if 1B
        ll_debug       :  data.ll_debug,       $
        msgd_diag_name :  data.msgd_diag_name, $
        msgd_freq_name :  data.msgd_freq_name, $
        name_comm      :  name,                $   ; name of the communication process
        busy_comm      :  !AO_CONST.msgd_link,       $   ; freedom of communication flag
        mirrctrl_id    :  data.mirrctrl_id,    $
        bcu_ip         :  data.bcu_ip,         $
        field_port     :                       $
        {                                      $
            busy        :  0B,                 $  ; flag busy field point communication
            idl_port_id :  0B,                 $  ; file port opened under idl
            field_com   :  data.field_com,     $  ; type of connection with field point
            ip_address  :  data.field_ip,      $  ; structure that dfin the socket port
            port        :  data.field_port     $
        },                                     $
        fp_config      :  ptr_new(/alloc),     $
        act_name       :  act_name,            $
        model          :  data.adsec_model,    $  ; model of adaptive secondary mirror ("P36","S336","A336","P45","A672")
        prog_ver       :  dsp_prog_path,       $  ; version of the DSP program for Dspboard
        bcu_prog_ver   :  bcu_prog_path,       $  ; version of the DSP program for CrateBCU 
        switch_prog_ver:  switch_prog_path,    $  ; version of the DSP program for SwitchBCU 
        siggen_prog_ver:  siggen_prog_path,    $  ; version of the DSP program for Siggen
        accel_prog_ver :  accel_prog_path,     $  ; version of the DSP program for Siggen
        host_comm      :  host_comm,           $   ; OBSOLETE TO REMOVE
        adsec_comm     :  data.adsec_comm,     $   ; adsec crate communication board type ("L001", "L002", "L003", ...)
        dll_name       :  dll_name,            $   ; dll name vector (dummy, PCIHotLink, SwiftNet)
        min_timeout    :  data.min_timeout,    $ ; minimum communication timeout [ms]
        all_actuators  :  -1L,                 $
        all_dsp        :  -1L,                 $
        all_boards     :  -1L,                 $
        all            :  -1L                  $
    }

    return, adsec_error.ok

End
