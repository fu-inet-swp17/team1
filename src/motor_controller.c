/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

#include "motor_controller.h"
#define ENABLE_DEBUG (0)
#include "debug.h"

#include "thread.h"
#include "xtimer.h"

char motor_ctrl_thread_stack[THREAD_STACKSIZE_DEFAULT - 512];
void *motor_ctrl_thread_handler(void *arg)
{
  (void) arg;
  msg_t m;
  motor_controller_t *dev;

  while (1) {
    msg_receive(&m);

    dev = m.content.ptr;
    printf("Setting speed to %d with timeout %ldms.\n", dev->param.speed, dev->param.timeout);

    dcmotor_set_speed(dev->motor_a, dev->param.speed);
    dcmotor_set_speed(dev->motor_b, dev->param.speed);

    if (dev->param.timeout > 0) {
      xtimer_usleep(dev->param.timeout);

      dcmotor_set_speed(dev->motor_a, 0);
      dcmotor_set_speed(dev->motor_b, 0);

      m.type = 0;
      m.content.value = dev->param.timeout;
      msg_send(&m, dev->param.pid);
    }
  }
}

kernel_pid_t motor_controller_init(motor_controller_t *dev, dcmotor_t *motor_a, dcmotor_t *motor_b) {
  dev->ctrl_pid = thread_create(motor_ctrl_thread_stack, sizeof(motor_ctrl_thread_stack),
        THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST,
        motor_ctrl_thread_handler, NULL, "motor ctrl thread");

  if (dev->ctrl_pid < 0)
    return -1;

  dev->motor_a = motor_a;
  dev->motor_b = motor_b;

  return dev->ctrl_pid;
}

int motor_controller_set_speed(motor_controller_t *dev, int16_t speed, uint32_t timeout)
{
  msg_t msg;

  dev->param.speed = speed;
  dev->param.timeout = timeout;
  dev->param.pid = thread_getpid();

  msg.type = 0;
  msg.content.ptr = dev;

  if (msg_send(&msg, dev->ctrl_pid) == 0)
    return -1;

  return 0;
}
