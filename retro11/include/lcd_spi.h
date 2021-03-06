/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    driver_lcd_spi LCD over SPI Driver
 * @ingroup     drivers_displays
 * @brief       High-level driver for LCD over SPI
 * @{
 *
 * @file
 * @brief       High-level driver for easy handling of displays over spi
 *
 * @author      Steve Dierker <dierker.steve@fu-berlin.de>
 */

#ifndef LCD_SPI_H
#define LCD_SPI_H

#include "periph/gpio.h"
#include "periph/spi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Descriptor struct for a lcd
 */
typedef struct {
  spi_t spi;
  spi_mode_t mode;
  spi_clk_t clk;
  gpio_t cs;
  gpio_t cmdselect;
  gpio_t reset;
  uint8_t ram[128][64];
  bool reflection;
} lcd_spi_t;

/**
 * @brief Initialize a dc motor by assigning it a PWM device and channel
 *
 * @param[out] dev          struct describing the servo
 * @param[in] pwm           the PWM device the servo is connected to
 * @param[in] channel   the PWM channel the servo is connected to
 * @param[in] freq          max pwm frequency
 * @param[in] res           resolution to set frequency
 * @param[in] dir_a         direction a gpio pin
 * @param[in] dir_b         direction b gpio pin
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
int lcd_spi_init(lcd_spi_t *dev, spi_t spi, gpio_t cs, gpio_t cmdselect, gpio_t reset);

void lcd_spi_set_contrast(lcd_spi_t *dev, uint8_t value);

void lcd_spi_set_display_normal(lcd_spi_t *dev, bool value);

void lcd_spi_reset(lcd_spi_t *dev);

void lcd_spi_show(lcd_spi_t *dev);

void lcd_spi_clear(lcd_spi_t *dev);

void lcd_spi_reset(lcd_spi_t *dev);

void lcd_spi_invert(lcd_spi_t *dev);

/* Drawing Methods */

int lcd_spi_set_pixel(lcd_spi_t *dev, uint8_t xpos, uint8_t ypos, uint8_t pixel);

int lcd_spi_draw_line(lcd_spi_t *dev, uint8_t startXpos, uint8_t startYpos, uint8_t stopXpos, uint8_t stopYpos);

int lcd_spi_draw_rect(lcd_spi_t *dev, uint8_t startXpos, uint8_t startYpos, uint8_t stopXpos, uint8_t stopYpos, bool fill);

int lcd_spi_draw_circle(lcd_spi_t *dev, uint8_t x, uint8_t y, uint8_t r, bool fill);

int lcd_spi_draw_c(lcd_spi_t *dev, uint8_t x, uint8_t y, char c);

int lcd_spi_draw_s(lcd_spi_t *dev, uint8_t x, uint8_t y, const char *c, uint8_t n);

#ifdef __cplusplus
}
#endif

#endif /* LCD_SPI_H */
/** @} */
