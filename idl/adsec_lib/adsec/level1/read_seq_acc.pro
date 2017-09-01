; $Id: read_seq_acc.pro,v 1.2 2005/08/03 16:00:30 labot Exp $$

;+
;   NAME:
;    READ_SEQ_ACC
;   
;   PURPOSE:
;    This routine read the accumulator address as float, long64 or long80 data.
;   
;   USAGE:
;    err = read_seq_acc(act_list, addr_ch0, buf, BIT80=bit80, BIT64=bit64 )
;    
;   INPUT:
;    act_list:   int vector[N] or scalar. Vector of actuator ID numbers.
;                Use sc.all as shortcut for "all actuators".
;                (0 <= act_list <= adsec.n_actuators-1)
;
;    addr_ch0:   long scalar. Address of the channel zero to read from.
;                Even if the target actuator is not the first channel of
;                the DSP, the address must be the one of the channel zero
;                in the DSP. The program computes the correct offset
;                for channels different from the channel zero. That allows
;                to address any channel in act_list using the same address.
;
;    buf:        named variable in input. Float/ulong/long vector[n_word,N] on output.
;                Buffer filled by accumulator values read from actuators.
;
;   KEYWORDS
;    BIT80:      read the data formed by 80bit in 3 word 32 bit
;    BIT64:      read the data formed by 64bit in 3 word 32 bit
;
; HISTORY
;   14 Apr 2005
;    Written by Marco Xompero (MX)
;    Osservatorio Astrofisico di Arcetri, ITALY
;    marco@arcetri.astro.it
;   03 Aug 2005, MX
;    Fixed bug on 80bit accumulator packing.
;-
Function read_seq_acc, act_list, addr_ch0, fixed, buf, BIT80=bit80, BIT64=bit64, ISPOS=is_pos

@adsec_common

if(check_channel(act_list)) then begin
    message, "Actuator list not valid.", CONT=(sc.debug eq 0B)
    return, adsec_error.act_not_valid
endif

if act_list[0] eq sc.all_actuators then begin
    the_act_list = indgen(adsec.n_actuators)
endif else begin
    the_act_list = fix(act_list)
endelse

n_act = n_elements(the_act_list)
vect = fix(the_act_list)/adsec.n_act_per_dsp
the_dsp_list = vect[uniq(vect, sort(vect))]
if (n_elements(bit80) eq 0) and (n_elements(bit64) eq 0) then begin
    n_words=1
    err = read_seq_ch(the_act_list,addr_ch0, n_words, buf)
    return, err
endif
channels = the_act_list mod adsec.n_act_per_dsp
ch1 = where(channels eq 1, c1)
ch2 = where(channels eq 2, c2)
rev_act_list = the_act_list
if c1 gt 0 then rev_act_list[ch1] = rev_act_list[ch1]+1
if c2 gt 0 then rev_act_list[ch2] = rev_act_list[ch2]-1
n_words=2
err = read_seq_ch(rev_act_list,addr_ch0, n_words, buf64, /ulong)

if n_elements(bit64) ne 0 then begin
    buf = dblarr(n_act)
    for i =0, n_act-1 do begin
        if keyword_set(is_pos) then temp=double(long64(buf64[0:1,i],0))*fixed else temp=double(long64(buf64[0:1,i],0))*fixed
        buf[i]=temp[0]
    endfor
    return, adsec_error.ok
endif


if n_elements(bit80) ne 0 then begin
    allocate_buf=1B
    n_words=1
    ;print, 'act list ', the_act_list
    tmp = floor(the_act_list/2.)
    word_list = tmp[uniq(tmp, sort(tmp))]
    id = word_list mod (adsec.n_act_per_dsp/2)
    ;print, 'Word list ', word_list
    for i_ch = 0,adsec.n_act_per_dsp/2-1 do begin
    ;    print, 'Id ', id
        word_idx = where(id eq i_ch, count)

        if count ne 0 then begin
            tmp2 = floor(word_idx/2.)
            iddsp = tmp2[uniq(tmp2, sort(tmp2))]
    ;        print, 'dsp list', the_dsp_list
    ;        print, 'iddsp ', iddsp
            dsp = the_dsp_list[iddsp]
    ;        print, 'dsp ', dsp
    ;        print, ''
            err = read_seq_dsp(dsp, addr_ch0+adsec.n_act_per_dsp*2+i_ch, n_words, dummy_buf, /ULONG)
            if err ne adsec_error.ok then return, err
            if allocate_buf then begin
                buf2 = make_array(n_words, n_elements(word_list), TYPE=size(dummy_buf,/TYPE))
                allocate_buf = 0B
            endif
            buf2[*,word_idx] = dummy_buf
    ;        print, 'buf2 ', buf2
        endif
    endfor
    ;buf2 = transpose(ulindgen(6))*'facd'x
    ;print, buf2
    buf16 = uint(buf2,0,2,n_elements(word_list))
    buf = transpose(dblarr(n_act))
    for i = 0, n_elements(the_act_list)-1 do begin
        wordi = the_act_list[i]/2
        msb = the_act_list[i] mod 2
        index = where(word_list eq wordi)
;        print, wordi
;        print, msb
;        print, index
        buf[i] = long80double(buf64[0,i], buf64[1,i], long(buf16[msb,index]), fixed, MSB=msb)
    endfor
endif

return, adsec_error.ok
end

