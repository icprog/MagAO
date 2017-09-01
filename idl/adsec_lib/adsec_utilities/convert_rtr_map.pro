pro convert_rtr_map, ROOT_DIR=root_dir, EXT=ext
;+
; HISTORY
;   6 April 2005, D.Zanotti
;   29 Aug 2005, Armando Riccardi (AR)
;    Number of slope reported to real value.
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;   18 Apr 2007, MX
;    Added/changed parameters for programs path.
;-
@adsec_common

;if n_elements(root_dir) eq 0 then $
root_dir=sc.prog_ver

;filename_sc = filepath(ROOT=root_dir,"SlopeComp_defaults.txt")
filename_sc = "/home/labot/Supervisor/BCUCtrl/SlopeComp_defaults.txt"
if n_elements(ext) eq 0 then ext = ""
print, "Slope computer map:"
print, filename_sc

slp_addr= read_ascii_structure(filename_sc, DEBUG=0)
addr_sl_rtr=lonarr(28)

first_dsp = 0
last_dsp =adsec.last_dsp
bit_12=long(2.^12)
bit_16=long(2.^16)
bit_13=long(2.^13)
bit_15=long(2.^15)
bit_19=long(2.^19)
bit_24=long(2.^24)
const=long(8704)
n_slope = long(rtr.n_slope)

addr_sl_rtr(0) = (n_slope*slp_addr.default_numsh+4)*bit_16+bit_13
addr_sl_rtr(1) = (0*bit_24+last_dsp*bit_12+first_dsp)
addr_sl_rtr(2) = rtr_map.slope_vector
addr_sl_rtr(3) = (n_slope*slp_addr.default_numsh+4)*bit_16 $
            +floor(slp_addr.default_slopeoutareaptr/bit_19)*bit_15+(slp_addr.default_slopeoutareaptr-bit_19)/2
addr_sl_rtr(4) = 2*bit_16+const
addr_sl_rtr(5) = (1*bit_24+last_dsp*bit_12+first_dsp)
addr_sl_rtr(6) = rtr_map.start_rtr
addr_sl_rtr(7) = 2*(last_dsp-first_dsp+1)*bit_16+floor(slp_addr.default_ReplyVectorPtr/bit_19)*bit_15 $
            +(slp_addr.default_ReplyVectorPtr-bit_19)/2
addr_sl_rtr(8) = adsec.n_act_per_dsp*bit_16+const
addr_sl_rtr(9) = (2*bit_24+last_dsp*bit_12+first_dsp)
addr_sl_rtr(10) = dsp_map.modes
addr_sl_rtr(11) = adsec.n_act_per_dsp*(last_dsp-first_dsp+1)*bit_16+floor(slp_addr.default_modesvectorptr/bit_19)*bit_15 $
            +(slp_addr.default_modesvectorptr-bit_19)/long(2)

addr_sl_rtr(12) = (adsec.n_actuators+2)*bit_16+bit_13
addr_sl_rtr(13) = (3*bit_24+last_dsp*bit_12+first_dsp)
addr_sl_rtr(14) = rtr_map.modes_vector
addr_sl_rtr(15) = (adsec.n_actuators+2)*bit_16+floor(slp_addr.default_modesvectorptr/bit_19)*bit_15 $
                +(slp_addr.default_modesvectorptr-bit_19)/2
addr_sl_rtr(16) = 2*bit_16+const
addr_sl_rtr(17) = (4*bit_24+last_dsp*bit_12+first_dsp)
addr_sl_rtr(18) = rtr_map.start_mm
addr_sl_rtr(19) = 2*(last_dsp-first_dsp+1)*bit_16+floor(slp_addr.default_ReplyVectorPtr/bit_19)*bit_15 $
            +(slp_addr.default_ReplyVectorPtr-bit_19)/long(2.)

addr_sl_rtr(20) = adsec.n_act_per_dsp*bit_16+const
addr_sl_rtr(21) = (5*bit_24+last_dsp*bit_12+first_dsp)
addr_sl_rtr(22) = dsp_map.ff_command
addr_sl_rtr(23) = adsec.n_act_per_dsp*(last_dsp-first_dsp+1)*bit_16 $
 +floor(slp_addr.default_FFCommandVectorPtr/bit_19)*bit_15+(slp_addr.default_FFCommandVectorPtr-bit_19)/long(2.)
            
addr_sl_rtr(24) = (adsec.n_actuators+2)*bit_16+bit_13
addr_sl_rtr(25) = (6*bit_24+last_dsp*bit_12+first_dsp)
addr_sl_rtr(26) = rtr_map.ff_command_vector
addr_sl_rtr(27) = (adsec.n_actuators+2)*bit_16+floor(slp_addr.default_FFCommandVectorPtr/bit_19)*bit_15 $
                +(slp_addr.default_FFCommandVectorPtr-bit_19)/long(2.)


map_filename = filepath(ROOT=root_dir, "sl_dsp_map"+ext+".txt")
n_addr=n_elements(addr_sl_rtr)
openw, unit, map_filename, /GET_LUN
for i=0,n_addr-1 do begin
    printf, unit, addr_sl_rtr(i), FORMAT="('0x',Z8.8)"
;    print, addr_sl_rtr(i), FORMAT="('0x',Z8.8)"
endfor
free_lun, unit

end
