function data_struct=aoGetBCUStatus(varargin)
% data_struct=aoGetBCUStatus([connectionNr],[raw_data])
% Gests BCU status and returns a struct with 26 fields.
% The PowerBackplane calibration values are read from the structure
% pbGainMatrix that shall be available in the workspace. 
% If not available, the function tries to load it from
% S:\Progetti\Engineering\Lbt\Matlab\Tests\PowerBackplane\pbGainMatrix.mat
% and to assign it to the workspace. A message is displayed.
% If the file is not available a message is displayed and default values are loaded.
%
% [connectionNr]: default is 1 is an optional argument indicating which connection to use for multiple crate operation
% [raw_data]: for the VLT power backplane returns the raw ADC values instead of the SI converted values
%
% Author(s): R. Biasi
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/11/2005
% $Revision 0.2 $ $Date: 04/09/2005 - load of pbGainMatrix added
% $Revision 0.3 $ $Date: 09/03/2007 - multiple Crate connection D. Pescoller
% $Revision 0.4 $ $Date: 30/03/2009 - increased reply struct with others data (mario)
% $Revision 0.5 $ $Date: 18/05/2010 - changed the order of the data struct
% $Revision 0.6 $ $Date: 04/02/2011 - improved the function for the reading of the VLT pwr bck (mario)

% Lettura dello status BCU: mgp_op_rd_sram(255,255,30,'0x38000')
% 0   uint16 crateID;                                 // identificazione del crate
%     uint16 who_ami;                                 // identificazione della board sul crate
% 1   uint32 software_release;                        // versione software del Nios
% 2   uint16 logic_release;                           // lettura del PIO con la versione della logica
%     uint8  mac_address[6];                          // indirizzo mac della scheda
% 4   uint8  ip_address[4];                           // indirizzo ip della scheda
% 5   uint8  crate_configuration[NUM_BOARD_ON_CRATE]; // stato del crate
% 10	uint16 local_current_threshold;                 // livello di corrente locale di thresholds (positiva e negativa)
% 	   int16  vp_set;                                  // impostazione tensione rami power (positivo e negativo)
% 11	uint16 total_current_thres_pos;                 // livello di corrente totale di thresholds (positiva)
%     uint16 total_current_thres_neg;                 // livello di corrente totale di thresholds (negativa)
% 12	uint32 frames_counter;                          // counter globale con il numero di frames arrivati dal DSP
% 13	uint32 relais_board_out;                        // variabile con lo status dell'uscita della scheda relais
% 14	uint16 serial_number;                           // numero seriale della scheda
%     uint16 pb_serial_number;                        // numero seriale del power backplane
% 15	uint32 relais_board_out_1;                      // variabile con lo status dell'uscita della seconda scheda relais
% 16	uint32 enable_pos_acc;                          // abilita(1) / disabilita(0) la lettura dal secondario degli accumulatori di posizione oltre a quelli di corrente per il calcolo mantegazza style
% 17	uint32 diagnostic_record_ptr;                   // puntatore alla memoria che contiene la diagnostica da scaricare in SDRAM
% 18	uint32 diagnostic_record_len;                   // dimensione del pacchetto diagnostico da salvare in DWORD
% 19	uint32 enable_master_diag;                      // abilita(1) / disabilita(0) la spedizione della diagnostica in modo automatico
% 20	uint8  remote_mac_address[6];                   // indirizzo mac della scheda dove spedire la diagnostica
%     uint8  remote_ip_address[4];                    // indirizzo ip della scheda dove spedire la diagnostica
%     uint16 remote_udp_port;                         // porta UDP alla quale mandare il pacchetto diagnostico BCU master
% 23	uint32 rd_diagnostic_record_ptr;                // puntatore alla SDRAM della diagnostica da spedire via ETH
% 24	uint32 wr_diagnostic_record_ptr;                // puntatore alla SDRAM della diagnostica da salvare in SDRAM
% 25	na_bcu_diagnostic_struct diagnostic_area;       // area contenete i valori diagnostici
% typedef volatile struct
% {
% 25	uint16 stratix_temp;     // temperatura del chip stratix
%   	uint16 power_temp;       // temperatura board nella zona di power
% 
% 26	uint32 bck_digitalIO;    // canale spi sul backplane (MAX7301 U1)
% 	
% 27  uint16 voltage_vccl;     // canale spi sul backplane (AD7927 U30)
%   	uint16 voltage_vcca;     // canale spi sul backplane
% 28	uint16 voltage_vssa;     // canale spi sul backplane
%     uint16 voltage_vccp;     // canale spi sul backplane
% 29	uint16 voltage_vssp;     // canale spi sul backplane
%     uint16 current_vccl;     // canale spi sul backplane
% 30  uint16 current_vcca;     // canale spi sul backplane
%     uint16 current_vssa;     // canale spi sul backplane
% 
% 31	uint16 current_vccp;     // canale spi sul backplane (AD7927 U31)
%   	uint16 current_vssp;     // canale spi sul backplane
% 32	uint16 tot_current_vccp; // canale spi sul backplane
%   	uint16 tot_current_vssp; // canale spi sul backplane
% 33	uint16 tot_current_vp;   // canale spi sul backplane
% 
%   	uint16 in0_temp;         // canale spi sul backplane (AD7927 U32)
% 34  uint16 in1_temp;         // canale spi sul backplane
%   	uint16 out0_temp;        // canale spi sul backplane
% 35  uint16 out1_temp;        // canale spi sul backplane
%   	uint16 ext_umidity;      // canale spi sul backplane
% 36  uint16 pressure;         // canale spi sul backplane
% } na_bcu_diagnostic_struct;

M4DP=0;
% setting default connection
connectionNr=1;
raw_data=0;
if nargin>=1
    connectionNr=varargin{1};
end
if nargin>=2
    raw_data=varargin{2};
end

MGP_DEFINE();

a=mgp_op_rd_sram(255,255,26,'0x38000',connectionNr,'uint8');
data_struct.crateNum=a(1);
data_struct.softwareRelease=[dec2hex(a(8),2),'.',dec2hex(a(7),2),'.',dec2hex(a(6),2),dec2hex(a(5),2)];
data_struct.logicRelease=[dec2hex(a(10),2),'.',dec2hex(a(9),2)];
data_struct.macAddress=[dec2hex(a(11),2),'.',dec2hex(a(12),2),'.',dec2hex(a(13),2),'.',dec2hex(a(14),2),'.',dec2hex(a(15),2),'.',dec2hex(a(16),2)];
data_struct.ipAddress=[num2str(a(17)),'.',num2str(a(18)),'.',num2str(a(19)),'.',num2str(a(20))];
data_struct.crateConf=a(21:40);
data_struct.framesCounter=typecast(uint8(a(49:52)),'uint32');
data_struct.serial_number=typecast(uint8(a(57:58)),'uint16');
data_struct.pb_serial_number=typecast(uint8(a(59:60)),'uint16');
data_struct.fast_link_port=typecast(uint8(a(61:64)),'uint32');
data_struct.enable_hl_commands=typecast(uint8(a(65:68)),'uint32');
data_struct.enable_pos_acc=typecast(uint8(a(69:72)),'uint32');
data_struct.diagnostic_record_ptr=typecast(uint8(a(73:76)),'uint32');
data_struct.diagnostic_record_len=typecast(uint8(a(77:80)),'uint32');
data_struct.enable_master_diag=typecast(uint8(a(81:82)),'uint16');
data_struct.decimation_factor=typecast(uint8(a(83:84)),'uint16');
data_struct.remote_mac_address=uint8(a(85:90));
data_struct.remote_ip_address=uint8(a(91:94));
data_struct.remote_udp_port=typecast(uint8(a(95:96)),'uint16');
data_struct.rd_diagnostic_record_ptr=typecast(uint8(a(97:100)),'uint32');
data_struct.wr_diagnostic_record_ptr=typecast(uint8(a(101:104)),'uint32');
a=mgp_op_rd_sram(255,255,1,'0x3801C',connectionNr,'int16');
data_struct.stratix_temp=a(1)*0.0078125;
data_struct.power_temp=a(2)*0.0078125;

if data_struct.pb_serial_number && data_struct.pb_serial_number<1000
   if isunix()
       pbGainMatrixFILE='/mnt/bzmicrogate/Progetti/Engineering/Lbt/Matlab/Tests/PowerBackplane/pbGainMatrix.mat';
   else
   %     pbGainMatrixFILE='S:\Progetti\Engineering\Lbt\Matlab\Tests\PowerBackplane\pbGainMatrix.mat';
       pbGainMatrixFILE='S:\Progetti\Engineering\Lbt\Matlab\Tests\PowerBackplane\pbGainMatrix.mat';
   end

   % fprintf(1,'FFDD\nSCSE\n%s\n',data_struct.resetWord(32-8:32-5));

   nom_gain_matrix=[
       0.0000E+00	1.0173E-03	0.0000E+00	% VCC_L
       0.0000E+00	3.4790E-03	0.0000E+00	% VCC_A
       0.0000E+00	-6.1035E-03	8.7925E+00	% VSS_A
       0.0000E+00	3.4790E-03	0.0000E+00	% VCC_P
       0.0000E+00	-6.1035E-03	8.7925E+00	% VSS_P
       0.0000E+00	1.2986E-02	0.0000E+00	% I_VCC_L
       0.0000E+00	2.7743E-03	0.0000E+00	% I_VCC_A
       0.0000E+00	-2.7785E-03	0.0000E+00	% I_VSS_A
       0.0000E+00	5.5235E-03	0.0000E+00	% I_VCC_P
       0.0000E+00	-5.5191E-03	0.0000E+00	% I_VSS_P
       0.0000E+00	2.0296E-03	0.0000E+00	% I_TOT_VCC_P
       0.0000E+00	-2.0280E-03	0.0000E+00	% I_TOT_VSS_P
       0.0000E+00	4.0592E-03	-8.3133E+00	% I_TOT_DIFF
       1.5823E-06	-3.4667E-02	7.7418E+01	% IN0_TEMP
       1.5823E-06	-3.4667E-02	7.7418E+01	% IN1_TEMP
       1.5823E-06	-3.4667E-02	7.7418E+01	% OUT0_TEMP
       1.5823E-06	-3.4667E-02	7.7418E+01	% OUT1_TEMP
       0.0000E+00  3.8639e-002 -2.5764e+001 % HUMIDITY
       0.0000E+00	-3.7602E-03	7.7009E+00	% PRESSURE
       0.0000E+00	2.8288E+00	0.0000E+00	% LOCAL VCC_P/VSS_P THRESHOLD (bit/A)
       0.0000E+00	1.1283E+01	-1.0391E+02	% VCC_P/VSS_P SET  (bit/V)
       0.0000E+00	2.5744E-01	3.2000E+01	% TOTAL VCC_P THRESHOLD  (bit/A) 
       0.0000E+00	-2.5744E-01	3.2000E+01	% TOTAL VSS_P THRESHOLD  (bit/A)
       0.0000E+00	0.353506787	0.0000E+00	% LOCAL VCC_P/VSS_P THRESHOLD (A/bit)
       0.0000E+00	0.08862734	9.2093E+00	% VCC_P/VSS_P SET  (V/bit)
       0.0000E+00	3.88437023	-1.2430E+02	% TOTAL VCC_P THRESHOLD  (A/bit) 
       0.0000E+00	-3.88437023	1.2430E+02	% TOTAL VSS_P THRESHOLD  (A/bit)
       ];

   a=mgp_op_rd_sram(255,255,1,'0x38028',connectionNr,'uint32');
   data_struct.resetWord=dec2bin(a(1),32);
   % bit  0: fpga_rst_sig
   % bit  1: busdsp_rst_sig
   % bit  2: dsp_rst_sig
   % bit  3: flash_rst_sig
   % bit  4: pci_rst_sig
   % bit  5: bus_driver_enable
   % bit  6: bus_driver_enable_status (read only)
   % bit  7: power_fault_ctrl_sig
   % bit  8: bus_power_fault (read only)
   % bit  9: enable system watchdog
   % bit 10: is_master
   % bit 11: clk_inuse

   pbSerNum=data_struct.pb_serial_number;

   a=mgp_op_rd_sram(255,255,1,'0x3801D',connectionNr,'uint32');
   data_struct.bck_digitalIO=dec2bin(a(1),32);

   try
       gain_matrix=evalin('base','pbGainMatrix;');
   catch
       if exist(pbGainMatrixFILE,'file'),
           load(pbGainMatrixFILE);
           assignin('base','pbGainMatrix',pbGainMatrix);
           gain_matrix=pbGainMatrix;
           fprintf(1,'pbGainMatrix loaded from:\n\t%s\n',pbGainMatrixFILE);
       else
           gain_matrix(:,:,pbSerNum)=nom_gain_matrix;
   %        disp('WARNING: pbGainMatrix not available - default gains used!');
       end
   end

   if size(gain_matrix,3) < pbSerNum,
   %    disp('WARNING: PowerBackplane serial number exceeds pbGainMatrix size! - default gains used!');
       gain_matrix(:,:,pbSerNum)=nom_gain_matrix;
   end;

   if M4DP,
       ADC_VREF=2.5;
       ADC_BIT=12;
       V2A_V48I_GAIN=-75e-3;
       V48I_OFFSET=2.5;
       V2A_GLOBAL_CURR_GAIN=-0.01515;
       GLOBAL_CURR_OFFSET=1.520;
       gain_matrix(2,:,pbSerNum)=zeros(1,3); % flux, not used
       gain_matrix(3,:,pbSerNum)=[0 1/(V2A_GLOBAL_CURR_GAIN*2^ADC_BIT/ADC_VREF) -GLOBAL_CURR_OFFSET/V2A_GLOBAL_CURR_GAIN]; % coil sum current ch0
       gain_matrix(4,:,pbSerNum)=zeros(1,3); % coil sum current ch1, not used
       gain_matrix(5,:,pbSerNum)=[0 1/(V2A_V48I_GAIN*2^ADC_BIT/ADC_VREF) -V48I_OFFSET/V2A_V48I_GAIN]; % 48V main current ch0
       gain_matrix(7,:,pbSerNum)=zeros(1,3); % 48V main current ch1, not used
   end;


   a=mgp_op_rd_sram(255,255,10,'0x3801E',connectionNr,'uint16');
   a=a(1:19); a=a(:);
   % a(18)
   out_data=[a.^2 a ones(size(a))].*gain_matrix(1:19,:,pbSerNum);
   out_data=sum(out_data');

   data_struct.voltage_vccl=out_data(1);
   data_struct.voltage_vcca=out_data(2);
   data_struct.voltage_vssa=out_data(3);
   data_struct.voltage_vccp=out_data(4);
   data_struct.voltage_vssp=out_data(5);
   data_struct.current_vccl=out_data(6);
   data_struct.current_vcca=out_data(7);
   data_struct.current_vssa=out_data(8);
   data_struct.current_vccp=out_data(9);
   data_struct.current_vssp=out_data(10);
   data_struct.tot_current_vccp=out_data(11);
   data_struct.tot_current_vssp=out_data(12);
   data_struct.tot_current_vp=out_data(13);
   data_struct.in0_temp=out_data(14);
   data_struct.in1_temp=out_data(15);
   data_struct.out0_temp=out_data(16);
   data_struct.out1_temp=out_data(17);
   data_struct.ext_humidity=out_data(18);
   data_struct.pressure=out_data(19);    

   a=mgp_op_rd_sram(255,255,2,'0x3800A',connectionNr,'uint16');
   a=a(1:4); a=a(:);
   out_data=[a.^2 a ones(size(a))].*gain_matrix(24:27,:,pbSerNum);
   out_data=sum(out_data');


   data_struct.local_current_threshold=out_data(1);
   data_struct.vp_set=out_data(2);
   data_struct.total_current_thres_pos=out_data(3);
   data_struct.total_current_thres_neg=out_data(4);

   % data_struct.local_current_threshold=(a(1)+0.4006)/2.8479;
   % data_struct.vp_set=(a(2)+104.4496)/11.3576;
   % data_struct.total_current_thres_pos=(a(3)-29.7445)/3.9717;
   % data_struct.total_current_thres_neg=(a(4)-29.9726)/-3.5416;
elseif data_struct.pb_serial_number && data_struct.pb_serial_number<2000

   pbSerNum=data_struct.pb_serial_number-1000;

   if isunix()
       pbGainStructFILE='/mnt/bzmicrogate/Progetti/Engineering/VLT_DSM/Design/Matlab/Test/powerBackplanes/pbGainStruct.mat';
   else
       pbGainStructFILE='S:\Progetti\Engineering\VLT_DSM\Design\Matlab\Test\powerBackplanes\pbGainStruct.mat';
   end

   nom_gain_struct.vcc3v3         = [8.8826e-004 -0.0381];
   nom_gain_struct.vcc5v5_ap      = [0.0015 0.0106];
   nom_gain_struct.vcc48v         = [0.0190 0.0720];
   nom_gain_struct.i_vcc3v3_a     = [0.0098 -1.5916];
   nom_gain_struct.i_vcc3v3_b     = [0.0098 -1.5916];
   nom_gain_struct.i_vcc5v5_ap_a  = [0.0066 0.3399];
   nom_gain_struct.i_vcc5v5_ap_b  = [0.0066 0.3399];
   nom_gain_struct.i_vcc5v5_ap_c  = [0.0066 0.3399];
   nom_gain_struct.i_vcc48v       = [0.0024 0.3130];
   nom_gain_struct.in0_temp       = [6.8359e-007 -0.0214 52.8443];
   nom_gain_struct.out0_temp      = [6.8365e-007 -0.0214 52.8622];
   nom_gain_struct.spare0_temp    = [6.8453e-007 -0.0214 52.8295];
   nom_gain_struct.spare1_temp    = [6.8474e-007 -0.0214 52.8220];
   nom_gain_struct.ext_humidity   = [0.0398 -25.6570];
   nom_gain_struct.pressure       = [5.2228e-006 1.8135e-005];
   nom_gain_struct.i_coil_sum     = [0.5188 -1.1170e+003];
   nom_gain_struct.accX           = [0.0720 0];
   nom_gain_struct.accY           = [0.0720 0];
   nom_gain_struct.accZ           = [0.0720 0];
   nom_gain_struct.set_ovcTh_pull = [0.0298 33.1204];
   nom_gain_struct.set_ovcTh_push = [0.0284 32.2869];
   nom_gain_struct.set_leakTh     = [20.7651 0.4332];
   nom_gain_struct.set_5V5        = [57.5360 -262.6904];
   
   try
       gain_struct=evalin('base','pbGainStruct;');
   catch
       if exist(pbGainStructFILE,'file'),
           load(pbGainStructFILE);
           assignin('base','pbGainStruct',pbGainStruct);
           gain_struct=pbGainStruct;
           fprintf(1,'pbGainStruct loaded from:\n\t%s\n',pbGainStructFILE);
       else
           gain_struct(pbSerNum)=nom_gain_struct;
       end
   end

   a=mgp_op_rd_sram(255,255,1,'0x3801D',connectionNr,'uint32');
   data_struct.resetWord=dec2bin(a(1),32);
   % bit  0: fpga_rst_sig
   % bit  1: busdsp_rst_sig
   % bit  2: dsp_rst_sig
   % bit  3: flash_rst_sig
   % bit  4: pci_rst_sig
   % bit  5: bus_driver_enable
   % bit  6: bus_driver_enable_status (read only)
   % bit  7: power_fault_ctrl_sig
   % bit  8: bus_power_fault (read only)
   % bit  9: enable system watchdog
   % bit 10: is_master
   % bit 11: clk_inuse

   isValid=0;
   while ~isValid
      readString = mgp_op_rd_sram(255,255,19,'0x3801F',connectionNr);
      readString = char(typecast(uint32(readString),'uint8'))';
      isValid=readString(75)==13;
   end

   if strcmp(readString(1:4),'!RdS')
      % Digital Output Status
      digitWord=dec2bin(hex2dec(readString(5:8)),16);
      data_struct.dOut.pic_spare_out               = digitWord(16-00)=='1';
      data_struct.dOut.vcc15v_ena                  = digitWord(16-01)=='1';
      data_struct.dOut.vcc3v3_ena                  = digitWord(16-02)=='1';
      data_struct.dOut.wesp_set0                   = digitWord(16-03)=='1';
      data_struct.dOut.wesp_set1                   = digitWord(16-04)=='1';
      data_struct.dOut.bus_sys_fault               = digitWord(16-05)=='1';
      data_struct.dOut.driver_ena_n                = digitWord(16-06)=='1';
      data_struct.dOut.driver_ena_dir              = digitWord(16-07)=='1';
      data_struct.dOut.bus_fpga_clr_2bp            = digitWord(16-08)=='1';
      data_struct.dOut.bus_boot_select_2bp         = digitWord(16-09)=='1';
      data_struct.dOut.bus_fpga_clr_2bp_dir        = digitWord(16-10)=='1';
      data_struct.dOut.bus_boot_select_2bp_dir     = digitWord(16-11)=='1';

      
%       data_struct.dOut.leakage_1_n_lclr            = digitWord(16-08)=='1';
%       data_struct.dOut.leakage_2_n_lclr            = digitWord(16-09)=='1';
%       data_struct.dOut.i_coil_sum_imbalance_n_lclr = digitWord(16-10)=='1';
%       data_struct.dOut.acc_test_ena                = digitWord(16-11)=='1';
%       data_struct.dOut.bus_fpga_clr_2bp            = digitWord(16-12)=='1';
%       data_struct.dOut.bus_boot_select_2bp         = digitWord(16-13)=='1';
      
      % Digital Input Status
      digitWord=dec2bin(hex2dec(readString(9:16)),32);
      data_struct.dIn.ac_power_fault0              = digitWord(32-00)=='1';
      data_struct.dIn.ac_power_fault1              = digitWord(32-01)=='1';
      data_struct.dIn.ac_power_fault2              = digitWord(32-02)=='1';
      data_struct.dIn.vcc5V5_fault_vic0            = digitWord(32-03)=='1';
      data_struct.dIn.vcc5V5_fault_vic1            = digitWord(32-04)=='1';
      data_struct.dIn.vcc5V5_fault_vic2            = digitWord(32-05)=='1';
      data_struct.dIn.leakage_1_n                  = digitWord(32-06)=='1';
      data_struct.dIn.leakage_2_n                  = digitWord(32-07)=='1';
      data_struct.dIn.bus_sys_rst_2bp              = digitWord(32-08)=='1';
      data_struct.dIn.bus_sys_fault_n_2bp          = digitWord(32-09)=='1';
      data_struct.dIn.ic_disconnected_n            = digitWord(32-10)=='1';
      data_struct.dIn.is_master                    = digitWord(32-11)=='1';
      data_struct.dIn.i_coil_sum_imbalance_n       = digitWord(32-12)=='1';
      data_struct.dIn.dsm_online_n                 = digitWord(32-13)=='1';
      data_struct.dIn.bus_fpga_clr_2bp             = digitWord(32-14)=='1';
      data_struct.dIn.bus_boot_select_2bp          = digitWord(32-15)=='1';
      data_struct.dIn.bus_driver_enable_n_2bp      = digitWord(32-16)=='1';
      data_struct.dIn.bus_spare0_2bp               = digitWord(32-17)=='1';
      data_struct.dIn.bus_spare1_2bp               = digitWord(32-18)=='1';
      data_struct.dIn.spi_clk                      = digitWord(32-19)=='1';
      data_struct.dIn.spi_cs                       = digitWord(32-20)=='1';
      data_struct.dIn.pic_prog_n                   = digitWord(32-21)=='1';
      data_struct.dIn.driver_ena_n                 = digitWord(32-22)=='1';
      data_struct.dIn.leakage_1_n_latch            = digitWord(32-23)=='1';
      data_struct.dIn.leakage_2_n_latch            = digitWord(32-24)=='1';
      data_struct.dIn.i_coil_sum_imbalance_n_latch = digitWord(32-25)=='1';

      % coolant flow counter
      data_struct.coolantFlowCounter               = hex2dec(readString(17:20));

      % ADC data
      data_struct.vcc3v3                           = hex2dec(readString(21:23));
      data_struct.vcc5v5_ap                        = hex2dec(readString(24:26));
      data_struct.vcc48v                           = hex2dec(readString(66:68));
      data_struct.i_vcc3v3_a                       = hex2dec(readString(27:29));
      data_struct.i_vcc3v3_b                       = hex2dec(readString(30:32));
      data_struct.i_vcc5v5_ap_a                    = hex2dec(readString(33:35));
      data_struct.i_vcc5v5_ap_b                    = hex2dec(readString(36:38));
      data_struct.i_vcc5v5_ap_c                    = hex2dec(readString(39:41));
      data_struct.i_vcc48v                         = hex2dec(readString(63:65));
      data_struct.in0_temp                         = hex2dec(readString(42:44));
      data_struct.out0_temp                        = hex2dec(readString(45:47));
      data_struct.spare0_temp                      = hex2dec(readString(48:50));
      data_struct.spare1_temp                      = hex2dec(readString(51:53));
      data_struct.ext_humidity                     = hex2dec(readString(54:56));
      data_struct.pressure                         = hex2dec(readString(57:59));
      data_struct.i_coil_sum                       = hex2dec(readString(60:62)); % fprintf(1,'Icoilsum %d\n',data_struct.i_coil_sum);
      data_struct.accX                             = double(typecast(uint8(hex2dec(readString(69:70))),'int8'));
      data_struct.accY                             = double(typecast(uint8(hex2dec(readString(71:72))),'int8'));
      data_struct.accZ                             = double(typecast(uint8(hex2dec(readString(73:74))),'int8'));

      if ~raw_data
         % Converts values in SI
         data_struct.vcc3v3        = [data_struct.vcc3v3 1]*gain_struct(pbSerNum).vcc3v3';
         data_struct.vcc5v5_ap     = [data_struct.vcc5v5_ap 1]*gain_struct(pbSerNum).vcc5v5_ap';
         data_struct.vcc48v        = [data_struct.vcc48v 1]*gain_struct(pbSerNum).vcc48v';
         data_struct.i_vcc3v3_a    = [data_struct.i_vcc3v3_a 1]*gain_struct(pbSerNum).i_vcc3v3_a';
         data_struct.i_vcc3v3_b    = [data_struct.i_vcc3v3_b 1]*gain_struct(pbSerNum).i_vcc3v3_b';
         data_struct.i_vcc5v5_ap_a = [data_struct.i_vcc5v5_ap_a 1]*gain_struct(pbSerNum).i_vcc5v5_ap_a';
         data_struct.i_vcc5v5_ap_b = [data_struct.i_vcc5v5_ap_b 1]*gain_struct(pbSerNum).i_vcc5v5_ap_b';
         data_struct.i_vcc5v5_ap_c = [data_struct.i_vcc5v5_ap_c 1]*gain_struct(pbSerNum).i_vcc5v5_ap_c';
         data_struct.i_vcc48v      = [data_struct.i_vcc48v 1]*gain_struct(pbSerNum).i_vcc48v';
         data_struct.in0_temp      = [data_struct.in0_temp^2 data_struct.in0_temp 1]*gain_struct(pbSerNum).in0_temp';
         data_struct.out0_temp     = [data_struct.out0_temp^2 data_struct.out0_temp 1]*gain_struct(pbSerNum).out0_temp';
         data_struct.spare0_temp   = [data_struct.spare0_temp^2 data_struct.spare0_temp 1]*gain_struct(pbSerNum).spare0_temp';
         data_struct.spare1_temp   = [data_struct.spare1_temp^2 data_struct.spare1_temp 1]*gain_struct(pbSerNum).spare1_temp';
         data_struct.ext_humidity  = [data_struct.ext_humidity 1]*gain_struct(pbSerNum).ext_humidity';
         data_struct.pressure      = [data_struct.pressure 1]*gain_struct(pbSerNum).pressure';
         data_struct.i_coil_sum    = [data_struct.i_coil_sum 1]*gain_struct(pbSerNum).i_coil_sum';
         data_struct.accX          = [data_struct.accX 1]*gain_struct(pbSerNum).accX';
         data_struct.accY          = [data_struct.accY 1]*gain_struct(pbSerNum).accY';
         data_struct.accZ          = [data_struct.accZ 1]*gain_struct(pbSerNum).accZ';
      end
   end

elseif data_struct.pb_serial_number
   error('Invalid power backplane serial number');
end
