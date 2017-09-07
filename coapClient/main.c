#include <stdlib.h>
#include "msg.h"

#include "net/ipv6/addr.h"
#include "net/gnrc/ipv6/netif.h"

#include "coap_client.h"

#define MAIN_QUEUE_SIZE (8)

static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

char own_addr[IPV6_ADDR_MAX_STR_LEN];

int main(void) {
  /* for the thread running the shell */
  msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

  ipv6_addr_t addr;
  ipv6_addr_from_str(&addr, "fe80::");

  ipv6_addr_t* out = NULL;
  gnrc_ipv6_netif_find_by_prefix(&out, &addr);

  ipv6_addr_to_str(own_addr, out, IPV6_ADDR_MAX_STR_LEN);

  coap_client_init();

  coap_client_run();

  return 0;
}
