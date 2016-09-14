/*
 * File:   LCD.c
 * Author: Alex Devyatkin
 *
 */

#include "LCD.h"
#include "core.h"

// Program ID
char *firstLineTest  = "First Line =)";
char *secondLineTest = "Second Line =(";

static void lcd_write ( uint8_t theByte )
{ 
    D3_bit = theByte & 1 << 3 ? 1 : 0;
    D2_bit = theByte & 1 << 2 ? 1 : 0;
    D1_bit = theByte & 1 << 1 ? 1 : 0;
    D0_bit = theByte & 1 << 0 ? 1 : 0;

    E_bit = 1;
    delay_us( 1 );
    E_bit = 0;
    delay_us( 1 );
}

static void lcd_write_instruction ( uint8_t theInstruction )
{
    RS_bit = 0;                                     
    E_bit = 0;
    lcd_write( theInstruction >> 4 );
    lcd_write( theInstruction );
}

static void lcd_write_character ( uint8_t theData )
{
    RS_bit = 1;
    E_bit = 0;
    lcd_write( theData >> 4 );
    lcd_write( theData );
}

static void lcd_init_internal( void )
{
    delay_ms( 40 );                                 // initial 40 mSec delay

    RS_bit = 0;
    E_bit = 0;
    
    lcd_write( cmd_FunctionReset >> 4 );                 // first part of reset sequence
    delay_ms( 6 );                                  // 4.1 mS delay (min)

    lcd_write( cmd_FunctionReset >> 4 );                 // second part of reset sequence
    delay_us( 300 );                                 // 100uS delay (min)

    lcd_write( cmd_FunctionReset >> 4 );                 // third part of reset sequence
    delay_ms( 2 );                                 // this delay is omitted in the data sheet

    lcd_write( cmd_FunctionSet4bit >> 4 );               // set 4-bit mode
    delay_ms( 2 );                                  // 40uS delay (min)

    lcd_write_instruction( cmd_FunctionSet4bit );   // set mode, lines, and font
    delay_us( 40 );                                  // 40uS delay (min)

    lcd_write_instruction( cmd_DisplayOff );        // turn display OFF
    delay_us( 40 );                                  // 40uS delay (min)

    lcd_clear();

    lcd_write_instruction(cmd_EntryMode);         // set desired shift characteristics
    delay_us( 40 );                                  // 40uS delay (min)

    lcd_write_instruction(cmd_DisplayOn);         // turn the display ON
    delay_us( 40 );                                  // 40uS delay (min)
}

void lcd_write_string ( char *string )
{
    while ( *string )
    {
        lcd_write_character( *string++ );
        delay_us( 40 );                              // 40 uS delay (min)
    }
}

void lcd_init( void )
{
    D3_ddr = D2_ddr = D1_ddr = D0_ddr = E_ddr = RS_ddr = 0;
    D3_bit = D2_bit = D1_bit = D0_bit = E_bit = RS_bit = 0;
    
    lcd_init_internal();

    lcd_write_string( firstLineTest );

    lcd_setLineTwo();

    lcd_write_string( secondLineTest );
}

void lcd_clear ( void )
{
    lcd_write_instruction( cmd_Clear );
    delay_ms( 4 );
}

void lcd_setLineOne ( void )
{
    lcd_write_instruction ( cmd_SetCursor | cmd_LineOne );
    delay_us( 40 );
}

void lcd_setLineTwo ( void )
{
    lcd_write_instruction ( cmd_SetCursor | cmd_LineTwo );
    delay_us( 40 );
}
