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
#include "neopixel.h"
#include "irq.h"
#include "lcd_spi.h"
#include "menu.h"

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
kernel_pid_t mainPid;
neopixel_t led_stripe;
menu_t menu;
bool enableBtns = false;

void microcoap_server_loop(void);
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

void int_multiplexer_receive(void *arg)
{
  if (!enableBtns)
    return;

  msg_t msg;

  printf("Interrupt: address %d value %d\n", multiplexer.curr_addr, 1);

  msg.type = multiplexer.curr_addr;
  msg.content.value = 1;
  msg_send_int(&msg, mainPid);
}

int start_reaction_game_cmd(int argc, char **argv)
{
  (void) argc;
  (void) argv;

  msg_t msg;
  uint32_t start_time, motor_timeout;

  /* Set Multiplexer to start button, enable interrupt */
  multiplexer_receive(&multiplexer, 0);
  enableBtns = true;

  puts("Waiting 10sec for player to press start button.");

  /* Wait for 10 seconds */
  if (xtimer_msg_receive_timeout(&msg, 10000000) == -1) {
    puts("Timeout while waiting for start button.");
    return 1;
  }

  motor_timeout = random_uint32_range(2000000, 20000000);
  multiplexer_receive(&multiplexer, 1);
  enableBtns = false;

  printf("Starting game with timeout %ldms.", motor_timeout);

  motor_controller_set_speed(&motor_controller, 200, motor_timeout);

  while (1) {
    if (xtimer_msg_receive_timeout(&msg, motor_timeout + 10000000) == -1) {
      puts("Something went wrong, the motor did not react.");
      break;
    }

    if (msg.sender_pid == motor_controller.ctrl_pid) {
      enableBtns = true;
      start_time = xtimer_now_usec();

      if (xtimer_msg_receive_timeout(&msg, 100000000) == -1) {
        puts("Player did not react in time.");
        return 1;
      }

      printf("Player reacted in %ldms.\n", xtimer_now_usec() - start_time);
      break;
    }
  }
  return 0;
}

int start_game_cmd(void)
{
  /* Announce to server */

  /* Wait for player to join */

  /* Start game */

  /* Input name if winner for highscore */

  /* Send to server */


  return 0;
}

int highscore_cmd(void)
{
  /* Get list of names from server */

  /* Print list on LCD */

  return 0;
}

int start_server(int argc, char **argv) {
    (void)argv;
    (void)argc;

    /* start coap server loop */
    puts("Starting microcoap server");
    microcoap_server_loop();

    return 0;
}

static const menu_command_t menu_commands[] = {
  {"START GAME", "start game", start_game_cmd},
  {"HIGHSCORE", "highscore", highscore_cmd},
  {"start_server", "starts the server", start_server},
  {NULL, NULL, NULL}
};

int main(void)
{
    int32_t act_freq;
    color_rgb_t curr_color;
    printf("Welcome to Retro11.");

    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

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
    /* Draw top and  bottom borders */
    lcd_spi_draw_line(&display, 0, 0, 127, 0);
    lcd_spi_draw_line(&display, 0, 1, 127, 1);
    lcd_spi_draw_line(&display, 0, 63, 127, 63);
    lcd_spi_draw_line(&display, 0, 62, 127, 62);

    lcd_spi_draw_s(&display, 80, 40, "Retro11", 7);

    lcd_spi_show(&display);

    printf("Display is done.\n");

    /* TODO: We do not have a battery, so its always the same. :) */
    random_init(xtimer_now_usec());

    /* Save PID to enable messaging later on. */
    mainPid = thread_getpid();

    menu_init(&menu, &multiplexer, &display, &led_stripe, &motor_controller, menu_commands, 2);
    menu_run(&menu);

    return 0;
}
