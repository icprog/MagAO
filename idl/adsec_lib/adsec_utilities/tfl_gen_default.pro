; $Id: tfl_gen_default.pro,v 1.2 2004/12/03 19:03:49 labot Exp $
;+
; NAME:
;    tfl_gen_default
;
; PURPOSE:
;    tfl_gen_default generates the default parameter structure
;    for the TFL module and returns it.
;
;    The default filter is the discrete recursive
;    filter obtained appling the bilinear transformation (see
;    bilinear.pro) to a pure proportional gain:
;
;                TF(s) = 1
;
;    giving in the z-domain:
;
;                TF(z) = 1
;                         
;
; CATEGORY:
;    module's utility routine
;
; CALLING SEQUENCE:
;    par = tfl_gen_default()
;
; ROUTINE MODIFICATION HISTORY:
;    Written on February 1999 by Armando Riccardi (OAA) 
;    riccardi@arcetri.astro.it]
;
;    01 Dec 2004: Marco Xompero (MX)
;     adsec_commom structures embedded and max_n_coeff value changed.     
;
;-
;
function tfl_gen_default

@adsec_common
max_n_coeff = dsp_const.max_filters_coeff 
										  ; max nb of allowed coeffs, i.e.
                                ; the max number
                                ; of poles or zeros is max_n_coeff-1
                                ; poles and 0s can be complex

s_zero = dcomplexarr(max_n_coeff-1)
s_pole = dcomplexarr(max_n_coeff-1)
z_num_coeff = dblarr(max_n_coeff)
z_den_coeff = dblarr(max_n_coeff)

;; The default filter is a pure proportional one:
;; TF(s)=1
double = 1B                     ; used filter precision: double prec.
type = 1                        ;              1 PID
                                ;              2 generic gain-zero-pole
                                ;              0 pure integrator
; pure proportional as special case of a PID(type=1)
s_const   = 1d0
n_s_zero  = 0
n_s_pole  = 0
s_pole[0] = 0d0                ; single pole at zero

method = 0                     ; design methods of discrete from continuos
                               ; filter: 0 tustin (bilinear) (othet
                               ; possibilities in future
                               ; implementations)

negative_fb = 0B               ; is the filter used for a Negative feedback?
                               ; 1B: yes (output = - filtered(input))
                               ; 0B: no  (output = filtered(input))


par = $
   {  $
   s_const    : s_const,     $ ; constant factor of the s-domain filt. tf
   n_s_zero   : n_s_zero,    $ ; number of zeros of the s-domain filt. tf
   n_s_pole   : n_s_pole,    $ ; number of poles of the s-domain filt. tf
   s_zero     : s_zero,      $ ; zeros of the s-domain filter tf
   s_pole     : s_pole,      $ ; poles of the s-domain filter tf
   ;filename   : '',          $ ; filename of the filter data (ascii format)
   negative_fb: negative_fb, $ ; is the filter used for a neg. feedback?
   type       : type,        $ ; filter type, usefull for the gui
   method     : method,      $ ; chosen method index from the method list
   double     : double       $ ; coeffs precision: 0B single, 1B double
   }

return, par
end
