/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     driver_multiplexer
 * @{
 *
 * @file
 * @brief       Implementation of Multiplexer Driver
 *
 * @author      Steve Dierker <dierker.steve@fu-berlin.de>
 *
 * @}
 */

#include "multiplexer.h"
#include "periph/gpio.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

int init_gpio_outs(multiplexer_t *dev) {
  if (gpio_init(dev->addr_a, GPIO_OUT) < 0) {
    DEBUG("multiplexer_init: failed to set gpio addr_a %d.\n", addr_a);
    return -1;
  }

  if (gpio_init(dev->addr_b, GPIO_OUT) < 0) {
    DEBUG("multiplexer_init: failed to set gpio addr_b %d.\n", addr_b);
    return -1;
  }

  if (gpio_init(dev->addr_c, GPIO_OUT) < 0) {
    DEBUG("multiplexer_init: failed to set gpio addr_c %d.\n", addr_c);
    return -1;
  }

  gpio_clear(dev->addr_a);
  gpio_clear(dev->addr_b);
  gpio_clear(dev->addr_c);

  return 0;
}

int multiplexer_init(multiplexer_t *dev, gpio_t receive, gpio_t addr_a, gpio_t addr_b, gpio_t addr_c)
{
  if (gpio_init(receive, GPIO_IN) < 0) {
    DEBUG("multiplexer_init: failed to set gpio receive %d.\n", receive);
    return -1;
  }

  dev->receive = receive;
  dev->addr_a = addr_a;
  dev->addr_b = addr_b;
  dev->addr_c = addr_c;
  dev->with_int = -1;

  if (init_gpio_outs(dev) < 0)
    return -1;

  return 0;
}

int multiplexer_init_int(multiplexer_t *dev, gpio_t receive, gpio_t addr_a, gpio_t addr_b,
    gpio_t addr_c, gpio_cb_t cb, void *arg)
{
  if (gpio_init_int(receive, GPIO_IN, GPIO_RISING, cb, arg) < 0) {
    DEBUG("multiplexer_init: failed to set gpio receive %d.\n", receive);
    return -1;
  }

  dev->receive = receive;
  dev->addr_a = addr_a;
  dev->addr_b = addr_b;
  dev->addr_c = addr_c;
  dev->curr_addr = 0;
  dev->with_int = 1;

  if (init_gpio_outs(dev) < 0)
    return -1;

  return 0;
}

int multiplexer_receive(multiplexer_t *dev, uint16_t addr)
{
  int value;
  multiplexer_int_disable(dev);

  dev->curr_addr = addr;

  if ((addr & 1) == 1) {
    gpio_set(dev->addr_a);
  } else {
    gpio_clear(dev->addr_a);
  }

  if ((addr & 2) == 2) {
    gpio_set(dev->addr_b);
  } else {
    gpio_clear(dev->addr_b);
  }

  if ((addr & 4) == 4) {
    gpio_set(dev->addr_c);
  } else {
    gpio_clear(dev->addr_c);
  }

  value = gpio_read(dev->receive);
  multiplexer_int_enable(dev);

  return value;
}

int multiplexer_int_enable(multiplexer_t *dev)
{
  if (dev->with_int < 0)
    return -1;

  gpio_irq_enable(dev->receive);
  return 0;
}

int multiplexer_int_disable(multiplexer_t *dev)
{
  if (dev->with_int < 0)
    return -1;

  gpio_irq_disable(dev->receive);
  return 0;
}
