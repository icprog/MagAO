% this script initializes the DSP CHANNEL NUMBERING CONFIGURATION
% it is needed to use the readChannelVar

% Author(s): D. Pescoller
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/12/2004
% $Revision 0.2 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller

% ALL indexes must be in [0..CHANNEL_NR]
% index to use
DSP_MY_INDEX=5;

% number of channel per DSP
DSP_NUM_CHANNELS=4;

%          1           2           3           4           5           6           7
%    IncrementalNr  CrateNr      DspNr     ChannelNr   ActNr_Dsp  ActNr_Dist  ActNr_Mirror
DSP_CONF_MAT=[
           1           1           0           0           0           0           0
           2           1           0           1           1           1           1
           3           1           0           2           2           2           2
           4           1           0           3           3           3           3
   ];

