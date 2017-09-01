;$Id: fit_d0.pro,v 1.2 2006/11/14 11:38:50 labot Exp $
;+
;   NAME:
;    FIT_D0
;
;   PURPOSE:
;    Retrieve the d0 and Cstray parameters from unwapped data
;
;   USAGE:
;    output = fit_d0(xx, yy, time)
;
;   INPUT:
;    xx:     normalized voltage data
;    yy:     video data at the same sampling time
;    time:   time vector of x acquisition
;
;   OUTPUT:
;    output:    fitted parameters
;
;   KEYWORD:
;    None.
;
; HISTORY
;   Oct 2006:   written by M.Xompero (MX)
;               marco@arcetri.astro.it
;-


Function iper_func, x,coeff

	return, coeff[0]/(x+coeff[1]);+coeff[2]

End



Function iper_model, p, X=x, Y=y, ERR=err

	model = iper_func(x, p)
;	residue = y-model
;	wait, 0.01
	return, (y-model)/err

End



Function fit_d0, xx, yy, PARAM=param, ERR=err, RESIDUE=residue, PERR=perr

    common display_block, fit_display

	x = double(xx)
	y = double(yy)
	nx = n_elements(x)

	par = [56e-6,0.1];, 100e-6]
	if n_elements(err) eq 0 then err = replicate((machar()).eps,nx)


	yfit = mpfit('iper_model',par,functarg={X:x, Y:y, ERR:err}, STATUS=status, PARINFO=parinfo $
				  , maxiter=500, BESTNORM=best, DOF=dof, QUIET=~fit_display, PERROR=perror)

    dof  = n_elements(x) - n_elements(par) ; deg of freedom
    perr = perror * sqrt(best / dof)   ; scaled uncertainties (scalato per Chi-square)

	model = iper_func(x, yfit)
	residue = y-model

    if fit_display then begin
        print, 'Fit Signal, Normalized CHISQ: '+ string(mean(best)/sqrt(dof),format='(F6.3)')
        window, /free, TITLE="Residue FIT d0, Cs"
    	plot, x, residue/abs(y), xtitle='[Vnorm]', ytitle='[Relative Error]', xs=17
    endif
    
	return, yfit
End

