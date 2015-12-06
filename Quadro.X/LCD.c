#include "LCD.h"
#include "per_proto.h"

/*** NOT READY ***/

// Program ID
uint8_t program_author[]   = "Donald Weiman";
uint8_t program_version[]  = "LCD-AVR-4d (gcc)";
uint8_t program_date[]     = "Sep 16, 2013";

/******************************* Main Program Code *************************/
void lcd_init(void)
{
// configure the microprocessor pins for the data and command lines
    D7_ddr = D6_ddr = D5_ddr = D4_ddr = E_ddr = RS_ddr = 0;

// initialize the LCD controller as determined by the defines (LCD instructions)
    lcd_init_4d();                                  // initialize the LCD display for a 4-bit interface

// display the first line of information
    lcd_write_string_4d(program_author);

// set cursor to start of second line
    lcd_write_instruction_4d(cmd_SetCursor | cmd_LineTwo);
    delay_us(80);                                  // 40 uS delay (min)

// display the second line of information
    lcd_write_string_4d(program_version);
}
/******************************* End of Main Program Code ******************/

/*============================== 4-bit LCD Functions ======================*/
/*
  Name:     lcd_init_4d
  Purpose:  initialize the LCD module for a 4-bit data interface
  Entry:    equates (LCD instructions) set up for the desired operation
  Exit:     no parameters
  Notes:    uses time delays rather than checking the busy flag
*/
void lcd_init_4d(void)
{
// Power-up delay
    delay_ms(100);                                 // initial 40 mSec delay

// IMPORTANT - At this point the LCD module is in the 8-bit mode and it is expecting to receive  
//   8 bits of data, one bit on each of its 8 data lines, each time the 'E' line is pulsed.
//
// Since the LCD module is wired for the 4-bit mode, only the upper four data lines are connected to 
//   the microprocessor and the lower four data lines are typically left open.  Therefore, when 
//   the 'E' line is pulsed, the LCD controller will read whatever data has been set up on the upper 
//   four data lines and the lower four data lines will be high (due to internal pull-up circuitry).
//
// Fortunately the 'FunctionReset' instruction does not care about what is on the lower four bits so  
//   this instruction can be sent on just the four available data lines and it will be interpreted 
//   properly by the LCD controller.  The 'lcd_write_4' subroutine will accomplish this if the 
//   control lines have previously been configured properly.

// Set up the RS and E lines for the 'lcd_write_4' subroutine.
    RS_bit = 0;
    E_bit = 0;

// Reset the LCD controller
    lcd_write_4(cmd_FunctionReset);                 // first part of reset sequence
    delay_us(10);                                  // 4.1 mS delay (min)

    lcd_write_4(cmd_FunctionReset);                 // second part of reset sequence
    delay_us(200);                                 // 100uS delay (min)

    lcd_write_4(cmd_FunctionReset);                 // third part of reset sequence
    delay_us(200);                                 // this delay is omitted in the data sheet

// Preliminary Function Set instruction - used only to set the 4-bit mode.
// The number of lines or the font cannot be set at this time since the controller is still in the
//  8-bit mode, but the data transfer mode can be changed since this parameter is determined by one 
//  of the upper four bits of the instruction.
 
    lcd_write_4(cmd_FunctionSet4bit);               // set 4-bit mode
    delay_us(80);                                  // 40uS delay (min)

// Function Set instruction
    lcd_write_instruction_4d(cmd_FunctionSet4bit);   // set mode, lines, and font
    delay_us(80);                                  // 40uS delay (min)

// The next three instructions are specified in the data sheet as part of the initialization routine, 
//  so it is a good idea (but probably not necessary) to do them just as specified and then redo them 
//  later if the application requires a different configuration.

// Display On/Off Control instruction
    lcd_write_instruction_4d(cmd_DisplayOff);        // turn display OFF
    delay_us(80);                                  // 40uS delay (min)

// Clear Display instruction
    lcd_write_instruction_4d(cmd_Clear);             // clear display RAM
    delay_us(4);                                   // 1.64 mS delay (min)

// ; Entry Mode Set instruction
    lcd_write_instruction_4d(cmd_EntryMode);         // set desired shift characteristics
    delay_us(80);                                  // 40uS delay (min)

// This is the end of the LCD controller initialization as specified in the data sheet, but the display
//  has been left in the OFF condition.  This is a good time to turn the display back ON.
 
// Display On/Off Control instruction
    lcd_write_instruction_4d(cmd_DisplayOn);         // turn the display ON
    delay_us(80);                                  // 40uS delay (min)
}

/*...........................................................................
  Name:     lcd_write_string_4d
; Purpose:  display a string of characters on the LCD
  Entry:    (theString) is the string to be displayed
  Exit:     no parameters
  Notes:    uses time delays rather than checking the busy flag
*/
void lcd_write_string_4d(uint8_t theString[])
{
    volatile int i = 0;                             // character counter*/
    while (theString[i] != 0)
    {
        lcd_write_character_4d(theString[i]);
        i++;
        delay_us(80);                              // 40 uS delay (min)
    }
}

/*...........................................................................
  Name:     lcd_write_character_4d
  Purpose:  send a byte of information to the LCD data register
  Entry:    (theData) is the information to be sent to the data register
  Exit:     no parameters
  Notes:    does not deal with RW (busy flag is not implemented)
*/

void lcd_write_character_4d(uint8_t theData)
{
    RS_bit = 1;
    E_bit = 0;
    lcd_write_4(theData);                           // write the upper 4-bits of the data
    lcd_write_4(theData << 4);                      // write the lower 4-bits of the data
}

/*...........................................................................
  Name:     lcd_write_instruction_4d
  Purpose:  send a byte of information to the LCD instruction register
  Entry:    (theInstruction) is the information to be sent to the instruction register
  Exit:     no parameters
  Notes:    does not deal with RW (busy flag is not implemented)
*/
void lcd_write_instruction_4d(uint8_t theInstruction)
{
    RS_bit = 0;                                     
    E_bit = 0;
    lcd_write_4(theInstruction); 
    lcd_write_4(theInstruction << 4);
}


/*...........................................................................
  Name:     lcd_write_4
  Purpose:  send a byte of information to the LCD module
  Entry:    (theByte) is the information to be sent to the desired LCD register
            RS is configured for the desired LCD register
            E is low
            RW is low
  Exit:     no parameters
  Notes:    use either time delays or the busy flag
*/
void lcd_write_4(uint8_t theByte)
{
    D7_bit = 0;
    if (theByte & 1<<7) 
        D7_bit = 1;
            
    D6_bit = 0;
    if (theByte & 1<<6) 
        D6_bit = 1;

    D5_bit = 0;
    if (theByte & 1<<5) 
        D5_bit = 1;

    D4_bit = 0;
    if (theByte & 1<<4) 
        D4_bit = 1;

    E_bit = 1;                                      // Enable pin high
    delay_us(1);                                   // implement 'Data set-up time' (80 nS) and 'Enable pulse width' (230 nS)
    E_bit = 0;                                      // Enable pin low
    delay_us(1);                                   // implement 'Data hold time' (10 nS) and 'Enable cycle time' (500 nS)
}
