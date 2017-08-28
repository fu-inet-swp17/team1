#include "saul_reg.h"
#include "net/gcoap.h"
#include "senml.h"
#include "floatint.h"
#include "smart_environment.h"

#define PING_TIMEOUT	60
#define SENML_LEN       75

extern int entry_counter;
extern char * nameslist[];
extern char * resultslist[];

int8_t senml_json_strout(char* json_buf, uint8_t dev_type) {
//    phydat_t res;
    saul_reg_t* dev = saul_reg_find_type(dev_type);

    char dev_name[21];
    strncpy(dev_name, dev->name, 20);
    dev_name[20] = 0;

    char names[25];
    strncpy(names, "", 1);
    for (size_t i=0; i<entry_counter; i++) {
        strncat(names, nameslist[i], 10);
        if (i<entry_counter-1) {
            strncat(names, ",", 1);
        }
    }

    char results[25];
    strncpy(results, "", 1);
    for (size_t i=0; i<entry_counter; i++) {
        strncat(results, resultslist[i], 10);
        if (i<entry_counter-1) {
            strncat(results, ",", 1);
        }
    }

    senml_base_info_t base_info = {
        .version = SENML_SUPPORTED_VERSION,
        .base_name = dev_name,
    };
    senml_record_t records = {
        .name = names,
        .value_sum = 0,
        .value_type = SENML_TYPE_STRING,
        .value.s = results
    };
    senml_pack_t pack = {
        .base_info = &base_info,
        .num = 1,
        .records = &records
    };
    int8_t senml_res = senml_encode_json_s(&pack, json_buf, SENML_LEN);

    if (!senml_res) {

        entry_counter = 0;
        
        return 0;
    } else {
        return -1;
    }
}

ssize_t senml_json_send(coap_pkt_t* pdu, uint8_t *buf, size_t len, uint8_t dev_type) {
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);

    char* json_buf = malloc(SENML_LEN);
    int8_t senml_res = senml_json_strout(json_buf, dev_type);

    if (!senml_res) {
        printf("Successfully created SenML JSON string: %s\n", json_buf);

        size_t payload_len = snprintf(
            (char*)pdu->payload,
            GCOAP_PDU_BUF_SIZE,
            json_buf
        );

        free(json_buf);

        int8_t gcoap_res = gcoap_finish(pdu, payload_len, COAP_FORMAT_JSON);
        if (gcoap_res < 0) {
            puts("Failure sending message.");
        } else {
            printf("Message successfully sent with PDU size %d.\n", gcoap_res);
        }
        return gcoap_res;
    } else {
        puts("Failed to create full SenML JSON string. Message will not be sent.");
        free(json_buf);
        return -1;
    }
}

ssize_t entry_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len) {
    puts("Data from entrys requested.");
    return senml_json_send(pdu, buf, len, SAUL_CLASS_UNDEF);
}

void* ping_handler(void* args) {
    (void)args;
    sock_udp_ep_t remote = SOCK_IPV6_EP_ANY;
    remote.port = SERVER_CONN_PORT;
    ipv6_addr_from_str((ipv6_addr_t *)&remote.addr.ipv6, "ff02::1");
    
    puts("Ping thread running");
    
    while(true) {
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

const coap_resource_t coap_resources[] = {
    {"/se-app/entrys", COAP_GET, &entry_handler}
};

gcoap_listener_t coap_listener = {
    (coap_resource_t *)&coap_resources,
    sizeof(coap_resources) / sizeof(coap_resources[0]),
    NULL
};
