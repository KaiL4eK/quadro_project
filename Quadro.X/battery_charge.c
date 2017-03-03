/*
 * File:   battery_charge.c
 * Author: alex
 *
 * Created on February 4, 2017, 4:12 PM
 */

#define BATTERY_CHANNEL 5
// Connection to AN5

#include <xc.h>
#include <stdint.h>

int battery_charge_initialize ( void ) 
{
    AD1CON1bits.ADON    = 0;
    
    AD1PCFGL &= ~(1 << BATTERY_CHANNEL); // Analog mode pin setup
    
    AD1CON1bits.ASAM    = 1;        // Auto sample
    AD1CON1bits.SSRC    = 0b111;    // Auto convertion
    AD1CON1bits.AD12B   = 1;       // 0 = 10 bit ADC; 1 = 12 bit ADC
    AD1CON2             = 0;	
    AD1CON3bits.SAMC    = 0b11111;	     // Sample time 
    AD1CON3bits.ADCS    = 0b11111111;      // Conversion clock select 	         	
	AD1CHS0             = 5;
    AD1CON1bits.ADON    = 1;
    
    return 0;
}

static uint16_t    raw_battery_voltage = 0;
static const float translate_rate      = 0.064;

// 16 V == 2490 ADC
// 14 V == 2180 ADC

uint16_t battery_charge_get_voltage_x10 ( void )
{
    return raw_battery_voltage * translate_rate;
}

static float filter_value = 0.0;

void battery_charge_set_filter_value ( float value )
{
    filter_value = value;
}

void battery_charge_read_value ( void )
{
    if ( AD1CON1bits.DONE ) {
        AD1CON1bits.DONE    = 0;            // reset DONE bit
        
        raw_battery_voltage = filter_value * raw_battery_voltage + (1 - filter_value) * ADC1BUF0;
    }
}

