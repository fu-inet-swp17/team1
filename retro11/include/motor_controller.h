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
/**
 * @brief Message struct for IPC
 */
typedef struct {
  int16_t speed;
  uint32_t timeout;
  kernel_pid_t pid;
} motor_control_msg_t;

/**
 * @brief Descriptor struct for motor controller thread
 */
typedef struct {
  dcmotor_t *motor_a;
  dcmotor_t *motor_b;
  kernel_pid_t ctrl_pid;
  motor_control_msg_t param;
} motor_controller_t;

/**
 * @brief Initialize motor controller thread
 *
 * @param[out] dev          struct describing the motor controller thread
 * @param[in] motor_a       motor_a to control
 * @param[in] motor_b       motor_b to control
 * @param[in] ctrl_pid      pid of executing thread
 * @param[in] param         last received message
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
kernel_pid_t motor_controller_init(motor_controller_t *dev, dcmotor_t *motor_a, dcmotor_t *motor_b);

/**
 * @brief Set speed of controlling motors
 *
 * @param[out] dev          struct describing the motor controller thread
 * @param[in] speed         speed for the motors
 * @param[in] timeout       time until stopping motors
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
int motor_controller_set_speed(motor_controller_t *dev, int16_t speed, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_CONTROLLER_H */
/** @} */
