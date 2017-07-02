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

dcmotor_t motor_a, motor_b;
multiplexer_t multiplexer;
kernel_pid_t mainPid, motorPid;
bool enableBtns = false;

char motor_ctrl_thread_stack[THREAD_STACKSIZE_MAIN];
void *motor_ctrl_thread_handler(void *arg)
{
  (void) arg;
  msg_t m;

  while (1) {
    msg_receive(&m);

    switch(m.type) {
      case 0:
        printf("Setting speed to %ld.\n", m.content.value);
        dcmotor_set_speed(&motor_a, m.content.value);
        dcmotor_set_speed(&motor_b, m.content.value);
        break;
      default:
        printf("Setting speed to %ld with timeout %dms.\n", m.content.value, m.type);
        dcmotor_set_speed(&motor_a, m.content.value);
        dcmotor_set_speed(&motor_b, m.content.value);

        xtimer_usleep(m.type);

        dcmotor_set_speed(&motor_a, 0);
        dcmotor_set_speed(&motor_b, 0);
        msg_send(&m, mainPid);
        break;
    }
  }
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

int set_speed_cmd(int argc, char **argv)
{
  if (argc < 2) {
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
  uint32_t start_time;
  uint16_t motor_timeout;

  /* Set Multiplexer to start button, enable interrupt */
  multiplexer_receive(&multiplexer, 0);
  multiplexer_int_enable(&multiplexer);
  enableBtns = true;

  puts("Waiting 10sec for player to press start button.");

  /* Wait for 10 seconds */
  if (xtimer_msg_receive_timeout(&msg, 10000000) == -1) {
    puts("Timeout while waiting for start button.");
    return 1;
  }

  multiplexer_receive(&multiplexer, 0);
  motor_timeout = (uint16_t) random_uint32_range(0, 65535);
  msg.type = motor_timeout;
  msg.content.value = 200;

  printf("Starting game with timeout %dms.", motor_timeout);

  msg_send(&msg, motorPid);


  while (1) {
    if (xtimer_msg_receive_timeout(&msg, motor_timeout + 10000000) == -1) {
      puts("Something went wrong, the motor did not react.");
      break;
    }

    printf("Received: Pid: %d Type: %d\n", msg.sender_pid, msg.type);

    if (msg.sender_pid == motorPid) {
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

    motorPid = thread_create(motor_ctrl_thread_stack, sizeof(motor_ctrl_thread_stack),
        THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
        motor_ctrl_thread_handler, NULL, "motor ctrl thread");

    printf("Motor init done.\n");

    if (multiplexer_init_int(&multiplexer, CONF_MULTIPLEXER_RECV, CONF_MULTIPLEXER_ADR_A,
          CONF_MULTIPLEXER_ADR_B, CONF_MULTIPLEXER_ADR_C, &int_multiplexer_receive, NULL) < 0) {
      puts("Erro initializing multiplexer");
      return 0;
    }
    multiplexer_int_disable(&multiplexer);

    printf("Multiplexer is done.\n");

    /* We do not have a battery, so its kind of always the same. :) */
    random_init(xtimer_now_usec());

    /* Save PID to enable messaging later on. */
    mainPid = thread_getpid();

    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    return 0;
}
