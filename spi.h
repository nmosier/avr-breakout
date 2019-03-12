/* spi.h
 * Nicholas Mosier 2019
 */

#ifndef __SPI_H
#define __SPI_H

void spi_writeb(uint8_t data);
void spi_write(const uint8_t *buf, unsigned int len);

#endif
