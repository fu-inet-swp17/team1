#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "xtimer.h"
#include "shell.h"
#include "retro11_conf.h"
#include "periph/gpio.h"
#include "periph/pwm.h"

void set_speed_a(int16_t speed)
{
    if (speed > 0) {
        gpio_set(CONF_MOTOR_A_DIRA);
        gpio_clear(CONF_MOTOR_A_DIRB);
    } else {
        gpio_clear(CONF_MOTOR_A_DIRA);
        gpio_set(CONF_MOTOR_A_DIRB);
        speed *= -1;
    }
    pwm_set(CONF_MOTOR_A_PWM, CONF_MOTOR_A_PWM_CHAN, speed);
}

void set_speed_b(int16_t speed)
{
    if (speed > 0) {
        gpio_set(CONF_MOTOR_B_DIRA);
        gpio_clear(CONF_MOTOR_B_DIRB);
    } else {
        gpio_clear(CONF_MOTOR_B_DIRA);
        gpio_set(CONF_MOTOR_B_DIRB);
        speed *= -1;
    }
    pwm_set(CONF_MOTOR_B_PWM, CONF_MOTOR_B_PWM_CHAN, speed);
}

int set_speed_cmd(int argc, char **argv)
{
  if(argc < 2) {
    puts("usage: set_speed [speed]");
    return 0;
  }
  // TODO: Safe atoi.
  int value = atoi(argv[1]);
  printf("setting speed to %d.\n", value);
  set_speed_a(value);
  set_speed_b(value);
  return 0;
}

static const shell_command_t shell_commands[] = {
    { "set_speed", "set speed for motor", set_speed_cmd },
    { NULL, NULL, NULL }
};

int main(void)
{
    printf("First DC Motor trials new.\n");

    gpio_init(CONF_MOTOR_A_DIRA, GPIO_OUT);
    gpio_init(CONF_MOTOR_A_DIRB, GPIO_OUT);
    gpio_init(CONF_MOTOR_B_DIRA, GPIO_OUT);
    gpio_init(CONF_MOTOR_B_DIRB, GPIO_OUT);
    printf("GPIO Init done.\n");

    if (pwm_init(CONF_MOTOR_A_PWM, CONF_MOTOR_A_PWM_CHAN,
                 CONF_MOTOR_A_FREQ, CONF_MOTOR_A_RES) < 0) {
        puts("ERROR initializing the DRIVE PWM\n");
        return 1;
    }
    pwm_set(CONF_MOTOR_A_PWM, CONF_MOTOR_A_PWM_CHAN, 0);
    pwm_set(CONF_MOTOR_B_PWM, CONF_MOTOR_B_PWM_CHAN, 0);
    printf("PWM Init done.\n");

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
