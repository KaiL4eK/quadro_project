#include <stdint.h>
#include "hx711.h"
#include "per_proto.h"

int32_t tare_value = 0;

int init_hx711 ( void )
{
    CLK_PIN = 0;
    CLK_TRIS_PIN = 0;
    DAT_TRIS_PIN = 1;
    
    //Receiving tare
    int i = 0;
    uint32_t load_sum = 0;
    for ( i = 0; i < NUM_DATA_TARE; i++ )
    {
        load_sum += read_tenzo_data();
    }
    tare_value = load_sum/NUM_DATA_TARE;
    return( 0 );
}

int32_t read_tenzo_data ( void ) // Max out is 2^24-1 as ADC is 24 bit
{
    int32_t ADC_value = 0;
    
    while ( DAT_PIN );
    
    int i = 0;
    for ( i = 0; i < 24; i++ )
    {
        CLK_PIN = 1;
        ADC_value <<= 1;
        CLK_PIN = 0;
        if ( DAT_PIN )
            ADC_value++;
    }
    
    CLK_PIN = 1;
    ADC_value ^= 0x800000;
    CLK_PIN = 0;
    
    return ( ADC_value );
}

int32_t read_tared_tenzo_data ( void )
{
    return( read_tenzo_data() - tare_value );
}

int16_t read_calibrated_tenzo_data ( void )
{
    return( read_tared_tenzo_data()/HX711_CALIBR_VAL );
}
