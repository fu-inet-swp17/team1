/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    driver_multiplexer Multiplexer Driver
 * @ingroup     drivers_multiplexer
 * @brief       High-level driver for multiplexer
 * @{
 *
 * @file
 * @brief       High-level driver for multiplexer
 *
 * @author      Steve Dierker <dierker.steve@fu-berlin.de>
 */

#ifndef MULTIPLEXER_H
#define MULTIPLEXER_H

#include "periph/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Descriptor struct for a multiplexer
 */
typedef struct {
  gpio_t receive;
  gpio_t addr_a;
  gpio_t addr_b;
  gpio_t addr_c;
} multiplexer_t;

/**
 * @brief Initialize a multiplexer by assigning it multiple gpio pins
 *
 * @param[out] dev          struct describing the multiplexer
 * @param[in] receive       gpio pin to receive value
 * @param[in] addr_a         gpio pin to set address a
 * @param[in] addr_b         gpio pin to set address b
 * @param[in] addr_c         gpio pin to set address c
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
int multiplexer_init(multiplexer_t *dev, gpio_t receive, gpio_t addr_a, gpio_t addr_b, gpio_t addr_c);

/**
 * @brief Receive value of the given address
 *
 * @param[in] dev           struct describing the multiplexer
 * @param[in] addr          address to read value from
 *
 * @return                  0 on LOW
 * @return                  >0 on HIGH
 */
int multiplexer_receive(multiplexer_t *dev, uint16_t addr);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLEXER_H */
/** @} */
