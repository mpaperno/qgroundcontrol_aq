// MESSAGE groupLeave PACKING

#define MAVLINK_MSG_ID_groupLeave 162

typedef struct __mavlink_groupleave_t
{
 uint8_t componentID; ///< swarmController
 uint8_t groupID; ///< ID of group
 uint8_t leaderID; ///< 
 uint8_t uavID; ///< Remove this uav
} mavlink_groupleave_t;

#define MAVLINK_MSG_ID_groupLeave_LEN 4
#define MAVLINK_MSG_ID_162_LEN 4



#define MAVLINK_MESSAGE_INFO_groupLeave { \
	"groupLeave", \
	4, \
	{  { "componentID", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_groupleave_t, componentID) }, \
         { "groupID", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_groupleave_t, groupID) }, \
         { "leaderID", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_groupleave_t, leaderID) }, \
         { "uavID", NULL, MAVLINK_TYPE_UINT8_T, 0, 3, offsetof(mavlink_groupleave_t, uavID) }, \
         } \
}


/**
 * @brief Pack a groupleave message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param componentID swarmController
 * @param groupID ID of group
 * @param leaderID 
 * @param uavID Remove this uav
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_groupleave_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t componentID, uint8_t groupID, uint8_t leaderID, uint8_t uavID)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[4];
	_mav_put_uint8_t(buf, 0, componentID);
	_mav_put_uint8_t(buf, 1, groupID);
	_mav_put_uint8_t(buf, 2, leaderID);
	_mav_put_uint8_t(buf, 3, uavID);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 4);
#else
	mavlink_groupleave_t packet;
	packet.componentID = componentID;
	packet.groupID = groupID;
	packet.leaderID = leaderID;
	packet.uavID = uavID;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 4);
#endif

	msg->msgid = MAVLINK_MSG_ID_groupLeave;
	return mavlink_finalize_message(msg, system_id, component_id, 4, 224);
}

/**
 * @brief Pack a groupleave message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param componentID swarmController
 * @param groupID ID of group
 * @param leaderID 
 * @param uavID Remove this uav
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_groupleave_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t componentID,uint8_t groupID,uint8_t leaderID,uint8_t uavID)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[4];
	_mav_put_uint8_t(buf, 0, componentID);
	_mav_put_uint8_t(buf, 1, groupID);
	_mav_put_uint8_t(buf, 2, leaderID);
	_mav_put_uint8_t(buf, 3, uavID);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 4);
#else
	mavlink_groupleave_t packet;
	packet.componentID = componentID;
	packet.groupID = groupID;
	packet.leaderID = leaderID;
	packet.uavID = uavID;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 4);
#endif

	msg->msgid = MAVLINK_MSG_ID_groupLeave;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 4, 224);
}

/**
 * @brief Encode a groupleave struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param groupleave C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_groupleave_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_groupleave_t* groupleave)
{
	return mavlink_msg_groupleave_pack(system_id, component_id, msg, groupleave->componentID, groupleave->groupID, groupleave->leaderID, groupleave->uavID);
}

/**
 * @brief Send a groupleave message
 * @param chan MAVLink channel to send the message
 *
 * @param componentID swarmController
 * @param groupID ID of group
 * @param leaderID 
 * @param uavID Remove this uav
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_groupleave_send(mavlink_channel_t chan, uint8_t componentID, uint8_t groupID, uint8_t leaderID, uint8_t uavID)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[4];
	_mav_put_uint8_t(buf, 0, componentID);
	_mav_put_uint8_t(buf, 1, groupID);
	_mav_put_uint8_t(buf, 2, leaderID);
	_mav_put_uint8_t(buf, 3, uavID);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_groupLeave, buf, 4, 224);
#else
	mavlink_groupleave_t packet;
	packet.componentID = componentID;
	packet.groupID = groupID;
	packet.leaderID = leaderID;
	packet.uavID = uavID;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_groupLeave, (const char *)&packet, 4, 224);
#endif
}

#endif

// MESSAGE groupLeave UNPACKING


/**
 * @brief Get field componentID from groupleave message
 *
 * @return swarmController
 */
static inline uint8_t mavlink_msg_groupleave_get_componentID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field groupID from groupleave message
 *
 * @return ID of group
 */
static inline uint8_t mavlink_msg_groupleave_get_groupID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Get field leaderID from groupleave message
 *
 * @return 
 */
static inline uint8_t mavlink_msg_groupleave_get_leaderID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  2);
}

/**
 * @brief Get field uavID from groupleave message
 *
 * @return Remove this uav
 */
static inline uint8_t mavlink_msg_groupleave_get_uavID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  3);
}

/**
 * @brief Decode a groupleave message into a struct
 *
 * @param msg The message to decode
 * @param groupleave C-struct to decode the message contents into
 */
static inline void mavlink_msg_groupleave_decode(const mavlink_message_t* msg, mavlink_groupleave_t* groupleave)
{
#if MAVLINK_NEED_BYTE_SWAP
	groupleave->componentID = mavlink_msg_groupleave_get_componentID(msg);
	groupleave->groupID = mavlink_msg_groupleave_get_groupID(msg);
	groupleave->leaderID = mavlink_msg_groupleave_get_leaderID(msg);
	groupleave->uavID = mavlink_msg_groupleave_get_uavID(msg);
#else
	memcpy(groupleave, _MAV_PAYLOAD(msg), 4);
#endif
}
