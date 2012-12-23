// MESSAGE groupCreate PACKING

#define MAVLINK_MSG_ID_groupCreate 160

typedef struct __mavlink_groupcreate_t
{
 uint8_t componentID; ///< swarmController
 uint8_t groupID; ///< ID of group
 uint8_t nUAV; ///< No of uavs in Group
 uint8_t leaderID; ///< 
 uint8_t uavID[5]; ///< Max of 5 in Group
} mavlink_groupcreate_t;

#define MAVLINK_MSG_ID_groupCreate_LEN 9
#define MAVLINK_MSG_ID_160_LEN 9

#define MAVLINK_MSG_groupCreate_FIELD_UAVID_LEN 5

#define MAVLINK_MESSAGE_INFO_groupCreate { \
	"groupCreate", \
	5, \
	{  { "componentID", NULL, MAVLINK_TYPE_UINT8_T, 0, 0, offsetof(mavlink_groupcreate_t, componentID) }, \
         { "groupID", NULL, MAVLINK_TYPE_UINT8_T, 0, 1, offsetof(mavlink_groupcreate_t, groupID) }, \
         { "nUAV", NULL, MAVLINK_TYPE_UINT8_T, 0, 2, offsetof(mavlink_groupcreate_t, nUAV) }, \
         { "leaderID", NULL, MAVLINK_TYPE_UINT8_T, 0, 3, offsetof(mavlink_groupcreate_t, leaderID) }, \
         { "uavID", NULL, MAVLINK_TYPE_UINT8_T, 5, 4, offsetof(mavlink_groupcreate_t, uavID) }, \
         } \
}


/**
 * @brief Pack a groupcreate message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param componentID swarmController
 * @param groupID ID of group
 * @param nUAV No of uavs in Group
 * @param leaderID 
 * @param uavID Max of 5 in Group
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_groupcreate_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t componentID, uint8_t groupID, uint8_t nUAV, uint8_t leaderID, const uint8_t *uavID)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_uint8_t(buf, 0, componentID);
	_mav_put_uint8_t(buf, 1, groupID);
	_mav_put_uint8_t(buf, 2, nUAV);
	_mav_put_uint8_t(buf, 3, leaderID);
	_mav_put_uint8_t_array(buf, 4, uavID, 5);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_groupcreate_t packet;
	packet.componentID = componentID;
	packet.groupID = groupID;
	packet.nUAV = nUAV;
	packet.leaderID = leaderID;
	mav_array_memcpy(packet.uavID, uavID, sizeof(uint8_t)*5);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_groupCreate;
	return mavlink_finalize_message(msg, system_id, component_id, 9, 39);
}

/**
 * @brief Pack a groupcreate message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param componentID swarmController
 * @param groupID ID of group
 * @param nUAV No of uavs in Group
 * @param leaderID 
 * @param uavID Max of 5 in Group
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_groupcreate_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t componentID,uint8_t groupID,uint8_t nUAV,uint8_t leaderID,const uint8_t *uavID)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_uint8_t(buf, 0, componentID);
	_mav_put_uint8_t(buf, 1, groupID);
	_mav_put_uint8_t(buf, 2, nUAV);
	_mav_put_uint8_t(buf, 3, leaderID);
	_mav_put_uint8_t_array(buf, 4, uavID, 5);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 9);
#else
	mavlink_groupcreate_t packet;
	packet.componentID = componentID;
	packet.groupID = groupID;
	packet.nUAV = nUAV;
	packet.leaderID = leaderID;
	mav_array_memcpy(packet.uavID, uavID, sizeof(uint8_t)*5);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 9);
#endif

	msg->msgid = MAVLINK_MSG_ID_groupCreate;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 9, 39);
}

/**
 * @brief Encode a groupcreate struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param groupcreate C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_groupcreate_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_groupcreate_t* groupcreate)
{
	return mavlink_msg_groupcreate_pack(system_id, component_id, msg, groupcreate->componentID, groupcreate->groupID, groupcreate->nUAV, groupcreate->leaderID, groupcreate->uavID);
}

/**
 * @brief Send a groupcreate message
 * @param chan MAVLink channel to send the message
 *
 * @param componentID swarmController
 * @param groupID ID of group
 * @param nUAV No of uavs in Group
 * @param leaderID 
 * @param uavID Max of 5 in Group
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_groupcreate_send(mavlink_channel_t chan, uint8_t componentID, uint8_t groupID, uint8_t nUAV, uint8_t leaderID, const uint8_t *uavID)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[9];
	_mav_put_uint8_t(buf, 0, componentID);
	_mav_put_uint8_t(buf, 1, groupID);
	_mav_put_uint8_t(buf, 2, nUAV);
	_mav_put_uint8_t(buf, 3, leaderID);
	_mav_put_uint8_t_array(buf, 4, uavID, 5);
	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_groupCreate, buf, 9, 39);
#else
	mavlink_groupcreate_t packet;
	packet.componentID = componentID;
	packet.groupID = groupID;
	packet.nUAV = nUAV;
	packet.leaderID = leaderID;
	mav_array_memcpy(packet.uavID, uavID, sizeof(uint8_t)*5);
	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_groupCreate, (const char *)&packet, 9, 39);
#endif
}

#endif

// MESSAGE groupCreate UNPACKING


/**
 * @brief Get field componentID from groupcreate message
 *
 * @return swarmController
 */
static inline uint8_t mavlink_msg_groupcreate_get_componentID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  0);
}

/**
 * @brief Get field groupID from groupcreate message
 *
 * @return ID of group
 */
static inline uint8_t mavlink_msg_groupcreate_get_groupID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  1);
}

/**
 * @brief Get field nUAV from groupcreate message
 *
 * @return No of uavs in Group
 */
static inline uint8_t mavlink_msg_groupcreate_get_nUAV(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  2);
}

/**
 * @brief Get field leaderID from groupcreate message
 *
 * @return 
 */
static inline uint8_t mavlink_msg_groupcreate_get_leaderID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  3);
}

/**
 * @brief Get field uavID from groupcreate message
 *
 * @return Max of 5 in Group
 */
static inline uint16_t mavlink_msg_groupcreate_get_uavID(const mavlink_message_t* msg, uint8_t *uavID)
{
	return _MAV_RETURN_uint8_t_array(msg, uavID, 5,  4);
}

/**
 * @brief Decode a groupcreate message into a struct
 *
 * @param msg The message to decode
 * @param groupcreate C-struct to decode the message contents into
 */
static inline void mavlink_msg_groupcreate_decode(const mavlink_message_t* msg, mavlink_groupcreate_t* groupcreate)
{
#if MAVLINK_NEED_BYTE_SWAP
	groupcreate->componentID = mavlink_msg_groupcreate_get_componentID(msg);
	groupcreate->groupID = mavlink_msg_groupcreate_get_groupID(msg);
	groupcreate->nUAV = mavlink_msg_groupcreate_get_nUAV(msg);
	groupcreate->leaderID = mavlink_msg_groupcreate_get_leaderID(msg);
	mavlink_msg_groupcreate_get_uavID(msg, groupcreate->uavID);
#else
	memcpy(groupcreate, _MAV_PAYLOAD(msg), 9);
#endif
}
