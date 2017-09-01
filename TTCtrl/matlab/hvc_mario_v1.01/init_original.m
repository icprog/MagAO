DO_INIT        = 1;  % set to 0 if dsp board initialization should be skipped
UPDATE_MAP     = 0;  % flag per abilitare l'update della mappa delle variabili 
BCU_IP         = '192.168.0.59';

BANK_1         = hex2dec('00080000');
BANK_2         = hex2dec('00100000');

BASE_FREQ          = 106.25e6;
CNT_LOOP_DIV       = 1;
BOARD_FREQ         = BASE_FREQ*4/7;
DSP_FREQ           = BOARD_FREQ*4;
SYNC_BASE_FREQ     = BOARD_FREQ/256;
CMD_HIST_BUFF_SIZE = 65536;
FRAME_RATE         = 1000;     % desired CCD frame frequency in Hz (for CCD emulation)
CCD_TYPE           = 0;
CCD_SIZE           = 6400;    % total amount of pixels arriving from the CCD
CCD_SRAM_FRAME_ADD = hex2dec('000D0000'); % SRAM frame address 0x000E0000 (byte based)

clear sys_data;
clear aoBufferArray;

% path dei files con il codice DSP
HvcMainProgram = './firmware/HVCMainProgram_1_01.ldr';
% HvcMainProgram = 'C:\Srv\Progetti\Engineering\Lbt\Codes\HVC_dsp_MainProgram\v1.01\Release\HVCMainProgram.ldr';

run('./firmware/DSP_CONF');
load('./firmware/aoVariablesLBT.mat');
load('./firmware/aoBufferArray.mat');

% update della mappa variabili
if UPDATE_MAP
%    var=aoReadMapFile('S:\Progetti\Engineering\Lbt\Matlab\test HVC\v1.00\firmware\HVCMainProgram_1_00.map.xml');
   var=aoReadMapFile('./fimrware/HVCMainProgram.map.xml');
   aoImportVariables(var,'./firmware/aoVariablesLBT.mat');
   break;
end

sys_data.nBoards         = 1;
sys_data.nCrates         = 1;
sys_data.nDsp            = 1;
sys_data.nAct            = 4;
sys_data.hvc_board       = 0;
sys_data.cmdPreshTimeTT  = .003; % in seconds (0.2 e' il massimo tempo inseribile) - ceil(PRES_BUFF_SIZE/sys_data.cmdPreshTime/double(sys_data.cntFreq));
sys_data.ffPreshTimeTT   = .003; % in seconds (0.2 e' il massimo tempo inseribile) - ceil(PRES_BUFF_SIZE/sys_data.ffPreshTime/double(sys_data.cntFreq));
sys_data.cmdPreshTimeXY  = .2; % in seconds (0.2 e' il massimo tempo inseribile) - ceil(PRES_BUFF_SIZE/sys_data.cmdPreshTime/double(sys_data.cntFreq));
sys_data.ffPreshTimeXY   = .2; % in seconds (0.2 e' il massimo tempo inseribile) - ceil(PRES_BUFF_SIZE/sys_data.ffPreshTime/double(sys_data.cntFreq));
sys_data.cmdPreshTimeSet  = .2; % in seconds (0.2 e' il massimo tempo inseribile) - ceil(PRES_BUFF_SIZE/sys_data.cmdPreshTime/double(sys_data.cntFreq));
sys_data.ffPreshTimeSet   = .2; % in seconds (0.2 e' il massimo tempo inseribile) - ceil(PRES_BUFF_SIZE/sys_data.ffPreshTime/double(sys_data.cntFreq));
sys_data.cntFreq         = BOARD_FREQ/1012;  % frequency of control loop (initialized by Nios)
sys_data.diagBuffFreq    = sys_data.cntFreq;
sys_data.mirrorActMap    = 7;
sys_data.dspActMap       = 5;
sys_data.DACV2BitGain_c  = -32768/120.51; % questo e' un guadagno Volt2bit per il canale common
sys_data.DACBitOffset_c  = 32768;         % 32768 bias in bit del canale common
sys_data.DAC_SatVal_c    = 101;            % questa e' la tensione massima alla quale arriva il driver di tensione con gli alimentatori che abbiamo, anche se lo specchio accetterebbe di arrivare a +100V
sys_data.DAC_nSatVal_c   = 0;             % si potrebbe arrivare a -10 ma lo specchio non e' molto contento quindi meglio lasciare 0
% sys_data.DACr2BitGain_x  = -32768/120.51; % guadagno Volt2bit
% sys_data.DACr2BitGain_y  = -32768/120.51; % guadagno Volt2bit
% sys_data.DACr2BitGain_x  = -8.3364e+005;  % calibrazione per specchio ARGOS SN #001 (questi valori vangono dalla calibrazione manuale ottenuta dall script mirrorCalib)
% sys_data.DACr2BitGain_y  = -8.3171e+005;  % calibrazione per specchio ARGOS SN #001 (questi valori vangono dalla calibrazione manuale ottenuta dall script mirrorCalib)
sys_data.DACr2BitGain_x  = -5.3959e+006;  % calibrazione per specchio MAGELLAN (questi valori vangono dalla calibrazione manuale ottenuta dall script mirrorCalib)
sys_data.DACr2BitGain_y  = -5.4712e+006;  % calibrazione per specchio MAGELLAN (questi valori vangono dalla calibrazione manuale ottenuta dall script mirrorCalib)
sys_data.DACm2BitGain_x  = -2.1566e+008;    % calibrazione per translation stage Piezosystem Jena PXY 200 [m/bit]
sys_data.DACm2BitGain_y  = -2.1566e+008;    % calibrazione per translation stage Piezosystem Jena PXY 200 [m/bit]
sys_data.DACBitOffset    = 32768;         % 32768 bias in bit per i due canali xy
sys_data.DACN2AGain      = 1;             % not used sulle schede HVC, mettiamo semplicemente 1
sys_data.DAC_SatmVal     = 120e-6;        % corsa translation stage Piezosystem Jena PXY 200[m]. Attenzione, NON e' conservativa, per effetto dell'isteresi a volte non arriva in posizione. 115e-6 ok
sys_data.DAC_nSatmVal    = 0;             % corsa translation stage Piezosystem Jena PXY 200[m]. Attenzione, NON e' conservativa, per effetto dell'isteresi a volte non arriva in posizione. 5e-6 ok
sys_data.DAC_SatVal     = -25000/sys_data.DACr2BitGain_x; % questa e' la corsa massima dello specchio (32768-8192) infatti i primi 8192 valori sono saturati a ~90V
sys_data.DAC_nSatVal    = 0;             % sotto a zero non andiamo
% sys_data.DAC_SatVal      = 101;           % -25000/sys_data.DACr2BitGain_x; % questa e' la corsa massima dello specchio (32768-8192) infatti i primi 8192 valori sono saturati a ~90V
% sys_data.DAC_nSatVal     = 0;             % sotto a zero non andiamo
sys_data.DACMaxDelta     = 100;           % salto massimo sul DAC per evitare di instabilizzare l'amplificatore di alta tensione
sys_data.DACDelayTime    = 4.5e-6;        % in sec (SOCCLK = CCLK/2) - sys_data.DACDelayTime*DSP_FREQ/2 - 2.8us e' il tempo minimo per il calcolo del loop di controllo, controllare se pero' ci sono dei jitter in particolari condizioni che richiedono di alzare questo valore;
sys_data.delayAcc        = 0.02;          % in sec - sys_data.delayAcc*sys_data.cntFreq;
sys_data.averageAcc      = 0.1;           % in sec - sys_data.averageAcc*sys_data.cntFreq;
sys_data.SmoothStep      = 0.01;
sys_data.CLErrTrigLevel  = 0.001;
% sys_data.ADCbit2rad_x    = 1.8251e-006;   % calibrazione per specchio ARGOS SN #001 (questi valori vangono dalla calibrazione manuale ottenuta dall script mirrorCalib)
% sys_data.ADCbit2rad_y    = 1.7620e-006;   % calibrazione per specchio ARGOS SN #001 (questi valori vangono dalla calibrazione manuale ottenuta dall script mirrorCalib)
% sys_data.ADCradOffset_x  = -.024;         % calibrazione per specchio ARGOS SN #001 (questi valori vangono dalla calibrazione manuale ottenuta dall script mirrorCalib)
% sys_data.ADCradOffset_y  = -.026;         % calibrazione per specchio ARGOS SN #001 (questi valori vangono dalla calibrazione manuale ottenuta dall script mirrorCalib)
sys_data.ADCbit2rad_x    = 1.7486e-007;   % calibrazione per specchio MAGELLAN (questi valori vangono dalla calibrazione manuale ottenuta dall script mirrorCalib)
sys_data.ADCbit2rad_y    = 1.7485e-007;   % calibrazione per specchio MAGELLAN (questi valori vangono dalla calibrazione manuale ottenuta dall script mirrorCalib)
sys_data.ADCradOffset_x  = -.0025;        % calibrazione per specchio MAGELLAN (questi valori vangono dalla calibrazione manuale ottenuta dall script mirrorCalib)
sys_data.ADCradOffset_y  = -.0025;        % calibrazione per specchio MAGELLAN (questi valori vangono dalla calibrazione manuale ottenuta dall script mirrorCalib)
sys_data.ADCbit2m_x      = 5.1942e-009;   % calibrazione per translation stage Piezosystem Jena PXY 200 [m/bit]
sys_data.ADCbit2m_y      = 5.1942e-009;   % calibrazione per translation stage Piezosystem Jena PXY 200 [m/bit]
sys_data.ADCmOffset_x    = -37e-6-60e-6;  % calibrazione per translation stage Piezosystem Jena PXY 200 [m/bit]
sys_data.ADCmOffset_y    = -71e-6-60e-6;  % calibrazione per translation stage Piezosystem Jena PXY 200 [m/bit]
sys_data.pre_loop_gain   = [1 1 0 0];
sys_data.post_loop_gain  = [1 1 0 0];
% sys_data.bias_current    = [46 46 46 0]; % per lavorare in volt
sys_data.bias_command_TT = [0 0 0 0];
sys_data.bias_current_TT = [-12507/sys_data.DACr2BitGain_x -12507/sys_data.DACr2BitGain_y 92 0]; % circa 46 46 92
sys_data.bias_command_XY = [0 0 0 0];
sys_data.bias_current_XY = [58e-6 58e-6 0 0]; % per Piezosystem Jena PXY 200
sys_data.pres_buff_size  = 16384;
sys_data.syncA_gain = 0;

if DO_INIT

   AOConnect(BCU_IP);

   stopTimeHistory;
   mirrorRip;

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % scarico del codice DSP                       %
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   fprintf('Downloading HVC-DSP code...\n');
   aoDownloadCode(sys_data.hvc_board(1),sys_data.hvc_board(end),HvcMainProgram,0);

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % firmware variables initilization             %
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   disp('Initilizing HVC variables...');

   aoWrite('hvc_delay_DAC',round(sys_data.DACDelayTime*DSP_FREQ/2)*ones(sys_data.nDsp,1),sys_data.hvc_board);
   aoWrite('hvc_DACMaxDelta',sys_data.DACMaxDelta*ones(sys_data.nDsp,1),sys_data.hvc_board);
   aoWrite('hvc_TT1_delay_acc',round(sys_data.delayAcc*sys_data.cntFreq)*ones(sys_data.nDsp,1),sys_data.hvc_board);
   aoWrite('hvc_TT1_num_samples',round(sys_data.averageAcc*sys_data.cntFreq)*ones(sys_data.nDsp,1),sys_data.hvc_board);
   aoWrite('hvc_TT1_inv_num_samples',1/round(sys_data.averageAcc*sys_data.cntFreq)*ones(sys_data.nDsp,1),sys_data.hvc_board);
   aoWrite('hvc_TT2_delay_acc',round(sys_data.delayAcc*sys_data.cntFreq)*ones(sys_data.nDsp,1),sys_data.hvc_board);
   aoWrite('hvc_TT2_num_samples',round(sys_data.averageAcc*sys_data.cntFreq)*ones(sys_data.nDsp,1),sys_data.hvc_board);
   aoWrite('hvc_TT2_inv_num_samples',1/round(sys_data.averageAcc*sys_data.cntFreq)*ones(sys_data.nDsp,1),sys_data.hvc_board);

   aoWrite('hvc_TT1_step_ptr_preshaper_cmd',round(sys_data.pres_buff_size/sys_data.cmdPreshTimeTT/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
   aoWrite('hvc_TT1_step_ptr_preshaper_curr',round(sys_data.pres_buff_size/sys_data.ffPreshTimeTT/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
   aoWrite('hvc_TT1_dist_A_coeff',  [sys_data.ADCbit2rad_x   sys_data.ADCbit2rad_y   0                       0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT1_dist_B_coeff',  [sys_data.ADCradOffset_x sys_data.ADCradOffset_y 0                       0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT1_sat_DAC_value', [sys_data.DAC_SatVal     sys_data.DAC_SatVal     sys_data.DAC_SatVal_c   0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT1_nsat_DAC_value',[sys_data.DAC_nSatVal    sys_data.DAC_nSatVal    sys_data.DAC_nSatVal_c  0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT1_DAC_N2A_gain',  [sys_data.DACN2AGain     sys_data.DACN2AGain     sys_data.DACN2AGain     0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT1_DAC_A2bit_gain',[sys_data.DACr2BitGain_x sys_data.DACr2BitGain_y sys_data.DACV2BitGain_c 0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT1_DAC_bit_offset',[sys_data.DACBitOffset   sys_data.DACBitOffset   sys_data.DACBitOffset_c 0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT1_pre_smoothed_step',sys_data.SmoothStep*ones(sys_data.nAct,1),0:(sys_data.nAct-1));
   aoWrite('hvc_TT1_post_smoothed_step',sys_data.SmoothStep*ones(sys_data.nAct,1),0:(sys_data.nAct-1));
   aoWrite('hvc_TT1CLErrorTrigLevel',sys_data.CLErrTrigLevel*ones(sys_data.nAct,1),0:(sys_data.nAct-1));
   aoWrite('hvc_TT2_step_ptr_preshaper_cmd',round(sys_data.pres_buff_size/sys_data.cmdPreshTimeXY/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
   aoWrite('hvc_TT2_step_ptr_preshaper_curr',round(sys_data.pres_buff_size/sys_data.ffPreshTimeXY/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
   aoWrite('hvc_TT2_dist_A_coeff',  [sys_data.ADCbit2m_x     sys_data.ADCbit2m_y     0                       0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT2_dist_B_coeff',  [sys_data.ADCmOffset_x   sys_data.ADCmOffset_y   0                       0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT2_sat_DAC_value', [sys_data.DAC_SatmVal    sys_data.DAC_SatmVal    0                       0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT2_nsat_DAC_value',[sys_data.DAC_nSatmVal   sys_data.DAC_nSatmVal   0                       0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT2_DAC_N2A_gain',  [sys_data.DACN2AGain     sys_data.DACN2AGain     sys_data.DACN2AGain     0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT2_DAC_A2bit_gain',[sys_data.DACm2BitGain_x sys_data.DACm2BitGain_y 0                       0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT2_DAC_bit_offset',[sys_data.DACBitOffset   sys_data.DACBitOffset   sys_data.DACBitOffset_c 0],0:(sys_data.nAct-1));
   aoWrite('hvc_TT2_pre_smoothed_step',sys_data.SmoothStep*ones(sys_data.nAct,1),0:(sys_data.nAct-1));
   aoWrite('hvc_TT2_post_smoothed_step',sys_data.SmoothStep*ones(sys_data.nAct,1),0:(sys_data.nAct-1));
   aoWrite('hvc_TT2CLErrorTrigLevel',sys_data.CLErrTrigLevel*ones(sys_data.nAct,1),0:(sys_data.nAct-1));

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % error & position filter coeffs (Ziegler�Nichols method) %
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   sys_data.coeff=zeros(4,9);
%    ku=0.05;pu=4.4e-4;
   ku=0.01;pu=4.4e-4;
%        pid_K   pid_I          pid_D        pid_a
%    p=[ 0.8*ku  2.0*0.6*ku/pu  0.6*ku*pu/8  2*pi*10e3]; % parametri classici per il filtro di Ziegler�Nichols
%    p=[ 0.6*ku  2.2*0.8*ku/pu  0.8*ku*pu/8  2*pi*10e3]; % parametri usati per il magellan
%    p=[ 0.1*ku 1.5*ku/pu  0.15*ku*pu  2*pi*10e3]; % parametri usati per argos
   p=0.01*[ 0.01*ku 20.0*ku/pu  0.01*ku*pu  2*pi*10e3]; % parametri usati per argos
   sys_data.pid_coeff=p;
   pid_sys=tf([p(1)+p(3)*p(4)  p(1)*p(4)+p(2)   p(2)*p(4)],[1 p(4) 0]);
   pid_sysD=c2d(pid_sys,1/sys_data.cntFreq,'tustin');
   [num,den]=tfdata(pid_sysD,'v');
   sys_data.coeff(1,:)=[num(1),0,0,num(3),num(2),0,0,den(3),den(2)];
   sys_data.coeff(2,:)=[num(1),0,0,num(3),num(2),0,0,den(3),den(2)];
   coeffVector=reshape(sys_data.coeff,1,4*9);
   aoWrite('hvc_TT1_pos_coeff',coeffVector,sys_data.hvc_board);

   ku=0.3;pu=3e-3;
   p=[ 0.8*ku  2.0*0.6*ku/pu  0.6*ku*pu/8  2*pi*10e3]; p=p*.05; % parametri usati per Piezosystem Jena PXY200. Controllo morbido, � quasi statico!
   sys_data.pid_coeff=p;
   pid_sys=tf([p(1)+p(3)*p(4)  p(1)*p(4)+p(2)   p(2)*p(4)],[1 p(4) 0]);
   pid_sysD=c2d(pid_sys,1/sys_data.cntFreq,'tustin');
   [num,den]=tfdata(pid_sysD,'v');
   sys_data.coeff(1,:)=[num(1),0,0,num(3),num(2),0,0,den(3),den(2)];
   sys_data.coeff(2,:)=[num(1),0,0,num(3),num(2),0,0,den(3),den(2)];
   coeffVector=reshape(sys_data.coeff,1,4*9);
   aoWrite('hvc_TT2_pos_coeff',coeffVector,sys_data.hvc_board);

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % inizializzazione delle time history ad ampiezza unitaria %
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   CmdHistA=1*(sin(-pi:2*pi/(CMD_HIST_BUFF_SIZE-1):pi));
   aoWrite('hvc_CmdHistVect',CmdHistA,sys_data.hvc_board);
%    aoWrite('hvc_TT2CmdHistA',CmdHistA,sys_data.hvc_board);
%    CmdHistB_ch0=3e-3*[-ones(1,1598) -cos(0:pi/(900-1):pi) ones(1,3196) cos(0:pi/(900-1):pi) -ones(1,1598)];
%    CmdHistB_ch0=3e-3*[-ones(1,1148) -cos(0:pi/(1800-1):pi) ones(1,2296) cos(0:pi/(1800-1):pi) -ones(1,1148)]; % onda quadra piu' smooth
%    CmdHistB_ch1=zeros(1,CMD_HIST_BUFF_SIZE);
%    shape=[CmdHistB_ch0;CmdHistB_ch1];
%    shape=reshape(shape,1,2*CMD_HIST_BUFF_SIZE);
%    aoWrite('hvc_TT1CmdHistB',shape,sys_data.hvc_board);
%    aoWrite('hvc_TT2CmdHistB',shape,sys_data.hvc_board);

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % inizializza il CCD emulator                  %
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   %mgp_op_wrsame_ccdi(255,255,1,0,0);                            % disable CCD logic interface
   %mgp_op_wrsame_ccdi(255,255,1,1,uint32(BOARD_FREQ*1e-6*FRAME_RATE)); % frame prescaler
   %mgp_op_wrsame_ccdi(255,255,1,2,CCD_SRAM_FRAME_ADD);           % frame address (SRAM address byte-based)
   %mgp_op_wrsame_ccdi(255,255,1,3,CCD_TYPE);

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % inizializzazione del preshaper               %
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   oneShape=(sin(-pi/2:2*pi/(sys_data.pres_buff_size*2-1):pi/2)+1)/2; shape=repmat(oneShape,1,size(sys_data.hvc_board,2));
   aoWrite('hvc_preshaper_cmd_buffer',shape,sys_data.hvc_board);
   aoWrite('hvc_preshaper_curr_buffer',shape,sys_data.hvc_board);

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % faccio partire il codice dei DSPs            %
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   aoDspStartCode(sys_data.hvc_board(1),sys_data.hvc_board(end));

   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   % abilitazione dell'isr                        %
   %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   aoWrite('hvc_EnableIsr',1*ones(size(sys_data.hvc_board)),sys_data.hvc_board);

end

disp('init done!!!');
