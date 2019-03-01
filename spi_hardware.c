/* SPI test */

#include <avr/io.h>
#include <util/delay.h>
#include "../AVR-Programming/AVR-Programming-Library/pinDefines.h"
#include "spi_test.h"

/* spi_writeb: write single byte to SPI */
void spi_writeb(uint8_t data) {
   SPDR = data;
   while (!(SPSR & (1 << SPIF))) {}
}

/* spi_write: write buffer using SPI */
void spi_write(const uint8_t *buf, unsigned int len) {
   const uint8_t *end = buf + len;
   for (const uint8_t *it = buf; it < end; ++it) {
      spi_writeb(*it);
   }
}

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

static const uint8_t letterE[] = {0b00000000,
                                  0b01111111,
                                  0b01001001,
                                  0b01001001,
                                  0b01001001,
                                  0b00000000,
                                  0b01111111,
                                  0b00010001,
                                  0b00110001,
                                  0b01001110,
                                  0b00000000,
                                  0b01111110,
                                  0b00000001,
                                  0b00011110,
                                  0b00000001,
                                  0b01111110,
                                  0b00000000,
                                  0b01111110,
                                  0b00010001,
                                  0b00010001,
                                  0b01111110,
                                  0b00000000};
                                  

int main(void) {
   display_config();
   display_init();
   display_clear(0x00);

   SLAVE_SELECT;
   SSD1306_COMMAND;
   const uint8_t cmds[] = {SSD1306_PAGEADDR, 0x0, 0xFF,
                           SSD1306_COLUMNADDR, 0x0, DISPLAY_WIDTH-1};
   spi_write(cmds, LEN(cmds));
   
   SSD1306_DATA;
   uint16_t count = DISPLAY_WIDTH;
   uint8_t pix = 0xf0;
   while (count--) {
      if (pix & 0x80) {
         pix = (pix << 1) | 0x01;
      } else {
         pix <<= 1;
      }
      spi_writeb(pix);
   }
   SLAVE_DESELECT;

   SLAVE_SELECT;
   SSD1306_COMMAND;
   const uint8_t cmds2[] = {SSD1306_PAGEADDR, 0x2, 0xFF,
                           SSD1306_COLUMNADDR, 0x0, DISPLAY_WIDTH-1};
   spi_write(cmds2, LEN(cmds));
   
   SSD1306_DATA;
   spi_write(letterE, LEN(letterE));
   SLAVE_DESELECT;
   
   //display_clear(0xff, &pins);
   
   return 0;
}
