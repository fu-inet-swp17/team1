#include "coap_client.h"

#include "net/gcoap.h"
#include "od.h"
#include "fmt.h"
#include "saul_reg.h"
#include "senml.h"
#include "smart_environment.h"

/* Board addresses */
static char M0_ADDR[] = "ff02::1:a0:a0";
static char M1_ADDR[] = "ff02::1:b1:b1";

/* Board variables */
static char * M0_name = "";
static char * M1_name = "";
static int M0_has_name = 0;
static int M1_has_name = 0;
static int M0_is_working = 0;
static int M1_is_working = 0;
static char * M0_result = "";
static char * M1_result = "";
static int M0_rcvd_result = 0;
static int M1_rcvd_result = 0;
static int restart_counter = 0;

int entry_counter = 0;
char * nameslist[50];
int resultslist[50];

int8_t senml_json_strout(char* json_buf, uint8_t dev_type)
{
  (void)dev_type;

  char *dev_name = "reaction_game";

  senml_base_info_t base_info = {
    .version = SENML_SUPPORTED_VERSION,
    .base_name = dev_name,
  };

  senml_record_t records[entry_counter];

  for (int i = 0; i < entry_counter; i++) {
    records[i].name = nameslist[i];
    records[i].value_type = SENML_TYPE_INT;
    records[i].value.f = resultslist[i];
  }

  senml_pack_t pack = {
    .base_info = &base_info,
    .num = entry_counter,
    .records = records
  };

  if(senml_encode_json_s(&pack, json_buf, SENML_LEN))
    return -1;

  entry_counter = 0;
  return 0;
}

ssize_t senml_json_send(coap_pkt_t* pdu, uint8_t *buf, size_t len, uint8_t dev_type)
{
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
  return senml_json_send(pdu, buf, len, SAUL_CLASS_UNDEF);
}

const coap_resource_t coap_resources[] = {
    {"/se-app/entries", COAP_GET, &entry_handler}
};

gcoap_listener_t coap_listener = {
    (coap_resource_t *)&coap_resources,
    sizeof(coap_resources) / sizeof(coap_resources[0]),
    NULL
};

char coap_client_thread_stack[THREAD_STACKSIZE_DEFAULT];

void set_name(char * name, int m) {
  if (m == 0){
    M0_name = name;
    M0_has_name = 1;
    printf("M0 Name: %s\n", name);
  } else {
    M1_name = name;
    M1_has_name = 1;
    printf("M1 Name: %s\n", name);
  }
}

void set_is_working(int m) {
  if (m == 0){
    M0_is_working = 1;
    printf("M0 Started\n");
  } else {
    M1_is_working = 1;
    printf("M1 Started\n");
  }
}

void set_result(char * result, int m) {
  if (m == 0){
    M0_result = result;
    M0_rcvd_result = 1;
    printf("M0 Result = %s\n", result);
  } else {
    M1_result = result;
    M1_rcvd_result = 1;
    printf("M1 Result = %s\n", result);
  }
}

/*
 * Response callback.
 */
static void _resp_handler(unsigned req_state, coap_pkt_t* pdu, sock_udp_ep_t *remote)
{
  (void)remote;       /* not interested in the source currently */

  if (req_state == GCOAP_MEMO_TIMEOUT) {
    printf("gcoap: timeout for msg ID %02u\n", coap_get_id(pdu));
    return;
  }

  if (req_state == GCOAP_MEMO_ERR) {
    printf("gcoap: error in response\n");
    return;
  }

  char *class_str = (coap_get_code_class(pdu) == COAP_CLASS_SUCCESS) ? "Success" : "Error";

  printf("gcoap: response %s, code %1u.%02u", class_str, coap_get_code_class(pdu), coap_get_code_detail(pdu));

  if (pdu->payload_len) {
    if (pdu->content_type == COAP_FORMAT_TEXT || pdu->content_type == COAP_FORMAT_LINK
            || coap_get_code_class(pdu) == COAP_CLASS_CLIENT_FAILURE
            || coap_get_code_class(pdu) == COAP_CLASS_SERVER_FAILURE) {

      /* Expecting diagnostic payload in failure cases */
      printf(", %u bytes\nResponse from Server: %.*s\n", pdu->payload_len, pdu->payload_len, (char *)pdu->payload);

      char machine[4];
      int pldlen = pdu->payload_len-3;
      char pld[pldlen];

      strncpy(machine, (char *)pdu->payload, 3);
      machine[3]='\0';

      strncpy(pld, (char *)pdu->payload + 3, pldlen);
      pld [pldlen] = '\0';

      int m;
      if (strcmp(machine, "M0.") == 0) {
        m = 0;
      } else if (strcmp(machine, "M1.") == 0) {
        m = 1;
      } else {
        printf("%.*s\n", pdu->payload_len, (char *)pdu->payload);
        return;
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
    } else {
      printf(", %u bytes\n", pdu->payload_len);
      od_hex_dump(pdu->payload, pdu->payload_len, OD_WIDTH_DEFAULT);
    }
  } else {
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

  if (argc == 1)
    return 1;

  for (size_t i = 0; i < sizeof(method_codes) / sizeof(char*); i++) {
    if (strcmp(argv[1], method_codes[i]) == 0) {
      len = gcoap_request(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, i+1, argv[4]);
      printf("gcoap_cli: sending msg ID %u, %u bytes\nRequested ressource: %s\n", coap_get_id(&pdu), (unsigned) len, argv[4]);

      if (!_send(&buf[0], len, argv[2], argv[3]))
          puts("gcoap_cli: msg send failed");

      return 0;
    }
  }

  return 1;
}

void *coap_client_thread_handler(void *arg) {
  (void) arg;

  while (1) {
    if (restart_counter > 40) {
      printf("\n\nNo answer received, resetting machine status\n\n\n");

      M0_name = "";
      M1_name = "";
      M0_has_name = 0;
      M1_has_name = 0;
      M0_is_working = 0;
      M1_is_working = 0;
      M0_result = "";
      M1_result = "";
      M0_rcvd_result = 0;
      M1_rcvd_result = 0;
      restart_counter = 0;
      xtimer_sleep(4);
    } else {
      if (M0_has_name == 0 || M1_has_name == 0) {
        char * message1[] = {"coap", "get", M0_ADDR, "5683", "/request/name"};
        gcoap_cli_cmd(5, message1);
        char * message2[] = {"coap", "get", M1_ADDR, "5683", "/request/name"};
        gcoap_cli_cmd(5, message2);
        xtimer_sleep(2);
      } else {
        if (M0_is_working == 0 || M1_is_working == 0) {
          char * message1[] = {"coap", "get", M0_ADDR, "5683", "/start/game"};
          gcoap_cli_cmd(5, message1);
          char * message2[] = {"coap", "get", M1_ADDR, "5683", "/start/game"};
          gcoap_cli_cmd(5, message2);
          xtimer_sleep(2);
        } else {
          if (M0_rcvd_result == 0 || M1_rcvd_result == 0) {
            restart_counter += 1;
            char * message1[] = {"coap", "get", M0_ADDR, "5683", "/request/result"};
            gcoap_cli_cmd(5, message1);
            char * message2[] = {"coap", "get", M1_ADDR, "5683", "/request/result"};
            gcoap_cli_cmd(5, message2);
            xtimer_sleep(2);
          } else {
            xtimer_sleep(3);
            if (atol(M0_result) < atol(M1_result)) {
              char * message1[] = {"coap", "get", M0_ADDR, "5683", "/set/winner"};
              gcoap_cli_cmd(5, message1);
              char * message2[] = {"coap", "get", M1_ADDR, "5683", "/set/looser"};
              gcoap_cli_cmd(5, message2);
            } else if (atol(M0_result) > atol(M1_result)) {
              char * message1[] = {"coap", "get", M0_ADDR, "5683", "/set/looser"};
              gcoap_cli_cmd(5, message1);
              char * message2[] = {"coap", "get", M1_ADDR, "5683", "/set/winner"};
              gcoap_cli_cmd(5, message2);
            }
            printf("\n\nResetting machine status\n\n\n");

            nameslist[entry_counter] = M0_name;
            nameslist[entry_counter+1] = M1_name;
            resultslist[entry_counter] = atoi(M0_result);
            resultslist[entry_counter+1] = atoi(M1_result);
            entry_counter += 2;

            if (entry_counter >= 48){
                entry_counter = 0;
            }

            M0_name = "";
            M1_name = "";
            M0_has_name = 0;
            M1_has_name = 0;
            M0_is_working = 0;
            M1_is_working = 0;
            M0_result = "";
            M1_result = "";
            M0_rcvd_result = 0;
            M1_rcvd_result = 0;
            restart_counter = 0;
            xtimer_sleep(4);
          }
        }
      }
    }
  }

  return NULL;
}


void coap_client_init(void)
{
  gcoap_register_listener(&coap_listener);
}

kernel_pid_t coap_client_run(void)
{
  return thread_create(coap_client_thread_stack, sizeof(coap_client_thread_stack),
      THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
      coap_client_thread_handler, NULL, "coap_client thread");
}