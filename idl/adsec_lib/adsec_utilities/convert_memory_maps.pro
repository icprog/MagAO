Pro remove_lines, file

    text = read_text_file(file)
    bool = strmatch(text,'*<DOCUMENT>*')
    bool_end = strmatch(text,'*</DOCUMENT>*')
    if total(bool) ne 1  or total(bool_end) ne 1 then message, 'Wrong XML file: '+file
    path = file_dirname(file)
    name = file_basename(file)
    openw, unit, filepath('cut_'+name, ROOT=path), /GET_LUN
    i=where(bool)
    endi = where(bool_end)
    while i le endi do begin
        printf, unit, text[i]
        i +=1
    endwhile
    
    free_lun, unit

    return

End



;$Id: convert_memory_maps.pro,v 1.3 2008/04/08 09:37:45 labot Exp $$
;+
;   NAME:
;    CONVERT_MEMORY_MAPS
;
;   PURPOSE:
;    Translate the .xml map provided by Firmware developer in order toeasy manage it into IDL.
;
;   USAGE:
;    err=convert_memory_maps()
;
;   INPUT:
;    None. (the files will be searched in adsec_path.prog/sc.prog_ver)
;
;   OUTPUT:
;    Error code.
;
;   KEYWORDS:
;    ROOT_DIR:  new dir to retrieve the LUT to add. Default is adsec_path.pro/sc.prog_ver
;    CHECK: check if the loaded files match with the maps in the default path
;
;   HISTORY
;    Created on 16 Jan 2007
;    by Marco Xompero (MX)
;    marco@arcetri.astro.it
;   18 Apr 2007, MX
;    Added/changed parameters for programs path.
;-
Function convert_memory_maps, FORCE=force

    @adsec_common

    xml_cut_crate =  file_info(filepath('cut_CrateBCU.map.xml', ROOT=sc.bcu_prog_ver))
    xml_cut_switch = file_info(filepath('cut_SwitchBCU.map.xml', ROOT=sc.switch_prog_ver))
    xml_cut_dsp =    file_info(filepath('cut_DspMainProgram.map.xml', ROOT=sc.prog_ver))
    xml_cut_acc =    file_info(filepath('cut_AccAcquisition.map.xml', ROOT=sc.accel_prog_ver))
    
    ;map.xml file searching
    xml_crate =  file_info(filepath('CrateBCU.map.xml', ROOT=sc.bcu_prog_ver))
    xml_switch = file_info(filepath('SwitchBCU.map.xml', ROOT=sc.switch_prog_ver))
    xml_dsp =    file_info(filepath('DspMainProgram.map.xml', ROOT=sc.prog_ver))
    xml_acc =    file_info(filepath('AccAcquisition.map.xml', ROOT=sc.accel_prog_ver))
    
    ;loading the previous data and check them
    txt_crate =  file_info(filepath('bcu_dm_map.txt', ROOT=sc.bcu_prog_ver))
    txt_switch = file_info(filepath('switch_dm_map.txt', ROOT=sc.switch_prog_ver))
    txt_dsp =    file_info(filepath('dsp_dm_map.txt', ROOT=sc.prog_ver))
    txt_acc =    file_info(filepath('accel_dm_map.txt', ROOT=sc.accel_prog_ver))

    ;map.xml file conversion
    ;CrateBCU map conversion
    if xml_crate.read then begin

        remove_lines, xml_crate.name
        xml_cut_crate =  file_info(filepath('cut_CrateBCU.map.xml', ROOT=sc.bcu_prog_ver))
        if txt_crate.read then text = read_text_file(txt_crate.name)
        err = convert_dsp_xml_map(xml_crate.name, TEXT=text, FORCE=force, /CRATE)
        if err ne adsec_error.ok then return, err

    endif else begin

        message, xml_crate.name+': FILE NOT FOUND!', CONT= (sc.debug ne 1)
        return, adsec_error.invalid_file

    endelse

    if xml_switch.read then begin

        remove_lines, xml_switch.name
        xml_cut_switch = file_info(filepath('cut_SwitchBCU.map.xml', ROOT=sc.switch_prog_ver))
        if txt_switch.read then text = read_text_file(txt_switch.name)
        err = convert_dsp_xml_map(xml_switch.name, TEXT=text, FORCE=force, /SWITCH)
        if err ne adsec_error.ok then return, err

    endif else begin

        message, xml_switch.name+': FILE NOT FOUND!', CONT= (sc.debug ne 1)
        return, adsec_error.invalid_file

    endelse


    if xml_dsp.read then begin

        remove_lines, xml_dsp.name
        xml_cut_dsp =    file_info(filepath('cut_DspMainProgram.map.xml', ROOT=sc.prog_ver))
        if txt_dsp.read then text = read_text_file(txt_dsp.name)
        err = convert_dsp_xml_map(xml_dsp.name, TEXT=text, FORCE=force)
        if err ne adsec_error.ok then return, err

    endif else begin

        message, xml_dsp.name+': FILE NOT FOUND!', CONT= (sc.debug ne 1)
        return, adsec_error.invalid_file

    endelse


    if xml_acc.read then begin

        remove_lines, xml_acc.name
        xml_cut_acc =    file_info(filepath('cut_AccAcquisition.map.xml', ROOT=sc.prog_ver))
        if txt_acc.read then text = read_text_file(txt_acc.name)
        err = convert_acc_xml_map(xml_acc.name, TEXT=text, FORCE=force)
        if err ne adsec_error.ok then return, err

    endif else begin

        message, xml_acc.name+': FILE NOT FOUND!', CONT= (sc.debug ne 1)
        return, adsec_error.invalid_file

    endelse

    
    return, adsec_error.ok

End
