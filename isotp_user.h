#ifndef __ISOTP_USER_H__
#define __ISOTP_USER_H__

#include "isotp_defines.h"


/// Define as non-zero if isotp_user_debug() required.
#define USE_USER_DEBUG  0

/// User implemented, print debug message. If required, USE_USER_DEBUG should be defined as non-zero.
/// Turned-off by default.
#if USE_USER_DEBUG == 0
#define isotp_user_debug(x)
#else
void isotp_user_debug(const UNSIGNED_MAU* message, ...);
#endif

/// @brief Is called every time message is complety sent.
/// @param link - link used to send a message.
void isotp_send_done(struct IsoTpLink *link);

/// @brief Is called every time isotp-c fails to send a message.
/// @param link - link used to send a message.
void isotp_send_fail(struct IsoTpLink *link, int error);

/// @brief Is called every time message is complety received.
/// @param link - link to be used to read message.
void isotp_recv_done(struct IsoTpLink *link);

/// @brief Is called every time isotp-c fails to receive a message.
/// @param link - link used to receive a message.
void isotp_recv_fail(struct IsoTpLink *link, int error);

/// @brief User defined function to send CAN message.
/// @param arbitration_id - CAN message id.
/// @param data - Pointer to the data to be sent. Each UNSIGNED_MAU element in data buffer 
///               represent one 8-bit value (buffer is unpacked).
/// @param size - Size in bytes of the data to be sent. Valid values are
///               in range [0 .. 8]
/// @return ISOTP_RET_OK if success, otherwise one of the appropriate ISOTP_RET_XXX codes.
int  isotp_user_send_can(const uint32_t arbitration_id,
                         const UNSIGNED_MAU* data,
                         const UNSIGNED_MAU size);

/// @brief User defined function to obtaine 1ms timestamp.
/// @return Number of millisecond period of time being passed.
uint32_t isotp_user_get_ms(void);

#endif // __ISOTP_H__

