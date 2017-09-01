Function slave_commands, slave, hist, CL_ACT=cl_act, FF_MATRIX=ffmat_in, VERBOSE=verbose, adsec_save=adsec_save, PMAT=pmat, FIXFORCE=fixforce, CUT=cut

	@adsec_common
	
	;set default params if not provided
	if n_elements(adsec_save) eq 0 then adsec_save=adsec
	if n_elements(cl_act) eq 0 then cl_act=adsec_save.act_w_cl
	if n_elements(ffmat_in)  eq 0 then ffmat =adsec_save.ff_matrix else ffmat=ffmat_in
	;if n_elements(cl_act) eq adsec.n_actuators do begin
	;	;TBW
	;endif else begin
    ;		err = complement(cl_act, indgen(adsec.n_actuators), wo_cl_act)
	;		if err ne 0 then message, "Error on getting wo_cl_act"
	;endelse
	;compute master and slave lists
	err = intersection(slave, cl_act, slave_cl)
	if err ne 0 then message, "Error on slave intersection"

	err = complement(slave_cl, cl_act, master_cl)
	if err ne 0 then message, "Error on slave intersection"
	
	vec = lonarr(adsec_save.n_actuators)
	vec[slave_cl]=1
	slave_on_ff = where(vec[cl_act], COMP=master_on_ff)

	;reduce ffmatrix
	tmp = ffmat[cl_act, *]
	ffmat_cl = tmp[*,cl_act]
    dummy = ff_matrix_reduction(ffmat_cl, slave_on_ff, dx, k00, k01, k10, k11)

    histf_on_mas = (ffmat ## hist)[*, master_cl]
    histp_on_mas = hist[*, master_cl]
    k00plus = pseudo_invert(k00, VERBOSE=verbose) 

	;compute slave commands for hist
	pmat = - k00plus ## k01 
    histp_on_sl = - k00plus ## k01 ## histp_on_mas

	;return slaved hist
	newhist=hist
	newhist[*, slave_cl] = histp_on_sl

	if keyword_set(FIXFORCE) then begin
		if n_elements(cut) eq 0 then cut=100
		mm = transpose(adsec.ff_p_svec) ## hist
		fm = transpose(adsec.ff_f_svec) ## ffmat ## hist
		fm[*, 0:cut]=0
		mm[*, cut+1:*]=0
		kk_cut = adsec.ff_p_svec ## mm
		fm_cut = adsec.ff_f_svec ## fm
		histf_on_mas = fm_cut[*, master_cl]
		histp_on_sl = kk_cut[*, slave_cl]
		histp_on_mas =  pseudo_invert(k11) ## (histf_on_mas - k10 ## histp_on_sl) 

		;histp_on_mas = pseudo_invert(k11 - (k10 ## pseudo_invert(k00) ## k01)) ## histf_on_mas
		;histp_on_sl = - pseudo_invert(k00) ## k01 ## histp_on_mas
	    newhist=hist*0
	    newhist[*, master_cl] = histp_on_mas
	    newhist[*, slave_cl] = histp_on_sl

	endif 
		return, newhist

End

