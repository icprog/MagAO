;$Id: bcuip2mac.pro,v 1.1 2009/11/25 08:52:46 marco Exp $$
;+
; NAME:
;    BCUIP2MAC
;
; PURPOSE:
;    Get LOCAL MAC addresses of the current interface speaking with given BCU
;
; USAGE:
;    err = bcuip2mac(ip, MAC=mac, MYIP=myip)
;
; INPUT: 
;    ip: IP number to analyze (like 192.168.0.54)
;
; OUTPUT: 
;    err: if 0 all done, if -1 an error was occurred.
;    mac: MAC of local network board passing through (like 00:40:F4:8D:88:44)
;    myip: IP address of local network board (like 192.168.0.100)
;
;-
Function compare_ip, ip1, ip2, netmask
   
    @adsec_common 
    ip1s = bytarr(4)
    ip2s = bytarr(4)
    nets = bytarr(4)
    reads, strsplit(ip1, '.', /EXT), ip1s
    reads, strsplit(ip2, '.', /EXT), ip2s
    reads, strsplit(netmask, '.', /EXT), nets
    result = 1B
    for i=0, 3 do result = result && ((ip1s[i] and nets[i]) eq (ip2s[i] and nets[i]) )
    result = result || (ip2 eq '127.0.0.1')
    if result then return, adsec_error.ok else return, -1
    
    
    
End

Function bcuip2mac, ip, MAC=mymac, MYIP=myip, MYNETMASK=mynet, MAXIF=maxif

    @adsec_common
    if n_elements(maxif) eq 0 then maxif=6
    if n_elements(ip) eq 0 then return, adsec_error.IDL_UNABLE_GET_MAC
    found = 0
    for i=0, maxif-1 do begin
        ifstr = 'eth'+strtrim(string(i), 2)
        outstr = getIfSpec_wrap(ifstr)
        split = strsplit(outstr, ' ', /EXT)
        mat = where(strmatch(split, '*-1*'), cc)
        if n_elements(split) eq 1 or cc gt 0 then begin
            log_print, ifstr+ " interface no matches for " + ip
        endif else begin
            ifc = split[0]
            mymac = split[1]
            myip = split[2]
            mynet = split[3]
            if compare_ip(myip, ip, mynet) eq adsec_error.ok then begin
                found = 1
                log_print, ifstr+ " interface matches for " + ip
                log_print, "BCU Ip  Address : "+ip
                log_print, "My  Ip  Address : "+myip
                log_print, "My  Mac Address : "+mymac
                log_print, "My  NetMask     : "+mynet
            break
            endif else begin
                log_print, ifstr+ " interface no matches for " + ip
            endelse
        endelse
    endfor

   if found then return, adsec_error.ok else return, adsec_error.IDL_UNABLE_GET_MAC

End
