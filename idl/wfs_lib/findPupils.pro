function findPupils, frame, th1, th2, fourPupils=fourPupils, chdir=chdir, method=method
    split =  n_elements(fourPupils) ne 0 ? 1B : 0B
    if n_elements(chdir) ne 0 then cd, chdir, curr=curr
    
     

    distanza_centri, frame, m, w, diagonale_lato, media_d, $
        /QUIET, TH1=float(th1), TH2=float(th2), SPLIT=split
    
    if n_elements(chdir) ne 0 then cd, curr

    return, {center:m[*,1:2], side:w, diameter:transpose(media_d)} 

end
