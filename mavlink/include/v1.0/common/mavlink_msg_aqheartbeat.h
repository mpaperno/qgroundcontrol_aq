// MESSAGE AQHEARTBEAT PACKING

#define MAVLINK_MSG_ID_AQHEARTBEAT 152

typedef struct __mavlink_aqheartbeat_t
{
 uint32_t custom_mode; ///< Navigation mode bitfield, see MAV_AUTOPILOT field is autopilot-specific.
 uint8_t type; ///< Type of the MAV (quadrotor, helicopter, etc., up to 15 types, defined in MAV_TYPE ENUM)
 uint8_t autopilot; ///< Autopilot type / class. defined in MAV_CLASS ENUM
 uint8_t base_mode; ///< System mode bitfield, see MAV_MODE_FLAGS ENUM in mavlink/include/mavlink_types.h
 uint8_t system_status; ///< System status flag, see MAV_STATUS ENUM
 uint8_t mavlink_version; ///< MAVLink version
} mavlink_aqheartbeat_t;

#define MAVLINK_MSG_ID_AQHEARTBEAT_LEN 9
#define MAVLINK_MSG_ID_152_LEN 9



#define MAVLINK_MESSAGE_INFO_AQHEARTBEAT { \
	"AQHEARTBEAT", \
	6, \
	{  { "custom_mode", NULL, MAVLINK_TYPE_UINT32_T, 0, 0, offsetof(mavlink_aqheartbeat_t, custom_mode) }, \
         { "type", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_aqheartbeat_t, type) }, \
         { "autopilot", NULL, MAVLINK_TYPE_UINT8_T, 0, 5, offsetof(mavlink_aqheartbeat_t, autopilot) }, \
         { "base_mode", NULL, MAVLINK_TYPE_UINT8_T, 0, 6, offsetof(mavlink_aqheartbeat_t, base_mode) }, \
         { "system_status", NULL, MAVLINK_TYPE_UINT8_T, 0, 7, offsetof(mavlink_aqheartbeat_t, system_status) }, \
         { "mavlink_version", NULL, MAVLINK_TYPE_UINT8_T, 0, 8, offsetof(mavlink_aqheartbeat_t, mavlink_version) }, \
         } \
}


/**
 * @brief Pack a aqheartbeat message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param type Type of the MAV (quadrotor, helicopter, etc., up to 15 types, defined in MAV_TYPE ENUM)
 * @param autopilot Autopilot type / class. defined in MAV_CLASS ENUM
 * @param base_mode System mode bitfield, see MAV_MODE_FLAGS ENUM in mavlink/include/mavlink_types.h
 * @param custom_mode Navigation mode bitfield, see MAV_AUTOPILOT field is autopilot-specific.
 * @param system_status System status flag, see MAV_STATUS ENUM
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_aqheartbeat_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t type, uint8_t autopilot, uint8_t base_mode, uint32_t custom_mode, uint8_t system_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_uint32_t(buf, 0, custom_mode);
	_mav_put_uint8_t(buf, 4, type);
	_mav_put_uint8_t(buf, 5, autopilot);
	_mav_put_uint8_t(buf, 6, base_mode);
	_mav_put_uint8_t(buf, 7, system_status);
	_mav_put_uint8_t(buf, 8, 3);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_aqheartbeat_t packet;
	packet.custom_mode = custom_mode;
	packet.type = type;
	packet.autopilot = autopilot;
	packet.base_mode = base_mode;
	packet.system_status = system_status;
	packet.mavlink_version = 3;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_AQHEARTBEAT;
	return mavlink_finalize_message(msg, system_id, component_id, 9, 0);
}

/**
 * @brief Pack a aqheartbeat message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param type Type of the MAV (quadrotor, helicopter, etc., up to 15 types, defined in MAV_TYPE ENUM)
 * @param autopilot Autopilot type / class. defined in MAV_CLASS ENUM
 * @param base_mode System mode bitfield, see MAV_MODE_FLAGS ENUM in mavlink/include/mavlink_types.h
 * @param custom_mode Navigation mode bitfield, see MAV_AUTOPILOT field is autopilot-specific.
 * @param system_status System status flag, see MAV_STATUS ENUM
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_aqheartbeat_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t type,uint8_t autopilot,uint8_t base_mode,uint32_t custom_mode,uint8_t system_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_uint32_t(buf, 0, custom_mode);
	_mav_put_uint8_t(buf, 4, type);
	_mav_put_uint8_t(buf, 5, autopilot);
	_mav_put_uint8_t(buf, 6, base_mode);
	_mav_put_uint8_t(buf, 7, system_status);
	_mav_put_uint8_t(buf, 8, 3);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_aqheartbeat_t packet;
	packet.custom_mode = custom_mode;
	packet.type = type;
	packet.autopilot = autopilot;
	packet.base_mode = base_mode;
	packet.system_status = system_status;
	packet.mavlink_version = 3;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_AQHEARTBEAT;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 9, 0);
}

/**
 * @brief Encode a aqheartbeat struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param aqheartbeat C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_aqheartbeat_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_aqheartbeat_t* aqheartbeat)
{
	return mavlink_msg_aqheartbeat_pack(system_id, component_id, msg, aqheartbeat->type, aqheartbeat->autopilot, aqheartbeat->base_mode, aqheartbeat->custom_mode, aqheartbeat->system_status);
}

/**
 * @brief Send a aqheartbeat message
 * @param chan MAVLink channel to send the message
 *
 * @param type Type of the MAV (quadrotor, helicopter, etc., up to 15 types, defined in MAV_TYPE ENUM)
 * @param autopilot Autopilot type / class. defined in MAV_CLASS ENUM
 * @param base_mode System mode bitfield, see MAV_MODE_FLAGS ENUM in mavlink/include/mavlink_types.h
 * @param custom_mode Navigation mode bitfield, see MAV_AUTOPILOT field is autopilot-specific.
 * @param system_status System status flag, see MAV_STATUS ENUM
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_aqheartbeat_send(mavlink_channel_t chan, uint8_t type, uint8_t autopilot, uint8_t base_mode, uint32_t custom_mode, uint8_t system_status)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_uint32_t(buf, 0, custom_mode);
	_mav_put_uint8_t(buf, 4, type);
	_mav_put_uint8_t(buf, 5, autopilot);
	_mav_put_uint8_t(buf, 6, base_mode);
	_mav_put_uint8_t(buf, 7, system_status);
	_mav_put_uint8_t(buf, 8, 3);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_AQHEARTBEAT, buf, 9, 0);
#else
	mavlink_aqheartbeat_t packet;
	packet.custom_mode = custom_mode;
	packet.type = type;
	packet.autopilot = autopilot;
	packet.base_mode = base_mode;
	packet.system_status = system_status;
	packet.mavlink_version = 3;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_AQHEARTBEAT, (const char *)&packet, 9, 0);
#endif
}

#endif

// MESSAGE AQHEARTBEAT UNPACKING


/**
 * @brief Get field type from aqheartbeat message
 *
 * @return Type of the MAV (quadrotor, helicopter, etc., up to 15 types, defined in MAV_TYPE ENUM)
 */
static inline uint8_t mavlink_msg_aqheartbeat_get_type(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  4);
}

/**
 * @brief Get field autopilot from aqheartbeat message
 *
 * @return Autopilot type / class. defined in MAV_CLASS ENUM
 */
static inline uint8_t mavlink_msg_aqheartbeat_get_autopilot(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  5);
}

/**
 * @brief Get field base_mode from aqheartbeat message
 *
 * @return System mode bitfield, see MAV_MODE_FLAGS ENUM in mavlink/include/mavlink_types.h
 */
static inline uint8_t mavlink_msg_aqheartbeat_get_base_mode(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  6);
}

/**
 * @brief Get field custom_mode from aqheartbeat message
 *
 * @return Navigation mode bitfield, see MAV_AUTOPILOT field is autopilot-specific.
 */
static inline uint32_t mavlink_msg_aqheartbeat_get_custom_mode(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint32_t(msg,  0);
}

/**
 * @brief Get field system_status from aqheartbeat message
 *
 * @return System status flag, see MAV_STATUS ENUM
 */
static inline uint8_t mavlink_msg_aqheartbeat_get_system_status(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  7);
}

/**
 * @brief Get field mavlink_version from aqheartbeat message
 *
 * @return MAVLink version
 */
static inline uint8_t mavlink_msg_aqheartbeat_get_mavlink_version(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  8);
}

/**
 * @brief Decode a aqheartbeat message into a struct
 *
 * @param msg The message to decode
 * @param aqheartbeat C-struct to decode the message contents into
 */
static inline void mavlink_msg_aqheartbeat_decode(const mavlink_message_t* msg, mavlink_aqheartbeat_t* aqheartbeat)
{
#if MAVLINK_NEED_BYTE_SWAP
	aqheartbeat->custom_mode = mavlink_msg_aqheartbeat_get_custom_mode(msg);
	aqheartbeat->type = mavlink_msg_aqheartbeat_get_type(msg);
	aqheartbeat->autopilot = mavlink_msg_aqheartbeat_get_autopilot(msg);
	aqheartbeat->base_mode = mavlink_msg_aqheartbeat_get_base_mode(msg);
	aqheartbeat->system_status = mavlink_msg_aqheartbeat_get_system_status(msg);
	aqheartbeat->mavlink_version = mavlink_msg_aqheartbeat_get_mavlink_version(msg);
#else
	memcpy(aqheartbeat, _MAV_PAYLOAD(msg), 9);
#endif
}
