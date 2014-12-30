// MESSAGE AQ_HOME_POSITION PACKING

#define MAVLINK_MSG_ID_AQ_HOME_POSITION 151

typedef struct __mavlink_aq_home_position_t
{
 int32_t lat; ///< Home position latitude, expressed as * 1E7.
 int32_t lon; ///< Home position longitude, expressed as * 1E7.
 float alt; ///< Home position altitude, in meters.
 float refalt; ///< Current (reference) altitude, in meters (eg. to calculate delta altitude -- AQ uses pressure altitude not gps altitude for home reference).
 float throttle; ///< Distance to home in meters, if known, -1 if unknown.
 uint16_t heading; ///< Heading to home in degrees * 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX.
} mavlink_aq_home_position_t;

#define MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN 22
#define MAVLINK_MSG_ID_151_LEN 22

#define MAVLINK_MSG_ID_AQ_HOME_POSITION_CRC 7
#define MAVLINK_MSG_ID_151_CRC 7



#define MAVLINK_MESSAGE_INFO_AQ_HOME_POSITION { \
	"AQ_HOME_POSITION", \
	6, \
	{  { "lat", NULL, MAVLINK_TYPE_INT32_T, 0, 0, offsetof(mavlink_aq_home_position_t, lat) }, \
         { "lon", NULL, MAVLINK_TYPE_INT32_T, 0, 4, offsetof(mavlink_aq_home_position_t, lon) }, \
         { "alt", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_aq_home_position_t, alt) }, \
         { "refalt", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_aq_home_position_t, refalt) }, \
         { "throttle", NULL, MAVLINK_TYPE_FLOAT, 0, 16, offsetof(mavlink_aq_home_position_t, throttle) }, \
         { "heading", NULL, MAVLINK_TYPE_UINT16_T, 0, 20, offsetof(mavlink_aq_home_position_t, heading) }, \
         } \
}


/**
 * @brief Pack a aq_home_position message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param lat Home position latitude, expressed as * 1E7.
 * @param lon Home position longitude, expressed as * 1E7.
 * @param alt Home position altitude, in meters.
 * @param refalt Current (reference) altitude, in meters (eg. to calculate delta altitude -- AQ uses pressure altitude not gps altitude for home reference).
 * @param heading Heading to home in degrees * 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX.
 * @param throttle Distance to home in meters, if known, -1 if unknown.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_aq_home_position_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       int32_t lat, int32_t lon, float alt, float refalt, uint16_t heading, float throttle)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN];
	_mav_put_int32_t(buf, 0, lat);
	_mav_put_int32_t(buf, 4, lon);
	_mav_put_float(buf, 8, alt);
	_mav_put_float(buf, 12, refalt);
	_mav_put_float(buf, 16, throttle);
	_mav_put_uint16_t(buf, 20, heading);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN);
#else
	mavlink_aq_home_position_t packet;
	packet.lat = lat;
	packet.lon = lon;
	packet.alt = alt;
	packet.refalt = refalt;
	packet.throttle = throttle;
	packet.heading = heading;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN);
#endif

	msg->msgid = MAVLINK_MSG_ID_AQ_HOME_POSITION;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN, MAVLINK_MSG_ID_AQ_HOME_POSITION_CRC);
#else
    return mavlink_finalize_message(msg, system_id, component_id, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN);
#endif
}

/**
 * @brief Pack a aq_home_position message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param lat Home position latitude, expressed as * 1E7.
 * @param lon Home position longitude, expressed as * 1E7.
 * @param alt Home position altitude, in meters.
 * @param refalt Current (reference) altitude, in meters (eg. to calculate delta altitude -- AQ uses pressure altitude not gps altitude for home reference).
 * @param heading Heading to home in degrees * 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX.
 * @param throttle Distance to home in meters, if known, -1 if unknown.
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_aq_home_position_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           int32_t lat,int32_t lon,float alt,float refalt,uint16_t heading,float throttle)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN];
	_mav_put_int32_t(buf, 0, lat);
	_mav_put_int32_t(buf, 4, lon);
	_mav_put_float(buf, 8, alt);
	_mav_put_float(buf, 12, refalt);
	_mav_put_float(buf, 16, throttle);
	_mav_put_uint16_t(buf, 20, heading);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN);
#else
	mavlink_aq_home_position_t packet;
	packet.lat = lat;
	packet.lon = lon;
	packet.alt = alt;
	packet.refalt = refalt;
	packet.throttle = throttle;
	packet.heading = heading;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN);
#endif

	msg->msgid = MAVLINK_MSG_ID_AQ_HOME_POSITION;
#if MAVLINK_CRC_EXTRA
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN, MAVLINK_MSG_ID_AQ_HOME_POSITION_CRC);
#else
    return mavlink_finalize_message_chan(msg, system_id, component_id, chan, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN);
#endif
}

/**
 * @brief Encode a aq_home_position struct
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param aq_home_position C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_aq_home_position_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_aq_home_position_t* aq_home_position)
{
	return mavlink_msg_aq_home_position_pack(system_id, component_id, msg, aq_home_position->lat, aq_home_position->lon, aq_home_position->alt, aq_home_position->refalt, aq_home_position->heading, aq_home_position->throttle);
}

/**
 * @brief Encode a aq_home_position struct on a channel
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message will be sent over
 * @param msg The MAVLink message to compress the data into
 * @param aq_home_position C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_aq_home_position_encode_chan(uint8_t system_id, uint8_t component_id, uint8_t chan, mavlink_message_t* msg, const mavlink_aq_home_position_t* aq_home_position)
{
	return mavlink_msg_aq_home_position_pack_chan(system_id, component_id, chan, msg, aq_home_position->lat, aq_home_position->lon, aq_home_position->alt, aq_home_position->refalt, aq_home_position->heading, aq_home_position->throttle);
}

/**
 * @brief Send a aq_home_position message
 * @param chan MAVLink channel to send the message
 *
 * @param lat Home position latitude, expressed as * 1E7.
 * @param lon Home position longitude, expressed as * 1E7.
 * @param alt Home position altitude, in meters.
 * @param refalt Current (reference) altitude, in meters (eg. to calculate delta altitude -- AQ uses pressure altitude not gps altitude for home reference).
 * @param heading Heading to home in degrees * 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX.
 * @param throttle Distance to home in meters, if known, -1 if unknown.
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_aq_home_position_send(mavlink_channel_t chan, int32_t lat, int32_t lon, float alt, float refalt, uint16_t heading, float throttle)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN];
	_mav_put_int32_t(buf, 0, lat);
	_mav_put_int32_t(buf, 4, lon);
	_mav_put_float(buf, 8, alt);
	_mav_put_float(buf, 12, refalt);
	_mav_put_float(buf, 16, throttle);
	_mav_put_uint16_t(buf, 20, heading);

#if MAVLINK_CRC_EXTRA
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_AQ_HOME_POSITION, buf, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN, MAVLINK_MSG_ID_AQ_HOME_POSITION_CRC);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_AQ_HOME_POSITION, buf, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN);
#endif
#else
	mavlink_aq_home_position_t packet;
	packet.lat = lat;
	packet.lon = lon;
	packet.alt = alt;
	packet.refalt = refalt;
	packet.throttle = throttle;
	packet.heading = heading;

#if MAVLINK_CRC_EXTRA
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_AQ_HOME_POSITION, (const char *)&packet, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN, MAVLINK_MSG_ID_AQ_HOME_POSITION_CRC);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_AQ_HOME_POSITION, (const char *)&packet, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN);
#endif
#endif
}

#if MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN <= MAVLINK_MAX_PAYLOAD_LEN
/*
  This varient of _send() can be used to save stack space by re-using
  memory from the receive buffer.  The caller provides a
  mavlink_message_t which is the size of a full mavlink message. This
  is usually the receive buffer for the channel, and allows a reply to an
  incoming message with minimum stack space usage.
 */
static inline void mavlink_msg_aq_home_position_send_buf(mavlink_message_t *msgbuf, mavlink_channel_t chan,  int32_t lat, int32_t lon, float alt, float refalt, uint16_t heading, float throttle)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char *buf = (char *)msgbuf;
	_mav_put_int32_t(buf, 0, lat);
	_mav_put_int32_t(buf, 4, lon);
	_mav_put_float(buf, 8, alt);
	_mav_put_float(buf, 12, refalt);
	_mav_put_float(buf, 16, throttle);
	_mav_put_uint16_t(buf, 20, heading);

#if MAVLINK_CRC_EXTRA
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_AQ_HOME_POSITION, buf, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN, MAVLINK_MSG_ID_AQ_HOME_POSITION_CRC);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_AQ_HOME_POSITION, buf, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN);
#endif
#else
	mavlink_aq_home_position_t *packet = (mavlink_aq_home_position_t *)msgbuf;
	packet->lat = lat;
	packet->lon = lon;
	packet->alt = alt;
	packet->refalt = refalt;
	packet->throttle = throttle;
	packet->heading = heading;

#if MAVLINK_CRC_EXTRA
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_AQ_HOME_POSITION, (const char *)packet, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN, MAVLINK_MSG_ID_AQ_HOME_POSITION_CRC);
#else
    _mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_AQ_HOME_POSITION, (const char *)packet, MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN);
#endif
#endif
}
#endif

#endif

// MESSAGE AQ_HOME_POSITION UNPACKING


/**
 * @brief Get field lat from aq_home_position message
 *
 * @return Home position latitude, expressed as * 1E7.
 */
static inline int32_t mavlink_msg_aq_home_position_get_lat(const mavlink_message_t* msg)
{
	return _MAV_RETURN_int32_t(msg,  0);
}

/**
 * @brief Get field lon from aq_home_position message
 *
 * @return Home position longitude, expressed as * 1E7.
 */
static inline int32_t mavlink_msg_aq_home_position_get_lon(const mavlink_message_t* msg)
{
	return _MAV_RETURN_int32_t(msg,  4);
}

/**
 * @brief Get field alt from aq_home_position message
 *
 * @return Home position altitude, in meters.
 */
static inline float mavlink_msg_aq_home_position_get_alt(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Get field refalt from aq_home_position message
 *
 * @return Current (reference) altitude, in meters (eg. to calculate delta altitude -- AQ uses pressure altitude not gps altitude for home reference).
 */
static inline float mavlink_msg_aq_home_position_get_refalt(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  12);
}

/**
 * @brief Get field heading from aq_home_position message
 *
 * @return Heading to home in degrees * 100, 0.0..359.99 degrees. If unknown, set to: UINT16_MAX.
 */
static inline uint16_t mavlink_msg_aq_home_position_get_heading(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  20);
}

/**
 * @brief Get field throttle from aq_home_position message
 *
 * @return Distance to home in meters, if known, -1 if unknown.
 */
static inline float mavlink_msg_aq_home_position_get_throttle(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  16);
}

/**
 * @brief Decode a aq_home_position message into a struct
 *
 * @param msg The message to decode
 * @param aq_home_position C-struct to decode the message contents into
 */
static inline void mavlink_msg_aq_home_position_decode(const mavlink_message_t* msg, mavlink_aq_home_position_t* aq_home_position)
{
#if MAVLINK_NEED_BYTE_SWAP
	aq_home_position->lat = mavlink_msg_aq_home_position_get_lat(msg);
	aq_home_position->lon = mavlink_msg_aq_home_position_get_lon(msg);
	aq_home_position->alt = mavlink_msg_aq_home_position_get_alt(msg);
	aq_home_position->refalt = mavlink_msg_aq_home_position_get_refalt(msg);
	aq_home_position->throttle = mavlink_msg_aq_home_position_get_throttle(msg);
	aq_home_position->heading = mavlink_msg_aq_home_position_get_heading(msg);
#else
	memcpy(aq_home_position, _MAV_PAYLOAD(msg), MAVLINK_MSG_ID_AQ_HOME_POSITION_LEN);
#endif
}
