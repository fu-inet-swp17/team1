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
#include "xtimer.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

// LCD display commands
#define CMDDISPLOFF 0xAE
#define CMDDISPLON  0xAF
// startline address + line (0 to 63)
#define CMDSETSTARTLINE 0x40
// +pagenumber (0 to 7)
#define CMDSETPAGEADR 0xB0
// + colnr_hi
#define CMDSETCOLADRHI 0x10
// + colnr_lo
#define CMDSETCOLADRLO 0x00
#define CMDADCNORMAL 0xA0
#define CMDADCREVERSE 0xA1
#define CMDDISPLAYNORMAL 0xA6
#define CMDDISPLAYREVERSE 0xA7
#define CMDALLPOINTSOFF 0xA4
#define CMDALLPOINTSON 0xA5
#define CMDSETLCDBIAS 0xA2
#define CMDRESET 0xE2
#define CMDCOMOUTMODE 0xC0
#define CMDSETPOWERCTRL 0x2F
// + ratio (0 to 7)
#define CMDSETRESISTORRATIO 0x20
#define CMDSETVOLMODE 0x81
// + val (0 to 63)
#define CMDSETVOLMODEVAL 0x00
#define CMDINDICATOROFF 0xAC
#define CMDINDICATORVAL 0x00
#define CMDSETBOOSTERRATIO 0xF8
// + val (0 to 3)
#define CMDSETBOOSTERRATIOVAL 0x00
#define CMDNOP 0xE3

void set_page_address(lcd_spi_t *dev, uint8_t page, uint8_t colhi, uint8_t collo)
{
  uint8_t cmd;

  gpio_set(dev->cmdselect);
  spi_acquire(dev->spi, dev->cs, dev->mode, dev->clk);

  cmd = CMDSETPAGEADR + page;
  spi_transfer_byte(dev->spi, dev->cs, true, cmd);
  cmd = CMDSETCOLADRHI + colhi;
  spi_transfer_byte(dev->spi, dev->cs, true, cmd);
  cmd = CMDSETCOLADRLO + collo;
  spi_transfer_byte(dev->spi, dev->cs, true, cmd);

  spi_release(dev->spi);
  gpio_clear(dev->cmdselect);
}

bool position_on_screen(uint8_t xpos, uint8_t ypos)
{
  if (xpos >= 0 && xpos <= 127) {
    if (ypos >= 0 && ypos <= 63) {
      return true;
    }
  }

  return false;
}

int abs(int value)
{
  if (value < 0)
    return value * -1;
  else
    return value;
}

int lcd_spi_init(lcd_spi_t *dev, spi_t spi, gpio_t cs, gpio_t cmdselect, gpio_t reset)
{
  int status;
  status = spi_init_cs(spi, cs);
  if (status != SPI_OK ) {
    DEBUG("lcd_spi_init: initialize spi failed with %d\n", status);
    return -1;
  }

  gpio_init(cmdselect, GPIO_OUT);
  gpio_init(reset, GPIO_OUT);

  dev->spi = spi;
  dev->cs = cs;
  dev->cmdselect = cmdselect;
  dev->reset = reset;
  dev->mode = SPI_MODE_0;
  dev->clk = SPI_CLK_1MHZ;

  gpio_clear(reset);
  gpio_set(reset);

  /* Acquire SPI bus */
  gpio_clear(dev->cmdselect);
  spi_acquire(dev->spi, dev->cs, dev->mode, dev->clk);

  spi_transfer_byte(spi, cs, true, CMDSETSTARTLINE);        // 0x40
  spi_transfer_byte(spi, cs, true, CMDADCREVERSE);          // 0xA1
  spi_transfer_byte(spi, cs, true, CMDCOMOUTMODE);          // 0xC0
  spi_transfer_byte(spi, cs, true, CMDDISPLAYNORMAL);       // 0xA6
  spi_transfer_byte(spi, cs, true, CMDSETLCDBIAS);          // 0xA2
  spi_transfer_byte(spi, cs, true, CMDSETPOWERCTRL);        // 0x2F
  spi_transfer_byte(spi, cs, true, CMDSETBOOSTERRATIO);     // 0xF8
  spi_transfer_byte(spi, cs, true, CMDINDICATORVAL);        // 0x00
  spi_transfer_byte(spi, cs, true, CMDSETRESISTORRATIO + 7); // 0x27
  spi_transfer_byte(spi, cs, true, CMDSETVOLMODE);           // 0x81
  spi_transfer_byte(spi, cs, true, CMDSETVOLMODEVAL + 10);  // 0x10
  spi_transfer_byte(spi, cs, true, CMDINDICATOROFF);        // 0xAC
  spi_transfer_byte(spi, cs, true, CMDINDICATORVAL);        // 0x00
  spi_transfer_byte(spi, cs, true, CMDDISPLON);             // 0xAF

  spi_release(dev->spi);
  gpio_set(dev->cmdselect);

  return 0;
}

void lcd_spi_set_contrast(lcd_spi_t *dev, uint8_t value)
{
  if (value < 5)
    value = 5;
  if (value > 25)
    value = 25;

  gpio_clear(dev->cmdselect);
  spi_acquire(dev->spi, dev->cs, dev->mode, dev->clk);

  spi_transfer_byte(dev->spi, dev->cs, true, CMDSETVOLMODE);
  spi_transfer_byte(dev->spi, dev->cs, true, CMDSETVOLMODE + value);

  spi_release(dev->spi);
  gpio_set(dev->cmdselect);
}

void lcd_spi_set_display_normal(lcd_spi_t *dev, bool value)
{
  uint8_t cmd;

  if (value)
    cmd = CMDDISPLAYNORMAL;
  else
    cmd = CMDDISPLAYREVERSE;

  gpio_clear(dev->cmdselect);
  spi_acquire(dev->spi, dev->cs, dev->mode, dev->clk);

  spi_transfer_byte(dev->spi, dev->cs, true, cmd);

  spi_release(dev->spi);
  gpio_set(dev->cmdselect);
}



void lcd_spi_show(lcd_spi_t *dev)
{
  uint8_t page, byte;
  uint8_t x = 0;
  uint8_t line[8][128];
  uint8_t bit;

  for (page = 0; page < 8; page++) {
    for (x = 0;x < 128; x++) {
      byte = 0;
      for (bit = 0; bit < 8; bit++) {
        byte = byte + (dev->ram[x][page * 8 + bit] << bit);
      }
      line[page][x] = byte;
    }
  }

  for (page = 0;page<8;page++) {
    set_page_address(dev, page, 0, 0);

    gpio_set(dev->cmdselect);
    spi_acquire(dev->spi, dev->cs, dev->mode, dev->clk);

    spi_transfer_bytes(dev->spi, dev->cs, true, line[page], NULL, 128);

    spi_release(dev->spi);
    gpio_clear(dev->cmdselect);
  }
}

void lcd_spi_invert(lcd_spi_t *dev)
{
  for(int x = 0; x < 128; ++x) {
    for(int y = 0; y < 64; ++y) {
      if (dev->ram[x][y] == 1) {
        dev->ram[x][y] = 0;
      } else {
        dev->ram[x][y] = 1;
      }
    }
  }
}

void lcd_clear(lcd_spi_t *dev)
{
  for(int x = 0; x < 128; ++x) {
    for(int y = 0; y < 64; ++y) {
      dev->ram[x][y] = 0;
    }
  }
}

void lcd_spi_reset(lcd_spi_t *dev)
{
  gpio_clear(dev->reset);
  gpio_set(dev->reset);
}

int lcd_spi_set_pixel(lcd_spi_t *dev, uint8_t xpos, uint8_t ypos, uint8_t pixel)
{
  if (!position_on_screen(xpos, ypos))
    return -1;

  if (pixel > 1)
    pixel = 1;

  dev->ram[xpos][ypos] = pixel;

  return 0;
}

int lcd_spi_draw_line(lcd_spi_t *dev, uint8_t startXpos, uint8_t startYpos, uint8_t endXpos, uint8_t endYpos)
{
  int dx, dy, err, sx, sy, e2;

  if (!position_on_screen(startXpos, startYpos) || !position_on_screen(endXpos, endYpos))
    return -1;

  dx = abs(endXpos - startXpos);
  if (startXpos < endXpos) {
    sx = 1;
  } else {
    sx = -1;
  }
  dy = -abs(endYpos - startYpos);
  if (startYpos < endYpos) {
    sy = 1;
  } else {
    sy = -1;
  }
  err = dx + dy;
  while (true) {
    lcd_spi_set_pixel(dev, startXpos, startYpos, 1);
    if (startXpos == endXpos && startYpos == endYpos) break;
    e2 = 2*err;
    if (e2 > dy) {
      err += dy;
      startXpos += sx;
    }
    if (e2 < dx) {
      err += dx;
      startYpos += sy;
    }
  }

   return 0;
}

int lcd_spi_draw_rect(lcd_spi_t *dev, uint8_t startXpos, uint8_t startYpos, uint8_t endXpos, uint8_t endYpos, bool fill)
{
  int x1set, x2set, i;

  if (!position_on_screen(startXpos, startYpos) || !position_on_screen(endXpos, endYpos))
    return -1;

  if (fill) {
    if (startXpos <= endXpos) {
      x1set = startXpos;
      x2set = endXpos;
    } else {
      x1set = endXpos;
      x2set = startXpos;
    }

    for (i = x1set; i < x2set + 1; i++)
      lcd_spi_draw_line(dev, i, startYpos, i, endYpos);
  } else {
    lcd_spi_draw_line(dev, startXpos, startYpos, startXpos, endYpos);
    lcd_spi_draw_line(dev, startXpos, startYpos, endXpos, startYpos);
    lcd_spi_draw_line(dev, startXpos, endYpos, endXpos, endYpos);
    lcd_spi_draw_line(dev, endXpos, endYpos, endXpos, startYpos);
  }

  return 0;
}

int lcd_spi_draw_circle(lcd_spi_t *dev, uint8_t x, uint8_t y, uint8_t radius, bool fill)
{
   signed int a, b, P;
   a = 0;
   b = radius;
   P = 1 - radius;

   do
   {
      if(fill) {
         lcd_spi_draw_line(dev, x-a, y+b, x+a, y+b);
         lcd_spi_draw_line(dev, x-a, y-b, x+a, y-b);
         lcd_spi_draw_line(dev, x-b, y+a, x+b, y+a);
         lcd_spi_draw_line(dev, x-b, y-a, x+b, y-a);
      } else {
         lcd_spi_set_pixel(dev, a+x, b+y, 1);
         lcd_spi_set_pixel(dev, b+x, a+y, 1);
         lcd_spi_set_pixel(dev, x-a, b+y, 1);
         lcd_spi_set_pixel(dev, x-b, a+y, 1);
         lcd_spi_set_pixel(dev, b+x, y-a, 1);
         lcd_spi_set_pixel(dev, a+x, y-b, 1);
         lcd_spi_set_pixel(dev, x-a, y-b, 1);
         lcd_spi_set_pixel(dev, x-b, y-a, 1);
      }

      if(P < 0)
         P+= 3 + 2*a++;
      else
         P+= 5 + 2*(a++ - b--);
    } while(a <= b);

   return 0;
}

