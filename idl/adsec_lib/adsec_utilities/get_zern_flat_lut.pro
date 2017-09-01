Function get_zern_flat_lut, elevation, instrument

    @adsec_common
    if n_elements(elevation) gt 0 then elevation = double(elevation)
    if n_elements(instrument) eq 0 then instrument = "IRTC"
    case instrument of
        "IRTC": begin
                    lut_file = '$ADOPT_ROOT/calib/adsec/current/data/astig_lut.sav' 
                    ; remember: implement a check on the existence of the file; also retrieve the file from the configuration
                    zz = fltarr(22)
                    if file_test(lut_file) eq 1 then begin
    
                        ;el=[20,43,58,60,66, 71,84.5,60,53, 27,55,54,37,43]
                        ;astig=1e-6*[5.5,2.4,1.2,2,1.5,1.2,0,2,3,4.5,1.7,2,3.6,3.2]
                        ;vv=linfit(el, astig)
                        ;vv = linfit([20,35,90], [2.5e-6, 2e-6, 0])
                        ;zz[5] = vv[0]+vv[1]*elevation
                        ;if (elevation gt 88) then zz[5]=0
                        ;zz[5] = 0
                        el = [90.,80,70,60,50,40,26,40,50,60,70,80,90]
                        astig =[-8.23e-07,6.50e-07,1.74e-06,2.31e-06,2.70e-06,2.97e-06,3.29e-06,2.15e-06,1.21e-06,4.63e-07,-3.20e-08,-4.48e-07,-8.56e-7]
                        ; modification to implement astigmatism calculation as measured with optical calibration
                        restore, lut_file  ;restored variables: zern5 (Z5), elev(elevation)
                        astig=zern5[*]
                        m_el=elev
                        ; end of modification
                        cf =  poly_fit(m_el, astig, 3, yfit=yfit)
                        zz[5] = cf[0]+cf[1]*elevation+cf[2]*elevation^2+cf[3]*elevation^3
                    endif
                    return, zz
                 end
        "IRTC2": begin
                    zz = fltarr(22)
                    el=[20,43,58,60,66, 71,84.5,60,53, 27,55,54,37,43]
                    astig=1e-6*[5.5,2.4,1.2,2,1.5,1.2,0,2,3,4.5,1.7,2,3.6,3.2]
                    vv=linfit(el, astig)
                    ;vv = linfit([20,35,90], [2.5e-6, 2e-6, 0])
                    zz[5] = vv[0]+vv[1]*elevation
                    if (elevation gt 88) then zz[5]=0
                    ;zz[5] = 0
                    return, zz
                 end
        else: begin
                log_print, "No LUT availlable for instrument: "+instrument
                return, fltarr(22)
              end

    endcase    
    
End

