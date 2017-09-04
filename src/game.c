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

#define LED_DISTANCE (2U)

extern bool enableBtns;

void play_winner_animation(game_t *dev)
{
  bool flip = true;
  color_rgb_t color;

  for(int i = 0; i < 20; ++i) {
    for(int j = 0; j < (dev->led_stripe->led_count - 2); ++j) {
      if((j % 2 != 0)) {
        color.r = 0;
        color.b = 0;

        if(flip)
          color.g = 255;
        else
          color.g = 0;

        neopixel_set_pixel_color(dev->led_stripe, j, color);
      } else {
        color.r = 0;
        color.b = 0;

        if(flip)
          color.g = 0;
        else
          color.g = 255;


        neopixel_set_pixel_color(dev->led_stripe, j, color);
      }
    }

    neopixel_show(dev->led_stripe);
    xtimer_usleep(200000);
    flip = !flip;
  }
}

void play_looser_animation(game_t *dev)
{
  bool flip = true;
  color_rgb_t color;

  for(int i = 0; i < 20; ++i) {
    for(int j = 0; j < (dev->led_stripe->led_count - 2); ++j) {
      if((j % 2 != 0)) {
        color.g = 0;
        color.b = 0;

        if(flip)
          color.r = 255;
        else
          color.r = 0;

        neopixel_set_pixel_color(dev->led_stripe, j, color);
      } else {
        color.g = 0;
        color.b = 0;

        if(flip)
          color.r = 0;
        else
          color.r = 255;


        neopixel_set_pixel_color(dev->led_stripe, j, color);
      }
    }

    neopixel_show(dev->led_stripe);
    xtimer_usleep(200000);
    flip = !flip;
  }
}

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

    for(action = 0;; ++action) {
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
      default:
        break;
    }

    if (curr_idx >= 15) {
      name[16] = '\0';
      return;
    }

    xtimer_usleep(7500);
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
    puts("GameLoop: GetPlayerName.");
    get_player_name(dev, dev->playername);
    puts("GameLoop: GetPlayerName done.");
    dev->state = NAME_READY;

    puts("GameLoop: Display waiting for server.");
    lcd_spi_clear(dev->display);
    lcd_spi_draw_s(dev->display, 11, 28, "WAITING FOR SERVER", 18);
    lcd_spi_show(dev->display);

    /* Start Game */
    while (1) {
      if (dev->state == START_GAME)
        break;
    }

    lcd_spi_clear(dev->display);
    lcd_spi_draw_s(dev->display, 11, 28, "GO", 18);
    lcd_spi_show(dev->display);

    motor_timeout = random_uint32_range(2000000, 20000000);
    multiplexer_receive(dev->mplexer, 0);

    /* Start motor */
    puts("Starting motor.");
    enableBtns = true;
    motor_controller_set_speed(dev->mctrl, 200, motor_timeout);
    start_time = xtimer_now_usec();

    if (xtimer_msg_receive_timeout(&msg, motor_timeout) == -1) {
      puts("GameLoop: Player did not react in time.");
      motor_controller_set_speed(dev->mctrl, 0, 0);
      dev->state = FAILED;
      continue;
    }
    puts("Message received.");
    enableBtns = false;

    dev->reaction_time = xtimer_now_usec() - start_time;
    puts("Remember time.");
    motor_controller_set_speed(dev->mctrl, 0, 0);
    puts("Stop motor.");
    dev->state = GET_RESULT;
    puts("state is set.");

    /* Get Results */
    lcd_spi_clear(dev->display);
    lcd_spi_draw_s(dev->display, 11, 5, "YOU REACTED IN:", 15);
    sprintf(number, "%ldus", dev->reaction_time);
    lcd_spi_draw_s(dev->display, 11, 15, number, strlen(number));
    lcd_spi_show(dev->display);

    puts("waiting for winner.");
    /* Display winner */
    while(1) {
      if(dev->state == WINNER || dev->state == LOOSER)
        break;
    }
    puts("Received.");

    lcd_spi_clear(dev->display);
    lcd_spi_draw_s(dev->display, 11, 15, dev->playername, strlen(dev->playername));
    lcd_spi_show(dev->display);

    if (dev->state == WINNER)
      play_winner_animation(dev);
    else
      play_looser_animation(dev);
  }
}

void game_init(game_t *dev, motor_controller_t *mctrl, lcd_spi_t *display, multiplexer_t *mplexer, neopixel_t *led_stripe)
{
  dev->mctrl = mctrl;
  dev->display = display;
  dev->mplexer = mplexer;
  dev->led_stripe= led_stripe;
}

