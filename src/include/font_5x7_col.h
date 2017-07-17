/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_mineplex 5x5 Font 'Mineplex'
 * @ingroup     sys
 * @brief       The Mineplex font for containing 5x5 pixel ASCII characters
 * @{
 *
 * @file
 * @brief       Interface definition to access the Mineplex font
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef FONT_5x7_COL_H
#define FONT_5x7_COL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   The width of a single character in pixel
 */
#define FONT_5x7_COL_CHAR_W     (5U)

/**
 * @brief   The height of a single character in pixel
 */
#define FONT_5x7_COL_CHAR_H     (7U)

/**
 * @brief   Get the Mineplex representation of a given ASCII character
 *
 * The function returns the pointer to a 5 byte pointer containing the Mineplex
 * representation of the given ASCII character. The Mineplex character is
 * encoded row wise from top to bottom using the least significant 5 bit, where
 * byte 1, bit 1 is the top left pixel of the encoded character.
 *
 * @param[in] c         character to translate
 *
 * @return  a 5 byte big buffer containing the encoded Mineplex character
 */
const uint8_t *font_5x7_col_char(char c);

#ifdef __cplusplus
}
#endif

#endif /* FONT_5x7_COL_H */
/** @} */
