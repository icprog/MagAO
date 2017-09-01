%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% procedura di calibrazione degli specchi:                                                 %
% montare lo specchio con frontalmente un raggio laser che punti il piu' lontano           %
% possibile per misurare la corsa fatta dallo specchio mentre lo si pilota                 %
% l'angolo che si misura e' l'atan del rapporto tra movimento sul muro e distanza dal muro %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% step 1: questo step in realta' non serve tutte le volte, e' solo per verificare il range massimo in tensione della scheda HVC  %
%         che si sta utilizzando                                                                                                 %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% relaisOn;
% 
% aoWrite('hvc_TT0_step_ptr_preshaper_cmd',round(sys_data.pres_buff_size/sys_data.cmdPreshTime/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
% aoWrite('hvc_TT0_step_ptr_preshaper_curr',round(sys_data.pres_buff_size/sys_data.ffPreshTime/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
% 
% aoWrite('hvc_TT0_sat_DAC_value', [65535 65535 65535 0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT0_nsat_DAC_value',[    0     0     0 0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT0_DAC_N2A_gain',  [    1     1     1 0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT0_DAC_A2bit_gain',[    1     1     1 0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT0_DAC_bit_offset',[    0     0     0 0],0:(sys_data.nAct-1));
% 
% dac_seq=[0:1024:57344 65535];
% for i=dac_seq(:)'
%    aoWrite('hvc_TT0_bias_current',i*ones(sys_data.nAct,1),0:sys_data.nAct-1);
%    pause(.5);
%    a=aoRead('hvc_DAC_value',0);
%    fprintf('ch #0: 0x%04x - ch #1: 0x%04x - ch #2: 0x%04x\n',bitand(a(1),65535),bitand(a(1)/65536,65535),bitand(a(2),65535));
%    pause;
% end
% 
% mirrorRip;
% 
% % acquisizione del canale #0 della scheda HVC SN #006
% vol_out=[89.8*ones(1,9) 86.2 82.6 78.9 75.2 71.4 67.7 63.9 60.2 56.4 52.7 48.9 45.1 41.4 37.6 33.8 30.1 26.4 22.6 18.8 15.1 11.3 7.53 3.76 0 -3.77 -7.55 -10.08*ones(1,23)];
% figure(1);
% plot(dac_seq,vol_out);
% figure(2);
% plot(dac_seq(1:end-1),diff(vol_out));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% step 2: set dei guadagni del canale common che sono fissi e dipendono dall'HW                                                  %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

sys_data.DACV2BitGain_c  = -32768/120.51;             % questo e' un guadagno Volt2bit per il canale common ed e' calcolato dall'HW pero' il match con il misurato e' perfetto! (vedi step 1)
sys_data.DACBitOffset_c  = 32768;                     % 32768 bias in bit del canale common
sys_data.DAC_SatVal_c    = 90;                        % questa e' la tensione massima alla quale arriva il driver di tensione con gli alimentatori che abbiamo, anche se lo specchio accetterebbe di arrivare a +100V
sys_data.DAC_nSatVal_c   = 0;                         % si potrebbe arrivare a -10 ma lo specchio non e' molto contento quindi meglio lasciare 0

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% step 3: per una serie di valori di DAC misurare il movimento effettivo dello specchio sul muro e il corrispettivo strain gauge %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% aoBufferArray(1).bufferName='buff_1';	                    % name of buffer, is not mandatory, just useful
% aoBufferArray(1).triggerPointer=0;		                    % pointer of trigger
% aoBufferArray(1).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(1).triggerDsp=0;			                    % number of DSP 0 or 1
% aoBufferArray(1).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
% aoBufferArray(1).triggerValue=0;                           % trigger value
% aoBufferArray(1).triggerCompare=0;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
% aoBufferArray(1).dspPointer=aoGetAddress('hvc_TT1_distance'); % pointer of read/write element
% aoBufferArray(1).nrItem=2;	                                % size of single read/write operation normally=1
% aoBufferArray(1).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(1).dsp=0;					                    	  % which DSP 0 or 1
% aoBufferArray(1).len=16384;		                          % length of buffer
% aoBufferArray(1).decFactor=0;                              % decimation factor
% aoBufferArray(1).sdramPointerValue=0;	                    % pointer where to/from store/read data
% aoBufferArray(1).direction=0;				                    % direction 0=read data 1=write data
% aoBufferArray(1).circular=0;				                    % 0=linear 1=circular
% aoBufferArray(1).bufferNumber=1;			                    % which buffer number 1 to 6 for each board
% aoBufferArray(1).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
% aoBufferArray(1).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
% 
% relaisOn;
% 
% aoWrite('hvc_TT1_step_ptr_preshaper_cmd',round(sys_data.pres_buff_size/sys_data.cmdPreshTime/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
% aoWrite('hvc_TT1_step_ptr_preshaper_curr',round(sys_data.pres_buff_size/sys_data.ffPreshTime/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
% 
% aoWrite('hvc_TT1_sat_DAC_value', [65535 65535 sys_data.DAC_SatVal_c   0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT1_nsat_DAC_value',[    0     0 sys_data.DAC_nSatVal_c  0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT1_DAC_N2A_gain',  [    1     1 sys_data.DACN2AGain     0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT1_DAC_A2bit_gain',[   -1    -1 sys_data.DACV2BitGain_c 0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT1_DAC_bit_offset',[32768 32768 sys_data.DACBitOffset_c 0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT1_dist_A_coeff',  [    1     1 0                       0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT1_dist_B_coeff',  [    0     0 0                       0],0:(sys_data.nAct-1));
% 
% dac_seq=[32768:-2048:0]; % piu' su non vado perche' sono le tensioni negative
% x_pos=[];
% y_pos=[];
% for i=dac_seq(:)'
%    aoWrite('hvc_TT1_bias_current',[i 12288 84 0],0:sys_data.nAct-1); % metto il secondo asse a mid range (12288) - sul canale common metto una tensione di 84V che e' il massimo che riesco a dare senza andare a saturazione dell'amplificatore di alta tensione
% %    aoWrite('hvc_TT1_bias_current',[12288 i 84 0],0:sys_data.nAct-1); % metto il primo asse a mid range (12288) - sul canale common metto una tensione di 84V che e' il massimo che riesco a dare senza andare a saturazione dell'amplificatore di alta tensione
%    pause(.5);
%    aoBufferWriteSetup(aoBufferArray(1));
%    aoBufferStart(1,sys_data.hvc_board);
%    aoBufferWaitStop(1,sys_data.hvc_board);
%    xy_pos=aoBufferReadData(aoBufferArray(1));
%    xy_pos=reshape(xy_pos,2,aoBufferArray(1).len);
%    x_pos=[x_pos mean(xy_pos(1,:))];
%    y_pos=[y_pos mean(xy_pos(2,:))];
%    fprintf('mean x: %5.0f - mean y: %5.0f\n',mean(xy_pos(1,:)),mean(xy_pos(2,:)));
%    a=aoRead('hvc_DAC_value',0);
%    fprintf('ch #0: 0x%04x - ch #1: 0x%04x - ch #2: 0x%04x\n',bitand(a(1),65535),bitand(a(1)/65536,65535),bitand(a(2),65535));
%    pause(1);
% %    plot(xy_pos');
% end
% 
% mirrorRip;
% break;

% %------------------------------------------------------%
% % acquisizione dello specchio ARGOS SN #001            %
% %------------------------------------------------------%
% % movimento su asse x 
% x0_pos=[23323 23351 23363 23371 23376 23382 23385 23388 23391 22987 22397 21773 21123 20452 19760 19047 18318 17576 16821 16054 15275 14492 13696 12895 12086 11274 10448 9625 8795 7959 7116 6273 5423];
% x0_real_pos=[0 0 0 0 0 0 0 0 0 13 33 53 74 96 119 142 166 191 216 242 266 293 320 347 374 401 428 454 482 510 537 566 596];
% y0_pos=[14570 14570 14570 14570 14570 14569 14570 14569 14569 14579 14592 14605 14619 14634 14649 14665 14681 14698 14715 14732 14750 14768 14787 14806 14826 14846 14865 14886 14906 14927 14948 14969 14990];
% y0_real_pos=zeros(1,33);
% x0_real_pos=atan(x0_real_pos/9060)/2;
% x_poly=polyfit(x0_pos,x0_real_pos,1);
% figure(1);
% hold off;
% plot(x0_pos,x0_real_pos,'-*b');
% hold on;
% plot(x0_pos,polyval(x_poly,x0_pos),'*r');
% figure(2);
% plot(x0_real_pos-polyval(x_poly,x0_pos),'*r');
% % movimento su asse y
% x1_pos=[13495 13496 13496 13496 13496 13495 13495 13495 13494 13502 13514 13528 13542 13556 13571 13587 13604 13620 13638 13656 13675 13694 13714 13735 13754 13777 13798 13820 13842 13864 13885 13908 13930];
% x1_real_pos=zeros(1,33);
% y1_pos=[23788 23836 23853 23864 23872 23878 23883 23888 23891 23572 23071 22531 21953 21332 20675 19980 19256 18509 17744 16969 16186 15382 14560 13719 12856 11971 11066 10150 9227 8288 7344 6397 5445];
% y1_real_pos=589-[589 589 589 589 589 589 589 589 589 578 562 546 527 508 487 466 440 418 393 368 343 318 294 264 236 208 180 151 121 91 61 30 0];
% y1_real_pos=atan(y1_real_pos/9060)/2;
% y_poly=polyfit(y1_pos,y1_real_pos,1);
% figure(1);
% hold off;
% plot(y1_pos,y1_real_pos,'-*b');
% hold on;
% plot(y1_pos,polyval(y_poly,y1_pos),'*r');
% figure(2);
% plot(y1_real_pos-polyval(y_poly,y1_pos),'*r');
%------------------------------------------------------%
% acquisizione dello specchio MAGELLAN PI330.4SL       %
%------------------------------------------------------%
% movimento su asse x 
x0_pos=[29810 29900 29936 29958 29975 28282 26263 24122 21877 19553 17152 14684 12150 9554 6912 4219 1486];
x0_real_pos=90-[90 90 90 90 90 85 78 72 65 57 50 41 34 25 17 9 0];
y0_pos=[14347 14358 14361 14363 14365 14370 14375 14379 14383 14387 14391 14394 14396 14398 14400 14402 14404];
y0_real_pos=zeros(1,13);
x0_real_pos=atan(x0_real_pos/9060)/2;
x_poly=polyfit(x0_pos,x0_real_pos,1);
figure(1);
hold off;
plot(x0_pos,x0_real_pos,'-*b');
hold on;
plot(x0_pos,polyval(x_poly,x0_pos),'*r');
figure(2);
plot(x0_real_pos-polyval(x_poly,x0_pos),'*r');
% movimento su asse y
x1_pos=[14428 14435 14437 14439 14441 14438 14435 14433 14429 14426 14426 14422 14421 14420 14418 14419 14419];
x1_real_pos=zeros(1,13);
y1_pos=[29049 29127 29157 29175 29189 27641 25705 23640 21466 19203 16855 14434 11947 9394 6797 4161 1502];
y1_real_pos=[0 0 0 0 0 5 10 17 24 31 38 46 54 62 71 79 88];
y1_real_pos=atan(y1_real_pos/9060)/2;
y_poly=polyfit(y1_pos,y1_real_pos,1);
figure(1);
hold off;
plot(y1_pos,y1_real_pos,'-*b');
hold on;
plot(y1_pos,polyval(y_poly,y1_pos),'*r');
figure(2);
plot(y1_real_pos-polyval(y_poly,y1_pos),'*r');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% step 4: calcolo dei guadagni dell'ADC                                                                                          %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

sys_data.ADCbit2rad_x    = -x_poly(1);                % questo guadagno viene fuori dal fitting lineare acquisito allo step #2 (muro + strain gauge)
sys_data.ADCbit2rad_y    = -y_poly(1);                % questo guadagno viene fuori dal fitting lineare acquisito allo step #2 (muro + strain gauge)
sys_data.ADCradOffset_x  = (max(x0_real_pos)+min(x0_real_pos))/2-x_poly(2); % la posizione centrale e' presa come la meta' dell'escursione massima
sys_data.ADCradOffset_y  = (max(y1_real_pos)+min(y1_real_pos))/2-y_poly(2); % la posizione centrale e' presa come la meta' dell'escursione massima

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% step 5: verifica del centro e del guadagno del coefficiente ADC to rad con loop chiuso                                         %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% aoBufferArray(1).bufferName='buff_1';	                   % name of buffer, is not mandatory, just useful
% aoBufferArray(1).triggerPointer=0;		                   % pointer of trigger
% aoBufferArray(1).triggerDataType=3;		                   % trigger data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(1).triggerDsp=0;			                      % number of DSP 0 or 1
% aoBufferArray(1).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
% aoBufferArray(1).triggerValue=0;                           % trigger value
% aoBufferArray(1).triggerCompare=0;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
% aoBufferArray(1).dspPointer=aoGetAddress('hvc_TT1_distance'); % pointer of read/write element
% aoBufferArray(1).nrItem=2;	                               % size of single read/write operation normally=1
% aoBufferArray(1).dataType=1;				                   % data type 1='single' 2='int32' 3='uint32'
% aoBufferArray(1).dsp=0;					                    	 % which DSP 0 or 1
% aoBufferArray(1).len=16384;		                            % length of buffer
% aoBufferArray(1).decFactor=0;                              % decimation factor
% aoBufferArray(1).sdramPointerValue=0;	                   % pointer where to/from store/read data
% aoBufferArray(1).direction=0;				                   % direction 0=read data 1=write data
% aoBufferArray(1).circular=0;				                   % 0=linear 1=circular
% aoBufferArray(1).bufferNumber=1;			                   % which buffer number 1 to 6 for each board
% aoBufferArray(1).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
% aoBufferArray(1).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
% aoBufferWriteSetup(aoBufferArray(1));
% 
% relaisOn;
% 
% aoWrite('hvc_TT1_step_ptr_preshaper_cmd',round(sys_data.pres_buff_size/sys_data.cmdPreshTime/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
% aoWrite('hvc_TT1_step_ptr_preshaper_curr',round(sys_data.pres_buff_size/sys_data.ffPreshTime/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
% 
% aoWrite('hvc_TT1_sat_DAC_value', [65535                   65535                   sys_data.DAC_SatVal_c   0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT1_nsat_DAC_value',[    0                       0                   sys_data.DAC_nSatVal_c  0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT1_DAC_N2A_gain',  [    1                       1                   sys_data.DACN2AGain     0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT1_DAC_A2bit_gain',[ -1e5                    -1e5                   sys_data.DACV2BitGain_c 0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT1_DAC_bit_offset',[32768                   32768                   sys_data.DACBitOffset_c 0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT1_dist_A_coeff',  [sys_data.ADCbit2rad_x   sys_data.ADCbit2rad_y   0                       0],0:(sys_data.nAct-1));
% aoWrite('hvc_TT1_dist_B_coeff',  [sys_data.ADCradOffset_x sys_data.ADCradOffset_y 0                       0],0:(sys_data.nAct-1));
% 
% aoWrite('hvc_TT1_pos_command',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
% aoWrite('hvc_TT1_curr_command',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
% aoWrite('hvc_TT1_bias_current',[0.12288 0.12288 84 0],0:sys_data.nAct-1);
% pause(.5);
% aoBufferStart(1,sys_data.hvc_board);
% aoBufferWaitStop(1,sys_data.hvc_board);
% xy_pos=aoBufferReadData(aoBufferArray(1));
% xy_pos=reshape(xy_pos,2,aoBufferArray(1).len);
% x_pos=mean(xy_pos(1,:));
% y_pos=mean(xy_pos(2,:));
% aoWrite('hvc_TT1_bias_command',[x_pos y_pos 0 0],0:sys_data.nAct-1);
% aoWrite('hvc_TT1_control_enable',[1 1],0:1);
% aoWrite('hvc_TT1_post_loop_gain',sys_data.post_loop_gain,0:sys_data.nAct-1);
% pause(.1);
% aoWrite('hvc_TT1_pre_loop_gain',sys_data.pre_loop_gain,0:sys_data.nAct-1);
% pause(.1);
% aoWrite('hvc_TT1_bias_command',[0 0 0 0],0:sys_data.nAct-1);

% % a questo punto per tentativi cercare il centro dello specchio con una serie di:
% % aoWrite('hvc_TT1_dist_B_coeff',  [-.xxx -.xxx 0 0],0:(sys_data.nAct-1));

% %------------------------------------------------------%
% % calibrazione per specchio ARGOS SN #001              %
% %------------------------------------------------------%
% sys_data.ADCradOffset_x  = -.024;
% sys_data.ADCradOffset_y  = -.026;
% aoWrite('hvc_TT1_dist_B_coeff',[sys_data.ADCradOffset_x sys_data.ADCradOffset_y 0 0],0:(sys_data.nAct-1));
% % check del guadagno: PERFETTO!!!
% aoWrite('hvc_TT1_bias_command',[+15e-3 0 0 0],0:sys_data.nAct-1); % +15 mrad
% aoWrite('hvc_TT1_bias_command',[-15e-3 0 0 0],0:sys_data.nAct-1); % -15 mrad
% x_real_pos=541;
% disp(atan(x_real_pos/9060)/2);
% aoWrite('hvc_TT1_bias_command',[0 +12e-3 0 0],0:sys_data.nAct-1); % +12 mrad
% aoWrite('hvc_TT1_bias_command',[0 -12e-3 0 0],0:sys_data.nAct-1); % -12 mrad
% y_real_pos=432;
% disp(atan(y_real_pos/9060)/2);

% %------------------------------------------------------%
% % acquisizione dello specchio MAGELLAN PI330.4SL       %
% %------------------------------------------------------%
sys_data.ADCradOffset_x  = -.0025;
sys_data.ADCradOffset_y  = -.0025;
% aoWrite('hvc_TT1_dist_B_coeff',[sys_data.ADCradOffset_x sys_data.ADCradOffset_y 0 0],0:(sys_data.nAct-1));
% % check del guadagno: PERFETTO!!!
% aoWrite('hvc_TT1_bias_command',[+2.0e-3 0 0 0],0:sys_data.nAct-1); % +2.0 mrad
% aoWrite('hvc_TT1_bias_command',[-2.0e-3 0 0 0],0:sys_data.nAct-1); % -2.0 mrad
% x_real_pos=72;
% disp(atan(x_real_pos/9060)/2);
% aoWrite('hvc_TT1_bias_command',[0 +2.0e-3 0 0],0:sys_data.nAct-1); % +2.0 mrad
% aoWrite('hvc_TT1_bias_command',[0 -2.0e-3 0 0],0:sys_data.nAct-1); % -2.0 mrad
% y_real_pos=72;
% disp(atan(y_real_pos/9060)/2);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% step 5: misura dei guadagni del DAC da Volt a rad mettendo lo specchio in close loop e utilizzando la misura degli strain      %
%         gauge che dai passi prima e' calibrata in radianti                                                                     %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

aoBufferArray(1).bufferName='buff_1';	                    % name of buffer, is not mandatory, just useful
aoBufferArray(1).triggerPointer=0;		                    % pointer of trigger
aoBufferArray(1).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
aoBufferArray(1).triggerDsp=0;			                    % number of DSP 0 or 1
aoBufferArray(1).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
aoBufferArray(1).triggerValue=0;                           % trigger value
aoBufferArray(1).triggerCompare=0;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
aoBufferArray(1).dspPointer=aoGetAddress('hvc_TT1_distance'); % pointer of read/write element
aoBufferArray(1).nrItem=2;	                                % size of single read/write operation normally=1
aoBufferArray(1).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
aoBufferArray(1).dsp=0;					                    	  % which DSP 0 or 1
aoBufferArray(1).len=16384;		                          % length of buffer
aoBufferArray(1).decFactor=0;                              % decimation factor
aoBufferArray(1).sdramPointerValue=0;	                    % pointer where to/from store/read data
aoBufferArray(1).direction=0;				                    % direction 0=read data 1=write data
aoBufferArray(1).circular=0;				                    % 0=linear 1=circular
aoBufferArray(1).bufferNumber=1;			                    % which buffer number 1 to 6 for each board
aoBufferArray(1).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
aoBufferArray(1).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
aoBufferArray(2).bufferName='buff_2';	                    % name of buffer, is not mandatory, just useful
aoBufferArray(2).triggerPointer=0;		                    % pointer of trigger
aoBufferArray(2).triggerDataType=3;		                    % trigger data type 1='single' 2='int32' 3='uint32'
aoBufferArray(2).triggerDsp=0;			                    % number of DSP 0 or 1
aoBufferArray(2).triggerMask=4294967295;                   % trigger mask (these bits are checked) normaly should be 0xFFFFFFFF
aoBufferArray(2).triggerValue=0;                           % trigger value
aoBufferArray(2).triggerCompare=0;		                    % trigger compare type: 0: none 1: "value<*pointer" 2: "value>*pointer" 3: "value=*pointer"
aoBufferArray(2).dspPointer=aoGetAddress('hvc_TT1_float_DAC_value'); % pointer of read/write element
aoBufferArray(2).nrItem=2;	                                % size of single read/write operation normally=1
aoBufferArray(2).dataType=1;				                    % data type 1='single' 2='int32' 3='uint32'
aoBufferArray(2).dsp=0;					                    	  % which DSP 0 or 1
aoBufferArray(2).len=16384;		                          % length of buffer
aoBufferArray(2).decFactor=0;                              % decimation factor
aoBufferArray(2).sdramPointerValue=aoBufferArray(1).sdramPointerValue+aoBufferArray(1).nrItem*aoBufferArray(1).len; % pointer where to/from store/read data
aoBufferArray(2).direction=0;				                    % direction 0=read data 1=write data
aoBufferArray(2).circular=0;				                    % 0=linear 1=circular
aoBufferArray(2).bufferNumber=2;			                    % which buffer number 1 to 6 for each board
aoBufferArray(2).firstDsp=sys_data.hvc_board;              % first DSP where to write setup
aoBufferArray(2).lastDsp=sys_data.hvc_board;               % last DSP where to write setup
aoBufferWriteSetup(aoBufferArray(1:2));

relaisOn;

aoWrite('hvc_TT1_step_ptr_preshaper_cmd',round(sys_data.pres_buff_size/sys_data.cmdPreshTime/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);
aoWrite('hvc_TT1_step_ptr_preshaper_curr',round(sys_data.pres_buff_size/sys_data.ffPreshTime/sys_data.cntFreq)*ones(sys_data.nAct,1),0:sys_data.nAct-1,1,sys_data.mirrorActMap);

aoWrite('hvc_TT1_sat_DAC_value', [65535                   65535                   sys_data.DAC_SatVal_c   0],0:(sys_data.nAct-1));
aoWrite('hvc_TT1_nsat_DAC_value',[    0                       0                   sys_data.DAC_nSatVal_c  0],0:(sys_data.nAct-1));
aoWrite('hvc_TT1_DAC_N2A_gain',  [    1                       1                   sys_data.DACN2AGain     0],0:(sys_data.nAct-1));
aoWrite('hvc_TT1_DAC_A2bit_gain',[ -1e5                    -1e5                   sys_data.DACV2BitGain_c 0],0:(sys_data.nAct-1));
aoWrite('hvc_TT1_DAC_bit_offset',[32768                   32768                   sys_data.DACBitOffset_c 0],0:(sys_data.nAct-1));
aoWrite('hvc_TT1_dist_A_coeff',  [sys_data.ADCbit2rad_x   sys_data.ADCbit2rad_y   0                       0],0:(sys_data.nAct-1));
aoWrite('hvc_TT1_dist_B_coeff',  [sys_data.ADCradOffset_x sys_data.ADCradOffset_y 0                       0],0:(sys_data.nAct-1));

aoWrite('hvc_TT1_pos_command',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
aoWrite('hvc_TT1_curr_command',zeros(sys_data.nAct,1),0:sys_data.nAct-1);
aoWrite('hvc_TT1_bias_current',[0.12288 0.12288 84 0],0:sys_data.nAct-1);
pause(.5);
aoBufferStart(1,sys_data.hvc_board);
aoBufferWaitStop(1,sys_data.hvc_board);
xy_pos=aoBufferReadData(aoBufferArray(1));
xy_pos=reshape(xy_pos,2,aoBufferArray(1).len);
x_pos=mean(xy_pos(1,:));
y_pos=mean(xy_pos(2,:));
aoWrite('hvc_TT1_bias_command',[x_pos y_pos 0 0],0:sys_data.nAct-1);
aoWrite('hvc_TT1_control_enable',[1 1],0:1);
aoWrite('hvc_TT1_post_loop_gain',sys_data.post_loop_gain,0:sys_data.nAct-1);
pause(.1);
aoWrite('hvc_TT1_pre_loop_gain',sys_data.pre_loop_gain,0:sys_data.nAct-1);
pause(.1);
aoWrite('hvc_TT1_bias_command',[0 0 0 0],0:sys_data.nAct-1);
pause(.5);
% aoWrite('hvc_TT1_bias_command',[12e-3 0 0 0],0:sys_data.nAct-1); % +12 mrad - for PI334
aoWrite('hvc_TT1_bias_command',[2.0e-3 0 0 0],0:sys_data.nAct-1); % +2 mrad - for PI330
pause(.5);
aoBufferStart(1:2,sys_data.hvc_board);
aoBufferWaitStop(1,sys_data.hvc_board);
xy_pos=aoBufferReadData(aoBufferArray(1));
xy_pos=reshape(xy_pos,2,aoBufferArray(1).len);
x0_pos=mean(xy_pos(1,:));
y0_pos=mean(xy_pos(2,:));
xy_cmd=aoBufferReadData(aoBufferArray(2));
xy_cmd=reshape(xy_cmd,2,aoBufferArray(2).len);
x0_cmd=mean(xy_cmd(1,:));
y0_cmd=mean(xy_cmd(2,:));
% aoWrite('hvc_TT1_bias_command',[-12e-3 0 0 0],0:sys_data.nAct-1); % -12 mrad - for PI334
aoWrite('hvc_TT1_bias_command',[-2.0e-3 0 0 0],0:sys_data.nAct-1); % -2 mrad - for PI330
pause(.5);
aoBufferStart(1:2,sys_data.hvc_board);
aoBufferWaitStop(1,sys_data.hvc_board);
xy_pos=aoBufferReadData(aoBufferArray(1));
xy_pos=reshape(xy_pos,2,aoBufferArray(1).len);
x1_pos=mean(xy_pos(1,:));
y1_pos=mean(xy_pos(2,:));
xy_cmd=aoBufferReadData(aoBufferArray(2));
xy_cmd=reshape(xy_cmd,2,aoBufferArray(2).len);
x1_cmd=mean(xy_cmd(1,:));
y1_cmd=mean(xy_cmd(2,:));
sys_data.DACr2BitGain_x=-1e+5*(x0_cmd-x1_cmd)/(x0_pos-x1_pos);
aoWrite('hvc_TT1_bias_command',[0 0 0 0],0:sys_data.nAct-1);
pause(.5);
% aoWrite('hvc_TT1_bias_command',[0 12e-3 0 0],0:sys_data.nAct-1); % +12 mrad - for PI334
aoWrite('hvc_TT1_bias_command',[0 2.0e-3 0 0],0:sys_data.nAct-1); % +2 mrad - for PI330
pause(.5);
aoBufferStart(1:2,sys_data.hvc_board);
aoBufferWaitStop(1,sys_data.hvc_board);
xy_pos=aoBufferReadData(aoBufferArray(1));
xy_pos=reshape(xy_pos,2,aoBufferArray(1).len);
x0_pos=mean(xy_pos(1,:));
y0_pos=mean(xy_pos(2,:));
xy_cmd=aoBufferReadData(aoBufferArray(2));
xy_cmd=reshape(xy_cmd,2,aoBufferArray(2).len);
x0_cmd=mean(xy_cmd(1,:));
y0_cmd=mean(xy_cmd(2,:));
% aoWrite('hvc_TT1_bias_command',[0 -12e-3 0 0],0:sys_data.nAct-1); % -12 mrad - for PI334
aoWrite('hvc_TT1_bias_command',[0 -2.0e-3 0 0],0:sys_data.nAct-1); % -2 mrad - for PI330
pause(.5);
aoBufferStart(1:2,sys_data.hvc_board);
aoBufferWaitStop(1,sys_data.hvc_board);
xy_pos=aoBufferReadData(aoBufferArray(1));
xy_pos=reshape(xy_pos,2,aoBufferArray(1).len);
x1_pos=mean(xy_pos(1,:));
y1_pos=mean(xy_pos(2,:));
xy_cmd=aoBufferReadData(aoBufferArray(2));
xy_cmd=reshape(xy_cmd,2,aoBufferArray(2).len);
x1_cmd=mean(xy_cmd(1,:));
y1_cmd=mean(xy_cmd(2,:));
sys_data.DACr2BitGain_y=-1e+5*(y0_cmd-y1_cmd)/(y0_pos-y1_pos);

mirrorRip;

a=aoRead('hvc_DAC_value',0);
fprintf('ch #0: 0x%04x - ch #1: 0x%04x - ch #2: 0x%04x\n',bitand(a(1),65535),bitand(a(1)/65536,65535),bitand(a(2),65535));
