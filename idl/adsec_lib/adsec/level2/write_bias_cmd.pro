; $Id: write_bias_cmd.pro,v 1.3 2007/02/07 16:57:58 marco Exp $$
;+
; WRITE_BIAS_CMD
;
; PURPOSE:
;  The routine writes the bias_command_vector into the dsps. This is a bias position command.
;
; HISTORY
;   Created on Jan 2005 by Marco Xompero(MX) and Daniela Zanotti(DZ).
;   07 Feb 2007 MX
;       LUT of variable deleted and name refurbished.
;       Polling on nendof_ff removed.
;-
function write_bias_cmd, pos_vector, CHECK=check

    @adsec_common

   pos_vector_new = float(pos_vector)
   if n_elements(pos_vector) eq 1 then begin
       print, 'The bias position command will be the same for all actuators.'
       pos_vector_new = fltarr(adsec.n_actuators)+pos_vector[0]

   endif else begin
      if n_elements(pos_vector_new) ne adsec.n_actuators then begin
              message, "The elements of input position must be scalar or as same of act_list.", CONT= (sc.debug eq 0B)
              return, adsec_error.input_type
      endif
   endelse

   pos_vector_new = transpose(pos_vector_new)
   err = write_seq_ch(sc.all, dsp_map.bias_command, pos_vector_new, CHECK=check)
   return, err

end
