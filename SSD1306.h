#ifndef __SSD1306_H
#define __SSD1306_H

#include <avr/io.h>
#include "../AVR-Programming/AVR-Programming-Library/pinDefines.h"

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

#define SSD1306_MEMORYMODE          0x20 ///< See datasheet
#define SSD1306_COLUMNADDR          0x21 ///< See datasheet
#define SSD1306_PAGEADDR            0x22 ///< See datasheet
#define SSD1306_SETCONTRAST         0x81 ///< See datasheet
#define SSD1306_CHARGEPUMP          0x8D ///< See datasheet
#define SSD1306_SEGREMAP            0xA0 ///< See datasheet
#define SSD1306_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define SSD1306_DISPLAYALLON        0xA5 ///< Not currently used
#define SSD1306_NORMALDISPLAY       0xA6 ///< See datasheet
#define SSD1306_INVERTDISPLAY       0xA7 ///< See datasheet
#define SSD1306_SETMULTIPLEX        0xA8 ///< See datasheet
#define SSD1306_DISPLAYOFF          0xAE ///< See datasheet
#define SSD1306_DISPLAYON           0xAF ///< See datasheet
#define SSD1306_COMSCANINC          0xC0 ///< Not currently used
#define SSD1306_COMSCANDEC          0xC8 ///< See datasheet
#define SSD1306_SETDISPLAYOFFSET    0xD3 ///< See datasheet
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5 ///< See datasheet
#define SSD1306_SETPRECHARGE        0xD9 ///< See datasheet
#define SSD1306_SETCOMPINS          0xDA ///< See datasheet
#define SSD1306_SETVCOMDETECT       0xDB ///< See datasheet

#define SSD1306_SETLOWCOLUMN        0x00 ///< Not currently used
#define SSD1306_SETHIGHCOLUMN       0x10 ///< Not currently used
#define SSD1306_SETSTARTLINE        0x40 ///< See datasheet

#define SSD1306_EXTERNALVCC         0x01 ///< External display voltage source
#define SSD1306_SWITCHCAPVCC        0x02 ///< Gen. display voltage from 3.3V

#define SSD1306_RIGHT_HORIZONTAL_SCROLL              0x26 ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL               0x27 ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A ///< Init diag scroll
#define SSD1306_DEACTIVATE_SCROLL                    0x2E ///< Stop scroll
#define SSD1306_ACTIVATE_SCROLL                      0x2F ///< Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA             0xA3 ///< Set scroll range

/* display-specific pins */
#define SSD1306_DC      PD6
#define SSD1306_DC_PORT PORTD
#define SSD1306_DC_PIN  PIND
#define SSD1306_DC_DDR  DDRD

#define SSD1306_RST      PD5
#define SSD1306_RST_PORT PORTD
#define SSD1306_RST_PIN  PIND
#define SSD1306_RST_DDR  DDRD

#define SSD1306_COMMAND (SSD1306_DC_PORT &= ~(1 << SSD1306_DC))
#define SSD1306_DATA    (SSD1306_DC_PORT |= (1 << SSD1306_DC))

#define SLAVE_SELECT (SPI_SS_PORT &= ~(1 << SPI_SS))
#define SLAVE_DESELECT (SPI_SS_PORT |= (1 << SPI_SS))

/* display-specific macros */
#define SSD1306_PAGE_HEIGHT 0x08
#define SSD1306_PAGE_MASK   0xf8
#define SSD1306_PAGE_ROUND_UP(row)              \
   (((row) + 7) / SSD1306_PAGE_HEIGHT)
#define SSD1306_PAGE_ROUND_DOWN(row)            \
   ((row) / SSD1306_PAGE_HEIGHT)


#include "spi.h"
#include "util.h"

void display_config();
void display_init();
void display_clear(uint8_t pix);

/* note: height & width must be nonzero
 * note: slave should be selected */
#define DISPLAY_SELECT_IOBYTES 6
inline void display_select(uint8_t page, uint8_t col, uint8_t height,
                           uint8_t width) {
   SSD1306_COMMAND;
   const uint8_t cmds[] = {SSD1306_PAGEADDR, page, page + height - 1,
                           SSD1306_COLUMNADDR, col, col + width - 1};
   spi_write(cmds, LEN(cmds));
}

inline void display_selectbnds(const struct bounds *bnds) {
   display_select(bnds->crds.y, bnds->crds.x, bnds->ext.h, bnds->ext.w);
}

inline uint8_t display_row2page(uint8_t row) {
   return row / SSD1306_PAGE_HEIGHT;
}

#endif
