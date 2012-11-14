// MESSAGE AQ_TELE_F PACKING

#define MAVLINK_MSG_ID_AQ_TELE_F 150

typedef struct __mavlink_aq_tele_f_t
{
 float value1; ///< value1
 float value2; ///< value2
 float value3; ///< value3
 float value4; ///< value4
 float value5; ///< value5
 float value6; ///< value6
 float value7; ///< value7
 float value8; ///< value8
 float value9; ///< value9
 float value10; ///< value10
 float value11; ///< value11
 float value12; ///< value12
 float value13; ///< value13
 float value14; ///< value14
 float value15; ///< value15
 uint16_t Index; ///< Index of message
} mavlink_aq_tele_f_t;

#define MAVLINK_MSG_ID_AQ_TELE_F_LEN 62
#define MAVLINK_MSG_ID_150_LEN 62



#define MAVLINK_MESSAGE_INFO_AQ_TELE_F { \
	"AQ_TELE_F", \
	16, \
	{  { "value1", NULL, MAVLINK_TYPE_FLOAT, 0, 0, offsetof(mavlink_aq_tele_f_t, value1) }, \
         { "value2", NULL, MAVLINK_TYPE_FLOAT, 0, 4, offsetof(mavlink_aq_tele_f_t, value2) }, \
         { "value3", NULL, MAVLINK_TYPE_FLOAT, 0, 8, offsetof(mavlink_aq_tele_f_t, value3) }, \
         { "value4", NULL, MAVLINK_TYPE_FLOAT, 0, 12, offsetof(mavlink_aq_tele_f_t, value4) }, \
         { "value5", NULL, MAVLINK_TYPE_FLOAT, 0, 16, offsetof(mavlink_aq_tele_f_t, value5) }, \
         { "value6", NULL, MAVLINK_TYPE_FLOAT, 0, 20, offsetof(mavlink_aq_tele_f_t, value6) }, \
         { "value7", NULL, MAVLINK_TYPE_FLOAT, 0, 24, offsetof(mavlink_aq_tele_f_t, value7) }, \
         { "value8", NULL, MAVLINK_TYPE_FLOAT, 0, 28, offsetof(mavlink_aq_tele_f_t, value8) }, \
         { "value9", NULL, MAVLINK_TYPE_FLOAT, 0, 32, offsetof(mavlink_aq_tele_f_t, value9) }, \
         { "value10", NULL, MAVLINK_TYPE_FLOAT, 0, 36, offsetof(mavlink_aq_tele_f_t, value10) }, \
         { "value11", NULL, MAVLINK_TYPE_FLOAT, 0, 40, offsetof(mavlink_aq_tele_f_t, value11) }, \
         { "value12", NULL, MAVLINK_TYPE_FLOAT, 0, 44, offsetof(mavlink_aq_tele_f_t, value12) }, \
         { "value13", NULL, MAVLINK_TYPE_FLOAT, 0, 48, offsetof(mavlink_aq_tele_f_t, value13) }, \
         { "value14", NULL, MAVLINK_TYPE_FLOAT, 0, 52, offsetof(mavlink_aq_tele_f_t, value14) }, \
         { "value15", NULL, MAVLINK_TYPE_FLOAT, 0, 56, offsetof(mavlink_aq_tele_f_t, value15) }, \
         { "Index", NULL, MAVLINK_TYPE_UINT16_T, 0, 60, offsetof(mavlink_aq_tele_f_t, Index) }, \
         } \
}


/**
 * @brief Pack a aq_tele_f message
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 *
 * @param Index Index of message
 * @param value1 value1
 * @param value2 value2
 * @param value3 value3
 * @param value4 value4
 * @param value5 value5
 * @param value6 value6
 * @param value7 value7
 * @param value8 value8
 * @param value9 value9
 * @param value10 value10
 * @param value11 value11
 * @param value12 value12
 * @param value13 value13
 * @param value14 value14
 * @param value15 value15
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_aq_tele_f_pack(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg,
						       uint16_t Index, float value1, float value2, float value3, float value4, float value5, float value6, float value7, float value8, float value9, float value10, float value11, float value12, float value13, float value14, float value15)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[62];
	_mav_put_float(buf, 0, value1);
	_mav_put_float(buf, 4, value2);
	_mav_put_float(buf, 8, value3);
	_mav_put_float(buf, 12, value4);
	_mav_put_float(buf, 16, value5);
	_mav_put_float(buf, 20, value6);
	_mav_put_float(buf, 24, value7);
	_mav_put_float(buf, 28, value8);
	_mav_put_float(buf, 32, value9);
	_mav_put_float(buf, 36, value10);
	_mav_put_float(buf, 40, value11);
	_mav_put_float(buf, 44, value12);
	_mav_put_float(buf, 48, value13);
	_mav_put_float(buf, 52, value14);
	_mav_put_float(buf, 56, value15);
	_mav_put_uint16_t(buf, 60, Index);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 62);
#else
	mavlink_aq_tele_f_t packet;
	packet.value1 = value1;
	packet.value2 = value2;
	packet.value3 = value3;
	packet.value4 = value4;
	packet.value5 = value5;
	packet.value6 = value6;
	packet.value7 = value7;
	packet.value8 = value8;
	packet.value9 = value9;
	packet.value10 = value10;
	packet.value11 = value11;
	packet.value12 = value12;
	packet.value13 = value13;
	packet.value14 = value14;
	packet.value15 = value15;
	packet.Index = Index;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 62);
#endif

	msg->msgid = MAVLINK_MSG_ID_AQ_TELE_F;
	return mavlink_finalize_message(msg, system_id, component_id, 62, 173);
}

/**
 * @brief Pack a aq_tele_f message on a channel
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param chan The MAVLink channel this message was sent over
 * @param msg The MAVLink message to compress the data into
 * @param Index Index of message
 * @param value1 value1
 * @param value2 value2
 * @param value3 value3
 * @param value4 value4
 * @param value5 value5
 * @param value6 value6
 * @param value7 value7
 * @param value8 value8
 * @param value9 value9
 * @param value10 value10
 * @param value11 value11
 * @param value12 value12
 * @param value13 value13
 * @param value14 value14
 * @param value15 value15
 * @return length of the message in bytes (excluding serial stream start sign)
 */
static inline uint16_t mavlink_msg_aq_tele_f_pack_chan(uint8_t system_id, uint8_t component_id, uint8_t chan,
							   mavlink_message_t* msg,
						           uint16_t Index,float value1,float value2,float value3,float value4,float value5,float value6,float value7,float value8,float value9,float value10,float value11,float value12,float value13,float value14,float value15)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[62];
	_mav_put_float(buf, 0, value1);
	_mav_put_float(buf, 4, value2);
	_mav_put_float(buf, 8, value3);
	_mav_put_float(buf, 12, value4);
	_mav_put_float(buf, 16, value5);
	_mav_put_float(buf, 20, value6);
	_mav_put_float(buf, 24, value7);
	_mav_put_float(buf, 28, value8);
	_mav_put_float(buf, 32, value9);
	_mav_put_float(buf, 36, value10);
	_mav_put_float(buf, 40, value11);
	_mav_put_float(buf, 44, value12);
	_mav_put_float(buf, 48, value13);
	_mav_put_float(buf, 52, value14);
	_mav_put_float(buf, 56, value15);
	_mav_put_uint16_t(buf, 60, Index);

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), buf, 62);
#else
	mavlink_aq_tele_f_t packet;
	packet.value1 = value1;
	packet.value2 = value2;
	packet.value3 = value3;
	packet.value4 = value4;
	packet.value5 = value5;
	packet.value6 = value6;
	packet.value7 = value7;
	packet.value8 = value8;
	packet.value9 = value9;
	packet.value10 = value10;
	packet.value11 = value11;
	packet.value12 = value12;
	packet.value13 = value13;
	packet.value14 = value14;
	packet.value15 = value15;
	packet.Index = Index;

        memcpy(_MAV_PAYLOAD_NON_CONST(msg), &packet, 62);
#endif

	msg->msgid = MAVLINK_MSG_ID_AQ_TELE_F;
	return mavlink_finalize_message_chan(msg, system_id, component_id, chan, 62, 173);
}

/**
 * @brief Encode a aq_tele_f struct into a message
 *
 * @param system_id ID of this system
 * @param component_id ID of this component (e.g. 200 for IMU)
 * @param msg The MAVLink message to compress the data into
 * @param aq_tele_f C-struct to read the message contents from
 */
static inline uint16_t mavlink_msg_aq_tele_f_encode(uint8_t system_id, uint8_t component_id, mavlink_message_t* msg, const mavlink_aq_tele_f_t* aq_tele_f)
{
	return mavlink_msg_aq_tele_f_pack(system_id, component_id, msg, aq_tele_f->Index, aq_tele_f->value1, aq_tele_f->value2, aq_tele_f->value3, aq_tele_f->value4, aq_tele_f->value5, aq_tele_f->value6, aq_tele_f->value7, aq_tele_f->value8, aq_tele_f->value9, aq_tele_f->value10, aq_tele_f->value11, aq_tele_f->value12, aq_tele_f->value13, aq_tele_f->value14, aq_tele_f->value15);
}

/**
 * @brief Send a aq_tele_f message
 * @param chan MAVLink channel to send the message
 *
 * @param Index Index of message
 * @param value1 value1
 * @param value2 value2
 * @param value3 value3
 * @param value4 value4
 * @param value5 value5
 * @param value6 value6
 * @param value7 value7
 * @param value8 value8
 * @param value9 value9
 * @param value10 value10
 * @param value11 value11
 * @param value12 value12
 * @param value13 value13
 * @param value14 value14
 * @param value15 value15
 */
#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS

static inline void mavlink_msg_aq_tele_f_send(mavlink_channel_t chan, uint16_t Index, float value1, float value2, float value3, float value4, float value5, float value6, float value7, float value8, float value9, float value10, float value11, float value12, float value13, float value14, float value15)
{
#if MAVLINK_NEED_BYTE_SWAP || !MAVLINK_ALIGNED_FIELDS
	char buf[62];
	_mav_put_float(buf, 0, value1);
	_mav_put_float(buf, 4, value2);
	_mav_put_float(buf, 8, value3);
	_mav_put_float(buf, 12, value4);
	_mav_put_float(buf, 16, value5);
	_mav_put_float(buf, 20, value6);
	_mav_put_float(buf, 24, value7);
	_mav_put_float(buf, 28, value8);
	_mav_put_float(buf, 32, value9);
	_mav_put_float(buf, 36, value10);
	_mav_put_float(buf, 40, value11);
	_mav_put_float(buf, 44, value12);
	_mav_put_float(buf, 48, value13);
	_mav_put_float(buf, 52, value14);
	_mav_put_float(buf, 56, value15);
	_mav_put_uint16_t(buf, 60, Index);

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_AQ_TELE_F, buf, 62, 173);
#else
	mavlink_aq_tele_f_t packet;
	packet.value1 = value1;
	packet.value2 = value2;
	packet.value3 = value3;
	packet.value4 = value4;
	packet.value5 = value5;
	packet.value6 = value6;
	packet.value7 = value7;
	packet.value8 = value8;
	packet.value9 = value9;
	packet.value10 = value10;
	packet.value11 = value11;
	packet.value12 = value12;
	packet.value13 = value13;
	packet.value14 = value14;
	packet.value15 = value15;
	packet.Index = Index;

	_mav_finalize_message_chan_send(chan, MAVLINK_MSG_ID_AQ_TELE_F, (const char *)&packet, 62, 173);
#endif
}

#endif

// MESSAGE AQ_TELE_F UNPACKING


/**
 * @brief Get field Index from aq_tele_f message
 *
 * @return Index of message
 */
static inline uint16_t mavlink_msg_aq_tele_f_get_Index(const mavlink_message_t* msg)
{
	return _MAV_RETURN_uint16_t(msg,  60);
}

/**
 * @brief Get field value1 from aq_tele_f message
 *
 * @return value1
 */
static inline float mavlink_msg_aq_tele_f_get_value1(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  0);
}

/**
 * @brief Get field value2 from aq_tele_f message
 *
 * @return value2
 */
static inline float mavlink_msg_aq_tele_f_get_value2(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  4);
}

/**
 * @brief Get field value3 from aq_tele_f message
 *
 * @return value3
 */
static inline float mavlink_msg_aq_tele_f_get_value3(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  8);
}

/**
 * @brief Get field value4 from aq_tele_f message
 *
 * @return value4
 */
static inline float mavlink_msg_aq_tele_f_get_value4(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  12);
}

/**
 * @brief Get field value5 from aq_tele_f message
 *
 * @return value5
 */
static inline float mavlink_msg_aq_tele_f_get_value5(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  16);
}

/**
 * @brief Get field value6 from aq_tele_f message
 *
 * @return value6
 */
static inline float mavlink_msg_aq_tele_f_get_value6(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  20);
}

/**
 * @brief Get field value7 from aq_tele_f message
 *
 * @return value7
 */
static inline float mavlink_msg_aq_tele_f_get_value7(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  24);
}

/**
 * @brief Get field value8 from aq_tele_f message
 *
 * @return value8
 */
static inline float mavlink_msg_aq_tele_f_get_value8(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  28);
}

/**
 * @brief Get field value9 from aq_tele_f message
 *
 * @return value9
 */
static inline float mavlink_msg_aq_tele_f_get_value9(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  32);
}

/**
 * @brief Get field value10 from aq_tele_f message
 *
 * @return value10
 */
static inline float mavlink_msg_aq_tele_f_get_value10(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  36);
}

/**
 * @brief Get field value11 from aq_tele_f message
 *
 * @return value11
 */
static inline float mavlink_msg_aq_tele_f_get_value11(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  40);
}

/**
 * @brief Get field value12 from aq_tele_f message
 *
 * @return value12
 */
static inline float mavlink_msg_aq_tele_f_get_value12(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  44);
}

/**
 * @brief Get field value13 from aq_tele_f message
 *
 * @return value13
 */
static inline float mavlink_msg_aq_tele_f_get_value13(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  48);
}

/**
 * @brief Get field value14 from aq_tele_f message
 *
 * @return value14
 */
static inline float mavlink_msg_aq_tele_f_get_value14(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  52);
}

/**
 * @brief Get field value15 from aq_tele_f message
 *
 * @return value15
 */
static inline float mavlink_msg_aq_tele_f_get_value15(const mavlink_message_t* msg)
{
	return _MAV_RETURN_float(msg,  56);
}

/**
 * @brief Decode a aq_tele_f message into a struct
 *
 * @param msg The message to decode
 * @param aq_tele_f C-struct to decode the message contents into
 */
static inline void mavlink_msg_aq_tele_f_decode(const mavlink_message_t* msg, mavlink_aq_tele_f_t* aq_tele_f)
{
#if MAVLINK_NEED_BYTE_SWAP
	aq_tele_f->value1 = mavlink_msg_aq_tele_f_get_value1(msg);
	aq_tele_f->value2 = mavlink_msg_aq_tele_f_get_value2(msg);
	aq_tele_f->value3 = mavlink_msg_aq_tele_f_get_value3(msg);
	aq_tele_f->value4 = mavlink_msg_aq_tele_f_get_value4(msg);
	aq_tele_f->value5 = mavlink_msg_aq_tele_f_get_value5(msg);
	aq_tele_f->value6 = mavlink_msg_aq_tele_f_get_value6(msg);
	aq_tele_f->value7 = mavlink_msg_aq_tele_f_get_value7(msg);
	aq_tele_f->value8 = mavlink_msg_aq_tele_f_get_value8(msg);
	aq_tele_f->value9 = mavlink_msg_aq_tele_f_get_value9(msg);
	aq_tele_f->value10 = mavlink_msg_aq_tele_f_get_value10(msg);
	aq_tele_f->value11 = mavlink_msg_aq_tele_f_get_value11(msg);
	aq_tele_f->value12 = mavlink_msg_aq_tele_f_get_value12(msg);
	aq_tele_f->value13 = mavlink_msg_aq_tele_f_get_value13(msg);
	aq_tele_f->value14 = mavlink_msg_aq_tele_f_get_value14(msg);
	aq_tele_f->value15 = mavlink_msg_aq_tele_f_get_value15(msg);
	aq_tele_f->Index = mavlink_msg_aq_tele_f_get_Index(msg);
#else
	memcpy(aq_tele_f, _MAV_PAYLOAD(msg), 62);
#endif
}
