SC_BCU_IP      = '192.168.0.59';

addpath('../aoHighLevelFunction');
addpath('../aoHighLevelFunction/lib');
addpath('../aoLowLevelComunication');
addpath('../aoLowLevelComunication/lib');
addpath('../general/tcp_udp_ip');
addpath('../general/matematiche');

AO_VARIABLE_DEFINE();

% TS101 bank names
BANK_0             = hex2dec('00000000');
BANK_1             = hex2dec('00080000');
BANK_2             = hex2dec('00100000');

clear sys_data;
clear aoBufferArray;

% path dei files con il codice DSP
HVCMainProgramPath = './firmware/';
HVCMainProgram     = [HVCMainProgramPath 'HVCMainProgram.ldr'];

run('./firmware/DSP_CONF');
load('./firmware/aoVariablesLBT.mat');

% Description of param selector word
% 0x00000001 parameters block selection 0 = block #0, 1 = block #1 (relevant for ARGOS & RTR)
% 0x00000002 slopes normalizzate con la sommatoria dei 4 pixel (relevant for SC)
% 0x00000004 slopes normalizzate con la sommatoria di tutti i pixel del passo precedente (relevant for SC)
% 0x00000008 slopes normalizzate con la costante fissa (relevant for SC)
% 0x00000010 enable (1) o disable (0) il linearizzatore di slopes (relevant for SC)
% 0x00000020 enable (1) or disable (0) the delta command calculation using the actual position of the mirror respect to the old command (relevant for RTR)
% 0x00000040 enable (1) or disable (0) the diagnostic data storage to the SDRAM (relevant for SC, ARGOS & RTR) – excluding the pixel frames
% 0x00000080 enable (1) or disable (0) the fast-link commands used to send the slope vector to the reconstructor (relevant for SC & ARGOS)
% 0x00000100 select between the MDC (0) or MCF (1) reconstructor algorithm (relevant for RTR)
% 0x00000200 reserved (for internal enable use only)
% 0x00000400 reserved (for internal enable use only)
% 0x00000800 enable (1) or disable (0) the DM accelerometers acquisition (relevant for RTR)
% 0x00001000 enable (1) or disable (0) the pixel frame transfer from the DSP16ch SDRAM to the BCU SDRAM (relevant for ARGOS)
% 0x00002000 enable (1) or disable (0) the real time commands to the HVC board

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% parametri principali generali
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sys_data.baseFreq        = 106.25e6;
sys_data.boardFreq       = sys_data.baseFreq/7*4;
sys_data.dspFreq         = sys_data.boardFreq*4;
sys_data.bcu_board       = 255;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% parametri principali per la scheda HVC (loop di controllo degli specchi di TT)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sys_data.hvcCntFreq      = sys_data.boardFreq/1012;  % frequency of HVC local control loop (initialized by Nios)
sys_data.hvcDiagFreq     = sys_data.hvcCntFreq;
sys_data.hvc_nAct        = 4;
sys_data.hvc_board       = 0;
sys_data.mirrorActMap    = 7;
sys_data.dspActMap       = 5;
sys_data.DACRefVoltage   = 5;                               % voltage reference of DAC
sys_data.DSPAmpGain      = 2;                               % gain amplification on DSP board
sys_data.HVAmpGain       = 10;                              % gain amplification on HVC board (new model)
% sys_data.HVAmpGain       = 12.05;                              % gain amplification on HVC board (old model)
sys_data.commonVoltage   = 90;                              % tensione del canale common
sys_data.DACV2BitGain_c  = -32768/sys_data.DACRefVoltage/sys_data.DSPAmpGain/sys_data.HVAmpGain; % questo e' un guadagno Volt2bit per il canale common
sys_data.DACBitOffset_c  = 32768;                           % 32768 bias in bit del canale common
sys_data.DAC_SatVal_c    = sys_data.commonVoltage + 5;
sys_data.DAC_nSatVal_c   = -6;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% calibrazione PI S-331.2SL
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sys_data.DACr2BitGain_x  = (140/4.2e-3)*sys_data.DACV2BitGain_c; % questo e' il guadagno rad2bit calcolato per lo specchio PI S-331.2SL che ha uno stroke di 4.2mrad su 140V di tensione (in uso al magellan)
sys_data.DACr2BitGain_y  = (140/4.2e-3)*sys_data.DACV2BitGain_c; % questo e' il guadagno rad2bit calcolato per lo specchio PI S-331.2SL che ha uno stroke di 4.2mrad su 140V di tensione (in uso al magellan)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% calibrazione PI S-330.4SL
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% sys_data.DACr2BitGain_x  = (140/7.0e-3)*sys_data.DACV2BitGain_c; % questo e' il guadagno rad2bit calcolato per lo specchio PI S-331.2SL che ha uno stroke di 4.2mrad su 140V di tensione (in uso al magellan)
% sys_data.DACr2BitGain_y  = (140/7.0e-3)*sys_data.DACV2BitGain_c; % questo e' il guadagno rad2bit calcolato per lo specchio PI S-331.2SL che ha uno stroke di 4.2mrad su 140V di tensione (in uso al magellan)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% translation stage Piezosystem Jena PXY 200 con nuova scheda HV
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sys_data.DACm2BitGain_x  = (150/200e-6)*sys_data.DACV2BitGain_c; % questo e' il guadagno m2bit calcolato per il translation stage Piezosystem Jena PXY 200 che ha uno stroke di 200um su 150V di tensione (in uso al magellan)
sys_data.DACm2BitGain_y  = (150/200e-6)*sys_data.DACV2BitGain_c; % questo e' il guadagno m2bit calcolato per il translation stage Piezosystem Jena PXY 200 che ha uno stroke di 200um su 150V di tensione (in uso al magellan)
sys_data.DACBitOffset    = 32768;                     % 32768 bias in bit per i due canali xy
sys_data.TTMinCmd        =  -2.5e-3;                  % in rad
sys_data.TTMaxCmd        =  +2.5e-3;                  % in rad
sys_data.TTMaxDeltaCmd   =   1.0e-3;                  % in rad
sys_data.XYMinCmd        = -80.0e-6;                  % in meters
sys_data.XYMaxCmd        = +80.0e-6;                  % in meters
sys_data.XYMaxDeltaCmd   =  10.0e-6;                  % in meters
sys_data.DAC_SatValTT_x  =  0.95*sys_data.commonVoltage*sys_data.DACV2BitGain_c/sys_data.DACr2BitGain_x; % in rad come valore massimo prendo il 90% della tensione di common
sys_data.DAC_nSatValTT_x = -0.95*sys_data.commonVoltage*sys_data.DACV2BitGain_c/sys_data.DACr2BitGain_x; % in rad come valore minimo prendo il 90% della tensione di common
sys_data.DAC_SatValTT_y  =  0.95*sys_data.commonVoltage*sys_data.DACV2BitGain_c/sys_data.DACr2BitGain_y; % in rad
sys_data.DAC_nSatValTT_y = -0.95*sys_data.commonVoltage*sys_data.DACV2BitGain_c/sys_data.DACr2BitGain_y; % in rad
sys_data.DAC_SatValXY_x  =  96*sys_data.DACV2BitGain_c/sys_data.DACm2BitGain_x; % in rad come valore massimo prendo il 90% della tensione di common
sys_data.DAC_nSatValXY_x =  -6*sys_data.DACV2BitGain_c/sys_data.DACm2BitGain_x; % in rad come valore minimo prendo il 90% della tensione di common
sys_data.DAC_SatValXY_y  =  96*sys_data.DACV2BitGain_c/sys_data.DACm2BitGain_y; % in rad
sys_data.DAC_nSatValXY_y =  -6*sys_data.DACV2BitGain_c/sys_data.DACm2BitGain_y; % in rad
sys_data.DACN2AGain      = 1;                         % not used per il canale common
sys_data.DACDelayTime    = 2.6e-6;                    % in sec   sys_data.DACDelayTime*DSP_FREQ - 2.8us e' il tempo minimo per il calcolo del loop di controllo, controllare se pero' ci sono dei jitter in particolari condizioni che richiedono di alzare questo valore;
sys_data.delayAcc        = 0.02;                      % in sec - sys_data.delayAcc*sys_data.hvcCntFreq;
sys_data.averageAcc      = 0.1;                       % in sec - sys_data.averageAcc*sys_data.hvcCntFreq;
sys_data.SmoothStep      = 0.01;
sys_data.CLErrTrigLevel  = 0.001;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% specchio PI S-331.2S con vecchia scheda HV
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% sys_data.ADCbit2rad_x   = 7.675793e-08;    % questo e' un guadagno rad2bit per il canale x misurato partendo dal guadagno V2rad tirato fuori dal data sheet (vedi sys_data.DACr2BitGain_x) e imponendo che rotazione in tensione corrisponde alla stessa rotazione in starin gauge
% sys_data.ADCbit2rad_y   = 7.561029e-08;    % questo e' un guadagno rad2bit per il canale x misurato partendo dal guadagno V2rad tirato fuori dal data sheet (vedi sys_data.DACr2BitGain_x) e imponendo che rotazione in tensione corrisponde alla stessa rotazione in starin gauge
% sys_data.ADCradOffset_x = -2.038078e-03;   % questo e' il valore dello strain gauge letto quando accendo lo specchio e setto la bias_current ( cosi' metto lo specchio a mid range)
% sys_data.ADCradOffset_y = -2.760928e-03;   % questo e' il valore dello strain gauge letto quando accendo lo specchio e setto la bias_current ( cosi' metto lo specchio a mid range)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% specchio PI S-331.2S con nuova scheda HV
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sys_data.ADCbit2rad_x    = 1.191339e-07;     % questo e' un guadagno rad2bit per il canale y misurato con uno step sul muro
sys_data.ADCbit2rad_y    = 1.191339e-07;     % questo e' un guadagno rad2bit per il canale y misurato con uno step sul muro
sys_data.ADCradOffset_x  = -2.797266e-03;    % questo e' il valore dello strain gauge letto quando accendo lo specchio e setto la bias_current ( cosi' metto lo specchio a mid range)
sys_data.ADCradOffset_y  = -3.939659e-03;    % questo e' il valore dello strain gauge letto quando accendo lo specchio e setto la bias_current ( cosi' metto lo specchio a mid range)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% specchio S-330.4SL con nuova scheda HV
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% sys_data.ADCbit2rad_x    = 1.422195e-007;     % questo e' un guadagno rad2bit per il canale y misurato con uno step sul muro
% sys_data.ADCbit2rad_y    = 1.119040e-007;       % questo e' un guadagno rad2bit per il canale y misurato con uno step sul muro
% sys_data.ADCradOffset_x  = -1.395445e-003;    % questo e' il valore dello strain gauge letto quando accendo lo specchio e setto la bias_current ( cosi' metto lo specchio a mid range)
% sys_data.ADCradOffset_y  = -1.036888e-003;    % questo e' il valore dello strain gauge letto quando accendo lo specchio e setto la bias_current ( cosi' metto lo specchio a mid range)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% translation stage Piezosystem Jena PXY 200 con nuova scheda HV
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sys_data.ADCbit2m_x      = 5.713620e-009;    % calibrazione per translation stage Piezosystem Jena PXY 200 [m/bit]
sys_data.ADCbit2m_y      = 5.713620e-009;    % calibrazione per translation stage Piezosystem Jena PXY 200 [m/bit]
sys_data.ADCmOffset_x    = -7.971965e-005;   % calibrazione per translation stage Piezosystem Jena PXY 200 [m/bit]
sys_data.ADCmOffset_y    = -1.138219e-004;   % calibrazione per translation stage Piezosystem Jena PXY 200 [m/bit]

sys_data.pre_loop_gain   = [1 1 0 0];
sys_data.post_loop_gain  = [1 1 0 0];
sys_data.bias_current_TT = [sys_data.commonVoltage/2*sys_data.DACV2BitGain_c/sys_data.DACr2BitGain_x ...
                            sys_data.commonVoltage/2*sys_data.DACV2BitGain_c/sys_data.DACr2BitGain_y ...
                            sys_data.commonVoltage 0];  % come bias current setto meta tensione common sui due canali pilotati e common sul canale fisso
sys_data.bias_command_TT = [0 0 0 0];
sys_data.bias_current_XY = [45*sys_data.DACV2BitGain_c/sys_data.DACm2BitGain_x ...
                            45*sys_data.DACV2BitGain_c/sys_data.DACm2BitGain_y ...
                            0 0];                       % bias current setto meta tensione common sui due canali pilotati per Piezosystem Jena PXY 200
sys_data.bias_command_XY = [0 0 0 0];

sys_data.cmdPreshTimeTT  = .2; % in seconds (0.2 e' il massimo tempo inseribile) - ceil(PRES_BUFF_SIZE/sys_data.cmdPreshTime/double(sys_data.cntFreq));
sys_data.ffPreshTimeTT   = .2; % in seconds (0.2 e' il massimo tempo inseribile) - ceil(PRES_BUFF_SIZE/sys_data.ffPreshTime/double(sys_data.cntFreq));
sys_data.cmdPreshTimeXY  = .2; % in seconds (0.2 e' il massimo tempo inseribile) - ceil(PRES_BUFF_SIZE/sys_data.cmdPreshTime/double(sys_data.cntFreq));
sys_data.ffPreshTimeXY   = .2; % in seconds (0.2 e' il massimo tempo inseribile) - ceil(PRES_BUFF_SIZE/sys_data.ffPreshTime/double(sys_data.cntFreq));
sys_data.pres_buff_size  = 32768;
sys_data.minModFreq      = 10;                        % in Hz, questa e' la frequenza minima alla quale e' possibile modulare la sinusoide
sys_data.MMForgetFact    = 1e-8;                      % questo non e' un vero forget factor ma e' un valore in radianti di quanto diminuisco e incremento in valore max e min per tenerlo "adattivo"
sys_data.HistIntGain     = 1e-5;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% error & position filter coeffs               %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
sys_data.coeff_TT=zeros(4,9);
%    ku=0.05;pu=4.4e-4;
%    ku=0.01;pu=4.4e-4;
   %         pid_K  pid_I        pid_D        pid_a
   %    p=[ 0.8*ku  2*0.6*ku/pu  0.6*ku*pu/8  2*pi*10e3];
%    p=[ 0.6*ku  2.2*0.8*ku/pu  0.8*ku*pu/8  2*pi*10e3];
%    p=[ 0.3*ku  2*0.8*ku/pu  0.5*ku*pu/8  2*pi*10e3];
pid_coeff.Kp      = 6e-3;
pid_coeff.Ki      = 6;     % V/(rad/s^2) -> 20000 is the value used fot the test_control_loop routine
pid_coeff.Ki_leak = 40;    % V/(rad/s^2) -> 20000 is the value used fot the test_control_loop routine
pid_coeff.fPole   = 0.3;   % Hz        -> 100 is the value used fot the test_control_loop routine
pid_coeff.wPole   = pid_coeff.fPole*2*pi;
sys_data.pid_coeff_TT=pid_coeff;
% pid_sys = tf([pid_coeff.Kp pid_coeff.Ki_leak*pid_coeff.wPole],[1 pid_coeff.wPole]); % leak integrator + proportional    (PI)
% pid_sys = tf([pid_coeff.Kp pid_coeff.Ki*pid_coeff.wPole],[1 0]);                    % pure integrator + proportional    (PI)
pid_sys = tf([0            pid_coeff.Ki*pid_coeff.wPole],[1 0]);                    % pure integrator + no-proportional (I)
%    pid_sys = tf([p(1)+p(3)*p(4)  p(1)*p(4)+p(2)   p(2)*p(4)],[1 p(4) 0]); % vecchia TF
pid_sysD=c2d(pid_sys,1/sys_data.hvcCntFreq,'tustin');
[num,den]=tfdata(pid_sysD,'v');
if     length(num) == 2
   sys_data.coeff_TT(1,:)=[num(1),0,0,0,num(2),0,0,0,den(2)];
   sys_data.coeff_TT(2,:)=[num(1),0,0,0,num(2),0,0,0,den(2)];
elseif length(num) == 3
   sys_data.coeff_TT(1,:)=[num(1),0,0,num(3),num(2),0,0,den(3),den(2)];
   sys_data.coeff_TT(2,:)=[num(1),0,0,num(3),num(2),0,0,den(3),den(2)];
else
   error('Invalud number of elements in the digital filter, check the code please');
end

sys_data.coeff_XY=zeros(4,9);
%    ku=0.05;pu=4.4e-4;
%    ku=0.01;pu=4.4e-4;
   %         pid_K  pid_I        pid_D        pid_a
   %    p=[ 0.8*ku  2*0.6*ku/pu  0.6*ku*pu/8  2*pi*10e3];
%    p=[ 0.6*ku  2.2*0.8*ku/pu  0.8*ku*pu/8  2*pi*10e3];
%    p=[ 0.3*ku  2*0.8*ku/pu  0.5*ku*pu/8  2*pi*10e3];
pid_coeff.Kp      = 6e-3;
pid_coeff.Ki      = 6;     % V/(rad/s^2) -> 20000 is the value used fot the test_control_loop routine
pid_coeff.Ki_leak = 40;    % V/(rad/s^2) -> 20000 is the value used fot the test_control_loop routine
pid_coeff.fPole   = 0.3;   % Hz        -> 100 is the value used fot the test_control_loop routine
pid_coeff.wPole   = pid_coeff.fPole*2*pi;
sys_data.pid_coeff_XY=pid_coeff;
% pid_sys = tf([pid_coeff.Kp pid_coeff.Ki_leak*pid_coeff.wPole],[1 pid_coeff.wPole]); % leak integrator + proportional    (PI)
% pid_sys = tf([pid_coeff.Kp pid_coeff.Ki*pid_coeff.wPole],[1 0]);                    % pure integrator + proportional    (PI)
pid_sys = tf([0            pid_coeff.Ki*pid_coeff.wPole],[1 0]);                    % pure integrator + no-proportional (I)
%    pid_sys = tf([p(1)+p(3)*p(4)  p(1)*p(4)+p(2)   p(2)*p(4)],[1 p(4) 0]); % vecchia TF
pid_sysD=c2d(pid_sys,1/sys_data.hvcCntFreq,'tustin');
[num,den]=tfdata(pid_sysD,'v');
if     length(num) == 2
   sys_data.coeff_XY(1,:)=[num(1),0,0,0,num(2),0,0,0,den(2)];
   sys_data.coeff_XY(2,:)=[num(1),0,0,0,num(2),0,0,0,den(2)];
elseif length(num) == 3
   sys_data.coeff_XY(1,:)=[num(1),0,0,num(3),num(2),0,0,den(3),den(2)];
   sys_data.coeff_XY(2,:)=[num(1),0,0,num(3),num(2),0,0,den(3),den(2)];
else
   error('Invalud number of elements in the digital filter, check the code please');
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% inizializzazione delle variabili per la scheda HVC, come da spread sheet
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
timeHist=linspace(0,2*pi,uint32(sys_data.hvcCntFreq)/sys_data.minModFreq);
mvcfb_CurrHistory=[sin(timeHist(1:end-1));sin(timeHist(1:end-1)+pi/2);0*timeHist(1:end-1);0*timeHist(1:end-1)];
mvcfb_CurrHistory=mvcfb_CurrHistory(:);
mvcub_CurrHistStartPtr = hex2dec('88000');

nameList={'_hvcfb_CurrHistory'};
memPtrList={mvcub_CurrHistStartPtr};
nrItemList={length(mvcfb_CurrHistory)};

idx=length(aoVariables)+1;
for ii=1:length(nameList)
   aoVariables(idx).name=cell2mat(nameList(ii)); % set to 1 to enable the ouput signal of the driver to propagate the adc_sync signal
   aoVariables(idx).memPointer=cell2mat(memPtrList(ii));
   aoVariables(idx).description=[];
   if     aoVariables(idx).name(5)=='u'
      aoVariables(idx).type=TYPE_UINT32;                                      % TYPE_SINGLE, TYPE_UINT32, TYPE_INT32, TYPE_STRUCT
   elseif aoVariables(idx).name(5)=='f'
      aoVariables(idx).type=TYPE_SINGLE;                                      % TYPE_SINGLE, TYPE_UINT32, TYPE_INT32, TYPE_STRUCT
   else
      error('Unrecognized variable type!!!');
   end
   aoVariables(idx).nrItem=cell2mat(nrItemList(ii)); % in case of CAT_CHANNEL, nrItem is the number of elements per channels
   aoVariables(idx).stepPtr=0;
   if     aoVariables(idx).name(6)=='b'
      aoVariables(idx).category=CAT_BOARD;                                    % CAT_BOARD, CAT_CHANNEL
   elseif aoVariables(idx).name(6)=='c'
      aoVariables(idx).category=CAT_CHANNEL;                                  % CAT_BOARD, CAT_CHANNEL
   else
      error('Unrecognized variable category!!!');
   end
   aoVariables(idx).device='DSP';
   aoVariables(idx).operation='';
   idx=idx+1;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% apertura dell' UDP/IP socket                 %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
AOConnect(SC_BCU_IP);
sys_data.SCConnectionID=1;

disp('init env done!!!');
