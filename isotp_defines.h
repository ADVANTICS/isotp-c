#ifndef __ISOTP_TYPES__
#define __ISOTP_TYPES__

///////////////////////////////////////////////////////////////
/// compiler specific defines.
///////////////////////////////////////////////////////////////
#ifdef __GNUC__
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ISOTP_BYTE_ORDER_LITTLE_ENDIAN
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#else
#error "unsupported byte ordering"
#endif
#endif





///////////////////////////////////////////////////////////////
/// OS specific defines
///////////////////////////////////////////////////////////////
#ifdef _WIN32
#define snprintf _snprintf
#endif

#ifdef _WIN32
#define ISOTP_BYTE_ORDER_LITTLE_ENDIAN
#define __builtin_bswap8  _byteswap_uint8
#define __builtin_bswap16 _byteswap_uint16
#define __builtin_bswap32 _byteswap_uint32
#define __builtin_bswap64 _byteswap_uint64
#endif

///////////////////////////////////////////////////////////////
/// Minimum addressable unit and it's size definitions.
///////////////////////////////////////////////////////////////

/// Number of 8-bit units in one byte.
#define MAU_SIZE    (__CHAR_BIT__ / 8)
#if MAU_SIZE == 1
    typedef uint8_t UNSIGNED_MAU;
#elif MAU_SIZE == 2
    typedef uint16_t UNSIGNED_MAU;
#else
    #error "Not supported minimum addressable unit"
#endif


/// @brief Struct containing the data for linking an application to a CAN instance.
/// The data stored in this struct is used internally and may be used by software programs
/// using this library.
typedef struct IsoTpLink {
    // sender paramters.
    uint32_t                    send_arbitration_id;    // used to reply consecutive frame

    // message buffer.
    UNSIGNED_MAU*               send_buffer;            // Note: This buffer is packed if UNSIGNED_MAU is 16 bit value.
    uint16_t                    send_buf_size;          // Note: The value is always in bytes.
    uint16_t                    send_size;              // Note: The value is always in bytes.
    uint16_t                    send_offset;            // Note: The value is always in bytes.

    // multi-frame flags.
    UNSIGNED_MAU                send_sn;
    uint16_t                    send_bs_remain;         // Remaining block size. Note: The value is always in classical 8-bit bytes.
    UNSIGNED_MAU                send_st_min;            // Separation Time between consecutive frames, unit millis.
    UNSIGNED_MAU                send_wtf_count;         // Maximum number of FC.Wait frame transmissions.
    uint32_t                    send_timer_st;          // Last time send consecutive frame.
    uint32_t                    send_timer_bs;          // Time until reception of the next FlowControl N_PDU
                                                        // start at sending FF, CF, receive FC
                                                        // end at receive FC
    int                         send_protocol_result;
    UNSIGNED_MAU                send_status;

    // receiver paramters.
    uint32_t                    receive_arbitration_id;

    // message buffer.
    UNSIGNED_MAU*               receive_buffer;         // Note: This buffer is packed if UNSIGNED_MAU is 16 bit value.
    uint16_t                    receive_buf_size;       // Note: The value is always in bytes.
    uint16_t                    receive_size;           // Note: The value is always in bytes.
    uint16_t                    receive_offset;         // Note: The value is always in bytes.

    // multi-frame control.
    UNSIGNED_MAU                receive_sn;
    UNSIGNED_MAU                receive_bs_count;       // Maximum number of FC.Wait frame transmissions.
    uint32_t                    receive_timer_cr;       // Time until transmission of the next ConsecutiveFrame N_PDU
                                                        // start at sending FC, receive CF 
                                                        // end at receive FC.

    int                         receive_protocol_result;
    UNSIGNED_MAU                receive_status;                                                     
} IsoTpLink;


///////////////////////////////////////////////////////////////
/// internal used defines
///////////////////////////////////////////////////////////////
#define ISOTP_RET_OK           0
#define ISOTP_RET_ERROR        -1
#define ISOTP_RET_INPROGRESS   -2
#define ISOTP_RET_OVERFLOW     -3
#define ISOTP_RET_WRONG_SN     -4
#define ISOTP_RET_NO_DATA      -5
#define ISOTP_RET_TIMEOUT      -6
#define ISOTP_RET_LENGTH       -7
#define ISOTP_RET_PROTOCOL     -8

/// Private: network layer result code.
#define ISOTP_PROTOCOL_RESULT_TIMEOUT_A    -10
#define ISOTP_PROTOCOL_RESULT_TIMEOUT_BS   -11
#define ISOTP_PROTOCOL_RESULT_TIMEOUT_CR   -12
#define ISOTP_PROTOCOL_RESULT_WRONG_SN     -13
#define ISOTP_PROTOCOL_RESULT_INVALID_FS   -14
#define ISOTP_PROTOCOL_RESULT_UNEXP_PDU    -15
#define ISOTP_PROTOCOL_RESULT_WFT_OVRN     -16
#define ISOTP_PROTOCOL_RESULT_BUFFER_OVFLW -17
#define ISOTP_PROTOCOL_RESULT_ERROR        -18

static inline 
int isotp_protocol_to_err(int protocol_err_code) {
    switch (protocol_err_code) {
        case ISOTP_PROTOCOL_RESULT_TIMEOUT_A:
        case ISOTP_PROTOCOL_RESULT_TIMEOUT_BS:
        case ISOTP_PROTOCOL_RESULT_TIMEOUT_CR:
            return ISOTP_RET_TIMEOUT;

        case ISOTP_PROTOCOL_RESULT_BUFFER_OVFLW:
            return ISOTP_RET_OVERFLOW;

        case ISOTP_PROTOCOL_RESULT_WRONG_SN:
        case ISOTP_PROTOCOL_RESULT_INVALID_FS:
        case ISOTP_PROTOCOL_RESULT_UNEXP_PDU:
        case ISOTP_PROTOCOL_RESULT_WFT_OVRN:
            return ISOTP_RET_PROTOCOL;

        case ISOTP_PROTOCOL_RESULT_ERROR:
            return ISOTP_RET_ERROR;
    };
    return protocol_err_code;
}

/// return logic true if 'a' is after 'b'
#define IsoTpTimeAfter(a,b) ((int32_t)((int32_t)(b) - (int32_t)(a)) < 0)

/// invalid bs
#define ISOTP_INVALID_BS       0xFFFF

/// ISOTP sender status
typedef enum {
    ISOTP_SEND_STATUS_IDLE,
    ISOTP_SEND_STATUS_INPROGRESS,
    ISOTP_SEND_STATUS_ERROR
} IsoTpSendStatusTypes;

/// ISOTP receiver status
typedef enum {
    ISOTP_RECEIVE_STATUS_IDLE,
    ISOTP_RECEIVE_STATUS_INPROGRESS,
    ISOTP_RECEIVE_STATUS_FULL
} IsoTpReceiveStatusTypes;

/// can frame defination
/// TI C28x family compiler uses __little_endian__ macro  to specify it's endianess.
/// ["TMS320C28x Optimizing C/C++ Compiler v22.6.0.LTS", Page 37]
#if defined(ISOTP_BYTE_ORDER_LITTLE_ENDIAN) || __little_endian__==1

typedef struct {
    UNSIGNED_MAU reserve_1:4;
    UNSIGNED_MAU type:4;
    UNSIGNED_MAU reserve_2[7];
} IsoTpPciType;

typedef struct {
    UNSIGNED_MAU SF_DL:4;
    UNSIGNED_MAU type:4;
    UNSIGNED_MAU data[7];
} IsoTpSingleFrame;

typedef struct {
    UNSIGNED_MAU FF_DL_high:4;
    UNSIGNED_MAU type:4;
    UNSIGNED_MAU FF_DL_low;
    UNSIGNED_MAU data[6];
} IsoTpFirstFrame;

typedef struct {
    UNSIGNED_MAU SN:4;
    UNSIGNED_MAU type:4;
    UNSIGNED_MAU data[7];
} IsoTpConsecutiveFrame;

typedef struct {
    UNSIGNED_MAU FS:4;
    UNSIGNED_MAU type:4;
    UNSIGNED_MAU BS;
    UNSIGNED_MAU STmin;
    UNSIGNED_MAU reserve[5];
} IsoTpFlowControl;

#else

typedef struct {
    UNSIGNED_MAU type:4;
    UNSIGNED_MAU reserve_1:4;
    UNSIGNED_MAU reserve_2[7];
} IsoTpPciType;


/// single frame
/// +-------------------------+-----+
/// | byte #0                 | ... |
/// +-------------------------+-----+
/// | nibble #0   | nibble #1 | ... |
/// +-------------+-----------+ ... +
/// | PCIType = 0 | SF_DL     | ... |
/// +-------------+-----------+-----+
typedef struct {
    UNSIGNED_MAU type:4;
    UNSIGNED_MAU SF_DL:4;
    UNSIGNED_MAU data[7];
} IsoTpSingleFrame;

/// first frame
/// +-------------------------+-----------------------+-----+
/// | byte #0                 | byte #1               | ... |
/// +-------------------------+-----------+-----------+-----+
/// | nibble #0   | nibble #1 | nibble #2 | nibble #3 | ... |
/// +-------------+-----------+-----------+-----------+-----+
/// | PCIType = 1 | FF_DL                             | ... |
/// +-------------+-----------+-----------------------+-----+
typedef struct {
    UNSIGNED_MAU type:4;
    UNSIGNED_MAU FF_DL_high:4;
    UNSIGNED_MAU FF_DL_low;
    UNSIGNED_MAU data[6];
} IsoTpFirstFrame;

/// consecutive frame
/// +-------------------------+-----+
/// | byte #0                 | ... |
/// +-------------------------+-----+
/// | nibble #0   | nibble #1 | ... |
/// +-------------+-----------+ ... +
/// | PCIType = 0 | SN        | ... |
/// +-------------+-----------+-----+
typedef struct {
    UNSIGNED_MAU type:4;
    UNSIGNED_MAU SN:4;
    UNSIGNED_MAU data[7];
} IsoTpConsecutiveFrame;


/// flow control frame
/// +-------------------------+-----------------------+-----------------------+-----+
/// | byte #0                 | byte #1               | byte #2               | ... |
/// +-------------------------+-----------+-----------+-----------+-----------+-----+
/// | nibble #0   | nibble #1 | nibble #2 | nibble #3 | nibble #4 | nibble #5 | ... |
/// +-------------+-----------+-----------+-----------+-----------+-----------+-----+
/// | PCIType = 1 | FS        | BS                    | STmin                 | ... |
/// +-------------+-----------+-----------------------+-----------------------+-----+
typedef struct {
    UNSIGNED_MAU type:4;
    UNSIGNED_MAU FS:4;
    UNSIGNED_MAU BS;
    UNSIGNED_MAU STmin;
    UNSIGNED_MAU reserve[5];
} IsoTpFlowControl;

#endif

typedef struct {
    UNSIGNED_MAU ptr[8];
} IsoTpDataArray;

typedef struct {
    union {
        IsoTpPciType          common;
        IsoTpSingleFrame      single_frame;
        IsoTpFirstFrame       first_frame;
        IsoTpConsecutiveFrame consecutive_frame;
        IsoTpFlowControl      flow_control;
        IsoTpDataArray        data_array;
    } as;
} IsoTpCanMessage;

///////////////////////////////////////////////////////////////
/// protocol specific defines
///////////////////////////////////////////////////////////////

/// Private: Protocol Control Information (PCI) types, for identifying each frame of an ISO-TP message.
typedef enum {
    ISOTP_PCI_TYPE_SINGLE             = 0x0,
    ISOTP_PCI_TYPE_FIRST_FRAME        = 0x1,
    TSOTP_PCI_TYPE_CONSECUTIVE_FRAME  = 0x2,
    ISOTP_PCI_TYPE_FLOW_CONTROL_FRAME = 0x3
} IsoTpProtocolControlInformation;

/// Private: Protocol Control Information (PCI) flow control identifiers.
typedef enum {
    PCI_FLOW_STATUS_CONTINUE = 0x0,
    PCI_FLOW_STATUS_WAIT     = 0x1,
    PCI_FLOW_STATUS_OVERFLOW = 0x2
} IsoTpFlowStatus;



#endif

