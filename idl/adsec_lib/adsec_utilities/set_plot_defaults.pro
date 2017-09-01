pro set_plot_defaults, RESET=reset, WHITE_BACKGROUND=white_bg


if keyword_set(white_bg) then begin
	!P.BACKGROUND='FFFFFF'XL
	!P.COLOR='000000'XL
endif else begin
	!P.BACKGROUND='000000'XL
	!P.COLOR='FFFFFF'XL
endelse

if keyword_set(reset) then begin
	!X.THICK=1.0
	!Y.THICK=1.0
	!P.THICK=1.0
	!X.CHARSIZE=1.0
	!Y.CHARSIZE=1.0
	!P.CHARSIZE=1.0
	!P.CHARTHICK=1.0
	!X.STYLE=0
	!Y.STYLE=0
    !X.MARGIN = [10,3]
    !Y.MARGIN = [4,3]
endif else begin
	!X.THICK=3.0
	!Y.THICK=3.0
	!P.THICK=3.0
	!X.CHARSIZE=1.4 ;1.2
	!Y.CHARSIZE=1.4 ;1.2
	!P.CHARSIZE=1.4 ;1.2
	!P.CHARTHICK=1.8;  2.0
	!X.STYLE=19
	!Y.STYLE=19
    !X.MARGIN = [11,3]
    !Y.MARGIN = [5,3]
endelse
end


