/*
 * Copyright (C) 2014 Freie Universität Berlin
 * Copyright (C) 2015 Eistec AB
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for more
 * details.
 */

/**
 * @ingroup     driver_dcmotor
 * @{
 *
 * @file
 * @brief       DC motor driver implementation
 *
 * @author      Steve Dierker <dierker.steve@fu-berlin.de>
 *
 * @}
 */

#include "lcd_spi.h"
#include "periph/gpio.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

int lcd_spi_init(lcd_spi_t *dev, spi_t spi, gpio_t cs, gpio_t cmdselect)
{
  int status;
  status = spi_init_cs(spi, cs);
  if (status != SPI_OK ) {
    DEBUG("lcd_spi_init: initialize spi failed with %d\n", status);
    return -1;
  }

  gpio_init(cmdselect, GPIO_OUT);

  dev->spi = spi;
  dev->chipselect = cs;
  dev->cmdselect = cmdselect;
  dev->mode = SPI_MODE_2;
  dev->clk = SPI_CLK_1MHZ;

  gpio_clear(cmdselect);

  uint8_t init_data[] = {0x40, 0xA1, 0xC0, 0xA6, 0xA2, 0x2F, 0xF8, 0x00, 0x27, 0x81,
    0x10, 0xAC, 0x00, 0xAF};
  puts("Print.");
  for(int i =0; i < 14; ++i) {
    printf("ORIG: %d MSB: %d LSB: %d\n", init_data[i], (uint8_t)bitarithm_msb(init_data[i]), (uint8_t)bitarithm_lsb(init_data[i]));
  }
  puts("Done.");

  /* Acquire SPI bus */
  spi_acquire(spi, cs, dev->mode, dev->clk);

  for(int i = 0; i < 14; ++i) {
    spi_transfer_byte(spi, cs, false, init_data[i]);
  }

  spi_release(spi);

  return 0;
}
