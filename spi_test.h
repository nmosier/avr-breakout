#ifndef __SPI_TEST_H
#define __SPI_TEST_H

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

/* general util macros */
#define LEN(arr) (sizeof(arr) / sizeof(*arr))

/* pin util macros */
#define MMIO_OFFSET 0x20
#define BANKS_BASE (0x03 + MMIO_OFFSET)

#define BANKB      0
#define BANKC      1
#define BANKD      2

#define PINS_OFFSET  0x0
#define DDR_OFFSET  0x1
#define PORT_OFFSET 0x2

/* PORT: given bank, get port address */
#define PINS(bank) ((volatile uint8_t *)                       \
                    (BANKS_BASE + (bank * 3) + PINS_OFFSET))
#define DDR(bank)                                                 \
   ((volatile uint8_t *) (BANKS_BASE + (bank * 3) + DDR_OFFSET))
#define PORT(bank)                                                \
   ((volatile uint8_t *) (BANKS_BASE + (bank * 3) + PORT_OFFSET))

#define OUTPUT(bank, mask) (*DDR(bank) |= mask)
#define INPUT(bank, mask) (*DDR(bank) &= ~mask)

#define HIGH(bank, mask) (*PORT(bank) |= mask)
#define LOW(bank, mask) (*PORT(bank) &= ~mask)

#define SELECT(cs_bank, cs_mask)   LOW(cs_bank, cs_mask)
#define DESELECT(cs_bank, cs_mask) HIGH(cs_bank, cs_mask)

#define COMMAND(dc_bank, dc_mask) LOW(dc_bank, dc_mask)
#define DATA(dc_bank, dc_mask) HIGH(dc_bank, dc_mask)

/* types */
struct display_pins {
   uint8_t mosi_bank, clk_bank, cs_bank, dc_bank, rst_bank;
   uint8_t mosi_mask, clk_mask, cs_mask, dc_mask, rst_mask;
};


#endif
