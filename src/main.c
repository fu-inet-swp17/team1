#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "periph/gpio.h"
#include "periph/pwm.h"
#include "xtimer.h"
#include "thread.h"
#include "random.h"
#include "retro11_conf.h"
#include "dcmotor.h"
#include "multiplexer.h"
#include "motor_controller.h"
#include "game.h"
#include "neopixel.h"
#include "irq.h"
#include "lcd_spi.h"

//coap Server
#include "msg.h"
#include "shell.h"
#include "net/fib.h"
#include "net/gnrc/ipv6.h"
#include "net/gnrc/ipv6/nc.h"
#include "net/gnrc/ipv6/netif.h"
#include "net/gnrc/netapi.h"
#include "net/gnrc/netif.h"
#include "net/ipv6/addr.h"
#include "net/netdev.h"
#include "net/netopt.h"

#define MAIN_QUEUE_SIZE     (8)
static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

dcmotor_t motor_a, motor_b;
multiplexer_t multiplexer;
motor_controller_t motor_controller;
lcd_spi_t display;
kernel_pid_t gamePid;
neopixel_t led_stripe;
game_t game;
volatile bool enableBtns = false;
char coap_server_thread_stack[THREAD_STACKSIZE_DEFAULT + 1024];
char game_server_thread_stack[THREAD_STACKSIZE_DEFAULT];

void microcoap_server_loop(void);
extern int _netif_config(int argc, char **argv);

void *coap_server_thread_handler(void *arg)
{
  puts("Starting network server.");
  microcoap_server_loop();
  puts("NetworkServer done.");

  return NULL;
}

void *game_server_thread_handler(void *arg)
{
  game_run(&game);

  return NULL;
}

void get_name_of_player(char *value) {
  if (game.state != NAME_READY)
    return;

  sprintf(value, "M0.na.%s", game.playername);
  return;
}

char * start_game(void) {
  game.state = START_GAME;
  return("M0.st");
}

void get_result(char *value) {
  if (game.state != GET_RESULT)
    return;

  sprintf(value, "M0.re.%ld", game.reaction_time);
  return;
}

void set_winner(void)
{
  puts("winner");
  game.state = WINNER;
  return;
}

void set_looser(void)
{
  puts("looser");
  game.state = LOOSER;
}

void int_multiplexer_receive(void *arg)
{
  if (!enableBtns)
    return;

  msg_t msg;

  msg.type = multiplexer.curr_addr;
  msg.content.value = 1;
  msg_send_int(&msg, gamePid);
  puts("Done sending message.");
}

static const shell_command_t shell_commands[] = {
  {NULL, NULL, NULL}
};

int main(void)
{
    int32_t act_freq;
    color_rgb_t curr_color;
    printf("Welcome to Retro11.");

    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    kernel_pid_t dev;
    kernel_pid_t ifs[GNRC_NETIF_NUMOF];
    size_t numof = gnrc_netif_get(ifs);


    dev = ifs[numof-1];

    ipv6_addr_t addr;
    ipv6_addr_from_str(&addr, "ff02::1:a0:a0");
    gnrc_ipv6_netif_add_addr( dev, &addr, 64 , GNRC_IPV6_NETIF_ADDR_FLAGS_UNICAST );

    act_freq = pwm_init(CONF_MOTOR_PWM, CONF_MOTOR_A_PWM_CHAN, CONF_MOTOR_FREQ, CONF_MOTOR_RES);
    if (act_freq <= 0) {
      puts("Error initializing PWM.");
      return 0;
    }

    if (dcmotor_init(&motor_a, CONF_MOTOR_PWM, CONF_MOTOR_A_PWM_CHAN, act_freq,
          CONF_MOTOR_RES, CONF_MOTOR_A_DIRA, CONF_MOTOR_A_DIRB) < 0) {
      puts("Error initializing motor a");
      return 0;
    }

    if (dcmotor_init(&motor_b, CONF_MOTOR_PWM, CONF_MOTOR_B_PWM_CHAN, act_freq,
          CONF_MOTOR_RES, CONF_MOTOR_B_DIRA, CONF_MOTOR_B_DIRB) < 0) {
      puts("Error initializing motor b");
      return 0;
    }

    if(motor_controller_init(&motor_controller, &motor_a, &motor_b) < 0) {
      puts("Error initializing motor controller");
      return 0;
    }

    printf("Motor init done.\n");

    if (neopixel_init(&led_stripe, CONF_LED_COUNT, CONF_LED_STRIPE) < 0){
     puts("Error initializing led stripe");
      return 0;
    }

    printf("Neopixel init done.\n");

    if (multiplexer_init_int(&multiplexer, CONF_MULTIPLEXER_RECV, CONF_MULTIPLEXER_ADR_A,
          CONF_MULTIPLEXER_ADR_B, CONF_MULTIPLEXER_ADR_C, &int_multiplexer_receive, NULL) < 0) {
      puts("Error initializing multiplexer");
      return 0;
    }

    enableBtns = false;

    printf("Multiplexer is done.\n");

    puts("Start init display.");

    if (lcd_spi_init(&display, CONF_DISPLAY_SPI, CONF_DISPLAY_CS, CONF_DISPLAY_CMD, CONF_DISPLAY_RESET) < 0) {
      puts("Error initializing display.");
      return 0;
    }

    curr_color.r = CONF_DISPLAY_BRIGHTNESS;
    curr_color.g = CONF_DISPLAY_BRIGHTNESS;
    curr_color.b = CONF_DISPLAY_BRIGHTNESS;
    neopixel_set_pixel_color(&led_stripe, CONF_DISPLAY_LED1, curr_color);
    neopixel_set_pixel_color(&led_stripe, CONF_DISPLAY_LED2, curr_color);
    neopixel_show(&led_stripe);

    lcd_spi_set_contrast(&display, 25);
    lcd_spi_set_display_normal(&display, false);
    lcd_spi_clear(&display);
    lcd_spi_show(&display);
    printf("Display is done.\n");
    /* TODO: We do not have a battery, so its always the same. :) */
    random_init(xtimer_now_usec());

    thread_create(coap_server_thread_stack,
        sizeof(coap_server_thread_stack),
        THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
        coap_server_thread_handler, NULL, "coap thread");

    game_init(&game, &motor_controller, &display, &multiplexer, &led_stripe);

    gamePid = thread_create(game_server_thread_stack,
        sizeof(game_server_thread_stack),
        THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
        game_server_thread_handler, NULL, "game thread");

    puts("Main: Starting shell.");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
    puts("Main: Shell done.");

    return 0;
}
