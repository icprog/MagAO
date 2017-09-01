function [DSP_CONF_MAT_col DSP_CONF_MAT DSP_MY_INDEX DSP_NUM_CHANNELS]=aoReadActuatorConf(inFileName)

fd=fopen(inFileName,'rt');
if fd==-1
   error('Invalid file name');
end

rd_state=0;
DSP_CONF_MAT_col=[];
DSP_CONF_MAT=[];
idx=1;

while 1
   line=fgetl(fd);
   if line==-1
      break;
   end
   if ~isempty(line) && length(line)>=9 && strcmp(line(1:9),'# Part 1:')
      rd_state=1;
   end
   if ~isempty(line) && line(1)~='#' && rd_state==1
      aa=textscan(line,'%s%d');
      switch cell2mat(aa{1})
         case 'IncrementalNr'
            DSP_CONF_MAT_col.inc=aa{2}+1;
         case 'CrateNr'
            DSP_CONF_MAT_col.crate=aa{2}+1;
         case 'DspNr'
            DSP_CONF_MAT_col.dsp_num=aa{2}+1;
         case 'ChannelNr'
            DSP_CONF_MAT_col.dsp_ch=aa{2}+1;
         case 'ActNr_Dsp'
            DSP_CONF_MAT_col.elec_ch=aa{2}+1;
         case 'ActNr_Dist'
            DSP_CONF_MAT_col.board_ch=aa{2}+1;
         case 'ActNr_Mirror'
            DSP_CONF_MAT_col.mirror_ch=aa{2}+1;
         case 'BoardNr'
            DSP_CONF_MAT_col.board_num=aa{2}+1;
         case 'DistributionName'
            DSP_CONF_MAT_col.distrib_name=aa{2}+1;
         case 'DistributionCh'
            DSP_CONF_MAT_col.distr_ch=aa{2}+1;
         case 'HalfMirror'
            DSP_CONF_MAT_col.half_mirr=aa{2}+1;
         case 'RingNr'
            DSP_CONF_MAT_col.ring=aa{2}+1;
         case 'ActSerNr'
            DSP_CONF_MAT_col.serNum=aa{2}+1;
         case 'Slaving'
            DSP_CONF_MAT_col.slaving=aa{2}+1;
         case 'InUse'
            DSP_CONF_MAT_col.inUse=aa{2}+1;
         otherwise
            error('Invalid column type');
      end
   end
   if ~isempty(line) && length(line)>=9 && strcmp(line(1:9),'# Part 2:')
      rd_state=2;
   end
   if ~isempty(line) && line(1)~='#' && rd_state==2
      aa=textscan(line,'%d');
      DSP_CONF_MAT(idx,:)=aa{1}';
      idx=idx+1;
   end
end

% DSP_CONF_MAT(:,DSP_CONF_MAT_col.crate)=DSP_CONF_MAT(:,DSP_CONF_MAT_col.crate)+1; % correct the crate connection column to be 1 based instead of 0 based
DSP_MY_INDEX=DSP_CONF_MAT_col.elec_ch;
DSP_NUM_CHANNELS=8;

fclose(fd);
