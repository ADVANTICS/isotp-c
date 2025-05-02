#ifndef __ISOTP_H__
#define __ISOTP_H__

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
#include <stdint.h>

extern "C" {
#endif

#include "isotp_defines.h"
#include "isotp_config.h"
#include "isotp_user.h"




static inline
void isotp_reset_receive(struct IsoTpLink *link) {
    link->receive_status = ISOTP_RECEIVE_STATUS_IDLE;
}

/// @brief Initialises the ISO-TP library.
/// @param link - The @code IsoTpLink @endcode instance used for transceiving data.
/// @param sendid - The ID used to send data to other CAN nodes.
/// @param sendbuf - A pointer to an area in memory which can be used as a sent buffer for given link.
///                  This buffer is packed if MAU_SIZE > 1 (UNSIGNED_MAU contains two or more classical 8-bit bytes).
/// @param sendbufsize - The size of the buffer area in UNSIGNED_MAU elements (native bytes).
/// @param recvbuf - A pointer to an area in memory which can be used as a buffer for data to be received.
///                  This buffer is packed if MAU_SIZE > 1 (UNSIGNED_MAU contains two or more classical 8-bit bytes).
/// @param recvbufsize - The size of the buffer area in UNSIGNED_MAU elements (native bytes).
void isotp_init_link(IsoTpLink *link, uint32_t sendid, 
    UNSIGNED_MAU *sendbuf, uint16_t sendbufsize,
    UNSIGNED_MAU *recvbuf, uint16_t recvbufsize);


/// @brief Polling function; call this function periodically to handle timeouts, send consecutive frames, etc.
/// @param - link The @code IsoTpLink @endcode instance used.
void isotp_poll(IsoTpLink *link);


/// @brief Handles incoming CAN messages. Determines whether an incoming message is a 
///        valid ISO-TP frame or not and handles it accordingly.
/// @param link - The @code IsoTpLink @endcode instance used for transceiving data.
/// @param data - The data received via CAN. Each UNSIGNED_MAU element in buffer represent exactly one classical 8-bit byte (data is unpacked).
/// @param len - The number of bytes received via CAN.
void isotp_on_can_message(IsoTpLink *link, UNSIGNED_MAU *data, UNSIGNED_MAU len);


/// @brief Sends ISO-TP frames via CAN, using the ID set in the initialising function.
///        Single-frame messages will be sent immediately when calling this function.
///        Multi-frame messages will be sent consecutively when calling isotp_poll.
/// @param link - The @code IsoTpLink @endcode instance used for transceiving data.
/// @param payload - The payload to be sent. (Up to 4095 bytes). 
///                  This buffer is packed if MAU_SIZE > 1 (UNSIGNED_MAU contains two or more classical 8-bit bytes).
/// @param size - The size of the payload to be sent in classical 8-bit bytes.
/// @return Possible return values:
///  - @code ISOTP_RET_INPROGRESS @endcode
///  - @code ISOTP_RET_OK @endcode
///  - The return value of the user shim function isotp_user_send_can().
int isotp_send(IsoTpLink *link, const UNSIGNED_MAU payload[], uint16_t size);


/// @brief See @link isotp_send @endlink, with the exception that this function is used only for functional addressing.
/// @param link - The @code IsoTpLink @endcode instance used for transceiving data.
/// @param id - CAN message id.
/// @param payload - The payload to be sent. (Up to 4095 bytes).
///                  This buffer is packed if MAU_SIZE > 1 (UNSIGNED_MAU contains two or more classical 8-bit bytes).
/// @param size - The size of the payload to be sent in classical 8-bit bytes.
int isotp_send_with_id(IsoTpLink *link, uint32_t id, const UNSIGNED_MAU payload[], uint16_t size);


/// @brief Copies recieved message from the internal buffer if any.
/// @param link - The @link IsoTpLink @endlink instance used to receive data.
/// @param payload - A pointer to an area in memory where the raw data is copied to.
///                  This buffer is packed if MAU_SIZE > 1 (UNSIGNED_MAU contains two or more classical 8-bit bytes).
/// @param payload_size - The size of the payload buffer in native bytes elements (UNSIGNED_MAU) units.
/// @param out_size - A reference to a variable which will contain the size of the actual (parsed) data in classical 8-bit bytes.
/// @return Possible return values:
///      - @link ISOTP_RET_OK @endlink
///      - @link ISOTP_RET_NO_DATA @endlink
///      - @link ISOTP_RET_OVERFLOW @endlink if payload size is not enough for the received message. In this case out_size will have actuall message size.
/// @warning If UNIT_BYTE is greater than 1, this function may copy additional (undefined) byte beyond message, if message length is odd. 
///          This is not an issue because this extra byte will not be written across buffer boundary, and out_size will return
///          correct received data size in bytes.
int isotp_receive(IsoTpLink *link, UNSIGNED_MAU *payload, const uint16_t payload_size, uint16_t *out_size);

/// @brief Provides access to received message without copying it, if any (inplace, within internal received buffer).
/// @param link - The @link IsoTpLink @endlink instance used to receive data.
/// @param payload - output argument, a pointer to a pointer which receives a pointer to received message.
/// @param out_size - output argument, a pointer to a number of bytes of received message.
/// @return Possible return values:
///      - @link ISOTP_RET_OK @endlink
///      - @link ISOTP_RET_NO_DATA @endlink
/// @warning isotp_reset_receive() MUST be called when message is processed, and no longer required. If not called, the 
///          following messages will not be received.
int isotp_receive_inplace(IsoTpLink *link, UNSIGNED_MAU **payload, uint16_t *out_size);

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_H__

