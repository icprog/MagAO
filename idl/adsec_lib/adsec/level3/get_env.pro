; $Id: get_env.pro,v 1.1 2004/04/08 11:18:42 marco Exp $$
;+
; NAME:
;	GET_ENV
;
; PURPOSE:
;	This function interfaces the update_enviroment routine with the most low level utils used for retrieving the data.
;
; CATEGORY:
;	Data retrieve function.
;
; CALLING SEQUENCE:
;	err =  get_env,temp_vector, volt_vector, FP_CONFIG=fp_config
;
; INPUTS:
;	
; OUTPUT:
;	temp_vector:	temperature data vector 
;	volt_vector:	volt data vector
;
; OUTPUT KEYWORDS:
;
; PROCEDURE:
;	The procedure interrogates the data acquisition device and collect the data.
;
; MODIFICATION HISTORY:
;	08 Apr 2004 	Initial creation (M.Xompero)
;
;-

function get_env,temp_vector, volt_vector

@adsec_common

; test if the connection with the field point is open, else open it
if sc.field_port.busy eq 0B then begin
	err = fp_open(sc.field_port, fp_config, VERBOSE=verbose, PORT=port_id)
	if err ne adsec_error.ok then return, err
	sc.field_port.busy=sc.true
	sc.field_port.idl_port_id = port_id
	*sc.fp_config = fp_config
endif 
temp_vector = fp_read(*sc.fp_config,0,indgen(8), VERBOSE=verbose, PORT_ID=sc.field_port.idl_port_id)
volt_vector = fp_read(*sc.fp_config,1,indgen(8), VERBOSE=verbose, PORT_ID=sc.field_port.idl_port_id) 

return,adsec_error.ok

end
