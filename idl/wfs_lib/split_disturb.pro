
; Procedure to split a disturbance files in chunks
;
; pathPrefix: prefix to use for all filenames.
; inputFile: filename of data to be splitted (fits)
; outputFile: filename where the chunk of data will be written (fits)
; chunkLen: length of a single chunk
; chunkNum: which chunk to extract (0 based)
;
; inputFile is supposed to be a NxM file where N is an integer multiple of chunkLen
;
; example to extract the 3rd chunk from a disturbance acquisition that requires 4000-frames long chunks:
;
; split_disturb, '/path/to/files/', 'in.fits', 'out.fits', 4000, 2


pro split_disturb, pathPrefix, inputFile, outputFile, chunkLen, chunkNum

    data = readfits(pathPrefix + path_sep() + inputFile)
    print,chunkLen, chunkNum
    outdata = data[ chunkNum * chunkLen : (chunkNum+1)*chunkLen-1 , *]
    help,outdata, outputFile
    writefits, pathPrefix + path_sep() + outputFile, outdata
end
