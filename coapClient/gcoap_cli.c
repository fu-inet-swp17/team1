#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "net/gcoap.h"
#include "od.h"
#include "fmt.h"

static void _resp_handler(unsigned req_state, coap_pkt_t* pdu);

extern void set_is_init(int);
extern void set_name(char * name, int);
extern void set_is_working(int);
extern void set_result(char * result, int);

/*
 * Response callback.
 */
static void _resp_handler(unsigned req_state, coap_pkt_t* pdu) {
    if (req_state == GCOAP_MEMO_TIMEOUT) {
        printf("gcoap: timeout for msg ID %02u\n", coap_get_id(pdu));
        return;
    }
    else if (req_state == GCOAP_MEMO_ERR) {
        printf("gcoap: error in response\n");
        return;
    }

    char *class_str = (coap_get_code_class(pdu) == COAP_CLASS_SUCCESS)
                            ? "Success" : "Error";
    printf("gcoap: response %s, code %1u.%02u", class_str, coap_get_code_class(pdu), coap_get_code_detail(pdu));
    
    if (pdu->payload_len) {
        if (pdu->content_type == COAP_FORMAT_TEXT || pdu->content_type == COAP_FORMAT_LINK
                || coap_get_code_class(pdu) == COAP_CLASS_CLIENT_FAILURE
                || coap_get_code_class(pdu) == COAP_CLASS_SERVER_FAILURE) {
            /* Expecting diagnostic payload in failure cases */
            printf(", %u bytes\nResponse from Server: %.*s\n", pdu->payload_len, pdu->payload_len, (char *)pdu->payload);

            char machine[3];
            int pldlen = pdu->payload_len-3;
            char pld[pldlen];
            
            strncpy(machine, (char *)pdu->payload, 3);
            machine[3]='\0';
            
            strncpy(pld, (char *)pdu->payload + 3, pldlen);
            pld [pldlen] = '\0';

            int m;
            if (strcmp(pld, "M0.")) {
                m = 0;
            }
            else if (strcmp(pld, "M1.")) {
                m = 1;
            }
            else {
                printf("%s\n",(char *)pdu->payload);
                return;
            }

            if (strncmp(pld, "in", 2) == 0) {
                set_is_init(m);
            }
            if (strncmp(pld, "st", 2) == 0) {
                set_is_working(m);   
            }
            if (strncmp(pld, "na", 2) == 0) {
                char name[pldlen-3];
                strncpy(name, pld+3, pldlen-3);
                name [pldlen-3] = '\0';
                set_name(name,m);
            }
            if (strncmp(pld, "re", 2) == 0) {
                char result[pldlen-3];
                strncpy(result, pld+3, pldlen-3);
                result [pldlen-3] = '\0';
                set_result(result,m);
            }
        }
        else {
            printf(", %u bytes\n", pdu->payload_len);
            od_hex_dump(pdu->payload, pdu->payload_len, OD_WIDTH_DEFAULT);
        }
    }
    else {
        printf(", empty payload\n");
    }
}

static size_t _send(uint8_t *buf, size_t len, char *addr_str, char *port_str) {
    ipv6_addr_t addr;
    size_t bytes_sent;
    sock_udp_ep_t remote;

    remote.family = AF_INET6;
    remote.netif  = SOCK_ADDR_ANY_NETIF;

    /* parse destination address */
    if (ipv6_addr_from_str(&addr, addr_str) == NULL) {
        puts("gcoap_cli: unable to parse destination address");
        return 0;
    }
    memcpy(&remote.addr.ipv6[0], &addr.u8[0], sizeof(addr.u8));

    /* parse port */
    remote.port = (uint16_t)atoi(port_str);
    if (remote.port == 0) {
        puts("gcoap_cli: unable to parse destination port");
        return 0;
    }

    bytes_sent = gcoap_req_send2(buf, len, &remote, _resp_handler);
    return bytes_sent;
}

int gcoap_cli_cmd(int argc, char **argv) {
    char *method_codes[] = {"get", "post", "put"};
    uint8_t buf[GCOAP_PDU_BUF_SIZE];
    coap_pkt_t pdu;
    size_t len;

    if (argc == 1) {
        goto end;
    }

    for (size_t i = 0; i < sizeof(method_codes) / sizeof(char*); i++) {
        if (strcmp(argv[1], method_codes[i]) == 0) {            
            len = gcoap_request(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, i+1, argv[4]);                
            printf("gcoap_cli: sending msg ID %u, %u bytes\n", coap_get_id(&pdu), (unsigned) len);
            if (!_send(&buf[0], len, argv[2], argv[3])) {
                puts("gcoap_cli: msg send failed");
            }
            return 0;
        }
    }
    end:
    printf("falsche Eingabe\n");
    return 1;
}