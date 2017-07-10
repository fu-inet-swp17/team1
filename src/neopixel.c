/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     driver_ws2811
 * @{
 *
 * @file
 * @brief       ws2811 led stripe driver implementation
 *
 * @author      semjon kerner <semjon.kerner@fu-berlin.de>
 *
 * @}
 */

#include "neopixel.h"
#include "periph/gpio.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

int neopixel_init(neopixel_t *dev, uint32_t led_count, gpio_t data_out)
{
  if (gpio_init(data_out, GPIO_OUT) < 0) {
    DEBUG("neopixel_init: failed to init gpio %d\n", data_out);
    return -1;
  }

  gpio_clear(data_out);

  dev->led_count = led_count;
  dev->data_out = data_out;

  return 0;
}

void neopixel_free(neopixel_t *dev)
{
  if(dev->pixels)
    free(dev->pixels);
}

void neopixel_set_brightness(neopixel_t *dev, uint8_t brightness)
{
  dev->brightness = brightness;
}

int neopixel_set_pixel_color(neopixel_t *dev, uint32_t index, color_rgb_t pixel)
{
  if (index > dev->led_count) {
    DEBUG("neopixel_set_pixel_color: index out of bound.\n");
    return -1;
  }

  /* NOTE: Colors can be ordered differently for each chip
   *  WS2812B: 8Bit Green; 8Bit Red; 8Bit Blue
   */
  dev->pixels[index].r = pixel.r;
  dev->pixels[index].g = pixel.g;
  dev->pixels[index].b = pixel.b;

  return 0;
}

int neopixel_get_pixel_color(neopixel_t *dev, uint32_t index, color_rgb_t *pixel)
{
  if (index > dev->led_count) {
    DEBUG("neopixel_get_pixel_color: index ouf of bound.\n");
    return -1;
  }

  /* NOTE: For color order see neopixel_set_pixel_color */
  pixel->r = dev->pixels[index].r;
  pixel->g = dev->pixels[index].g;
  pixel->b = dev->pixels[index].b;

  return 0;
}

void neopixel_show(neopixel_t *dev)
{
  int irq_state;
  int i,j;
  color_rgb_t *pixel;
  PortGroup * group = (PortGroup*)(dev->data_out & ~(0x1f));
  int pin = (1 << (dev->data_out & (0x1f)));

  gpio_clear(dev->data_out);
  for(i = 0; i < dev->led_count; i++) {
    printf("LED %03d: r: %d g: %d b: %d\n", i, dev->pixels[i].r, dev->pixels[i].g, dev->pixels[i].b);
  }

  /* Disable IRQ's, we have hard time bounds here.
   * Restore it afterwards */
  irq_state = irq_disable();
  /* Loop over all connected leds
   * For the WS2812B only the HIGH Signal has to be distinctive,
   * for LOW we don't need any NOP's.
   * We could also loop over all three colors for each LED,
   * but since we need hard timings for each 24bit we unroled the loop.
   */
  for (i = 0; i < dev->led_count; ++i) {
    pixel = &(dev->pixels[i]);
    for (j = 7; j > -1; --j) {
      if ((pixel->g >> j & 1) == 1) {
        group->OUTSET.reg = pin;
        /* Wait for
         * WS2811: 600ns +- 150ns
         * */
        __asm volatile(
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            );

        group->OUTCLR.reg = pin;
        /* Wait for
         * WS2811: 650ns +- 150ns
         * */
        __asm volatile(
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            );
      } else {
        group->OUTSET.reg = pin;
        /* Wait 400ns */
        __asm volatile(
            "nop \n\t"
            );

        group->OUTCLR.reg = pin;
        /* Wait 850ns */
        __asm volatile(
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            );
      }
    }
    for (j = 0; j < 8; ++j) {
      if ((pixel->r >> j & 1) == 1) {
        group->OUTSET.reg = pin;
        /* Wait 800ns */
        __asm volatile(
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            );

        group->OUTCLR.reg = pin;
        /* Wait 450ns */
        __asm volatile(
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            );
      } else {
        group->OUTSET.reg = pin;
        /* Wait 400ns */
        __asm volatile(
            "nop \n\t"
            );

        group->OUTCLR.reg = pin;
        /* Wait 850ns */
        __asm volatile(
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            );
      }
    }
    for (j = 0; j < 8; ++j) {
      if ((pixel->b >> j & 1) == 1) {
        group->OUTSET.reg = pin;
        /* Wait 800ns */
        __asm volatile(
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            );

        group->OUTCLR.reg = pin;
        /* Wait 450ns */
        __asm volatile(
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            );
      } else {
        group->OUTSET.reg = pin;
        /* Wait 400ns */
        __asm volatile(
            "nop \n\t"
            );

        group->OUTCLR.reg = pin;
        /* Wait 850ns */
        __asm volatile(
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            "nop \n\t"
            );
      }
    }
  }

  /* Restore IRQ State */
  irq_restore(irq_state);
}

