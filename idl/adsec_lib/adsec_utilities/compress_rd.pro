;+
; PURPOSE:
;   Simple function to configure MAC ADDRESS, IP and PORT for BCU MASTER usage.
; HISTORY:
;   Created by Marco Xompero on 7 Feb 2007
;-


Function compress_rd, mac, ip, port
   
    ip_v = byte(fix(strsplit(ip, '.', /EXTRACT)))
    if n_elements(ip_v) ne 4 then return, -1

    mac_n =strsplit(mac, ':', /EXTRACT)
    mac_v = bytarr(6)
    if n_elements(mac_n) ne 6 then return, -2
    reads, mac_n, mac_v, format='(Z2.2)'

    port_n = byte(port,0,2)
   
    return, ulong([mac_v, ip_v, port_n], 0,3)
   
    
End
