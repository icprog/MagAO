AO_CONST = { telescope_side:"",   $
             unit:"",   $
             shell:"",   $
             msgd_link:0B,        $
             conf_file:"",        $
             dll_file:"",         $
             side_master:0B,      $
             LOG_LEV_FATAL:   3L, $
             LOG_LEV_ERROR:   4L, $
             LOG_LEV_WARNING: 5L, $
             LOG_LEV_INFO:    6L, $
             LOG_LEV_DEBUG:   7L, $
             LOG_LEV_TRACE:   8L, $
             LOG_LEV_DISABLE: 2L  $
           }

AO_STATUS = {                                 $
              fsm_state:"",                   $
              n_slope:0L,                     $
              n_slope_delay:0L ,              $
              n_modes_delay:0L ,              $
              n_modes2correct:0L ,            $
              oversampling_time:0.0 ,         $
              b0_a:"",                        $
              b0_b:"",                        $
              a_delay:"",                     $
              b_delay_a:"",                   $
              b_delay_b:"",                   $
              m2c:"",                         $
              g_gain_a:"",                    $
              g_gain_b:"",                    $
              disturb:"",                     $
              disturb_status:0L,              $
              shape:"",                       $
              ff_matrix:  "",                 $
              pmpos: "",                      $
              pmfor: "",                      $
              pmhofor: "",                    $
              pmz: "",                        $
              act_w_pos:bytarr(672),          $
              act_w_curr:bytarr(672),         $
              mean_gap: 1.0,                  $
              pgain: fltarr(672),             $
              dgain: fltarr(672),             $
              icc_cutfreq: fltarr(672),       $
              icc_dcgain: fltarr(672),        $
              wind_speed: 0.0,                $
              adam_in: intarr(8),             $
              adam_out: intarr(8),            $
              zern_applied:fltarr(22),        $
              ref_pos:fltarr(672),            $
              ref_for:fltarr(672),            $
              ref_wave:"",                    $
              flat_list:"",                   $   
              configuration:ptr_new(/ALLOC),  $
              firmware:ptr_new(/ALLOC),       $
              last_flat_written:"",           $
              last_flat_loaded: "",           $
              instrument_set: ""              $
}

AO_ERROR = 0L
DEFSYSV, '!AO_CONST', ao_const
DEFSYSV, '!AO_STATUS', ao_status
DEFSYSV, '!AO_ERROR', ao_error

             
              




