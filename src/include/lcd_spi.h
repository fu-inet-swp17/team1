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
  gpio_t chipselect;
  gpio_t cmdselect;
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
int lcd_spi_init(lcd_spi_t *dev, spi_t spi, gpio_t cs, gpio_t cmdselect);

#ifdef __cplusplus
}
#endif

#endif /* LCD_SPI_H */
/** @} */
