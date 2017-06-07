/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    driver_servo Servo Motor Driver
 * @ingroup     drivers_actuators
 * @brief       High-level driver for servo motors
 * @{
 *
 * @file
 * @brief       High-level driver for easy handling of dc motors
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Joakim Nohlgård <joakim.nohlgard@eistec.se>
 */

#ifndef DCMOTOR_H
#define DCMOTOR_H

#include "periph/pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Descriptor struct for a servo
 */
typedef struct {
    pwm_t pwm;           /**< the PWM device driving the servo */
    int channel;            /**< the channel the servo is connected to */
    unsigned int freq;      /**< frequency of PWM signal */
    unsigned int res;       /**< resolution of PWM signal */
    unsigned int dir_a;     /**< first pin to set direction */
    unsigned int dir_b;     /**< second pin to set direction */
} dcmotor_t;

/**
 * @brief Initialize a dc motor by assigning it a PWM device and channel
 *
 * @param[out] dev          struct describing the servo
 * @param[in] pwm           the PWM device the servo is connected to
 * @param[in] channel   the PWM channel the servo is connected to
 * @param[in] freq          max pwm frequency
 * @param[in] res           resolution to set frequency
 * @param[in] dir_a         direction a gpio pin
 * @param[in] dir_b         direction b gpio pin
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
int dcmotor_init(dcmotor_t *dev, pwm_t pwm, int pwm_channel, unsigned int freq,
    unsigned int res, unsigned int dir_a, unsigned int dir_b);

/**
 * @brief Set the dc motor to a specified speed
 *
 * @param[in] dev           the dcmotor to set
 * @param[in] speed         the speed to set the dcmotor to (in the resolution range)
 */
void dcmotor_set_speed(dcmotor_t *dev, int speed);

#ifdef __cplusplus
}
#endif

#endif /* DCMOTOR_H */
/** @} */
