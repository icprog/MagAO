; $Id: save_mwyko.pro,v 1.6 2005/11/30 10:09:53 labot Exp $

function save_mwyko, com_data, name_radix, nsamples, ROOT_WYKO=root_wyko, MIN_PIX=min_pix, $
				INTENSITY=intensity, NDEC=ndec, FILE_LIST=file_list, TYPE=type, ROOT_DIR=root
@adsec_common


n=1
nrep=0
if n_elements(ndec) eq 0 then ndec=3

file_list=strarr(nsamples)
repeat begin
    filename = name_radix+string(n,FORMAT="(I"+strtrim(ndec,2)+"."+strtrim(ndec,2)+")")+".opd"
   	file_wyko = filepath(ROOT=root_wyko, filename)
   	file = filepath(ROOT=root, filename)

   	err = save_wyko(com_data, file_wyko)
   	if err ne adsec_error.ok then return, err

   	if n_elements(min_pix) ne 0 then begin
   	  	opd = read_opd(file, BAD_PIXEL_MASK=mask, VALUES=values, TYPE=type)
   	  	if total(mask) ge min_pix then begin
			file_list[n-1]=filename
   	  		n=n+1
   	  	endif else begin
   	  		if keyword_set(intensity) then begin
   	  			answ="a"
   	  			read, "CHECK THE INTENSITY LEVEL OF WYKO [hit return].", answ
   	  		endif
   	  	endelse
   	endif else begin
   		file_list[n-1]= filename
   		n=n+1
   	endelse
   	nrep=nrep+1
endrep until (n gt nsamples) or (nrep gt 2*nsamples)
if nrep gt 2*nsamples then return,adsec_error.generic_error else return,adsec_error.ok
end

