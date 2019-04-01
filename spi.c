/* spi.c
 * Nicholas Mosier 2019
 */

#include <avr/io.h>
#include "spi.h"

#define DEBUG 0

#if DEBUG

void spi_writeb(uint8_t data) {}
void spi_write(const uint8_t *buf, unsigned int len) {}

#else

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

#endif
