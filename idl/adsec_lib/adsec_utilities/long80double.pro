; NAME:
;   LONG80DOUBLE
;
; PURPOSE:
;   Convert three long word in a double
;
; CATEGORY:
;   General math.
;
; CALLING SEQUENCE:
;   result = long80double(a,b,c, fixed)
;
; INPUTS:
;   a:  32 bit long word (LSB)
;   b:  32 bit long word (MSB)
;   c:  32 bit long word (only 16 bit used)
;   fixed:  position of the fixed decimal.
;
; KEYWORD PARAMETERS:
;   MSB: the c long word is filled on bits 16-31 bit. Default is c filled on bits 0-15.
;   
; OUTPUTS:
;   result: double value obtained after transforming the 80 bit longword with fixed point given.
;
; RESTRICTIONS:
;   None.
;
; MODIFICATION HISTORY:
;   14 Apr 2005
;    Created by Marco Xompero (MX)
;    marco@arcetri.astro.it
;   
;-


Function long80double, a, b, c, fixed, MSB=msb

    if keyword_set(MSB) then begin
        c_in= c and 'ffff0000'xul
        c_in=double(ishft(c_in,-16))
    endif else begin
        c_in=double(c and '0000ffff'xul)
    endelse
    dummy = double(ulong64([a,b],0))+double(c_in)*2d^64
    if n_elements(fixed) ne 0 then return, fixed*dummy else return, dummy

End
