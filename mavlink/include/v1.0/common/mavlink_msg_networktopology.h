// MESSAGE networkTopology PACKING

#define MAVLINK_MSG_ID_networkTopology 163

typedef struct __mavlink_networktopology_t
{
 float connectivityMap[5]; ///< 
 uint8_t componentID; ///< swarmController
 uint8_t groupID; ///< ID of group
 uint8_t leaderID; ///< 
 uint8_t followerID; ///< 
} mavlink_networktopology_t;

#define MAVLINK_MSG_ID_networkTopology_LEN 24
#define MAVLINK_MSG_ID_163_LEN 24

#define MAVLINK_MSG_networkTopology_FIELD_CONNECTIVITYMAP_LEN 5

#define MAVLINK_MESSAGE_INFO_networkTopology { \
	"networkTopology", \
	5, \
	{  { "connectivityMap", NULL, MAVLINK_TYPE_FLOAT, 5, 0, offsetof(mavlink_networktopology_t, connectivityMap) }, \
         { "componentID", NULL, MAVLINK_TYPE_UINT8_T, 0, 20, offsetof(mavlink_networktopology_t, componentID) }, \
         { "groupID", NULL, MAVLINK_TYPE_UINT8_T, 0, 21, offsetof(mavlink_networktopology_t, groupID) }, \
         { "leaderID", NULL, MAVLINK_TYPE_UINT8_T, 0, 22, offsetof(mavlink_networktopology_t, leaderID) }, \
         { "followerID", NULL, MAVLINK_TYPE_UINT8_T, 0, 23, offsetof(mavlink_networktopology_t, followerID) }, \
         } \
}


/**
 * @brief Pack a networktopology message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param componentID swarmController
 * @param groupID ID of group
 * @param leaderID 
 * @param followerID 
 * @param connectivityMap 
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_networktopology_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t componentID, uint8_t groupID, uint8_t leaderID, uint8_t followerID, const float *connectivityMap)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[24];
	_mav_put_uint8_t(buf, 20, componentID);
	_mav_put_uint8_t(buf, 21, groupID);
	_mav_put_uint8_t(buf, 22, leaderID);
	_mav_put_uint8_t(buf, 23, followerID);
	_mav_put_float_array(buf, 0, connectivityMap, 5);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 24);
#else
	mavlink_networktopology_t packet;
	packet.componentID = componentID;
	packet.groupID = groupID;
	packet.leaderID = leaderID;
	packet.followerID = followerID;
	mav_array_memcpy(packet.connectivityMap, connectivityMap, sizeof(float)*5);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 24);
#endif

	msg->msgid = MAVLINK_MSG_ID_networkTopology;
	return mavlink_finalize_message(msg, system_id, component_id, 24, 59);
}

/**
 * @brief Pack a networktopology message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param componentID swarmController
 * @param groupID ID of group
 * @param leaderID 
 * @param followerID 
 * @param connectivityMap 
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_networktopology_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t componentID,uint8_t groupID,uint8_t leaderID,uint8_t followerID,const float *connectivityMap)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[24];
	_mav_put_uint8_t(buf, 20, componentID);
	_mav_put_uint8_t(buf, 21, groupID);
	_mav_put_uint8_t(buf, 22, leaderID);
	_mav_put_uint8_t(buf, 23, followerID);
	_mav_put_float_array(buf, 0, connectivityMap, 5);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 24);
#else
	mavlink_networktopology_t packet;
	packet.componentID = componentID;
	packet.groupID = groupID;
	packet.leaderID = leaderID;
	packet.followerID = followerID;
	mav_array_memcpy(packet.connectivityMap, connectivityMap, sizeof(float)*5);
        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 24);
#endif

	msg->msgid = MAVLINK_MSG_ID_networkTopology;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 24, 59);
}

/**
 * @brief Encode a networktopology struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param networktopology C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_networktopology_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_networktopology_t* networktopology)
{
	return mavlink_msg_networktopology_pack(system_id, component_id, msg, networktopology->componentID, networktopology->groupID, networktopology->leaderID, networktopology->followerID, networktopology->connectivityMap);
}

/**
 * @brief Send a networktopology message
 * @param chan MAVLink channel to send the message
 *
 * @param componentID swarmController
 * @param groupID ID of group
 * @param leaderID 
 * @param followerID 
 * @param connectivityMap 
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_networktopology_send(mavlink_channel_t chan, uint8_t componentID, uint8_t groupID, uint8_t leaderID, uint8_t followerID, const float *connectivityMap)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[24];
	_mav_put_uint8_t(buf, 20, componentID);
	_mav_put_uint8_t(buf, 21, groupID);
	_mav_put_uint8_t(buf, 22, leaderID);
	_mav_put_uint8_t(buf, 23, followerID);
	_mav_put_float_array(buf, 0, connectivityMap, 5);
	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_networkTopology, buf, 24, 59);
#else
	mavlink_networktopology_t packet;
	packet.componentID = componentID;
	packet.groupID = groupID;
	packet.leaderID = leaderID;
	packet.followerID = followerID;
	mav_array_memcpy(packet.connectivityMap, connectivityMap, sizeof(float)*5);
	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_networkTopology, (const char *)&packet, 24, 59);
#endif
}

#endif

// MESSAGE networkTopology UNPACKING


/**
 * @brief Get field componentID from networktopology message
 *
 * @return swarmController
 */
static inline uint8_t mavlink_msg_networktopology_get_componentID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  20);
}

/**
 * @brief Get field groupID from networktopology message
 *
 * @return ID of group
 */
static inline uint8_t mavlink_msg_networktopology_get_groupID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  21);
}

/**
 * @brief Get field leaderID from networktopology message
 *
 * @return 
 */
static inline uint8_t mavlink_msg_networktopology_get_leaderID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  22);
}

/**
 * @brief Get field followerID from networktopology message
 *
 * @return 
 */
static inline uint8_t mavlink_msg_networktopology_get_followerID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  23);
}

/**
 * @brief Get field connectivityMap from networktopology message
 *
 * @return 
 */
static inline uint16_t mavlink_msg_networktopology_get_connectivityMap(const mavlink_message_t* msg, float *connectivityMap)
{
	return _MAV_RETURN_float_array(msg, connectivityMap, 5,  0);
}

/**
 * @brief Decode a networktopology message into a struct
 *
 * @param msg The message to decode
 * @param networktopology C-struct to decode the message contents into
 */
static inline void mavlink_msg_networktopology_decode(const mavlink_message_t* msg, mavlink_networktopology_t* networktopology)
{
#if MAVLINK_NEED_BYTE_SWAP
	mavlink_msg_networktopology_get_connectivityMap(msg, networktopology->connectivityMap);
	networktopology->componentID = mavlink_msg_networktopology_get_componentID(msg);
	networktopology->groupID = mavlink_msg_networktopology_get_groupID(msg);
	networktopology->leaderID = mavlink_msg_networktopology_get_leaderID(msg);
	networktopology->followerID = mavlink_msg_networktopology_get_followerID(msg);
#else
	memcpy(networktopology, _MAV_PAYLOAD(msg), 24);
#endif
}
