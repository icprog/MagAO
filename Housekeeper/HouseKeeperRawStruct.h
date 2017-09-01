// @File: housekeeper.h
//
// Definitions and prototypes for the module housekeeper
//
// HISTORY
// 21 mar 2005 Created by Lorenzo Busoni (LB) <lbusoni@arcetri.astro.it>
//
// 28 jun 2005 Aligned to Mario modifications (LB)
//
//             In na_bcu_nios_fixed_area_struct:
//             - software_release passes from uint16 to uint32 and contains flags useful to
//             retrieve the #defined variables during compiling
//             - mac_address is now uint8[6] with no more 2 padding bytes
//             - uint16 serial_number added
//             - uint16 dummy_word added to pad to 32-bit words
//             - uint32 relais_board_out_1 added
//
//             In na_dsp_nios_fixed_area_struct:
//             From	uint16 software_release;  uint16 logic_release;  uint16 dummy_word1;
//             to    uint16 logic_release; uint32 software_release; uint16 serial_number; uint16 dummy_word1;
//
// 08 aug 2005 by Armando Riccardi (AR)
//             Alignment to fixed area record structure introduced in BCU Nios v4.8
//             added: enable_pos_acc, diagnostic_record_ptr, diagnostic_record_len, enable_master_diag,
//                    remote_mac_address, remote_ip_address, remote_udp_port, rd_diagnostic_record_ptr,
//                    wr_diagnostic_record_ptr.
//
// 11 jan 2006 (LB)
//             New Power BackPlane. 
//             added: pressure, dummy and reset_status   to na_bcu_diagnostic_struct
//             added: local_current_threshold, vp_set, pb_serial_number, enable_hl_commands  
//                    to na_bcu_nios_fized_area_struct
//             removed: local_current_thres_pos, local_current_thres_neg, dummy_word1  
//                      from na_bcu_nios_fized_area_struct
//
// 12 Dec 2006 (LB)
//              Added explanation of various fields in the diagnostic areas. 
//              Source: email of Mario Andrighettoni (12Dec06)
//              
// 13 Jan 2007 (LB)
//              Data layout re-organized
//              Source: email of Mario Andrighettoni (10Jan07) "ultima (?) versione modifiche"
//              file Modifiche_LBT_SW_dopo_il_28-09-06_v1.5.doc
//
//@
#ifndef HOUSEKEEPER_INCLUDE

#define HOUSEKEEPER_INCLUDE


using namespace Arcetri;

namespace Arcetri {
	namespace HouseKeeperRaw {

        /*
         * Memory addresses of variables in the BCU/DSP storage areas.
         * NOTE: this should be aligned with corresponding files located in the "prog"
         * folder.
         */
        class AddressMap {
            public:
                static const int   BCU_NIOS_FIXED_AREA     =  0x00038000;  // crateBcu/dsp/1.00/bcu_sram_map.txt
                static const int   BCU_DIAGNOSTICS_AREA    =  0x0003801C; 
                static const int   SIGGEN_NIOS_FIXED_AREA  =  0x00018171;  // dspBoard/dsp/5.20/sram_map.txt
                static const int   DSP_NIOS_FIXED_AREA     =  0x00018171;

                static const int   DSP_GLOBAL_COUNTER      =  0x00080000;  // dsp dspBoard/dsp/5.20/dsp_dm_map.txt
                static const int   DSP_FF_UD_CURRENT       =  0x0008003C;  // act
                static const int   DSP_POS_COMMAND         =  0x0008004C;  // act
                static const int   DSP_BIAS_CURRENT        =  0x00080084;  // act
                static const int   DSP_FULLBIAS_CURR       =  0x00080088;  // act
                static const int   DSP_STEP_PTR_PRESHAPER_CMD  = 0x0008009C;  // act
                static const int   DSP_STEP_PTR_PRESHAPER_CURR = 0x000800B0;  // act
                static const int   DSP_FLOAT_ADC_VALUE     =  0x000800B8;  // act
                static const int   DSP_DIST_THRESHOLD      =  0x000800C8;  // act 
                static const int   DSP_DISTANCE            =  0x000800CC;  // act
                static const int   DSP_CONTROL_ENABLE      =  0x000800E4;  // act
                static const int   DSP_FLOAT_DAC_VALUE     =  0x000800E8;  // act
                static const int   DSP_ERR_LOOP_GAIN       =  0x00080100;  // act
                static const int   DSP_SPEED_LOOP_GAIN     =  0x0008010C;  // act
                static const int   DSP_BIAS_COMMAND        =  0x0008011C;  // act
                static const int   DSP_FULLBIAS_CMD        =  0x00080120;  // act
                static const int   DSP_CMOL_A              =  0x00080150;  // act
                static const int   DSP_FMOL_A              =  0x00080154;  // act
                static const int   DSP_CMOL_B              =  0x00080158;  // act
                static const int   DSP_FMOL_B              =  0x0008015C;  // act
                static const int   DSP_POS_CHECK_CNT       =  0x00080188;  // act
                static const int   DSP_MODAL_SKIPFRAME     =  0x00089643;  // dsp
                static const int   DSP_COMMAND_SKIPFRAME   =  0x000898E5;  // dsp
                static const int   DSP_FORCE_SKIPFRAME     =  0x00089E2D;  // dsp

                static const int   SWITCH_PARAM_SELECTOR         =  0x0000102E;  // switchbcu
                static const int   SWITCH_OFFLOAD_SELECTOR       =  0x0000102F;  // switchbcu
                static const int   SWITCH_MIRR_FRAMES_COUNTER    =  0x00001035;  // switchbcu
                static const int   SWITCH_SAFE_SKIP_FRAME_CNT    =  0x00001036;  // switchbcu
                static const int   SWITCH_PENDING_SKIP_FRAME_CNT =  0x00001037;  // switchbcu
                static const int   SWITCH_WFS_GLOBAL_TIMEOUT     =  0x00001038;  // switchbcu
                static const int   SWITCH_WFS_GLOBAL_TIMEOUT_CNT =  0x00001039;  // switchbcu
                static const int   SWITCH_NUM_FL_TIMEOUT         =  0x0000103B;  // switchbcu
                static const int   SWITCH_NUM_CRC_ERR            =  0x0000103C;  // switchbcu
                static const int   SWITCH_SCSTARTRTR             =  0x00080640;  // switchbcu

        };

    }
}

//// Indirizzi presi da sram_map.txt
//// TODO leggerli dal file in run-time
////BCU SRAM memory map
//#define bcu_nios_fixed_area     0x00038000   //na_bcu_nios_fixed_area_offset/4 = offset to CrateID
//#define bcu_diagnostics_area    0x0003801C   //bcu_nios_fixed_area+14 = diagnostics area offset
////SIGGEN SRAM memory map
//#define siggen_nios_fixed_area  0x00018171   // as a DSP board
////#define siggen_diagnostics_area 0x00018173   // as a DSP board
////DSP SRAM memory map
//#define dsp_nios_fixed_area     0x00018171   //na_dsp_nios_fixed_area_offset/4+(NIOS_INTERPRETER_SPACE+2)/4+1 = offset to who_ami
////#define dsp_diagnostics_area    0x00018173   //dsp_nios_fixed_area+2 = diagnostics area offset

// ------------------- Update Status structure -----------------
#include "AdSecConstants.h"
using namespace Arcetri::AdSecConstants;

typedef struct
{
    uint32    global_counter[BcuMirror::N_DSP];
    float32   ff_ud_current[BcuMirror::N_CH];
    float32   pos_command[BcuMirror::N_CH];
    float32   bias_current[BcuMirror::N_CH];
    float32   fullbias_curr[BcuMirror::N_CH];
    float32   step_ptr_preshaper_cmd[BcuMirror::N_CH];
    float32   step_ptr_preshaper_curr[BcuMirror::N_CH];
    float32   float_adc_value[BcuMirror::N_CH];
    float32   dist_threshold[BcuMirror::N_CH];
    float32   distance[BcuMirror::N_CH];
    float32   control_enable[BcuMirror::N_CH];
    float32   float_DAC_value[BcuMirror::N_CH];
    float32   err_loop_gain[BcuMirror::N_CH];
    float32   speed_loop_gain[BcuMirror::N_CH];
    float32   bias_command[BcuMirror::N_CH];
    float32   fullbias_cmd[BcuMirror::N_CH];
    float32   cmol_a[BcuMirror::N_CH];
    float32   cmol_b[BcuMirror::N_CH];
    float32   fmol_a[BcuMirror::N_CH];
    float32   fmol_b[BcuMirror::N_CH];
    uint32    pos_check_cnt[BcuMirror::N_CH];
    float32   modal_skipframe[BcuMirror::N_DSP];
    float32   command_skipframe[BcuMirror::N_DSP];
    float32   force_skipframe[BcuMirror::N_DSP];
    uint32    switch_param_selector;
    uint32    switch_offload_selector;
    uint32    switch_mirr_frames_counter;
    uint32    switch_safe_skip_frame_cnt;
    uint32    switch_pending_skip_frame_cnt;
    uint32    switch_wfs_global_timeout;
    uint32    switch_wfs_global_timeout_cnt;
    uint32    switch_num_fl_timeout;
    uint32    switch_num_crc_err;
    uint32    switch_scstartrtr;
} update_status_struct;



// ------------------ Raw structures ---------------------------
// These structures are used to retrieve data from the electronics
// and hold "raw" data in unreadable form 

// Presa da bcu_diagnostic.h di Mario

typedef struct
	{
	int16  stratix_temp;     // temperatura del chip stratix
	int16  power_temp;       // temperatura board nella zona di power

	uint32 bck_digitalIO;    // see below
	
	uint16 voltage_vccl;     // voltage logic (3.3V)
	uint16 voltage_vcca;     // voltage analogic + (12V)
	uint16 voltage_vssa;     // voltage analogic - (12V)
	uint16 voltage_vccp;     // voltage power + (11V)
	uint16 voltage_vssp;     // voltage power - (11V)
	
    uint16 current_vccl;     // current logic  
	uint16 current_vcca;     // current analogic + 
	uint16 current_vssa;     // current analogic -
	uint16 current_vccp;     // current power + 
	uint16 current_vssp;     // current power - 

	uint16 tot_current_vccp; // total current on + branch of "power" power-supply (all 6 crates). Only on master BCU.
	uint16 tot_current_vssp; // total current on - branch of "power" power-supply (all 6 crates). Only on master BCU.
	uint16 tot_current_vp;   // total current (sum of + and - branches) of "Power" power-supply (all 6 crates). Only on master BCU.
	
    uint16 in0_temp;         // canale spi sul backplane (AD7927 U32)
	uint16 in1_temp;         // canale spi sul backplane
	uint16 out0_temp;        // canale spi sul backplane
	uint16 out1_temp;        // canale spi sul backplane
	
    uint16 ext_umidity;      // canale spi sul backplane
	uint16 pressure;         // canale spi sul backplane
	uint16 dummy;

	uint32 reset_status;     // dword con lo status delle linee di reset
} na_bcu_diagnostic_struct;  // 13 dword = 52 bytes

////////////////////////////////////////////////////////////////////////////
// From an email of Mario Andrighettoni 12.12.06
////////////////////////////////////////////////////////////////////////////
/*
 - bck_digitalIO sono tutti gli ingressi digitali che arrivano dal power backplane

bit #0 - n_driver_enable                  -> input (0 = driver abilitati, 1 = driver disabilitati)
bit #1 - not used                -> input
bit #2 - not used                -> input
bit #3 - is_master               -> input (1 = crate master, 0 = crate slave) il crate #0 e' il master delle sicurezze HW mentre gli altri sono tutti slave
bit #4 - bus_sys_fault_n         -> input (0 = quando una scheda del crate (BCU, DSP, SIGGEN o POWER_BCK) ha generato un power fault dovuto ad un malfunzionamento altrimenti 1)
bit #5 - ena_temp_n              -> output (default = 1) usato per l'aquisizione dei sensori di temperatura (non ti interessa)
bit #6 - vcc_fault               -> input (1 = quando si verifica un fault sulle alimentazioni)
bit #7 - v_p_enable_ctrl                  -> output (default = 0)
bit #8 - v_a_enable_ctrl                  -> output (default = 0)
bit #9 - ac_power_fault0         -> input (1 = quando si verifica un fault sul primo alimentatore 48V)
bit #10 - ac_power_fault1        -> input (1 = quando si verifica un fault sul secondo alimentatore 48V)
bit #11 - ac_power_fault2        -> input (1 = quando si verifica un fault sul terzo alimentatore 48V)
bit #12 - ic_disconnected_n     -> input (0 = quando il cavo di interconnessione tra i vari crates e' disconnesso)
bit #13 - overcurrent            -> input (1 = quando entra la protezione HW sulle correnti a coils)
bit #14 - crate_id0              -> input (4 bits che indicano il numero del crate da 0 a 7)
bit #15 - crate_id1              -> input (...)
bit #16 - crate_id2              -> input (...)
bit #17 - crate_id3              -> input (...)
bit #18 - not used               -> input
bit #19 - dp_rdy                 -> input (seganle per uso interno)


- reset_status -> stato dei segnali di reset ed altro:

bit #0: fpga_rst_sig                   -> stato della linea di reset della FPGA
bit #1: busdsp_rst_sig                 -> stato della linea di reset del BUS
bit #2: dsp_rst_sig                    -> stato della linea di reset del DSP locale
bit #3: flash_rst_sig                  -> stato della linea di reset della FLASH
bit #4: pci_rst_sig                    -> stato della linea di reset del PCI
bit #5: bus_driver_enable              -> stato del segnale locale per abilitare i coils (abilitato di default)
bit #6: bus_driver_enable_status       -> stato del segnale globale di enable dei coil letto dal bus
bit #7: bus_power_fault_ctrl           -> stato del segnale generato internamente dalla BCU nel caso di malfunzionamento
bit #8: bus_power_fault                -> stato segnale di fault globale letto dal bus (puo' essere generato dalla BCU, DSP, SIGGEN o POWER_BCK)
bit #9: enable system watchdog         -> flag per abilitare il watchdog interno della BCU sui comandi ethernet 
                                        (se dopo un certo tempo non arriva un comando eth il sistema genera un reset)
*/

//////////////////////////////////////////////////////////////////////////////////////



// Presa da dsp_diagnostic.h di Mario
typedef struct
{
	int16  stratix_temp;     // temperatura del chip stratix
	int16  power_temp;       // temperatura board nella zona di power
	int16  dsps_temp;        // temperatura board tra i due dsp
	int16  driver_temp;      // temperatura board tra i driver di corrente
	uint32 driver_status;    // see below 
	uint16 coil_current[8]; // corrente sui coils
} na_dsp_diagnostic_struct;  // 7dword = 28bytes

////////////////////////////////////////////////////////////////////
// From an email of Mario Andrighettoni 12.12.06
//////////////////////////////////////////////////////////////////////
/*
- driver_status -> stato dei segnali di reset ed altro:

bit   #0: fpga_rst_sig                 -> stato della linea di reset della FPGA
bit   #1: dsp0_rst_sig                 -> stato della linea di reset del DSP#0 locale
bit   #2: dsp1_rst_sig                 -> stato della linea di reset del DSP#1 locale
bit   #3: flash_rst_sig                -> stato della linea di reset della FLASH
bit   #4: driver_enable_sig(0)         -> stato della linea di coil enable del driver #0
bit   #5: driver_enable_sig(1)         -> stato della linea di coil enable del driver #1
bit   #6: driver_enable_sig(2)         -> stato della linea di coil enable del driver #2
bit   #7: driver_enable_sig(3)         -> stato della linea di coil enable del driver #3
bit   #8: driver_enable_sig(4)         -> stato della linea di coil enable del driver #4
bit   #9: driver_enable_sig(5)         -> stato della linea di coil enable del driver #5
bit #10:  driver_enable_sig(6)          -> stato della linea di coil enable del driver #6
bit #11:  driver_enable_sig(7)          -> stato della linea di coil enable del driver #7
bit #12:  bus_power_fault_ctrl          -> segnale generato internamente dalla DSP nel caso di malfunzionamento
bit #13:  bus_power_fault               -> (rd only) segnale di fault generico letto dal bus (puo' essere generato dalla BCU, DSP, SIGGEN o POWER_BCK)
bit #14:  bus_driver_enable             -> (rd only) stato del segnale di enable dei coil

added 070327 see Modifiche_LBT_SW_*_v1.7 (email Mario 070330)
bit #15:  enable_dsp_watchdog           -> stato del watchdog sul codice DSP
bit #16:  dsp0_watchdog_expired         -> a uno quando si attiva il watchdog sul DSP #0
bit #17:  dsp1_watchdog_expired         -> a uno quando si attiva il watchdog sul DSP #1

*/

///////////////////////////////////////////////////////////////////////////////////////////////////////


// siggen boards have the same diagnostic area than dsp boards
typedef  na_dsp_diagnostic_struct  na_siggen_diagnostic_struct;


// Presa da bcu_nios_fixed_area.h di Mario
//#define MGP_HLEN                     14   // in bytes
//#define MAX_MGP_DATA_SIZE            1456 // in bytes
//#define NIOS_INTERPRETER_SPACE       MGP_HLEN+MAX_MGP_DATA_SIZE
#define NUM_BOARD_ON_CRATE           20

typedef  struct
	{
	uint16 crateID;                                 // identificazione del crate
	uint16 who_ami;                                 // identificazione della board sul crate
	uint32 software_release;                        // versione software del Nios
	uint16 logic_release;                           // lettura del PIO con la versione della logica
	uint8  mac_address[6];                          // indirizzo mac della scheda
	uint8  ip_address[4];                           // indirizzo ip della scheda
	uint8  crate_configuration[NUM_BOARD_ON_CRATE]; // stato del crate
	uint16 local_current_threshold;                 // livello di corrente locale di thresholds (positiva e negativa)
	uint16 vp_set;                                  // impostazione tensione rami power (positivo e negativo)
	uint16 total_current_thres_pos;                 // livello di corrente totale di thresholds (positiva)
	uint16 total_current_thres_neg;                 // livello di corrente totale di thresholds (negativa)
	uint32 frames_counter;                          // counter globale con il numero di frames arrivati dal DSP
	uint32 relais_board_out;                        // variabile con lo status dell'uscita della scheda relais
	uint16 serial_number;                           // numero seriale della scheda BCU
	uint16 pb_serial_number;                        // numero seriale del power backplane
	uint32 relais_board_out_1;                      // variabile con lo status dell'uscita della seconda scheda relais
	uint32 enable_hl_commands;                      // flag che indica se sono abilitati o disabilitati i comandi al secondario MMT
	uint32 enable_pos_acc;                          // abilita(1) / disabilita(0) la lettura dal secondario degli accumulatori di 
							// posizione oltre a quelli di corrente per il calcolo mantegazza style
	uint32 diagnostic_record_ptr;                   // puntatore alla memoria che contiene la diagnostica da scaricare in SDRAM
	uint32 diagnostic_record_len;                   // dimensione del pacchetto diagnostico da salvare in DWORD
	uint16 enable_master_diag;                      // abilita(1) / disabilita(0) la spedizione della diagnostica in modo automatico
	uint16 decimation_factor;                       // fattore di decimazione dei frame spediti via Master BCU
	uint8  remote_mac_address[6];                   // indirizzo mac della scheda dove spedire la diagnostica
	uint8  remote_ip_address[4];                    // indirizzo ip della scheda dove spedire la diagnostica
	uint16 remote_udp_port;                         // porta UDP alla quale mandare il pacchetto diagnostico BCU master
	uint32 rd_diagnostic_record_ptr;                // puntatore alla SDRAM della diagnostica da spedire via ETH
	uint32 wr_diagnostic_record_ptr;                // puntatore alla SDRAM della diagnostica da salvare in SDRAM
	uint32 rd_byte_serialAIA;                       // numero di byte ricevuti dalla seriale AIA
	uint32 wr_byte_serialAIA;                       // numero di byte da spedire alla seriale AIA mettere ad uno il bit 31 per partire
	na_bcu_diagnostic_struct BCU_diagnostic;        // area contenete i valori diagnostici della BCU
	na_dsp_diagnostic_struct DSP_diagnostic[NUM_BOARD_ON_CRATE-1]; // area contenete i valori diagnostici delle schede DSP se presenti
    
//    na_bcu_diagnostic_struct diagnostic_area;       // area contenete i valori diagnostici
} na_bcu_nios_fixed_area_struct;          // na_bcu_diagnostic_struct ha offset 28dword = 112B  = 0x1C 

typedef  struct
	{
   //uint32 internal_use_area[369];
	//uint8  nios_interpreter_area[NIOS_INTERPRETER_SPACE]; // area di copia di un comando backplane
	//uint16 dummy_word;                                    // dummy word per allineare a 32 bit
	//uint32 nios_interpreter_reply;                        // areola per il reply ad un comando backplane
	uint16 who_ami;                                       // identificazione della board
	uint16 logic_release;                                 // lettura del PIO con la versione della logica
	uint32 software_release;                              // versione software del Nios
	uint16 serial_number;                                 // numero seriale della scheda
	uint16 dummy_word1;                                   // dummy word per allineare a 32 bit
	float  ADC_spi_offset[8];                             // valori di taratura degli ADC SPI
	float  ADC_spi_gain[8];
	float  ADC_offset[8];                                 // valori di taratura degli ADC
	float  ADC_gain[8];
	float  DAC_offset[8];                                 // valori di taratura dei DAC
	float  DAC_gain[8];
	uint32 diagnostic_record_ptr;                         // puntatore alla memoria che contiene la diagnostica da scaricare in SDRAM
	uint32 diagnostic_record_len;                         // dimensione del pacchetto diagnostico da salvare
	uint32 rd_diagnostic_record_ptr;                      // puntatore alla SDRAM della diagnostica da spedire (not used)
	uint32 wr_diagnostic_record_ptr;                      // puntatore alla SDRAM della diagnostica da salvare in SDRAM
	na_dsp_diagnostic_struct diagnostic_area;             // area contenete i valori diagnostici
} na_dsp_nios_fixed_area_struct;

// siggen boards have the same diagnostic area than dsp boards
typedef  na_dsp_nios_fixed_area_struct  na_siggen_nios_fixed_area_struct;


#endif //HOUSEKEEPER_INCLUDE
