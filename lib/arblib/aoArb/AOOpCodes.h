#ifndef AOOPCODES_H_INCLUDE
#define AOOPCODES_H_INCLUDE

using namespace Arcetri::Arbitrator;

namespace Arcetri {
namespace Arbitrator {

/*
 * Codes for AOArbitrator commands
 */
class AOOpCodes {

    public:
        // The first available code is 2004 (see BasicOpCodes.h)
        // NOTE: This codes can be found in the Pload field of the Message
        //       with MsgCode = 14000

        // --- Commands exported by AOS --- //
        static const OpCode PRESET_FLAT     = 2008;
        static const OpCode PRESET_AO       = 2009;
        static const OpCode RESET_AO        = 2010;
        static const OpCode CHECKREF_AO     = 2011;
        static const OpCode ACQUIREREF_AO   = 2012;
        static const OpCode CORRECTMODES    = 2013;
        static const OpCode MODIFY_AO       = 2014;
        static const OpCode REFINE_AO       = 2015;
        static const OpCode START_AO        = 2016;
        static const OpCode OFFSETZ         = 2017;
        static const OpCode OFFSETXY        = 2018;
        static const OpCode PAUSE           = 2019;
        static const OpCode RESUME          = 2020;
        static const OpCode STOP            = 2021;
        static const OpCode GET_SNAP        = 2022;
        static const OpCode USER_PANIC      = 2023;
        static const OpCode POWER_ON_WFS    = 2024;
        static const OpCode POWER_OFF_WFS   = 2025;
        static const OpCode POWER_ON_ADSEC  = 2026;
        static const OpCode POWER_OFF_ADSEC = 2027;
        static const OpCode MIRROR_SET      = 2028;
        static const OpCode MIRROR_REST     = 2029;
        static const OpCode FAULT_RECOVERY  = 2030;
        static const OpCode STANDALONE_FAIL = 2031;
        static const OpCode SEEING_LMTD_FAIL= 2032;
        static const OpCode SETZERNIKES     = 2033;


        // --- Commands NOT exported by AOS --- //
        static const OpCode SET_AO_MODE           = 2100;
        static const OpCode REQUEST_AO_STATUS_CMD = 2101;

        static const OpCode ADJUST_GAIN     = 3001;
        static const OpCode ADJUST_INT_TIME = 3002;
        static const OpCode RECOVER_WFS_FAILURE = 3003;
        static const OpCode RECOVER_ADSEC_FAILURE   = 3004;
};

}
}

#endif /* AOOPCODES_H_INCLUDE */
