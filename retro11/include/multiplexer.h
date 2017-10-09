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
  gpio_t receive;    /**< GPIO Pin for receive */
  gpio_t addr_a;     /**< GPIO Pin for addr a */
  gpio_t addr_b;     /**< GPIO Pin for addr b */
  gpio_t addr_c;     /**< GPIO Pin for addr c */
  int16_t curr_addr; /**< Last accessed address */
  int with_int;      /**< Enable interrupts */
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
 * @brief Initialize a multiplexer by assigning it multiple gpio pins with additional interrupt.
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
int multiplexer_init_int(multiplexer_t *dev, gpio_t receive, gpio_t addr_a, gpio_t addr_b,
    gpio_t addr_c, gpio_cb_t cb, void *arg);

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

/**
 * @brief Enable interrupt for gpio receive
 *
 * @param[in] dev           struct describing the multiplexer
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
int multiplexer_int_enable(multiplexer_t *dev);

/**
 * @brief Disable interrupt for gpio receive
 *
 * @param[in] dev           struct describing the multiplexer
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
int multiplexer_int_disable(multiplexer_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* MULTIPLEXER_H */
/** @} */
