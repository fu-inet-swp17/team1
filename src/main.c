#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "periph/gpio.h"
#include "periph/pwm.h"
#include "shell.h"
#include "retro11_conf.h"
#include "dcmotor.h"

dcmotor_t motor_a, motor_b;

int set_speed_cmd(int argc, char **argv)
{
  if(argc < 2) {
    puts("usage: set_speed [speed]");
    return 0;
  }

  // TODO: Safe atoi.
  int value = atoi(argv[1]);
  printf("setting speed to %d.\n", value);
  dcmotor_set_speed(&motor_a, value);
  dcmotor_set_speed(&motor_b, value);

  return 0;
}

static const shell_command_t shell_commands[] = {
    { "set_speed", "set speed for motor", set_speed_cmd },
    { NULL, NULL, NULL }
};

int main(void)
{
    int32_t act_freq;
    printf("Second DC Motor trials.\n");

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

    printf("Motor init done.\n");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
