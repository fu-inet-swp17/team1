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
#include "periph/gpio.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

int dcmotor_init(dcmotor_t *dev, pwm_t pwm, int channel, uint32_t freq, uint16_t res,
    gpio_t dir_a, gpio_t dir_b)
{
    if (gpio_init(dir_a, GPIO_OUT) < 0) {
      DEBUG("dcmotor_init: failed to init gpio %d\n", dir_a);
      return -1;
    }

    if (gpio_init(dir_b, GPIO_OUT) < 0) {
      DEBUG("dcmotor_init: failed to init gpio %d\n", dir_b);
      return -1;
    }

    pwm_set(pwm, channel, 0);

    dev->pwm = pwm;
    dev->channel = channel;
    dev->freq = freq;
    dev->res = res;
    dev->dir_a = dir_a;
    dev->dir_b = dir_b;

    return 0;
}

void dcmotor_set_speed(dcmotor_t *dev, int16_t speed)
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
