#include <stdio.h>
#include "msg.h"
#include "xtimer.h"
#include "shell.h"

void microcoap_server_loop(void);

/* import "ifconfig" shell command, used for printing addresses */
extern int _netif_config(int argc, char **argv);

char * get_name_of_player(void) {
    puts("Please insert name");
    //if error with name
        //request other name
    //else
    return("M0.na.Musterino");
}

char * start_game(void) {
    //if start successful
        return("M0.st");
    //else 
        //try to start game again
}

char * is_initalized(void) {
    //if initalized
        puts("is initalized?");
        return("M0.in");
    //else
        //return("not initalized");
}

char * get_result(void) {
    //if has result
        return("M0.re.0.123456");
    //else return
        //return("still playing")
}

int start_server(int argc, char **argv) {
    (void)argv;
    (void)argc;

    /* start coap server loop */
    puts("Starting microcoap server");
    microcoap_server_loop();

    return 0;
}

static const shell_command_t commands[] = {
    {"start_server", "starts the server", start_server},
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("Waiting for address autoconfiguration...");
    xtimer_sleep(3);

    /* print network addresses */
    puts("Configured network interfaces:");
    _netif_config(0, NULL);

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should be never reached */
    return 0;
}
