#include <stdio.h>
#include <stdlib.h>
#include "msg.h"
#include "shell.h"

#include "net/ipv6/addr.h"
#include "net/sock/udp.h"
#include "net/gnrc/ipv6/netif.h"
#include "saul.h"
#include "saul_reg.h"

#include "thread.h"
#include "net/gcoap.h"
#include "kernel_types.h"
#include "coap.h"
#define MAIN_QUEUE_SIZE (8)

static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

char coap_client_thread_stack[THREAD_STACKSIZE_DEFAULT];
char own_addr[IPV6_ADDR_MAX_STR_LEN];

extern int gcoap_cli_cmd(int argc, char **argv);

static char M0_ADDR[] = "ff02::1:a0:a0";
static char M1_ADDR[] = "ff02::1:b1:b1";

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

void set_name(char * name, int m) {
    if (m == 0){
        M0_name = name;
        M0_has_name = 1;
        printf("M0 Name: %s\n", name);
    }
    else {
        M1_name = name;
        M1_has_name = 1;
        printf("M1 Name: %s\n", name);
    }
}

void set_is_working(int m) {
    if (m == 0){
        M0_is_working = 1;
        printf("M0 Started\n");
    }
    else {
        M1_is_working = 1; 
        printf("M1 Started\n");
    }
}

void set_result(char * result, int m) {
    if (m == 0){
        M0_result = result;
        M0_rcvd_result = 1;
        printf("M0 Result = %s\n", result);
    }
    else {
        M1_result = result;
        M1_rcvd_result = 1;
        printf("M1 Result = %s\n", result); 
    }
}

void *thread_handler(void *arg) {
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
        }
        else {
            if (M0_has_name == 0 || M1_has_name == 0) {
                char * message1[] = {"coap", "get", M0_ADDR, "5683", "/request/name"};
                gcoap_cli_cmd(5, message1);
                char * message2[] = {"coap", "get", M1_ADDR, "5683", "/request/name"};
                gcoap_cli_cmd(5, message2);
                xtimer_sleep(2);
            }
            else {
                if (M0_is_working == 0 || M1_is_working == 0) {
                    char * message1[] = {"coap", "get", M0_ADDR, "5683", "/start/game"};
                    gcoap_cli_cmd(5, message1);
                    char * message2[] = {"coap", "get", M1_ADDR, "5683", "/start/game"};
                    gcoap_cli_cmd(5, message2);
                    xtimer_sleep(2);
                }
                else {
                    if (M0_rcvd_result == 0 || M1_rcvd_result == 0) {
                        restart_counter += 1;
                        char * message1[] = {"coap", "get", M0_ADDR, "5683", "/request/result"};
                        gcoap_cli_cmd(5, message1);
                        char * message2[] = {"coap", "get", M1_ADDR, "5683", "/request/result"};
                        gcoap_cli_cmd(5, message2);
                        xtimer_sleep(2);
                    }
                    else {
                        xtimer_sleep(3);
                        if (atol(M0_result) < atol(M1_result)) {
                            char * message1[] = {"coap", "get", M0_ADDR, "5683", "/set/winner"};
                            gcoap_cli_cmd(5, message1);
                            char * message2[] = {"coap", "get", M1_ADDR, "5683", "/set/looser"};
                            gcoap_cli_cmd(5, message2);
                        }
                        else if (atol(M0_result) > atol(M1_result)) {
                            char * message1[] = {"coap", "get", M0_ADDR, "5683", "/set/looser"};
                            gcoap_cli_cmd(5, message1);
                            char * message2[] = {"coap", "get", M1_ADDR, "5683", "/set/winner"};
                            gcoap_cli_cmd(5, message2);
                        }
                        else {
                            //send draw to both
                            //send result to database
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

int main(void) {

    /* for the thread running the shell */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    puts("gcoap client app fuer Bandgeraete Spiel");

    ipv6_addr_t addr;
    ipv6_addr_from_str(&addr, "fe80::");

    ipv6_addr_t* out = NULL;
    gnrc_ipv6_netif_find_by_prefix(&out, &addr);
    
    ipv6_addr_to_str(own_addr, out, IPV6_ADDR_MAX_STR_LEN);
    printf("own ipv6 ll-addr: %s\n", own_addr);

    gcoap_register_listener(&coap_listener);
    puts("coap ready");

    // thread für pings
    thread_create(ping_stack, THREAD_STACKSIZE_DEFAULT, THREAD_PRIORITY_MAIN - 1, 
        THREAD_CREATE_STACKTEST, ping_handler, NULL, "ping");

    thread_create(coap_client_thread_stack, sizeof(coap_client_thread_stack), THREAD_PRIORITY_MAIN - 2, 
        THREAD_CREATE_STACKTEST, thread_handler, NULL, "client_thread");

    puts("Main: Starting shell.");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
    puts("Main: Shell done.");

    return 0;
}
