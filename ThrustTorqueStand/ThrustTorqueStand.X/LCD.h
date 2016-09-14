#ifndef LCD_H_
#define	LCD_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xc.h>

// D7-D4 pins
#define D3_bit      _LATE3
#define D3_ddr      _TRISE3

#define D2_bit      _LATE2
#define D2_ddr      _TRISE2

#define D1_bit      _LATE1
#define D1_ddr      _TRISE1

#define D0_bit      _LATE0
#define D0_ddr      _TRISE0

//Enable pin
#define E_bit       _LATF0
#define E_ddr       _TRISF0

// Register select pin
#define RS_bit      _LATF1
#define RS_ddr      _TRISF1

// LCD module information
#define cmd_LineOne     0x00                    // start of line 1
#define cmd_LineTwo     0x40                    // start of line 2

// LCD instructions
#define cmd_Clear           0b00000001          // replace all characters with ASCII 'space'
#define cmd_Home            0b00000010          // return cursor to first position on first line
#define cmd_EntryMode       0b00000110          // shift cursor from left to right on read/write
#define cmd_DisplayOff      0b00001000          // turn display off
#define cmd_DisplayOn       0b00001100          // display on, cursor off, don't blink character
#define cmd_FunctionReset   0b00110000          // reset the LCD
#define cmd_FunctionSet4bit 0b00101000          // 4-bit data, 2-line display, 5 x 7 font
#define cmd_SetCursor       0b10000000          // set cursor position

#endif	/* LCD_H_ */

