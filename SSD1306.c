/* SPI test */

#include <avr/io.h>
#include <util/delay.h>
#include "SSD1306.h"
#include "spi.h"
#include "util.h"

const struct projection g_proj_pix2scrn =
   {.sx = 1,
    .sy = 8};

void display_config() {
   /* config SPI pins*/
   SPI_SS_DDR |= (1 << SPI_SS);
   SPI_SS_PORT |= (1 << SPI_SS);
   SPI_MOSI_DDR |= (1 << SPI_MOSI);
   SPI_MISO_PORT |= (1 << SPI_MISO);
   SPI_SCK_DDR |= (1 << SPI_SCK);
   SPCR |= (1 << SPR1);
   SPCR |= (1 << MSTR);
   SPCR |= (1 << SPE);
   SPI_SCK_PORT &= ~(1 << SPI_SCK);

   /* config SSD1306 pins */
   SSD1306_DC_DDR |= (1 << SSD1306_DC);
   SSD1306_RST_DDR |= (1 << SSD1306_RST);
   
   /* reset display */
   SSD1306_RST_PORT |= (1 << SSD1306_RST);
   _delay_ms(1);
   SSD1306_RST_PORT &= ~(1 << SSD1306_RST);
   _delay_ms(10);
   SSD1306_RST_PORT |= (1 << SSD1306_RST);
}

void display_init() {
   SLAVE_SELECT;
   SSD1306_COMMAND;
   
   /* write commands */
   uint8_t cmds[] = {SSD1306_DISPLAYOFF,
                     SSD1306_SETDISPLAYCLOCKDIV, 0x80,
                     SSD1306_SETMULTIPLEX, 0x3F - 1,
                     SSD1306_SETDISPLAYOFFSET, 0x0,
                     SSD1306_SETSTARTLINE | 0x0,
                     SSD1306_CHARGEPUMP, 0x14,
                     SSD1306_MEMORYMODE, 0x00,
                     SSD1306_SEGREMAP | 0x1,
                     SSD1306_COMSCANDEC,
                     SSD1306_SETCOMPINS, 0x12,
                     SSD1306_SETCONTRAST, 0xCF,
                     SSD1306_SETPRECHARGE, 0xF1,
                     SSD1306_SETVCOMDETECT, 0x40,
                     SSD1306_DISPLAYALLON_RESUME,
                     SSD1306_NORMALDISPLAY,
                     SSD1306_DEACTIVATE_SCROLL,
                     SSD1306_DISPLAYON};
   uint8_t *cmds_end = cmds + LEN(cmds);
   for (uint8_t *cmd_it = cmds; cmd_it < cmds_end; ++cmd_it) {
      spi_writeb(*cmd_it);
   }

   SLAVE_DESELECT;
}

void display_checkerboard() {
   SLAVE_SELECT;

   /* commands: set cursor to (0,0) */
   SSD1306_COMMAND;
   const uint8_t cmds[] = {SSD1306_PAGEADDR, 0x0, 0xFF,
                           SSD1306_COLUMNADDR, 0x0, DISPLAY_WIDTH - 1};
   spi_write(cmds, LEN(cmds));

   /* data: checkerboard */
   SSD1306_DATA;
   uint16_t count = DISPLAY_WIDTH * DISPLAY_HEIGHT / 8;
   while (count--) {
      spi_writeb(0xaa);
   }
   
   SLAVE_DESELECT;
}

void display_clear(uint8_t pix) {
   SLAVE_SELECT;
   SSD1306_COMMAND;
   const uint8_t cmds[] = {SSD1306_PAGEADDR, 0x0, 0xFF,
                           SSD1306_COLUMNADDR, 0x0, DISPLAY_WIDTH-1};
   spi_write(cmds, LEN(cmds));

   SSD1306_DATA;
   uint16_t count = DISPLAY_WIDTH * DISPLAY_HEIGHT / 8;
   while (count--) {
      spi_writeb(pix);
   }

   SLAVE_DESELECT;
}                                  

void display_select(uint8_t page, uint8_t col, uint8_t height,
                    uint8_t width) {
   SSD1306_COMMAND;
   const uint8_t cmds[] = {SSD1306_PAGEADDR, page, page + height - 1,
                           SSD1306_COLUMNADDR, col, col + width - 1};
   spi_write(cmds, LEN(cmds));
}
