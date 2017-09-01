;$Id: who_is_master.pro,v 1.1 2007/04/11 15:41:01 labot Exp $$
;+
;   NAME:
;    WHO_IS_MASTER
;
;   PURPOSE:
;    Get the BCUID for the Master BCU of the adaptive secondary 
;
;   USAGE:
;    err = who_is_master(id)
;
;   INPUT:
;    None.
;
;   OUTPUT:
;    id:   master bcu id
;    err: error code
;   
;   HISTORY
;
;    Written by M.Xompero (MX) on 20 Mar 2007
;    Osservatorio Asrofisico di Arcetri, ITALY
;    <marco@arcetri.astro.it>
;
;-

Function who_is_master, id, crate_id

    @adsec_common
    err =  getdiagnvalue(process_list.housekeeper.msgd_name, 'BCUDigitalIOCrateIsMaster*', -1,-1,buf, time=1)
    if err ne adsec_error.ok then return, err
    if n_elements(where(buf.last)) gt 1 then begin
        message, "Only one BCU can be used as master. Fix the configuration problem and reset", CONT = (sc.debug eq 0)
        return, adsec_error.IDL_CRATEID_ERROR
    endif
    id = where(buf.last)
;   err =  getdiagnvalue('HOUSEKPR00','BCUCrateId', -1,-1,buf, time=1)
;
    
;   if err ne adsec_error.ok then return, err

    return, adsec_error.ok
    
end




