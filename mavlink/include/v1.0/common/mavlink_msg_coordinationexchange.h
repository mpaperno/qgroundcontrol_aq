// MESSAGE coordinationExchange PACKING

#define MAVLINK_MSG_ID_coordinationExchange 164

typedef struct __mavlink_coordinationexchange_t
{
 float coordinationParameter; ///< 
 uint8_t componentID; ///< swarmController
 uint8_t groupID; ///< ID of group
 uint8_t senderID; ///< 
} mavlink_coordinationexchange_t;

#define MAVLINK_MSG_ID_coordinationExchange_LEN 7
#define MAVLINK_MSG_ID_164_LEN 7



#define MAVLINK_MESSAGE_INFO_coordinationExchange { \
	"coordinationExchange", \
	4, \
	{  { "coordinationParameter", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_coordinationexchange_t, coordinationParameter) }, \
         { "componentID", NULL, MAVLINK_TYPE_UINT8_T, 0, 4, offsetof(mavlink_coordinationexchange_t, componentID) }, \
         { "groupID", NULL, MAVLINK_TYPE_UINT8_T, 0, 5, offsetof(mavlink_coordinationexchange_t, groupID) }, \
         { "senderID", NULL, MAVLINK_TYPE_UINT8_T, 0, 6, offsetof(mavlink_coordinationexchange_t, senderID) }, \
         } \
}


/**
 * @brief Pack a coordinationexchange message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param componentID swarmController
 * @param groupID ID of group
 * @param senderID 
 * @param coordinationParameter 
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_coordinationexchange_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint8_t componentID, uint8_t groupID, uint8_t senderID, float coordinationParameter)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[7];
	_mav_put_float(buf, 0, coordinationParameter);
	_mav_put_uint8_t(buf, 4, componentID);
	_mav_put_uint8_t(buf, 5, groupID);
	_mav_put_uint8_t(buf, 6, senderID);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 7);
#else
	mavlink_coordinationexchange_t packet;
	packet.coordinationParameter = coordinationParameter;
	packet.componentID = componentID;
	packet.groupID = groupID;
	packet.senderID = senderID;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 7);
#endif

	msg->msgid = MAVLINK_MSG_ID_coordinationExchange;
	return mavlink_finalize_message(msg, system_id, component_id, 7, 129);
}

/**
 * @brief Pack a coordinationexchange message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param componentID swarmController
 * @param groupID ID of group
 * @param senderID 
 * @param coordinationParameter 
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_coordinationexchange_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint8_t componentID,uint8_t groupID,uint8_t senderID,float coordinationParameter)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[7];
	_mav_put_float(buf, 0, coordinationParameter);
	_mav_put_uint8_t(buf, 4, componentID);
	_mav_put_uint8_t(buf, 5, groupID);
	_mav_put_uint8_t(buf, 6, senderID);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 7);
#else
	mavlink_coordinationexchange_t packet;
	packet.coordinationParameter = coordinationParameter;
	packet.componentID = componentID;
	packet.groupID = groupID;
	packet.senderID = senderID;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 7);
#endif

	msg->msgid = MAVLINK_MSG_ID_coordinationExchange;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 7, 129);
}

/**
 * @brief Encode a coordinationexchange struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param coordinationexchange C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_coordinationexchange_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_coordinationexchange_t* coordinationexchange)
{
	return mavlink_msg_coordinationexchange_pack(system_id, component_id, msg, coordinationexchange->componentID, coordinationexchange->groupID, coordinationexchange->senderID, coordinationexchange->coordinationParameter);
}

/**
 * @brief Send a coordinationexchange message
 * @param chan MAVLink channel to send the message
 *
 * @param componentID swarmController
 * @param groupID ID of group
 * @param senderID 
 * @param coordinationParameter 
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_coordinationexchange_send(mavlink_channel_t chan, uint8_t componentID, uint8_t groupID, uint8_t senderID, float coordinationParameter)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[7];
	_mav_put_float(buf, 0, coordinationParameter);
	_mav_put_uint8_t(buf, 4, componentID);
	_mav_put_uint8_t(buf, 5, groupID);
	_mav_put_uint8_t(buf, 6, senderID);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_coordinationExchange, buf, 7, 129);
#else
	mavlink_coordinationexchange_t packet;
	packet.coordinationParameter = coordinationParameter;
	packet.componentID = componentID;
	packet.groupID = groupID;
	packet.senderID = senderID;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_coordinationExchange, (const char *)&packet, 7, 129);
#endif
}

#endif

// MESSAGE coordinationExchange UNPACKING


/**
 * @brief Get field componentID from coordinationexchange message
 *
 * @return swarmController
 */
static inline uint8_t mavlink_msg_coordinationexchange_get_componentID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  4);
}

/**
 * @brief Get field groupID from coordinationexchange message
 *
 * @return ID of group
 */
static inline uint8_t mavlink_msg_coordinationexchange_get_groupID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  5);
}

/**
 * @brief Get field senderID from coordinationexchange message
 *
 * @return 
 */
static inline uint8_t mavlink_msg_coordinationexchange_get_senderID(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint8_t(msg,  6);
}

/**
 * @brief Get field coordinationParameter from coordinationexchange message
 *
 * @return 
 */
static inline float mavlink_msg_coordinationexchange_get_coordinationParameter(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Decode a coordinationexchange message into a struct
 *
 * @param msg The message to decode
 * @param coordinationexchange C-struct to decode the message contents into
 */
static inline void mavlink_msg_coordinationexchange_decode(const mavlink_message_t* msg, mavlink_coordinationexchange_t* coordinationexchange)
{
#if MAVLINK_NEED_BYTE_SWAP
	coordinationexchange->coordinationParameter = mavlink_msg_coordinationexchange_get_coordinationParameter(msg);
	coordinationexchange->componentID = mavlink_msg_coordinationexchange_get_componentID(msg);
	coordinationexchange->groupID = mavlink_msg_coordinationexchange_get_groupID(msg);
	coordinationexchange->senderID = mavlink_msg_coordinationexchange_get_senderID(msg);
#else
	memcpy(coordinationexchange, _MAV_PAYLOAD(msg), 7);
#endif
}
