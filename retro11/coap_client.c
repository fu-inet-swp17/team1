#include "coap_client.h"

#include "net/gcoap.h"
#include "od.h"
#include "fmt.h"
#include "saul_reg.h"
#include "senml.h"
#include "net/ipv6/addr.h"
#include "net/gnrc/ipv6/netif.h"

/* Board addresses */
static char M0_ADDR[IPV6_ADDR_MAX_STR_LEN];
static char M1_ADDR[] = "ff02::1:b1:b1";

client_t client;

//arrays of entries for communication with smart-environment project
int entry_counter = 0;
char * nameslist[50];
int resultslist[50];

char coap_client_thread_stack[THREAD_STACKSIZE_DEFAULT];

//function to create senml packets for communication with smart-environment project
int8_t senml_json_strout(char* json_buf, uint8_t dev_type) {
  (void)dev_type;
  char *dev_name = "reaction_game";
  senml_record_t records[entry_counter];

  senml_base_info_t base_info = {
    .version = SENML_SUPPORTED_VERSION,
    .base_name = dev_name,
  };

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

//function to send json senml messages for communication with smart-environment project
ssize_t senml_json_send(coap_pkt_t* pdu, uint8_t *buf, size_t len, uint8_t dev_type) {

  gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);

  char* json_buf = malloc(SENML_LEN);
  int8_t senml_res = senml_json_strout(json_buf, dev_type);

  if (!senml_res) {
    printf("Successfully created SenML JSON string: %s\n", json_buf);
    size_t payload_len = snprintf((char*)pdu->payload, GCOAP_PDU_BUF_SIZE, json_buf);
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

//sets name state after message evaluation in response handler
void set_name(char * name, client_t m) {
  if (m == 0){
    client.M0_name = name;
    client.M0_state = HAS_NAME;
    printf("M0 Name: %s\n", name);
  } else {
    client.M1_name = name;
    client.M1_state = HAS_NAME;
    printf("M1 Name: %s\n", name);
  }
}

//sets working state after message evaluation in response handler
void set_is_working(int m) {
  if (m == 0){
    client.M0_state = IS_WORKING;
    printf("M0 Started\n");
  } else {
    client.M1_state = IS_WORKING;
    printf("M1 Started\n");
  }
}

//sets result state after message evaluation in response handler
void set_result(char * result, int m) {
  if (m == 0){
    client.M0_result = result;
    client.M0_state = RCVD_RESULT;
    printf("M0 Result = %s\n", result);
  } else {
    client.M1_result = result;
    client.M1_state = RCVD_RESULT;
    printf("M1 Result = %s\n", result);
  }
}

//response handler, handles coap responses
static void _resp_handler(unsigned req_state, coap_pkt_t* pdu, sock_udp_ep_t *remote) {

  (void)remote;

  if (req_state == GCOAP_MEMO_TIMEOUT) {
    //printf("gcoap: timeout for msg ID %02u\n", coap_get_id(pdu));
    return;
  }

  if (req_state == GCOAP_MEMO_ERR) {
    //printf("gcoap: error in response\n");
    return;
  }

  //char *class_str = (coap_get_code_class(pdu) == COAP_CLASS_SUCCESS) ? "Success" : "Error";
  //printf("gcoap: response %s, code %1u.%02u", class_str, coap_get_code_class(pdu), coap_get_code_detail(pdu));

  if (pdu->payload_len) {
    if (pdu->content_type == COAP_FORMAT_TEXT || pdu->content_type == COAP_FORMAT_LINK
            || coap_get_code_class(pdu) == COAP_CLASS_CLIENT_FAILURE
            || coap_get_code_class(pdu) == COAP_CLASS_SERVER_FAILURE) {

      /* Expecting diagnostic payload in failure cases */
      //printf(", %u bytes\nResponse from Server: %.*s\n", pdu->payload_len, pdu->payload_len, (char *)pdu->payload);

      char machine[4];
      int pldlen = pdu->payload_len-3;
      char pld[pldlen];

      strncpy(machine, (char *)pdu->payload, 3);
      machine[3]='\0';

      strncpy(pld, (char *)pdu->payload + 3, pldlen);
      pld [pldlen] = '\0';

      //evaulates received message of format MACHINE.SUBJECT.VALUE
      int m;
      if (strcmp(machine, "M0.") == 0) {
        m = 0;
      } else if (strcmp(machine, "M1.") == 0) {
        m = 1;
      } else { //leave message evaluation if machine is neither M0 or M1
        //printf("%.*s\n", pdu->payload_len, (char *)pdu->payload);
        return;
      }
      //if subject is st, set machine states to working
      if (strncmp(pld, "st", 2) == 0) {
        set_is_working(m);
      }
      //if subject is na, set name states according to value
      if (strncmp(pld, "na", 2) == 0) {
        char name[pldlen-3];
        strncpy(name, pld+3, pldlen-3);
        name [pldlen-3] = '\0';
        set_name(name,m);
      }
      //if subject is re, set result states according to value
      if (strncmp(pld, "re", 2) == 0) {
        char result[pldlen-3];
        strncpy(result, pld+3, pldlen-3);
        result [pldlen-3] = '\0';
        set_result(result,m);
      }
    } else {
      //if message has different format, print payload
      //printf(", %u bytes\n", pdu->payload_len);
      od_hex_dump(pdu->payload, pdu->payload_len, OD_WIDTH_DEFAULT);
    }
  } else {
    //printf(", empty payload\n");
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

void gcoap_req_cmd(char **argv) {
  uint8_t buf[GCOAP_PDU_BUF_SIZE];
  coap_pkt_t pdu;
  size_t len;

  len = gcoap_request(&pdu, &buf[0], GCOAP_PDU_BUF_SIZE, 1, argv[4]);
  //printf("gcoap_cli: sending msg ID %u, %u bytes\nRequested ressource: %s\n", coap_get_id(&pdu), (unsigned) len, argv[4]);

  _send(&buf[0], len, argv[2], argv[3]);
/*  if (!_send(&buf[0], len, argv[2], argv[3]))
    puts("gcoap_cli: msg send failed");*/
}

void reset_client_states(void) {
  client.M0_state = WAITING;
  client.M1_state = WAITING;
  client.M0_name = "";
  client.M1_name = "";
  client.M0_result = ""; 
  client.M1_result = "";
}

//adds the names and results of current game to their arrays
void set_entry_counter(void) {
  nameslist[entry_counter] = client.M0_name;
  nameslist[entry_counter+1] = client.M1_name;
  resultslist[entry_counter] = atoi(client.M0_result);
  resultslist[entry_counter+1] = atoi(client.M1_result);
  entry_counter += 2;

  if (entry_counter >= 48){
    entry_counter = 0;
  }
}

void *coap_client_thread_handler(void *arg) {
  (void) arg;

  while (1) {  
  if (client.M0_state == WAITING || client.M0_state == WAITING ) {
    xtimer_usleep(TIMER);
    char * message1[] = {"coap", "get", M0_ADDR, "5683", "/request/name"};
    char * message2[] = {"coap", "get", M1_ADDR, "5683", "/request/name"};
    gcoap_req_cmd(message1);
    gcoap_req_cmd(message2);
  }
  else if (client.M0_state == HAS_NAME || client.M1_state == HAS_NAME) {
    char * message1[] = {"coap", "get", M0_ADDR, "5683", "/start/game"};
    char * message2[] = {"coap", "get", M1_ADDR, "5683", "/start/game"};
    gcoap_req_cmd(message1);
    gcoap_req_cmd(message2);           
  }
  else if (client.M0_state == IS_WORKING || client.M1_state == IS_WORKING) {
    xtimer_usleep(TIMER);
    char * message1[] = {"coap", "get", M0_ADDR, "5683", "/request/result"};
    char * message2[] = {"coap", "get", M1_ADDR, "5683", "/request/result"};
    gcoap_req_cmd(message1);
    gcoap_req_cmd(message2);
  }
  else if (client.M0_state == RCVD_RESULT || client.M1_state == RCVD_RESULT) {
    if (atol(client.M0_result) < atol(client.M1_result)) {
      xtimer_usleep(TIMER);  
      char * message1[] = {"coap", "get", M0_ADDR, "5683", "/set/winner"};
      char * message2[] = {"coap", "get", M1_ADDR, "5683", "/set/looser"};
      gcoap_req_cmd(message1);
      gcoap_req_cmd(message2);
    } 
    else if (atol(client.M0_result) > atol(client.M1_result)) {
        xtimer_usleep(TIMER);  
        char * message1[] = {"coap", "get", M0_ADDR, "5683", "/set/looser"};
        char * message2[] = {"coap", "get", M1_ADDR, "5683", "/set/winner"};
        gcoap_req_cmd(message1);
        gcoap_req_cmd(message2);
      }
  printf("\n\nResetting machine status\n\n\n");

  set_entry_counter();
  reset_client_states();
  xtimer_usleep(TIMER);  
  }  
}

  return NULL;
}

kernel_pid_t coap_client_init(void) {

  //sets own local address for M0
  ipv6_addr_t addr;
  ipv6_addr_from_str(&addr, "fe80::");
  ipv6_addr_t* out = NULL;
  gnrc_ipv6_netif_find_by_prefix(&out, &addr);
  ipv6_addr_to_str(M0_ADDR, out, IPV6_ADDR_MAX_STR_LEN);

  reset_client_states();

  return thread_create(coap_client_thread_stack, sizeof(coap_client_thread_stack),
      THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
      coap_client_thread_handler, NULL, "coap_client thread");
}
