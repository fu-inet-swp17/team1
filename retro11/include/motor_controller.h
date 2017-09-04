/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */
#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include "dcmotor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int16_t speed;
  uint32_t timeout;
  kernel_pid_t pid;
} motor_control_msg_t;

typedef struct {
  dcmotor_t *motor_a;
  dcmotor_t *motor_b;
  kernel_pid_t ctrl_pid;
  motor_control_msg_t param;
} motor_controller_t;

kernel_pid_t motor_controller_init(motor_controller_t *dev, dcmotor_t *motor_a, dcmotor_t *motor_b);

int motor_controller_set_speed(motor_controller_t *dev, int16_t speed, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_CONTROLLER_H */
/** @} */
