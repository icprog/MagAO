

// 
// These values are taken from Mario's "Uso della BCU 47.doc" 
//
#define FRMGRAB_FRAMESCOUNTER_DEFAULT                       (0)
#define FRMGRAB_RESETFRAME_DEFAULT                          (0)
#define FRMGRAB_FRAMESCOUNTERIRQ_DEFAULT                    (0)
#define FRMGRAB_RESETFRAMEIRQ_DEFAULT                       (0)
#define FRMGRAB_PIXELAREASIZE_DEFAULT                       (29120)
//#define FRMGRAB_TOTFRAMEPIXELS_DEFAULT                      (131072) //numero di pixel del CCD/8  (1x1)
//#define FRMGRAB_TOTFRAMEPIXELS_DEFAULT                      (32768) //numero di pixel del CCD/8     (2x2)
//#define FRMGRAB_TOTFRAMEPIXELS_DEFAULT                      (8192) //numero di pixel del CCD/8     (4x4)
#define FRMGRAB_TOTFRAMEPIXELS_DEFAULT                      (512) //numero di pixel del CCD/8     (16x16)
// dipende dal CCD utilizzato:
// CCD39 – 6400/8    = 800 
// CCD47 – 1048576/8 = 131072
// CCD50 – 16384/8   = 2048

#define FRMGRAB_NUMPIXELSTOMOVEPTR_DEFAULT                  (0x00080000)
#define FRMGRAB_INPIXELAREAPTR_DEFAULT                      (0x00080004)
#define FRMGRAB_OUTPIXELAREAPTR_DEFAULT                     (0x00100000)

#define FRMGRAB_ENABLE_POS_ACC_DEFAULT                      (0)
#define FRMGRAB_DIAGNOSTIC_BUFF_PTR_DEFAULT                 FRMGRAB_OUTPIXELAREAPTR_DEFAULT
//#define FRMGRAB_DIAGNOSTIC_BUFF_LEN_DEFAULT                 (524288)   // (1x1)
//#define FRMGRAB_DIAGNOSTIC_BUFF_LEN_DEFAULT                 (131072)   // (2x2)
//#define FRMGRAB_DIAGNOSTIC_BUFF_LEN_DEFAULT                 (32768)   // (4x4)
#define FRMGRAB_DIAGNOSTIC_BUFF_LEN_DEFAULT                 (2048)   // (16x16)
#define FRMGRAB_ENABLE_MASTER_DIAG_DEFAULT                  (1)

