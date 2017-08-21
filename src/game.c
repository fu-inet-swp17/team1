/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

#define ENABLE_DEBUG (0)
#include "debug.h"

#include "thread.h"
#include "xtimer.h"
#include "game.h"
#include "random.h"
#include "string.h"

extern bool enableBtns;

void get_player_name(game_t *dev, char *name)
{
  int curr_idx = 0;
  int curr_char_idx = 0;
  int i;
  char action;
  char alphabet[28] = { '_', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
    'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' '};

  while(1) {
    /* Clear display */
    lcd_spi_clear(dev->display);
    lcd_spi_draw_s(dev->display, 10, 5, "ENTER PLAYER NAME: ", 19);
    for(i = 0; i < curr_idx; ++i)
      lcd_spi_draw_c(dev->display, 10 + (i * 6), 14, name[i]);
    lcd_spi_draw_c(dev->display, 10 + (curr_idx * 6), 14, alphabet[curr_char_idx]);

    lcd_spi_show(dev->display);

    for(action = 0;; action = (action + 1) % 3) {
      if (multiplexer_receive(dev->mplexer, action) == 1)
        break;
    }

    switch(action) {
      case 0:
        name[curr_idx] = alphabet[curr_char_idx];
        curr_idx++;
        curr_char_idx = 0;
        break;
      case 1:
        curr_char_idx = (curr_char_idx + 1) % 27;
        break;
      case 2:
        if (curr_char_idx == 0)
          curr_char_idx = 26;
        else
          curr_char_idx--;
        break;
    }

    if (curr_idx >= 15) {
      name[16] = '\0';
      return;
    }

    xtimer_usleep(250000);
  }
}

void game_run(game_t *dev)
{
  uint32_t motor_timeout, start_time;
  msg_t msg;
  char number[32];

  while (1) {
    dev->state = INIT;
    /* Ask for player name */
    get_player_name(dev, dev->playername);
    dev->state = NAME_READY;

    lcd_spi_clear(dev->display);
    lcd_spi_draw_s(dev->display, 11, 28, "WAITING FOR SERVER", 18);
    lcd_spi_show(dev->display);

    /* Start Game */
    while (1) {
      if (dev->state == START_GAME)
        break;
    }
    motor_timeout = random_uint32_range(2000000, 20000000);
    multiplexer_receive(dev->mplexer, 0);
    enableBtns = false;
    while (1) {
      if (xtimer_msg_receive_timeout(&msg, motor_timeout + 10000000) == -1) {
        puts("Something went wrong, the motor did not react.");
        dev->state = FAILED;
        break;
      }

      if (msg.sender_pid == (dev->mctrl)->ctrl_pid) {
        enableBtns = true;
        start_time = xtimer_now_usec();

        if (xtimer_msg_receive_timeout(&msg, 100000000) == -1) {
          puts("Player did not react in time.");
          dev->state = FAILED;
          break;
        }

        dev->reaction_time = xtimer_now_usec() - start_time;
        printf("Player reacted in %ldms.\n", dev->reaction_time);
        dev->state = GET_RESULT;
        break;
      }
    }

    if (dev->state == FAILED)
      continue;

    /* Get Results */
    lcd_spi_clear(dev->display);
    lcd_spi_draw_s(dev->display, 11, 5, "YOU REACTED IN:", 15);
    sprintf(number, "%ldms", dev->reaction_time);
    lcd_spi_draw_s(dev->display, 11, 15, number, strlen(number));
    lcd_spi_show(dev->display);

    /* Display winner */
  }
}

void game_init(game_t *dev, motor_controller_t *mctrl, lcd_spi_t *display, multiplexer_t *mplexer)
{
  dev->mctrl = mctrl;
  dev->display = display;
  dev->mplexer = mplexer;
}

