#ifndef __ISOTP_USER_H__
#define __ISOTP_USER_H__

/* user implemented, print debug message */
#define isotp_user_debug(x)

/* user implemented, send can message. should return ISOTP_RET_OK when success.
*/
int  isotp_user_send_can(const uint32_t arbitration_id,
                         const uint_least8_t* data, const uint_least8_t size);

/* user implemented, get millisecond */
uint32_t isotp_user_get_ms(void);

#endif // __ISOTP_H__

