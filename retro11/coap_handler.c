#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fmt.h"
#include "net/gcoap.h"
#include "smart_environment.h"

extern void get_name_of_player(char *value);
extern char * start_game(void);
extern void get_result(char *value);
extern void set_winner(void);
extern void set_looser(void);
extern ssize_t entry_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len);

char ping_stack[THREAD_STACKSIZE_DEFAULT];

static ssize_t _request_name_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    char str[40] = "unknown";
    get_name_of_player(str);
    int slen = strlen(str);

    gcoap_resp_init(pdu, buf, GCOAP_PDU_BUF_SIZE, COAP_CODE_205);
    memcpy((char*)pdu->payload, str, slen);    
    int8_t gcoap_resp = gcoap_finish(pdu, slen, COAP_FORMAT_TEXT);
    return gcoap_resp;
}

static ssize_t _start_game_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    const char *str = start_game();
    int slen = strlen(str);

    gcoap_resp_init(pdu, buf, GCOAP_PDU_BUF_SIZE, COAP_CODE_205);
    memcpy((char*)pdu->payload, str, slen);    
    int8_t gcoap_resp = gcoap_finish(pdu, slen, COAP_FORMAT_TEXT);
    return gcoap_resp;
}

static ssize_t _request_result_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    char str[40] = "result";
    get_result(str);
    int slen = strlen(str);

    gcoap_resp_init(pdu, buf, GCOAP_PDU_BUF_SIZE, COAP_CODE_205);
    memcpy((char*)pdu->payload, str, slen);    
    int8_t gcoap_resp = gcoap_finish(pdu, slen, COAP_FORMAT_TEXT);
    return gcoap_resp;
}

static ssize_t _set_winner_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    const char *str = "winner was displayed";
    int slen = strlen(str);

    set_winner();

    gcoap_resp_init(pdu, buf, GCOAP_PDU_BUF_SIZE, COAP_CODE_205);
    memcpy((char*)pdu->payload, str, slen);    
    int8_t gcoap_resp = gcoap_finish(pdu, slen, COAP_FORMAT_TEXT);
    return gcoap_resp;
}

static ssize_t _set_looser_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len)
{
    const char *str = "looser was displayed";
    int slen = strlen(str);

    set_looser();

    gcoap_resp_init(pdu, buf, GCOAP_PDU_BUF_SIZE, COAP_CODE_205);
    memcpy((char*)pdu->payload, str, slen);    
    int8_t gcoap_resp = gcoap_finish(pdu, slen, COAP_FORMAT_TEXT);
    return gcoap_resp;
}

/* must be sorted by path (alphabetically) */
const coap_resource_t coap_resources[] = {
    COAP_WELL_KNOWN_CORE_DEFAULT_HANDLER,
    { "/request/name", COAP_GET, _request_name_handler },
    { "/request/result", COAP_GET, _request_result_handler },
    { "/set/looser", COAP_GET, _set_looser_handler },
    { "/set/winner", COAP_GET, _set_winner_handler },
    { "/start/game", COAP_GET, _start_game_handler },
    { "/se-app/entries", COAP_GET, &entry_handler}
};

gcoap_listener_t coap_server_listener = {
    (coap_resource_t *)&coap_resources,
    sizeof(coap_resources) / sizeof(coap_resources[0]),
    NULL
};

const unsigned coap_resources_numof = sizeof(coap_resources) / sizeof(coap_resources[0]);

void* ping_handler(void* args) {
  (void)args;
  sock_udp_ep_t remote = SOCK_IPV6_EP_ANY;
  remote.port = SERVER_CONN_PORT;
  ipv6_addr_from_str((ipv6_addr_t *)&remote.addr.ipv6, "ff02::1");

  printf("\n\nPing thread running\n\n\n");

  while(true) {
    printf("\n\nPing thread running\n\n\n");
    ssize_t res = sock_udp_send(
      NULL,
      app_id,
      APP_ID_LEN,
      &remote
    );

    if(res == -EAFNOSUPPORT) {
      fputs("Ping: EAFNOSUPPORT", stderr);
    } else if(res == -EHOSTUNREACH) {
      fputs("Ping: EHOSTUNREACH", stderr);
    } else if(res == -EINVAL) {
      fputs("Ping: EINVAL", stderr);
    } else if(res == -ENOMEM) {
      fputs("Ping: ENOMEM", stderr);
    } else if(res == -ENOTCONN) {
      fputs("Ping: ENOTCONN", stderr);
    } else if(res < 0) {
      fprintf(stderr, "Ping error: %d\n", res);
    }

    if(res < 0) {
        break;
    } else {
        xtimer_sleep(PING_TIMEOUT);
    }
  }

  puts("ping thread terminating!");

  return NULL;
}

void coap_server_init(void)
{
    gcoap_register_listener(&coap_server_listener);
    thread_create(ping_stack, THREAD_STACKSIZE_DEFAULT, THREAD_PRIORITY_MAIN - 1,
        THREAD_CREATE_STACKTEST, ping_handler, NULL, "ping thread");
}
