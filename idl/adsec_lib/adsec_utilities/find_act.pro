function find_act, x, SHOW=show

@adsec_common
;!P.BACKGROUND='FFFFFF'XL
;  !P.COLOR='000000'XL
;  !P.CHARSIZE=1
;  !P.CHARTHICK=1.2
;  !P.THICK=2
;  !X.STYLE = 19
;  !Y.STYLE = 19
;  !X.THICK = 2
;  !Y.THICK = 2
  loadct, 23
  
  nx        =  n_elements(x)
  
  nact      =adsec.N_ACTUATORS       ;INT            672
  ncrates   =adsec.N_CRATES          ;INT              6
  nbcu      =adsec.N_BCU             ;INT              6
  nboard    =adsec.N_BOARD_PER_BCU   ;INT             14
  ndsp      =adsec.N_DSP_PER_BOARD   ;INT              2
  nact_dsp  =adsec.N_ACT_PER_DSP     ;INT              4
  id        =adsec.act_w_cl
  out       =adsec.act_wo_cl
  distr_side=['Left','Right']
  distr_names=['C0', 'C1','B2','B3','D4','D5','D6','B7','B8','B9','D10','A11','A12','A13']
  ;distr_type= ['C',
  ;strsplit(distr_names,1, /ex)
  distr_nboard=[[1,1,0,0,0,0,0,0,0,0,0,0,0,0],$;C0
                [1,1,0,0,0,0,0,0,0,0,0,0,0,0],$;C1
                [0,0,1,1,0,0,0,1,1,1,0,0,0,0],$;B2
                [0,0,1,1,0,0,0,1,1,1,0,0,0,0],$;B3
                [0,0,0,0,1,1,1,0,0,0,1,0,0,0],$;D4
                [0,0,0,0,1,1,1,0,0,0,1,0,0,0],$;D5
                [0,0,0,0,1,1,1,0,0,0,1,0,0,0],$;D6
                [0,0,1,1,0,0,0,1,1,1,0,0,0,0],$;B7
                [0,0,1,1,0,0,0,1,1,1,0,0,0,0],$;B8
                [0,0,1,1,0,0,0,1,1,1,0,0,0,0],$;B9
                [0,0,0,0,1,1,1,0,0,0,1,0,0,0],$;D10
                [0,0,0,0,0,0,0,0,0,0,0,1,1,1],$;A11
                [0,0,0,0,0,0,0,0,0,0,0,1,1,1],$;A12
                [0,0,0,0,0,0,0,0,0,0,0,1,1,1]];A13
  xcrate  =  intarr(nx)
  xboard  =  xcrate
  xdsp    =  xcrate
  xdistr  =  strarr(nx)
  xmirr   =  intarr(nx) 
  for i=0, nx-1 do begin
    xmirr[i]   =  adsec.act2mir_act[x[i]]
    crate=x[i]/(nact/nbcu)
    board=(x[i] mod (nact/nbcu))/ (nact_dsp*ndsp);(nboard)
    dsp=(x[i] mod (nact/nbcu)) /nact_dsp
    side=distr_side[crate mod 2]
    distr_id=distr_names[board]
    distr=side+distr_names[board]
   
    crate0act=crate*(nact/nbcu)
    board0act=crate0act+board*nact_dsp*ndsp
    xcrate[i]  =  crate
    xboard[i]  =  board+crate*nboard
    xdsp[i]    =  dsp+xboard[i]*ndsp
    xdistr[i]  =  distr_id
      if (keyword_set(show))then begin
            pp=rebin(distr_nboard[*,board],112,1, /sample)
            distr_act=crate0act+where(pp eq 1)
            
            board_act=indgen(nact_dsp*ndsp)+board0act
            pos=1+indgen(nact)/(nact/nbcu)
            pos=intarr(nact)
            pos[out]=0
            pos[distr_act]=7
            pos[board_act]=9
            pos[x[i]]=11
            
            display, (fix(pos)), /no_n, title='Act#'+strtrim(string(x[i]),2)+', Mir#='+strtrim(xmirr[i],2)+', Crate#'+$
                    strtrim(string(crate),2)+',!CBoard#'+strtrim(string(board),2)+', DistrBoard= '+distr,$
                     ymargin=[4,4]
            
            print, 'Distribution board name='+distr
            print, 'Act in distribution:'
            print, board_act
            print, 'Crate='+string(crate)+'   Board='+string(board)+'    Dsp='+string(dsp)
            print, 'First Act in crate='+string(crate0act)+'   First Act in board='+string(board0act)
      endif
  endfor
locations=create_struct('crate',xcrate,'board',xboard, 'dsp', xdsp, 'distrib',xdistr)

return, locations
end