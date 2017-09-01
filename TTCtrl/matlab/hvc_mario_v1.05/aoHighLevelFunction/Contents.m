% aoHighLevelFunction
%
% Files
%   aoBuffer           - M-file for aoVariable.fig
%   aoBufferReadSetup  - this reads a buffer struct from dsp
%   aoBufferStart      - aoBufferStart(selection,[firstDsp],[lastDsp])
%   aoBufferStop       - aoBufferStop(selection,[firstDsp],[lastDsp])
%   aoBufferTrigger    - aoBufferTrigger([array of buffer numbers to trigger])
%   aoBufferWriteSetup - aoBufferWriteSetup(bufferArray) writes a buffer setup to dsp
%   aoGetVar           - varStruct=aoGetVar([varName],[indexOffset])
%   aoImportVariables  - aoVariables=aoImportVariables(varArray,database)
%   aoRead             - x=aoRead((varname|varStruct),[indexArray])
%   aoReadMapFile      - reads file filename and returns an array of var 
%   aoVariable         - M-file for aoVariable.fig
%   aoWrite            - aoWrite((varname|varStruct),data,[indexArray])
%   aoBufferWaitStop   - aoBufferStart(selection,[firstDsp])
%   aoGetAddress       - address=aoGetVar([varName],[indexOffset])
%   aoClearFlash       - aoClearFlash(flashBlock,[firstDsp,[lastDsp]])
%   aoCreateBuffer     - creates an empty bufferStruct()
%   aoDownloadCode     - aoDownloadCode(firstDsp,lastDsp,filename,[startCode]) 
%   aoDspStartCode     - aoDspStartCode(firstDsp,lastDsp) 
%   aoDspStopCode      - aoDspStopCode(firstDsp,lastDsp) 
%   aoEnableDrives     - controls the drives enable
%   aoGetBCUStatus     - Gests BCU status and returns a struct with 26 fields.
%   aoProgramFlash     - aoProgramFlash(flashBlock,[filename],[firstDsp,[lastDsp]])
%   aoRele             - controls the rele board
%   aoSetDspCurrent    - aoSetDspCurrent(currentVector,[firstDsp])
%   sigGenWave         - Script per la generazione della sequenza di uscita per il DAC del signal generator.
%   aoBufferCreate      - creates a sample bufferStruct() b defined as follow:
%   aoBufferReadData    - aoBufferReadData(bufferStruct,[connectionNr]) reads buffer data 
%   aoBufferWriteData   - aoBufferWriteData(bufferStruct,data,[connectionNr]) writes buffer data 
