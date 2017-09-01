;$Id: get_slow_diagnostic.pro,v 1.6 2009/04/10 10:22:37 labot Exp $
;+
;   NAME:
;    GET_SLOW_DIAGNOSTIC
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
;    str : structure qith the diagnostic fields.
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
;   24 May 2006, DZ
;    Moved all the diagnostic structure of two positions in the shared
;    memory.
;   20 Jun 2006, MX
;    New PB data structure now matched.
;   03 Nov 2007, AR
;    Totally modified using the DiagnApp API and housekeeper_descr.txt file
;    FORCE_REBUILD_STRUCTURE keyword added
;-

function get_slow_diagnostic, diag_struct, status, FORCE_REBUILD_STRUCTURE=force_rebuild_structure $
                            , EMPTY=empty_struc

    @adsec_common
	common housekeeping_block, name_client
        name_client = process_list.housekeeper.msgd_name
if n_elements(diag_struct) eq 0 or keyword_set(force_rebuild_structure) or keyword_set(empty_struc) then begin
	root_dir = getenv("ADOPT_ROOT")
	filename = filepath(ROOT=root_dir, SUB=["conf","adsec", "current","processConf","housekeeper"],"housekeeper.conf")
	hk_conf = read_ascii_structure(filename)
	name_client = hk_conf.MyName+string(hk_conf.ID, FORMAT="(I2.2)")
	filename = filepath(ROOT=root_dir, hk_conf.VarDescriptionFile)
	h = read_ascii_structure(filename)
	n_vf = h.vars_family_num
	vf_name = strtrim(h.vars_family_name,2)
	tag_offset = n_tags(h)-n_vf
	for i=0,n_vf-1 do begin
		n = h.(tag_offset+i).qty
		if n eq 1 then var={average:0d0,status:0d0} else var={average:dblarr(n),status:dblarr(n)}
		if i eq 0 then $
			diag_struct = create_struct(vf_name[i],var) $
		else $
			diag_struct = create_struct(diag_struct, vf_name[i],var)
	endfor
	if keyword_set(empty_struc) then return, adsec_error.ok
endif

tn = tag_names(diag_struct)
n_vf = n_elements(tn)
for i=0,n_vf-1 do begin
	if (sc.host_comm ne "Dummy") then begin
		err = getdiagnvalue(name_client, tn[i], -1, -1, buf, tim=1.)
		if err ne adsec_error.ok then return, err
	endif else begin
		n=n_elements(diag_struct.(i))
		buf={average:randomu(seed,n), index:lindgen(n), status:double(fix(randomu(seed,n)*3))}
		;buf={average:findgen(n), index:lindgen(n), status:lonarr(n)}
	endelse
	diag_struct.(i).average[buf.index] = buf.average
	diag_struct.(i).status[buf.index] = buf.status
endfor
end
