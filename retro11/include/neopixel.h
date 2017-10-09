/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @defgroup    driver_neopixel
 * @ingroup     drivers_actuators
 * @brief       High-level driver for easy handling of neopixel led stripes
 * @{
 *
 * @file
 * @brief       High-level driver for easy handling of neopixel led stripes
 *
 * @author      Semjon Kerner <semjon.kerner@fu-berlin.de>
 * @author      Steve Dierker <dierker.steve@fu-berlin.de>
 */

#ifndef NEOPIXEL_H
#define NEOPIXEL_H

#include "periph/gpio.h"
#include "color.h"
#include "malloc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Descriptor struct for Neopixel LED stripe
 */
typedef struct {
    uint32_t led_count;      /**< count of leds to control */
    gpio_t data_out;         /**< pin for data out */
    color_rgb_t pixels[128]; /**< In memort mapping of colors */
} neopixel_t;

/**
 * @brief Initialize a Neopixel by assigning it a gpio pin
 *
 * @param[out] dev         struct describing the neopixel led stripe
 * @param[in] leds         LED count
 * @param[in] data         gpio pin for data out
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
int neopixel_init(neopixel_t *dev, uint32_t led_count, gpio_t data_out);

/**
 * @brief Set color of LED
 *
 * @param[out] dev         struct describing the neopixel led stripe
 * @param[in] index        LED address to set color
 * @param[in] color        new color for LED
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
int neopixel_set_pixel_color(neopixel_t *dev, uint32_t index, color_rgb_t pixel);

/**
 * @brief Get color of LED
 *
 * @param[out] dev         struct describing the neopixel led stripe
 * @param[in] index        LED address to get color
 * @param[in] color        new color for LED
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
int neopixel_get_pixel_color(neopixel_t *dev, uint32_t index, color_rgb_t *pixel);

/**
 * @brief Update LED controllers
 *
 * @param[out] dev         struct describing the neopixel led stripe
 *
 * @return                  0 on success
 * @return                  <0 on error
 */
void neopixel_show(neopixel_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* NEOPIXEL_H */
/** @} */
