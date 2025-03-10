#ifndef __ISOTP_USER_H__
#define __ISOTP_USER_H__

/// Define as non-zero if isotp_user_debug() required.
#define USE_USER_DEBUG  0

/// User implemented, print debug message. If required, USE_USER_DEBUG should be defined as non-zero.
/// Turned-off by default.
#if USE_USER_DEBUG == 0
#define isotp_user_debug(x)
#else
void isotp_user_debug(const UNSIGNED_MAU* message, ...);
#endif

/// @brief User defined function to send CAN message.
/// @param arbitration_id - CAN message id.
/// @param data - Pointer to the data to be sent. Each UNSIGNED_MAU element in data buffer 
///               represent one 8-bit value (buffer is unpacked).
/// @param size - Size in classical 8-bit bytes of the data to be sent. Valid values are
///               in range [0 .. 8]
/// @return ISOTP_RET_OK if success, otherwise one of the appropriate ISOTP_RET_XXX codes.
int  isotp_user_send_can(const uint32_t arbitration_id,
                         const UNSIGNED_MAU* data,
                         const UNSIGNED_MAU size);

/// @brief User defined function to obtaine 1ms timestamp.
/// @return Number of millisecond period of time being passed.
uint32_t isotp_user_get_ms(void);

#endif // __ISOTP_H__

