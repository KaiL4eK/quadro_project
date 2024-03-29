#ifndef AD7705_H_
#define	AD7705_H_

#include "core.h"

#define nDRDY_PIN_DIR    _TRISC4
#define nRESET_PIN_DIR   _TRISC3

#define nDRDY_PIN       _RC4
#define nRESET_PIN      _LATC3

/*
 * SCLK         GND
 * MCLK IN      Vdd
 * MCLK OUT     DIN
 * nCS          DOUT
 * nRESET       nDRDY
 * AN2+         AN2-
 * AN1+         REF IN-
 * AN1-         REF IN+
 */

typedef enum
{
                            // Power on val // Size
    COMMUNICATION = 0b000,                  // 8 
    SETUP = 0b001,          // 0x01         // 8
    CLOCK = 0b010,          // 0x05         // 8
    DATA = 0b011,                           // 16
    TEST = 0b100,           // 0x00         // 8
    OFFSET = 0b110,         // 0x1f4000     // 24
    GAIN = 0b111            // 0x5761ab     // 24

}AD7705_reg;

#define CHANNEL_MASK        0b11

#define CHANNEL_1           0b00
#define CHANNEL_2           0b01

#define SETUP_INITIAL_VAL   0x01
#define CLOCK_INITIAL_VAL   0x05
#define TEST_INITIAL_VAL    0x00
#define OFFSET_INITIAL_VAL  0x1f4000
#define GAIN_INITIAL_VAL    0x5761ab

#define GAIN_MASK           0b111

#define GAIN_VAL_1          0b000
#define GAIN_VAL_2          0b001
#define GAIN_VAL_4          0b010
#define GAIN_VAL_8          0b011
#define GAIN_VAL_16         0b100
#define GAIN_VAL_32         0b101
#define GAIN_VAL_64         0b110
#define GAIN_VAL_128        0b111

// Communication register |!DRDY|RS2|RS1|RS0|R/!W|STBY|CH1|CHO|

typedef union
{
    uint8_t val;
    struct
    {
        unsigned CH:2;
        unsigned STBY:1;
        unsigned RW:1;
        unsigned RS:3;
        unsigned nDRDY:1;
    }bits;
}Communication_reg_t;

#define READ_BIT    1
#define WRITE_BIT   0

// Setup register |MD1|MD0|G2|G1|G0|!B/U|BUF|FSYNC|

typedef union
{
    uint8_t val;
    struct
    {
        unsigned FSYNC:1;
        unsigned BUF:1;
        unsigned nBU:1;
        unsigned G:3;
        unsigned MD:2;
    }bits;
}Setup_reg_t;

// Clock register |ZERO|ZERO|ZERO|CLKDIS|CLKDIV|CLK|FS1|FS0|
// OUR - Output Update Rate

typedef union
{
    uint8_t val;
    struct
    {
        unsigned FS:2;
        unsigned CLK:1;
        unsigned CLKDIV:1;
        unsigned CLKDIS:1;
        unsigned ZERO:3;
    }bits;
}Clock_reg_t;

typedef enum
{
    INCORRECT_VAL = -2
}Error_t;

int ad7705_init ( void );
uint32_t ad7705_read_register ( AD7705_reg register_addr, uint8_t channel );
int ad7705_write_register ( AD7705_reg register_addr, uint8_t channel, uint8_t value );
bool ad7705_is_data_ready( void );
uint16_t ad7705_read_data( uint8_t channel );

#endif	/* AD7705_H_ */

