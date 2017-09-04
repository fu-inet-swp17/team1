/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */
#ifndef GAME_H
#define GAME_H

#include "motor_controller.h"
#include "lcd_spi.h"
#include "multiplexer.h"
#include "neopixel.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  INIT,
  NAME_READY,
  START_GAME,
  GET_RESULT,
  WINNER,
  LOOSER,
  FAILED
} game_state_t;

typedef struct {
  kernel_pid_t ctrl_pid;
  motor_controller_t *mctrl;
  lcd_spi_t *display;
  multiplexer_t *mplexer;
  neopixel_t *led_stripe;
  volatile game_state_t state;
  uint32_t reaction_time;
  char playername[17];
} game_t;

void game_run(game_t *dev);
void game_init(game_t *dev, motor_controller_t *mctrl, lcd_spi_t *display, multiplexer_t *mplexer, neopixel_t *led_stripe);

#ifdef __cplusplus
}
#endif

#endif /* GAME_H */
/** @} */
