#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "periph/gpio.h"
#include "periph/pwm.h"
#include "xtimer.h"
#include "thread.h"
#include "shell.h"
#include "random.h"
#include "retro11_conf.h"
#include "dcmotor.h"
#include "multiplexer.h"
#include "motor_controller.h"
#include "neopixel.h"
#include "irq.h"

dcmotor_t motor_a, motor_b;
multiplexer_t multiplexer;
motor_controller_t motor_controller;
kernel_pid_t mainPid;
neopixel_t led_stripe;
bool enableBtns = false;

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

int refresh_leds_cmd(int argc, char **argv)
{
  neopixel_show(&led_stripe);
  return 0;
}

int set_led_cmd(int argc, char **argv){
  if(argc < 5) {
    puts("usage: set_led [led number] [red green blue]");
    return 1;
  }

  int led = atoi(argv[1]);
  color_rgb_t pixel;

  if(led > led_stripe.led_count){
    printf("invalid led number %d, smaller then led count %ld\n", led, led_stripe.led_count);
    puts("usage: set_led [led number] [red] [green] [blue]\n");
    return 1;
  }

  /* NOTE: If it overflows the user has to be more careful. */
  pixel.r =	atoi(argv[2]);
  pixel.g =	atoi(argv[3]);
  pixel.b = atoi(argv[4]);

  printf("r: 0x%x g: 0x%x, b: 0x%x for led %d/%ld\n", pixel.r, pixel.g, pixel.b, led, led_stripe.led_count);
  neopixel_set_pixel_color(&led_stripe, led, pixel);

  return 0;
}

int start_color_animation_cmd(int argc, char **argv)
{
  if (argc < 2) {
    printf("start_color_animation: start_color_animation [rate]\n");
    return 0;
  }

  int rate = atoi(argv[1]);

  for(int k = 0; k < led_stripe.led_count; ++k) {
    uint8_t *curr = &(led_stripe.pixels[k].r);
    *(curr + (k % 3)) = 255;
  }
  neopixel_show(&led_stripe);

  for (int i = 0;;++i) {

    color_rgb_t curr;
    for (int k = 0; k < led_stripe.led_count; ++k) {
      int next = (k + 1) % led_stripe.led_count;
      curr.r = led_stripe.pixels[next].r;
      curr.g = led_stripe.pixels[next].g;
      curr.b = led_stripe.pixels[next].b;
      led_stripe.pixels[next].r = led_stripe.pixels[k].r;
      led_stripe.pixels[next].g = led_stripe.pixels[k].g;
      led_stripe.pixels[next].b = led_stripe.pixels[k].b;
      led_stripe.pixels[k].r = curr.r;
      led_stripe.pixels[k].g = curr.g;
      led_stripe.pixels[k].b = curr.b;
    }

    xtimer_usleep(rate);
    neopixel_show(&led_stripe);
  }

  return 0;
}

int set_speed_cmd(int argc, char **argv)
{
  if (argc < 2) {
    puts("usage: set_speed [speed]");
    return 0;
  }

  // TODO: Safe atoi.
  int value = atoi(argv[1]);
  printf("setting speed to %d.\n", value);
  motor_controller_set_speed(&motor_controller, value, 0);

  return 0;
}

int read_button_cmd(int argc, char **argv)
{
  if (argc < 2) {
    puts("usage: read_button [address]");
    return 0;
  }

  // TODO: Safe atoi.
  int value = atoi(argv[1]);
  printf("Button %d Value %d\n", value, multiplexer_receive(&multiplexer, value));

  return 0;
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

static const shell_command_t shell_commands[] = {
    { "set_speed", "set speed for motor", set_speed_cmd },
    { "read_button", "read input of button", read_button_cmd },
    { "start_reaction_game", "start reaction game mode", start_reaction_game_cmd },
    { "set_led", "set led color", set_led_cmd },
    { "refresh_leds", "refresh all leds", refresh_leds_cmd },
    { "start_color_animation", "starts a sample color animation", start_color_animation_cmd },
    { NULL, NULL, NULL }
};

int main(void)
{
    int32_t act_freq;
    printf("Welcome to Retro11.");

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

    if (neopixel_init(&led_stripe, 40, CONF_LED_STRIPE) < 0){
     puts("Error initializing led stripe");
      return 0;
    }

    printf("Neopixel init done.\n");

    if (multiplexer_init_int(&multiplexer, CONF_MULTIPLEXER_RECV, CONF_MULTIPLEXER_ADR_A,
          CONF_MULTIPLEXER_ADR_B, CONF_MULTIPLEXER_ADR_C, &int_multiplexer_receive, NULL) < 0) {
      puts("Erro initializing multiplexer");
      return 0;
    }

    enableBtns = false;

    printf("Multiplexer is done.\n");

    /* TODO: We do not have a battery, so its kind of always the same. :) */
    random_init(xtimer_now_usec());

    /* Save PID to enable messaging later on. */
    mainPid = thread_getpid();

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    neopixel_free(&led_stripe);

    return 0;
}
