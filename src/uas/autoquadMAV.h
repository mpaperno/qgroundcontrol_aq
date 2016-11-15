#ifndef AUTOQUADMAV_H
#define AUTOQUADMAV_H

#include <stdint.h>

#define MAV_DEFAULT_SYSTEM_COMPONENT            MAV_COMP_ID_MISSIONPLANNER
#define MAV_ADJUSTABLE_PARAMS_LIST_COMPONENT    MAV_COMP_ID_SYSTEM_CONTROL
#define MAV_CUSTOM_VERSION_PARSE_REGEX   "^(?:A(?:auto)?Q(?:quad)? FW ver: )?((\\d+)\\.(\\d+)(?:[\\-\\s\\dA-Zrev]*[\\.b](\\d+))?[\\s\\-\\dA-Z_]*),?((?: ?(?:HW ver: (\\d+) ?)?(?:hw)?(?:rev)?\\.?(\\d+)(?:\\.(\\d+))?))?"

namespace AUTOQUADMAV {

    enum mavlinkCustomDataSets {
        AQMAV_DATASET_LEGACY1 = 0,	// legacy sets can eventually be phased out
        AQMAV_DATASET_LEGACY2,
        AQMAV_DATASET_LEGACY3,
        AQMAV_DATASET_ALL,		// use this to toggle all datasets at once
        AQMAV_DATASET_GPS,
        AQMAV_DATASET_UKF,
        AQMAV_DATASET_SUPERVISOR,
        AQMAV_DATASET_MCU,
        AQMAV_DATASET_GIMBAL,
        AQMAV_DATASET_MOTORS,
        AQMAV_DATASET_MOTORS_PWM,
        AQMAV_DATASET_NAV,
        AQMAV_DATASET_IMU,
        AQMAV_DATASET_DEBUG,
        AQMAV_DATASET_RC,
        AQMAV_DATASET_CONFIG,
        AQMAV_DATASET_IMU2,
        AQMAV_DATASET_ENUM_END
    };

    enum commStreamTypes {
        COMM_TYPE_NONE          = 0,
        COMM_TYPE_MULTIPLEX	    = (1<<0),
        COMM_TYPE_MAVLINK	    = (1<<1),
        COMM_TYPE_TELEMETRY	    = (1<<2),
        COMM_TYPE_GPS           = (1<<3),
        COMM_TYPE_RX_TELEM	    = (1<<4),
        COMM_TYPE_CLI           = (1<<5),
        COMM_TYPE_OMAP_CONSOLE  = (1<<6),
        COMM_TYPE_OMAP_PPP	    = (1<<7)
    };

    // bitmasks for CONFIG_FLAGS param
    enum configFlags {
        CONFIG_FLAG_SAVE_ADJUSTED   = (1<<0),   // save adjusted params values back to flash/SD, true/false
        CONFIG_FLAG_ALWAYS_ALLOW_HF = (1<<1),   // enable/disable heading-free mode option in all flight modes (not just DVH)
        CONFIG_FLAG_PID_CTRL_TYPE_C = (1<<2),   // use cascading PID controller type
        CONFIG_FLAG_DISABLE_MSC     = (1<<3),   // disable mass storage component on USB connection
        CONFIG_FLAG_INVRT_TCUT_AUTO = (1<<4),   // sharply scale (cut) throttle when inverted and in altitude-hold
        CONFIG_FLAG_INVRT_TCUT_MAN  = (1<<5),   // cut throttle when inverted in manual modes
        CONFIG_FLAG_MVLNK_STREAM_RC = (1<<6),   // send Mavlink RC Channels message by default
        CONFIG_FLAG_NAV_GUIDED_PH   = (1<<7),   // allow remote guidance commands while in PH mode (vs. only in GUIDED mode)
    };

    enum SPortConfigFlags {
        SPORT_CFG_SEND_CUSTOM  = (1<<4),
        SPORT_CFG_SEND_ACC	   = (1<<5),
        SPORT_CFG_WAIT_GPS	   = (1<<6),
        SPORT_CFG_WAIT_ALT	   = (1<<7),
        SPORT_CFG_SEND_TXT_MSG = (1<<8),
    };

#ifdef _MSC_VER
    #pragma pack(push,1)
    typedef struct {
        unsigned __int64 state :    3;
        unsigned __int64 vin :	    12;	// x 100
        unsigned __int64 amps :	    14;	// x 100
        unsigned __int64 rpm :	    15;
        unsigned __int64 duty :	    8;	// x (255/100)
        unsigned __int64 temp :     9;  // (Deg C + 32) * 4
        unsigned __int64 errCode :  3;
    } esc32CanStatus_t;
    #pragma pack(pop)
#else
    struct esc32CanStatus {
        unsigned int state :    3;
        unsigned int vin :	    12;	// x 100
        unsigned int amps :	    14;	// x 100
        unsigned int rpm :	    15;
        unsigned int duty :	    8;	// x (255/100)
        unsigned int temp :     9;  // (Deg C + 32) * 4
        unsigned int errCode :  3;
    } __attribute__((__packed__));
    typedef esc32CanStatus esc32CanStatus_t;
#endif

}

#endif // AUTOQUADMAV_H
