; $Id: init_adsec_shell.pro,v 1.4 2007/07/10 08:56:33 riccardi Exp $$
;
;+
;  NAME:
;   INIT_ADSEC_SHELL
;
;  PURPOSE:
;   Load geometrical parameters of the shell in the adsec_geom common structure.
;
;  USAGE:
;   err = INIT_ADSEC_SHELL(filename)
;
;  INPUT:
;   filename(optional)  : filename of data to load. Default adsec_path.conf+'shell.txt'.
;
;  OUTPUT:
;   error               : error code.
;
;  COMMON BLOCKS:
;   ADSEC_SHELL         : secondary adaptive geometrical chars common block will be filled.
;
;  PACKAGE:
;   LBT Adsec Libraries.
;
;  HISTORY
;   Created on 26 Oct 2004 by Marco Xompero (MX).
;   marco@arcetri.astro.it
;   01 Dec 2004: MX
;     default configuration filename changed
;   13 December 2004: Daniela Zanotti(DZ)
;     default configuration filename changed
;-

Function init_adsec_shell, filename


   @adsec_common

   if n_elements(filename) eq 0 then filename=filepath(ROOT=adsec_path.conf,'shell.txt')

   exist = file_search(filename)

   if exist eq "" then begin
      message, "Filter parameters file not found!!", CONT = (sc.debug eq 0)
      return, adsec_error.invalid_file
   endif else begin
      data = read_ascii_structure(filename)
   endelse

;===========================================================================
; Definition of the mirror parameters

; radius of each ring (measured on the curved surface)
   ring_radius = data.curv_radius*(!PI/180.0)*data.angle_ring

   if (ring_radius[0] lt data.in_radius) $
               or (ring_radius[data.n_rings-1] gt data.out_radius) then begin
      message, "Error in the mirror inner/outer radius definition.", cont=(sc.debug eq 0)
   endif
;; total number of physical actuators
   n_true_acts  = fix(total(data.n_act_ring))
;; mirror mass + magnets + central membrane [g] 336
   mass = data.glass_mass+data.mag_mass*n_true_acts+data.membrane_ring_mass


   act_coord = fltarr(2, n_true_acts)          ;; x,y coords of the acts
   act0 = 0
   for nr=0,data.n_rings-1 do begin

      ;; azimuthal angle of the acts in the ring nr
      theta = findgen(data.n_act_ring[nr])*(360.0/data.n_act_ring[nr])+data.angle_act0[nr]
      theta = !PI/180.0*transpose(theta)

      ;; act x,y coordinates
      act_coord[0,act0] = shift(ring_radius[nr]*[cos(theta), sin(theta)],0,nr+1)
      act0 = act0+data.n_act_ring[nr]

endfor

;
; End of the Definition of the mirror parameters
;===========================================================================


; *** Mirror geometrical parameter specifications ***
   adsec_shell =                                      $
   {                                                  $
      glass_mass           :  data.glass_mass,        $
      mag_mass             :  data.mag_mass,          $
      membrane_ring_mass   :  data.membrane_ring_mass,$
      mass                 :  mass,                   $ ; mirror mass [g]
      vertex_thickness     :  data.vertex_thickness,  $
      n_true_act           :  n_true_acts,            $ ; total number of physical actuators
      n_rings              :  data.n_rings,           $ ; number of actuator rings
      n_act_ring           :  data.n_act_ring,        $ ; number of act. per ring
      angle_ring           :  data.angle_ring,        $ ; angle of the ring wrt the opt. axis [deg]
      angle_act0           :  data.angle_act0,        $ ; azimuthal angle act0 wrt x+ [deg]
      curv_radius          :  data.curv_radius,       $ ; radius of curvature (back face) [mm]
      in_radius            :  data.in_radius,         $ ; inner radius of the mirror [mm]
      out_radius           :  data.out_radius,        $ ; outer radius of the mirror [mm]
      ring_radius          :  ring_radius,            $ ; radius of the rings from the opt.axis [mm]
      true_act_coord       :  act_coord,              $ ; coordinates of true act's [mm],act#0->x+,act#9->y+
      in_cap_radius        :  data.in_cap_radius,     $ ; inner radius of the cap. anulus [mm]
      out_cap_radius       :  data.out_cap_radius     $ ; outer radius of the cap. anulus [mm]
   }


   return, adsec_error.ok

End
