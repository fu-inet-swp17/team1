/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

#ifndef MENU_H
#define MENU_H

#include "neopixel.h"
#include "lcd_spi.h"
#include "multiplexer.h"
#include "motor_controller.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*menu_command_handler_t)(void);

/**
 * @brief Descriptor struct for menu item
 */
typedef struct menu_command_t {
  const char *name;               /**< Name of menu item  */
  const char *desc;               /**< Desc of menu item */
  menu_command_handler_t handler; /**< Function to execute on selection */
} menu_command_t;

/**
 * @brief Descriptor struct for menu
 */
typedef struct {
  multiplexer_t *multiplexer;     /**< Multiplexer to access buttons */
  lcd_spi_t *display;             /**< LCD to display menu */
  neopixel_t *led_stripe;         /**< LED Stripe to control button highlighting */
  motor_controller_t *mcontroller;/**< Motor controller, not used */
  const menu_command_t *commands; /**< Menu items */
  int cmd_count;                  /**< Number of menu items */
} menu_t;

/**
 * @brief Initialize menu
 *
 * @param[out] menu         struct describing menu
 * @param[in] mplexer       multiplexer
 * @param[in] display       LCD display
 * @param[in] led_stripe    Strip of WS2811 LEDS
 * @param[in] mcontroller   Motor controller
 * @param[in] commands      Array of menu items
 * @param[in] cmd_count     Number of menu items
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
void menu_init(menu_t *menu, multiplexer_t *mplexer, lcd_spi_t *display, neopixel_t *led_stripe, motor_controller_t *mcontroller, const menu_command_t *commands, int cmd_count);

/**
 * @brief Run menu
 *
 * @param[out] dev          struct describing menu
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
void menu_run(menu_t *menu);

#ifdef __cplusplus
}
#endif

#endif /* MENU_H */
/** @} */
