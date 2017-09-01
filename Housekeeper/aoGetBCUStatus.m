function data_struct=aoGetBCUStatus()
% Gests BCU status and returns a struct with 26 fields.
% The PowerBackplane calibration values are read from the structure
% pbGainMatrix that shall be available in the workspace. 
% If not available, the function tries to load it from
% S:\Progetti\Engineering\Lbt\Matlab\Tests\PowerBackplane\pbGainMatrix.mat
% and to assign it to the workspace. A message is displayed.
% If the file is not available a message is displayed and default values are loaded.
%
% Author(s): R. Biasi
%
% Copyright 2004-2008 Microgate s.r.l.
% $Revision 0.1 $ $Date: 22/11/2005
% $Revision 0.2 $ $Date: 04/09/2005 - load of pbGainMatrix added

% Lettura dello status BCU: mgp_op_rd_sram(255,255,30,'0x38000')
% 0     uint16 crateID;                                 // identificazione del crate
%       uint16 who_ami;                                 // identificazione della board sul crate
% 1     uint32 software_release;                        // versione software del Nios
%       uint16 logic_release;                           // lettura del PIO con la versione della logica
%       uint8  mac_address[6];                          // indirizzo mac della scheda
% 4     uint8  ip_address[4];                           // indirizzo ip della scheda
% 5     uint8  crate_configuration[NUM_BOARD_ON_CRATE]; // stato del crate
% 10	uint16 local_current_threshold;                 // livello di corrente locale di thresholds (positiva e negativa)
% 	    uint16 vp_set;                                  // impostazione tensione rami power (positivo e negativo)
% 11	uint16 total_current_thres_pos;                 // livello di corrente totale di thresholds (positiva)
%       uint16 total_current_thres_neg;                 // livello di corrente totale di thresholds (negativa)
% 12	uint32 frames_counter;                          // counter globale con il numero di frames arrivati dal DSP
% 13	uint32 relais_board_out;                        // variabile con lo status dell'uscita della scheda relais
% 14	uint16 serial_number;                           // numero seriale della scheda
%       uint16 dummy_word1;                             // dummy word per allineare a 32 bit
% 15	uint32 relais_board_out_1;                      // variabile con lo status dell'uscita della seconda scheda relais
% 16	uint32 enable_pos_acc;                          // abilita(1) / disabilita(0) la lettura dal secondario degli accumulatori di posizione oltre a quelli di corrente per il calcolo mantegazza style
% 17	uint32 diagnostic_record_ptr;                   // puntatore alla memoria che contiene la diagnostica da scaricare in SDRAM
% 18	uint32 diagnostic_record_len;                   // dimensione del pacchetto diagnostico da salvare in DWORD
% 19	uint32 enable_master_diag;                      // abilita(1) / disabilita(0) la spedizione della diagnostica in modo automatico
% 20	uint8  remote_mac_address[6];                   // indirizzo mac della scheda dove spedire la diagnostica
%       uint8  remote_ip_address[4];                    // indirizzo ip della scheda dove spedire la diagnostica
%       uint16 remote_udp_port;                         // porta UDP alla quale mandare il pacchetto diagnostico BCU master
% 23	uint32 rd_diagnostic_record_ptr;                // puntatore alla SDRAM della diagnostica da spedire via ETH
% 24	uint32 wr_diagnostic_record_ptr;                // puntatore alla SDRAM della diagnostica da salvare in SDRAM
% 25	na_bcu_diagnostic_struct diagnostic_area;       // area contenete i valori diagnostici
% typedef volatile struct
% 	{
% 25	uint16 stratix_temp;     // temperatura del chip stratix
%   	uint16 power_temp;       // temperatura board nella zona di power
% 
% 26	uint32 bck_digitalIO;    // canale spi sul backplane (MAX7301 U1)
% 	
% 27  	uint16 voltage_vccl;     // canale spi sul backplane (AD7927 U30)
%   	uint16 voltage_vcca;     // canale spi sul backplane
% 28	uint16 voltage_vssa;     // canale spi sul backplane
%       uint16 voltage_vccp;     // canale spi sul backplane
% 29	uint16 voltage_vssp;     // canale spi sul backplane
%       uint16 current_vccl;     // canale spi sul backplane
% 30    uint16 current_vcca;     // canale spi sul backplane
%       uint16 current_vssa;     // canale spi sul backplane
% 
% 31	uint16 current_vccp;     // canale spi sul backplane (AD7927 U31)
%   	uint16 current_vssp;     // canale spi sul backplane
% 32	uint16 tot_current_vccp; // canale spi sul backplane
%   	uint16 tot_current_vssp; // canale spi sul backplane
% 33	uint16 tot_current_vp;   // canale spi sul backplane
% 
%   	uint16 in0_temp;         // canale spi sul backplane (AD7927 U32)
% 34    uint16 in1_temp;         // canale spi sul backplane
%   	uint16 out0_temp;        // canale spi sul backplane
% 35    uint16 out1_temp;        // canale spi sul backplane
%   	uint16 ext_umidity;      // canale spi sul backplane
% 36    uint16 pressure;         // canale spi sul backplane
% 	} na_bcu_diagnostic_struct;


pbGainMatrixFILE='S:\Progetti\Engineering\Lbt\Matlab\PowerBackplane_test\pbGainMatrix.mat';
            
MGP_DEFINE();

a=mgp_op_rd_sram(255,255,2,'0x3800E','uint16');
data_struct.serial_number=a(1);
data_struct.pb_serial_number=a(2);

pbSerNum=data_struct.pb_serial_number;

try
    gain_matrix=evalin('base','pbGainMatrix;');
catch
    if exist(pbGainMatrixFILE,'file'),
        load(pbGainMatrixFILE);
        assignin('base','pbGainMatrix',pbGainMatrix);
        gain_matrix=pbGainMatrix;
        fprintf(1,'pbGainMatrix loaded from:\n\t%s\n',pbGainMatrixFILE);
    else
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
            0.0000E+00	1.9299E-02	-2.5844E+01	% HUMIDITY
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
        gain_matrix(:,:,pbSerNum)=nom_gain_matrix;
        disp('WARNING: pbGainMatrix not available - default gains used!');
    end
end

if size(gain_matrix,3) < pbSerNum,
    error('PowerBackplane serial number exceeds pbGainMatrix size!');
end;

a=mgp_op_rd_sram(255,255,1,'0x3801C','uint16');
data_struct.stratix_temp=a(1)*0.0078125;
data_struct.power_temp=a(2)*0.0078125;
a=mgp_op_rd_sram(255,255,1,'0x3801D','uint32');
data_struct.bck_digitalIO=dec2bin(a(1));
a=mgp_op_rd_sram(255,255,10,'0x3801E','uint16');
a=a(1:19); a=a(:);

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

a=mgp_op_rd_sram(255,255,2,'0x3800A','uint16');
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
