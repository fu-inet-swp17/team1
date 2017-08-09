#include <stdio.h>
#include "msg.h"

#include "thread.h"
#include "net/gcoap.h"
#include "kernel_types.h"
#include "shell.h"

#define MAIN_QUEUE_SIZE (4)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

char stack[THREAD_STACKSIZE_MAIN];

extern int gcoap_cli_cmd(int argc, char **argv);

static char M0_ADDR[] = "2001:db8::1:1:1";
//static char M1_ADDR[] = "fe80::44e8:fbff:fea3:bfcb";

static int M0_init = 0;
static int M1_init = 0;
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

void set_is_init(int m) {
    if (m == 0){
        M0_init = 1;
        printf("M0 Initalized\n");
    }
    else {
        M1_init = 1;
        printf("M1 Initalized\n");
    }
}

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
        if (M0_init == 0) {
            char * message[] = {"coap", "get", M0_ADDR, "5683", "/init/status"};
            gcoap_cli_cmd(5, message);
            xtimer_sleep(5);
        }
        else {
            if (M0_has_name == 0) {
                char * message[] = {"coap", "get", M0_ADDR, "5683", "/request/name"};
                gcoap_cli_cmd(5, message);
                xtimer_sleep(20);
            }
            else {
                if (M0_is_working == 0) {
                    char * message[] = {"coap", "get", M0_ADDR, "5683", "/start/game"};
                    gcoap_cli_cmd(5, message);
                    xtimer_sleep(5);
                }
                else {
                    if (M0_rcvd_result == 0) {
                        char * message[] = {"coap", "get", M0_ADDR, "5683", "/request/result"};
                        gcoap_cli_cmd(5, message);
                        xtimer_sleep(5);
                    }
                    else {
                        /*if (toFloat(M0_result) < toFloat(M1_result)) {
                            //send winner to M1 and looser to M2
                            //send result to Database
                        }
                        else if (toFloat(M0_result) > toFloat(M1_result)) {
                            //send winner to M1 and looser to M2
                            //send result to Database
                        }
                        else {
                            //send draw to both
                            //send result to database
                        }*/

                        printf("\n\nResetting machine status\n\n\n");
                        
                        M0_init = 0;
                        M1_init = 0;
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
                        xtimer_sleep(5);
                    }    
                }
            }
        }
    }
    return NULL;
}

int start_thread(int argc, char **argv) {
    (void)argv;
    (void)argc;

    thread_create(stack, sizeof(stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST, thread_handler, NULL, "main_thread");

    return 0;
}

static const shell_command_t shell_commands[] = {
    { "coap", "CoAP example", gcoap_cli_cmd },
    { "start_thread", "starts main thread", start_thread },
    { NULL, NULL, NULL }
};

int main(void) {

    /* for the thread running the shell */
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);
    puts("gcoap client app fuer Bandgeraete Spiel");

    /* start shell */
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);
    //thread_create(stack, sizeof(stack), THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST, thread_handler, NULL, "main_thread");

    return 0;
}
