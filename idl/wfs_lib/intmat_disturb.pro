
; Procedure called from intmatDisturbGui.cpp, wrapper around modal_history_multi

pro intmat_disturb, modesNum, framesNum, cyclesNum, prefix, ampFile, modalFile, cmdFile, m2cFile, ampEnvelopeFile, ADDFILE = ADDFILE

    ; Build a 672 array with the first x members set from ampFile
    amp = fltarr(672)
    amp_fromfile = readfits(prefix+ path_sep() +ampFile)
    amp[0] = amp_fromfile

    ; Remove NaNs and company
    index = where(finite(amp) eq 0, count)
    if count gt 0 then amp[index] = 0

    mask = fltarr(672)
    mask[0:modesNum-1] = 1
    amp = amp*mask

    p = replicate(cyclesNum,672)

    hdr = [ "IM_TYPE", "PUSH-PULL", $
            "MODES_NO", strtrim(modesNum,2), $
            "PP_FRAMES", strtrim(framesNum,2), $
            "PP_CYCLES", strtrim(cyclesNum,2), $
            "PP_AMP_F", ampFile, $
            "M2C_F", m2cFile, $
            "AMPENV_F", ampEnvelopeFile]

    m = modal_history_multi( PATH_PREFIX = prefix, MODAL_FILENAME = modalFile, CMD_FILENAME = cmdFile, AMPVECTOR=amp, PERIODSVECTOR=p, /PUSHPULL, SEQ_LEN = framesNum*2*cyclesNum, M2C = m2cFile, ENVELOPE=ampEnvelopeFile, HDR = hdr, MULTIPLE_LEN=4000, ADDFILE = ADDFILE)
end




