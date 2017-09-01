
pro orthog, m2cfile, mode_list, m2c_new

; Change variabile because we will modify it
mmlist = mode_list

m2c_orig = readfits(m2cfile)
m2c_new = m2c_orig
nmodes =n_elements(mmlist)

; Check ortogonality

for i=0,nmodes-1 do begin

 print, 'Orthogonalizing mode ',mmlist[0]

 m2c = m2c_orig
 m2c[mmlist,*]=0
 z = where(total(m2c,2) ne 0)
 m2c = m2c[z,*]

 svdc, m2c, w, u, v

 c0 = m2c_orig[ mmlist[0],*]
 c0tilde = c0 # transpose(u)
 cc = transpose(u) # transpose(c0tilde)
 c0_new = c0-cc

 m2c_new[ mmlist[0],*] = c0_new
 if i ne nmodes-1 then mmlist = mmlist[1:*]
endfor

end

