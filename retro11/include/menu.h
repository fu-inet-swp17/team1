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

typedef struct menu_command_t {
  const char *name;
  const char *desc;
  menu_command_handler_t handler;
} menu_command_t;

typedef struct {
  multiplexer_t *multiplexer;
  lcd_spi_t *display;
  neopixel_t *led_stripe;
  motor_controller_t *mcontroller;
  const menu_command_t *commands;
  int cmd_count;
} menu_t;

void menu_init(menu_t *menu, multiplexer_t *mplexer, lcd_spi_t *display, neopixel_t *led_stripe, motor_controller_t *mcontroller, const menu_command_t *commands, int cmd_count);

void menu_run(menu_t *menu);

#ifdef __cplusplus
}
#endif

#endif /* MENU_H */
/** @} */
