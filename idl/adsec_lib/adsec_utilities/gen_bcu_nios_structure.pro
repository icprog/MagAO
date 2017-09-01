; $Id: gen_bcu_nios_structure.pro,v 1.2 2007/02/07 18:06:08 marco Exp $

function gen_bcu_nios_structure

uint16 = 0U
uint8 = 0B
uint32 = 0UL
NUM_BOARD_ON_CRATE = 20

bcu_nios_data = $
{ $
    crateID                :uint16, $; // identificazione del crate
    who_ami                :uint16, $; // identificazione della board sul crate
    software_release       :uint16, $; // lettura del PIO con la versione della logica
    logic_release          :uint16, $; // lettura del PIO con la versione della logica
    mac_address            :replicate(uint8,8), $; // indirizzo mac della scheda (+ 2 dummy words per allineare a 32 bit)
    ip_address             :replicate(uint8,4), $; // indirizzo ip della scheda
    crate_configuration    :replicate(uint8,NUM_BOARD_ON_CRATE), $; // stato del crate
    local_current_thres_pos:uint16, $; // livello di corrente locale di thresholds (positiva)
    local_current_thres_neg:uint16, $; // livello di corrente locale di thresholds (negativa)
    total_current_thres_pos:uint16, $; // livello di corrente totale di thresholds (positiva)
    total_current_thres_neg:uint16, $; // livello di corrente totale di thresholds (negativa)
    wfs_frames_counter         :uint32, $; // counter globale con il numero di frames arrivati dal DSP
    relais_board_out       :uint32  $; // variabile con lo status dell'uscita della scheda relais
}

return, bcu_nios_data
end
