#include <stdint.h>
#include "assert.h"
#include "isotp.h"

#if MAU_SIZE == 2
#include "buffer_pack_unpack_16.h"
#endif

///////////////////////////////////////////////////////
///                 STATIC FUNCTIONS                ///
///////////////////////////////////////////////////////

// st_min to microsecond
static UNSIGNED_MAU isotp_ms_to_st_min(UNSIGNED_MAU ms) {
    UNSIGNED_MAU st_min;

    st_min = ms;
    if (st_min > 0x7F) {
        st_min = 0x7F;
    }

    return st_min;
}

// st_min to msec
static UNSIGNED_MAU isotp_st_min_to_ms(UNSIGNED_MAU st_min) {
    UNSIGNED_MAU ms;
    
    if (st_min >= 0xF1 && st_min <= 0xF9) {
        ms = 1;
    } else if (st_min <= 0x7F) {
        ms = st_min;
    } else {
        ms = 0;
    }

    return ms;
}

static int isotp_send_flow_control(IsoTpLink* link, UNSIGNED_MAU flow_status, UNSIGNED_MAU block_size, UNSIGNED_MAU st_min_ms) {

    IsoTpCanMessage message;
    int ret;

    // setup message
    message.as.flow_control.type = ISOTP_PCI_TYPE_FLOW_CONTROL_FRAME;
    message.as.flow_control.FS = flow_status;
    message.as.flow_control.BS = block_size;
    message.as.flow_control.STmin = isotp_ms_to_st_min(st_min_ms);

    // send message
#ifdef ISO_TP_FRAME_PADDING
    (void) memset(message.as.flow_control.reserve, 0, sizeof(message.as.flow_control.reserve));
    ret = isotp_user_send_can(link->send_arbitration_id, message.as.data_array.ptr, sizeof(message));
#else    
    ret = isotp_user_send_can(link->send_arbitration_id,
            message.as.data_array.ptr,
            3);
#endif

    return ret;
}

static int isotp_send_single_frame(IsoTpLink* link, uint32_t id) {

    IsoTpCanMessage message;
    int ret;

    // multi frame message length must greater than 7
    assert(link->send_size <= 7);

    // setup message
    message.as.single_frame.type = ISOTP_PCI_TYPE_SINGLE;
    message.as.single_frame.SF_DL = (UNSIGNED_MAU) link->send_size;

#if MAU_SIZE == 2
    buffer_unpack16(message.as.single_frame.data, link->send_buffer, 0, link->send_size);
#elif MAU_SIZE == 1
    (void) memcpy(message.as.single_frame.data, link->send_buffer, link->send_size);
#else
    #error Unsupported MAU_SIZE
#endif


    // send message
#ifdef ISO_TP_FRAME_PADDING
    (void) memset(message.as.single_frame.data + link->send_size, 0, sizeof(message.as.single_frame.data) - link->send_size);
    ret = isotp_user_send_can(id, message.as.data_array.ptr, sizeof(message));
#else
    ret = isotp_user_send_can(id,
            message.as.data_array.ptr,
            link->send_size + 1);
#endif

    isotp_send_done(link);

    return ret;
}

static int isotp_send_first_frame(IsoTpLink* link, uint32_t id) {
    
    IsoTpCanMessage message;
    int ret;

    // multi frame message length must greater than 7
    assert(link->send_size > 7);

    // setup message
    message.as.first_frame.type = ISOTP_PCI_TYPE_FIRST_FRAME;
    message.as.first_frame.FF_DL_low = (UNSIGNED_MAU) link->send_size;
    message.as.first_frame.FF_DL_high = (UNSIGNED_MAU) (0x0F & (link->send_size >> 8));

#if MAU_SIZE == 2
    buffer_unpack16(message.as.first_frame.data, link->send_buffer, 0, sizeof(message.as.first_frame.data));
#elif MAU_SIZE == 1
    (void) memcpy(message.as.first_frame.data, link->send_buffer, sizeof(message.as.first_frame.data));
#else
    #error Unsupported MAU_SIZE
#endif

    // send message
    ret = isotp_user_send_can(id, message.as.data_array.ptr, sizeof(message));
    if (ISOTP_RET_OK == ret) {
        link->send_offset += sizeof(message.as.first_frame.data);
        link->send_bs_remain = link->send_size - sizeof(message.as.first_frame.data);
        link->send_sn = 1;
    }

    return ret;
}

static int isotp_send_consecutive_frame(IsoTpLink* link) {
    
    IsoTpCanMessage message;
    uint16_t data_length;
    int ret;

    // multi frame message length must greater than 7
    assert(link->send_size > 7);

    // setup message
    message.as.consecutive_frame.type = TSOTP_PCI_TYPE_CONSECUTIVE_FRAME;
    message.as.consecutive_frame.SN = link->send_sn;
    data_length = link->send_size - link->send_offset;
    if (data_length > sizeof(message.as.consecutive_frame.data)) {
        data_length = sizeof(message.as.consecutive_frame.data);
    }

#if MAU_SIZE == 2
    buffer_unpack16(message.as.consecutive_frame.data, link->send_buffer, link->send_offset, data_length);
#elif MAU_SIZE == 1
    (void) memcpy(message.as.consecutive_frame.data, link->send_buffer + link->send_offset, data_length);
#else
    #error Unsupported MAU_SIZE
#endif

    // send message
#ifdef ISO_TP_FRAME_PADDING
    (void) memset(message.as.consecutive_frame.data + data_length, 0, sizeof(message.as.consecutive_frame.data) - data_length);
    ret = isotp_user_send_can(link->send_arbitration_id, message.as.data_array.ptr, sizeof(message));
#else
    ret = isotp_user_send_can(link->send_arbitration_id,
            message.as.data_array.ptr,
            data_length + 1);
#endif
    if (ISOTP_RET_OK == ret) {
        link->send_offset += data_length;
        if (++(link->send_sn) > 0x0F) {
            link->send_sn = 0;
        }
    }
    
    return ret;
}

static int isotp_receive_single_frame(IsoTpLink *link, IsoTpCanMessage *message, UNSIGNED_MAU len) {
    // check data length
    if ((0 == message->as.single_frame.SF_DL) || (message->as.single_frame.SF_DL > (len - 1))) {
        isotp_user_debug("Single-frame length too small.");
        return ISOTP_RET_LENGTH;
    }

    // copying data
#if MAU_SIZE == 2
    buffer_pack16(link->receive_buffer, 0, message->as.single_frame.data, message->as.single_frame.SF_DL);
#elif MAU_SIZE == 1
    (void) memcpy(link->receive_buffer, message->as.single_frame.data, message->as.single_frame.SF_DL);
#else
    #error Unsupported MAU_SIZE
#endif

    link->receive_size = message->as.single_frame.SF_DL;
    
    return ISOTP_RET_OK;
}

static int isotp_receive_first_frame(IsoTpLink *link, IsoTpCanMessage *message, UNSIGNED_MAU len) {
    uint16_t payload_length;

    if (8 != len) {
        isotp_user_debug("First frame should be 8 bytes in length.");
        return ISOTP_RET_LENGTH;
    }

    // check data length
    payload_length = message->as.first_frame.FF_DL_high;
    payload_length = (payload_length << 8) + message->as.first_frame.FF_DL_low;

    // should not use multiple frame transmition
    if (payload_length <= 7) {
        isotp_user_debug("Should not use multiple frame transmission.");
        return ISOTP_RET_LENGTH;
    }
    
    if (payload_length > link->receive_buf_size) {
        isotp_user_debug("Multi-frame response too large for receiving buffer.");
        return ISOTP_RET_OVERFLOW;
    }
    
    // copying data
#if MAU_SIZE == 2
    buffer_pack16(link->receive_buffer, 0, message->as.first_frame.data, sizeof(message->as.first_frame.data));
#elif MAU_SIZE == 1
    (void) memcpy(link->receive_buffer, message->as.first_frame.data, sizeof(message->as.first_frame.data));
#else
    #error Unsupported MAU_SIZE
#endif

    link->receive_size = payload_length;
    link->receive_offset = sizeof(message->as.first_frame.data);
    link->receive_sn = 1;

    return ISOTP_RET_OK;
}

static int isotp_receive_consecutive_frame(IsoTpLink *link, IsoTpCanMessage *message, UNSIGNED_MAU len) {
    uint16_t remaining_bytes;
    
    // check sn
    if (link->receive_sn != message->as.consecutive_frame.SN) {
        return ISOTP_RET_WRONG_SN;
    }

    // check data length
    remaining_bytes = link->receive_size - link->receive_offset;
    if (remaining_bytes > sizeof(message->as.consecutive_frame.data)) {
        remaining_bytes = sizeof(message->as.consecutive_frame.data);
    }
    if (remaining_bytes > len - 1) {
        isotp_user_debug("Consecutive frame too short.");
        return ISOTP_RET_LENGTH;
    }

    // copying data
#if MAU_SIZE == 2
    buffer_pack16(link->receive_buffer, link->receive_offset, message->as.consecutive_frame.data, remaining_bytes);
#elif MAU_SIZE == 1
    (void) memcpy(link->receive_buffer + link->receive_offset, message->as.consecutive_frame.data, remaining_bytes);
#else
    #error Unsupported MAU_SIZE
#endif

    link->receive_offset += remaining_bytes;
    if (++(link->receive_sn) > 0x0F) {
        link->receive_sn = 0;
    }

    return ISOTP_RET_OK;
}

static int isotp_receive_flow_control_frame(IsoTpLink *link, IsoTpCanMessage *message, UNSIGNED_MAU len) {
    // check message length
    if (len < 3) {
        isotp_user_debug("Flow control frame too short.");
        return ISOTP_RET_LENGTH;
    }

    return ISOTP_RET_OK;
}


///////////////////////////////////////////////////////
///                 PUBLIC FUNCTIONS                ///
///////////////////////////////////////////////////////

int isotp_send(IsoTpLink *link, const UNSIGNED_MAU payload[], uint16_t size) {
    return isotp_send_with_id(link, link->send_arbitration_id, payload, size);
}

int isotp_send_with_id(IsoTpLink *link, uint32_t id, const UNSIGNED_MAU payload[], uint16_t size_in_bytes) {
    uint16_t size_in_words = (size_in_bytes + 1) / 2;
    int ret;

    if (link == 0x0) {
        isotp_user_debug("Link is null!");
        return ISOTP_RET_ERROR;
    }

    if (size_in_bytes > link->send_buf_size) {
        isotp_user_debug("Message size too large. Increase ISO_TP_MAX_MESSAGE_SIZE to set a larger buffer\n");
        char message[128];
        sprintf(&message[0], "Attempted to send %d bytes; max size is %d!\n", size_in_bytes, link->send_buf_size);
        return ISOTP_RET_OVERFLOW;
    }

    if (ISOTP_SEND_STATUS_INPROGRESS == link->send_status) {
        isotp_user_debug("Abort previous message, transmission in progress.\n");
        return ISOTP_RET_INPROGRESS;
    }

    // copy into local buffer
    // Note: the following code may copy 1 extra 8-byte byte unit for CPUs with MAU_SIZE > 1.
    // It's not an issue because local buffer size is multiple of native byte size, and data
    // sending is based on classical 8-bit units.
    link->send_size = size_in_bytes;
    link->send_offset = 0;
    (void) memcpy(link->send_buffer, payload, size_in_words);

    if (link->send_size < 8) {
        // send single frame
        ret = isotp_send_single_frame(link, id);
    } else {
        // send multi-frame
        ret = isotp_send_first_frame(link, id);

        // init multi-frame control flags
        if (ISOTP_RET_OK == ret) {
            link->send_st_min = 0;
            link->send_wtf_count = 0;
            link->send_timer_st = isotp_user_get_ms();
            link->send_timer_bs = isotp_user_get_ms() + ISO_TP_DEFAULT_RESPONSE_TIMEOUT;
            link->send_protocol_result = ISOTP_RET_OK;
            link->send_status = ISOTP_SEND_STATUS_INPROGRESS;
        }
    }

    return ret;
}

void isotp_on_can_message(IsoTpLink *link, UNSIGNED_MAU *data, UNSIGNED_MAU len) {
    IsoTpCanMessage message;
    int ret;
    
    if (len < 2 || len > 8) {
        return;
    }

    memcpy(message.as.data_array.ptr, data, len);
    memset(message.as.data_array.ptr + len, 0, sizeof(message.as.data_array.ptr) - len);

    switch (message.as.common.type) {
        case ISOTP_PCI_TYPE_SINGLE: {
            // update protocol result
            if (ISOTP_RECEIVE_STATUS_INPROGRESS == link->receive_status) {
                link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_UNEXP_PDU;
            } else {
                link->receive_protocol_result = ISOTP_RET_OK;
            }

            // handle message
            ret = isotp_receive_single_frame(link, &message, len);
            
            if (ISOTP_RET_OK == ret) {
                // change status
                link->receive_status = ISOTP_RECEIVE_STATUS_FULL;
                isotp_recv_done(link);
            }
            break;
        }
        case ISOTP_PCI_TYPE_FIRST_FRAME: {
            // update protocol result
            if (ISOTP_RECEIVE_STATUS_INPROGRESS == link->receive_status) {
                link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_UNEXP_PDU;
            } else {
                link->receive_protocol_result = ISOTP_RET_OK;
            }

            // handle message
            ret = isotp_receive_first_frame(link, &message, len);

            // if overflow happened
            if (ISOTP_RET_OVERFLOW == ret) {
                // update protocol result
                link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_BUFFER_OVFLW;

                // Call error callback
                isotp_recv_fail(link, isotp_protocol_to_err(link->receive_protocol_result));

                // change status
                isotp_reset_receive(link);

                // send error message
                isotp_send_flow_control(link, PCI_FLOW_STATUS_OVERFLOW, 0, 0);
                break;
            }

            // if receive successful
            if (ISOTP_RET_OK == ret) {
                // change status
                link->receive_status = ISOTP_RECEIVE_STATUS_INPROGRESS;
                // send fc frame
                link->receive_bs_count = ISO_TP_DEFAULT_BLOCK_SIZE;
                isotp_send_flow_control(link, PCI_FLOW_STATUS_CONTINUE, link->receive_bs_count, ISO_TP_DEFAULT_ST_MIN);
                // refresh timer cs
                link->receive_timer_cr = isotp_user_get_ms() + ISO_TP_DEFAULT_RESPONSE_TIMEOUT;
            }
            
            break;
        }
        case TSOTP_PCI_TYPE_CONSECUTIVE_FRAME: {
            // check if in receiving status
            if (ISOTP_RECEIVE_STATUS_INPROGRESS != link->receive_status) {
                link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_UNEXP_PDU;
                break;
            }

            // handle message
            ret = isotp_receive_consecutive_frame(link, &message, len);

            // if wrong sn
            if (ISOTP_RET_WRONG_SN == ret) {
                link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_WRONG_SN;

                // Call error callback
                isotp_recv_fail(link, isotp_protocol_to_err(link->receive_protocol_result));

                isotp_reset_receive(link);
                break;
            }

            // if success
            if (ISOTP_RET_OK == ret) {
                // refresh timer cs
                link->receive_timer_cr = isotp_user_get_ms() + ISO_TP_DEFAULT_RESPONSE_TIMEOUT;
                
                // receive finished
                if (link->receive_offset >= link->receive_size) {
                    link->receive_status = ISOTP_RECEIVE_STATUS_FULL;
                    isotp_recv_done(link);
                } else {
                    // send fc when bs reaches limit
                    if (0 == --link->receive_bs_count) {
                        link->receive_bs_count = ISO_TP_DEFAULT_BLOCK_SIZE;
                        isotp_send_flow_control(link, PCI_FLOW_STATUS_CONTINUE, link->receive_bs_count, ISO_TP_DEFAULT_ST_MIN);
                    }
                }
            }
            
            break;
        }
        case ISOTP_PCI_TYPE_FLOW_CONTROL_FRAME:
            // handle fc frame only when sending in progress 
            if (ISOTP_SEND_STATUS_INPROGRESS != link->send_status) {
                break;
            }

            // handle message
            ret = isotp_receive_flow_control_frame(link, &message, len);
            
            if (ISOTP_RET_OK == ret) {
                // refresh bs timer
                link->send_timer_bs = isotp_user_get_ms() + ISO_TP_DEFAULT_RESPONSE_TIMEOUT;

                // overflow
                if (PCI_FLOW_STATUS_OVERFLOW == message.as.flow_control.FS) {
                    link->send_protocol_result = ISOTP_PROTOCOL_RESULT_BUFFER_OVFLW;
                    isotp_send_fail(link, isotp_protocol_to_err(link->send_protocol_result));
                    link->send_status = ISOTP_SEND_STATUS_ERROR;
                }

                // wait
                else if (PCI_FLOW_STATUS_WAIT == message.as.flow_control.FS) {
                    link->send_wtf_count += 1;
                    // wait exceed allowed count
                    if (link->send_wtf_count > ISO_TP_MAX_WFT_NUMBER) {
                        link->send_protocol_result = ISOTP_PROTOCOL_RESULT_WFT_OVRN;
                        isotp_send_fail(link, isotp_protocol_to_err(link->send_protocol_result));
                        link->send_status = ISOTP_SEND_STATUS_ERROR;
                    }
                }

                // permit send
                else if (PCI_FLOW_STATUS_CONTINUE == message.as.flow_control.FS) {
                    if (0 == message.as.flow_control.BS) {
                        link->send_bs_remain = ISOTP_INVALID_BS;
                    } else {
                        link->send_bs_remain = message.as.flow_control.BS;
                    }
                    link->send_st_min = isotp_st_min_to_ms(message.as.flow_control.STmin);
                    link->send_wtf_count = 0;
                }
            }
            break;
        default:
            break;
    };
    
    return;
}


int isotp_receive(IsoTpLink *link, UNSIGNED_MAU *payload, const uint16_t payload_size, uint16_t *out_size) {
    int result = ISOTP_RET_NO_DATA;
    uint16_t copylen;
    
    if (ISOTP_RECEIVE_STATUS_FULL != link->receive_status) {
        return result;
    }

    result = ISOTP_RET_OK;
    copylen = (link->receive_size + 1)/ MAU_SIZE;
    if (copylen > payload_size) {
        copylen = payload_size;
        result = ISOTP_RET_OVERFLOW;
    }

    memcpy(payload, link->receive_buffer, copylen);
    *out_size = link->receive_size;

    isotp_reset_receive(link);

    return result;
}


void isotp_init_link(IsoTpLink *link, uint32_t sendid, UNSIGNED_MAU *sendbuf, uint16_t sendbufsize, UNSIGNED_MAU *recvbuf, uint16_t recvbufsize) {
    memset(link, 0, sizeof(*link));
    isotp_reset_receive(link);
    link->send_status = ISOTP_SEND_STATUS_IDLE;
    link->send_arbitration_id = sendid;

    link->send_buffer = sendbuf;
    link->send_buf_size = sendbufsize * MAU_SIZE;
    link->receive_buffer = recvbuf;
    link->receive_buf_size = recvbufsize * MAU_SIZE;
    
    return;
}

void isotp_poll(IsoTpLink *link) {
    int ret;

    // only polling when operation in progress
    if (ISOTP_SEND_STATUS_INPROGRESS == link->send_status) {

        // continue send data
        if (// send data if bs_remain is invalid or bs_remain large than zero
        (ISOTP_INVALID_BS == link->send_bs_remain || link->send_bs_remain > 0) &&
        // and if st_min is zero or go beyond interval time
        (0 == link->send_st_min || (0 != link->send_st_min && IsoTpTimeAfter(isotp_user_get_ms(), link->send_timer_st)))) {
            
            ret = isotp_send_consecutive_frame(link);
            if (ISOTP_RET_OK == ret) {
                if (ISOTP_INVALID_BS != link->send_bs_remain) {
                    link->send_bs_remain -= 1;
                }
                link->send_timer_bs = isotp_user_get_ms() + ISO_TP_DEFAULT_RESPONSE_TIMEOUT;
                link->send_timer_st = isotp_user_get_ms() + link->send_st_min;

                // check if send finish
                if (link->send_offset >= link->send_size) {
                    isotp_send_done(link);
                    link->send_status = ISOTP_SEND_STATUS_IDLE;
                }
            } else {
                link->send_protocol_result = ISOTP_PROTOCOL_RESULT_ERROR;
                isotp_send_fail(link, ret);
                link->send_status = ISOTP_SEND_STATUS_ERROR;
            }
        }

        // check timeout
        if (IsoTpTimeAfter(isotp_user_get_ms(), link->send_timer_bs)) {
            link->send_protocol_result = ISOTP_PROTOCOL_RESULT_TIMEOUT_BS;
            isotp_send_fail(link, isotp_protocol_to_err(link->send_protocol_result));
            link->send_status = ISOTP_SEND_STATUS_ERROR;
        }
    }

    // only polling when operation in progress
    if (ISOTP_RECEIVE_STATUS_INPROGRESS == link->receive_status) {
        
        // check timeout
        if (IsoTpTimeAfter(isotp_user_get_ms(), link->receive_timer_cr)) {
            link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_TIMEOUT_CR;

            // Call error callback
            isotp_recv_fail(link, isotp_protocol_to_err(link->receive_protocol_result));

            isotp_reset_receive(link);
        }
    }

    return;
}

