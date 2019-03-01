/* SPI test */

#include <avr/io.h>
#include <util/delay.h>
#include "../AVR-Programming/AVR-Programming-Library/pinDefines.h"
#include "spi_test.h"

/* spi_writeb: write single byte to SPI */
void spi_writeb(uint8_t data) {
   SPDR = data;
   //while (!(SPSR & (1 << SPIF))) {}
   loop_until_bit_is_set(SPSR, SPIF);
}

/* spi_write: write buffer using SPI */
void spi_write(const uint8_t *buf, unsigned int len,
               const struct display_pins *pins) {
   const uint8_t *end = buf + len;
   for (const uint8_t *it = buf; it < end; ++it) {
      spi_writeb(*it);
   }
}

void display_config(const struct display_pins *pins) {
   /* directions */
   OUTPUT(pins->mosi_bank, pins->mosi_mask);
   OUTPUT(pins->clk_bank, pins->clk_mask);
   OUTPUT(pins->cs_bank, pins->cs_mask);
   OUTPUT(pins->dc_bank, pins->dc_mask);
   OUTPUT(pins->rst_bank, pins->rst_mask);

   SPI_SS_DDR |= (1 << SPI_SS);
   SPI_SS_PORT |= (1 << SPI_SS);
   SPI_MOSI_DDR |= (1 << SPI_MOSI);
   SPI_MISO_PORT |= (1 << SPI_MISO);
   SPI_SCK_DDR |= (1 << SPI_SCK);
   SPCR |= (1 << SPR1);
   SPCR |= (1 << MSTR);
   SPCR |= (1 << SPE);
   SPI_SCK_PORT &= ~(1 << SPI_SCK);
   
   /* set output values */
   LOW(pins->clk_bank, pins->clk_mask);

   /* reset */
   HIGH(pins->rst_bank, pins->rst_mask);
   _delay_ms(1);
   LOW(pins->rst_bank, pins->rst_mask);
   _delay_ms(10);
   HIGH(pins->rst_bank, pins->rst_mask);
}

void display_init(const struct display_pins *pins) {
   SELECT(pins->cs_bank, pins->cs_mask);
   COMMAND(pins->dc_bank, pins->dc_mask);
   
   /* write commands */
   uint8_t cmds[] = {SSD1306_DISPLAYOFF,
                     SSD1306_SETDISPLAYCLOCKDIV, 0x80,
                     SSD1306_SETMULTIPLEX, 0x3F - 1,
                     SSD1306_SETDISPLAYOFFSET, 0x0,
                     SSD1306_SETSTARTLINE | 0x0,
                     SSD1306_CHARGEPUMP, 0x14, // internal (?)
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

   DESELECT(pins->cs_bank, pins->cs_mask);
}

void display_checkerboard(const struct display_pins *pins) {
   SELECT(pins->cs_bank, pins->cs_mask);

   /* commands: set cursor to (0,0) */
   COMMAND(pins->dc_bank, pins->dc_mask);
   const uint8_t cmds[] = {SSD1306_PAGEADDR, 0x0, 0xFF,
                           SSD1306_COLUMNADDR, 0x0, DISPLAY_WIDTH - 1};
   spi_write(cmds, LEN(cmds), pins);

   /* data: checkerboard */
   DATA(pins->dc_bank, pins->dc_mask);
   uint16_t count = DISPLAY_WIDTH * DISPLAY_HEIGHT / 8;
   while (count--) {
      spi_writeb(0xaa);
   }
   
   DESELECT(pins->cs_bank, pins->cs_mask);
}

void display_clear(uint8_t pix, const struct display_pins *pins) {
   SELECT(pins->cs_bank, pins->cs_mask);
   COMMAND(pins->dc_bank, pins->dc_mask);
   const uint8_t cmds[] = {SSD1306_PAGEADDR, 0x0, 0xFF,
                           SSD1306_COLUMNADDR, 0x0, DISPLAY_WIDTH-1};
   spi_write(cmds, LEN(cmds), pins);

   DATA(pins->dc_bank, pins->dc_mask);
   uint16_t count = DISPLAY_WIDTH * DISPLAY_HEIGHT / 8;
   while (count--) {
      spi_writeb(pix);
   }

   DESELECT(pins->cs_bank, pins->cs_mask);
}

int main(void) {
   /* test */
   struct display_pins pins = {.mosi_bank = BANKB, .mosi_mask = 0x01,
                               .clk_bank = BANKD, .clk_mask = 0x80,
                               .dc_bank = BANKD, .dc_mask = 0x40,
                               .rst_bank = BANKD, .rst_mask = 0x20,
                               .cs_bank = BANKB, .cs_mask = 0x04};

   display_config(&pins);
   display_init(&pins);
   //display_checkerboard(&pins);
   display_clear(0x00, &pins);

   SELECT(pins.cs_bank, pins.cs_mask);
   COMMAND(pins.dc_bank, pins.dc_mask);
   const uint8_t cmds[] = {SSD1306_PAGEADDR, 0x0, 0xFF,
                           SSD1306_COLUMNADDR, 0x0, DISPLAY_WIDTH-1};
   spi_write(cmds, LEN(cmds), &pins);
   
   DATA(pins.dc_bank, pins.dc_mask);
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
   
   display_clear(0xff, &pins);
   
   return 0;
}
