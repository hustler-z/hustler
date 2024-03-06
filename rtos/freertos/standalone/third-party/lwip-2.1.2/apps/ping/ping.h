#ifndef LWIP_PING_H
#define LWIP_PING_H

#include "lwip/ip_addr.h"

void
ping_init(const ip_addr_t* ping_addr,u32 cnt);
void ping_deinit();

#if !PING_USE_SOCKETS
void ping_send_now(void);
#endif /* !PING_USE_SOCKETS */

#endif /* LWIP_PING_H */
