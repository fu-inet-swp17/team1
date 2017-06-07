/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     driver_dcmotor
 * @{
 *
 * @file
 * @brief       DC motor driver implementation
 *
 * @author      Steve Dierker <dierker.steve@fu-berlin.de>
 *
 * @}
 */

#include "dcmotor.h"
#include "periph/pwm.h"
#include "timex.h" /* for US_PER_SEC */

#define ENABLE_DEBUG    (0)
#include "debug.h"

int dcmotor_init(dcmotor_t *dev, pwm_t pwm, int channel, unsigned int freq, unsigned int res,
    unsigned int dir_a, unsigned int dir_b)
{
    int actual_frequency;
    actual_frequency = pwm_init(pwm, channel, freq, res);

    if (actual_frequency < 0) {
      DEBUG("dcmotor_init: error initializing pwm %d.", pwm_t);
      return -1;
    }

    pwm_set(pwm, channel, 0);

    gpio_init(dir_a, GPIO_OUT);
    gpio_init(dir_b, GPIO_OUT);

    dev->pwm = pwm;
    dev->channel = channel;
    dev->frequency = freq;
    dev->res = res;
    dev->dir_a = dir_a;
    dev->dir_b = dir_b;

    return 0;
}

void dcmotor_set_speed(dcmotor_t *dev, int speed)
{
    if (speed == 0) {
      gpio_set(dev->dir_a);
      gpio_set(dev->dir_b);
    } else if (speed > 0) {
      speed = (speed >= dev->freq) ? (dev->freq) : (speed);
      gpio_set(dev->dir_a);
      gpio_clear(dev->dir_b);
    } else {
      speed *= -1;
      speed = (speed >= dev->freq) ? (dev->freq) : (speed);
      gpio_clear(dev->dir_a);
      gpio_set(dev->dir_b);
    }

    DEBUG("dcmotor_set_speed: speed %d\n", speed);
    pwm_set(dev->pwm, dev->channel, speed);
}
