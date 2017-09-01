//@File: ParamBlockSelector.h
//
// Definitions for BCU's param block selector bits
//@

#define PBS_BLOCK                      (0x01)      // Block enable 0/1
#define PBS_SH                         (0x02)      // Slope SH
#define PBS_TOMOGRAPHY                 (0x04)      // Slope Tomography
#define PBS_CONSTANT                   (0x08)      // Slope Constant
#define PBS_SLOPELUT                   (0x10)      // Slopes LUT enable
#define PBS_MANTEGAZZA_STYLE           (0x20)      // Mantegazza style
#define PBS_DIAGNOSTIC                 (0x40)      // Diagnostic enable
#define PBS_FASTLINK                   (0x80)      // Fastlink enable
#define PBS_DISTURB_WFS                (0x1000)    // WFS disturbance enable 
#define PBS_DISTURB_OVS                (0x8000)    // OVS disturbance enable

