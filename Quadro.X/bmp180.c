#include "BMP180.h"
#include "per_proto.h"

static uint8_t      buffer[3],
                    measure_mode = 0,
                    init_flag = 0;
static bool         calibration_loaded = false;
int16_t     ac1, ac2, ac3, b1, b2, mc, mb, md; 
uint16_t    ac4, ac5, ac6;
int32_t     B5;

inline void bmp180_load_calibration ( void )
{
    uint8_t c_buffer[22];
    memset( c_buffer, 0, sizeof(c_buffer) );
    i2c_read_bytes_eeprom(BMP085_I2C_ADDRESS, BMP085_RA_AC1_H, c_buffer, 22);
    ac1 = ((int16_t)c_buffer[0] << 8)  | c_buffer[1];
    ac2 = ((int16_t)c_buffer[2] << 8)  | c_buffer[3];
    ac3 = ((int16_t)c_buffer[4] << 8)  | c_buffer[5];
    ac4 = ((uint16_t)c_buffer[6] << 8) | c_buffer[7];
    ac5 = ((uint16_t)c_buffer[8] << 8) | c_buffer[9];
    ac6 = ((uint16_t)c_buffer[10] << 8)| c_buffer[11];
    b1 =  ((int16_t)c_buffer[12] << 8) | c_buffer[13];
    b2 =  ((int16_t)c_buffer[14] << 8) | c_buffer[15];
    mb =  ((int16_t)c_buffer[16] << 8) | c_buffer[17];
    mc =  ((int16_t)c_buffer[18] << 8) | c_buffer[19];
    md =  ((int16_t)c_buffer[20] << 8) | c_buffer[21];

    calibration_loaded = true;
}

int8_t bmp180_init ( uint8_t oversampling )
{
    if ( oversampling >= BMP085_ULTRALOWPOWER && oversampling <= BMP085_ULTRAHIGHRES )
        measure_mode = oversampling;
    else
        return( -1 );
    
    if ( bmp180_get_id() != 0x55 )
        return( -1 );
    
    bmp180_load_calibration();
    bmp180_send_temperature_signal();
    
    init_flag = 1;
    return( 0 );
}

int bmp180_rcv_press_temp_data( uint32_t *out_pressure, uint32_t *out_temp )
{
    static BMP180_Stage_t       bmp_rcv_data_stage = TEMP;
    static uint32_t             read_temp = 0,
                                read_press = 0;
    
    int                         res = 0;
    
    switch ( bmp_rcv_data_stage )
    {
        case TEMP:
            if ( bmp180_data_ready() )
            {
                read_temp = bmp180_read_temperature_C() * TEMP_MULTIPLYER;
                bmp180_send_pressure_signal();
                bmp_rcv_data_stage = PRESS;
            }
            break;
        case PRESS:
            if ( bmp180_data_ready() )
            {
                read_press = bmp180_read_pressure();
                bmp180_send_temperature_signal();
                bmp_rcv_data_stage = TEMP;
                res = 1;
            }
            break;
    }
    
    if ( out_pressure != NULL )
        *out_pressure = read_press;
    
    if ( out_temp != NULL )
        *out_temp = read_temp;
    return( res );
}

#define CALIBRATION_READINGS_AMOUNT 50
void bmp180_calibrate ( uint32_t *out_pressure )
{
    uint64_t    buffer_press = 0;
    uint32_t    tmp_press = 0;
    uint16_t    iter = 0;
    
    for ( iter = 0; iter < CALIBRATION_READINGS_AMOUNT; )
    {
        if ( bmp180_rcv_press_temp_data( &tmp_press, NULL ) )
        {
            buffer_press += tmp_press;
            iter++;
        }
    }
    
    if ( out_pressure != NULL )
        *out_pressure = buffer_press/CALIBRATION_READINGS_AMOUNT;
}

uint8_t bmp180_get_id ( void )
{
    return ( i2c_read_byte_eeprom( BMP085_I2C_ADDRESS, BMP085_RA_ID ) );
}

/* control register methods */

static uint8_t get_control ( void ) 
{
    i2c_read_bytes_eeprom(BMP085_I2C_ADDRESS, BMP085_RA_CONTROL, buffer, 1);
    return( buffer[0] );
}

static void set_control ( uint8_t value ) 
{
    i2c_write_byte_eeprom(BMP085_I2C_ADDRESS, BMP085_RA_CONTROL, value);
}

static uint16_t get_measurement_2 ( void ) 
{
    i2c_read_bytes_eeprom(BMP085_I2C_ADDRESS, BMP085_RA_MSB, buffer, 2);
    return ((uint16_t)buffer[0] << 8) | buffer[1];
}

static uint32_t get_measurement_3 ( void ) 
{
    i2c_read_bytes_eeprom(BMP085_I2C_ADDRESS, BMP085_RA_MSB, buffer, 3);
    return ((uint32_t)buffer[0] << 16) | ((uint16_t)buffer[1] << 8) | buffer[2];
}

static void delay_temperature ( void ) 
{
#ifdef DELAY_US_
    delay_us( 4500 );
#else
    delay_ms( 5 );
#endif
}

static void delay_pressure ( void ) 
{
    switch( measure_mode )
    {
        case BMP085_ULTRALOWPOWER:
#ifdef DELAY_US_
            delay_us( 4500 );
#else
            delay_ms( 5 );
#endif
            break;
        case BMP085_STANDARD:
#ifdef DELAY_US_
            delay_us( 7500 );
#else
            delay_ms( 8 );
#endif
            break;
        case BMP085_HIGHRES:
#ifdef DELAY_US_
            delay_us( 13500 );
#else
            delay_ms( 14 );
#endif
            break;
        case BMP085_ULTRAHIGHRES:
#ifdef DELAY_US_
            delay_us( 25500 );
#else
            delay_ms( 26 );
#endif
            break;
        default:
#ifdef DELAY_US_
            delay_us( 25500 );
#else
            delay_ms( 26 );
#endif
    }
}

static int32_t compute_B5 ( int32_t UT ) 
{
  int32_t X1 = (UT - (int32_t)ac6) * ((int32_t)ac5) >> 15;
  int32_t X2 = ((int32_t)mc << 11) / (X1+(int32_t)md);
  return( X1 + X2 );
}

/* Special API */

void bmp180_send_temperature_signal ( void )
{
    set_control( BMP085_MODE_TEMPERATURE );
}

void bmp180_send_pressure_signal ( void )
{
    set_control( BMP085_MODE_PRESSURE | (measure_mode << 6) );
}

uint8_t bmp180_data_ready ( void )
{
    if ( !init_flag )
        return( 0 );
    return( !((get_control() >> 5) & 0x1) );
}

float bmp180_read_temperature_C ( void ) 
{
    int32_t ut = get_measurement_2();
    B5 = compute_B5( ut );
    return( (float)((B5 + 8) >> 4) / 10.0f );
}

uint32_t bmp180_read_pressure ( void )
{
    int32_t up = (get_measurement_3() >> (8 - measure_mode)),
            p, B3, B6, X1, X2, X3;
    uint8_t oss = measure_mode;
    B6 = B5 - 4000;
    X1 = ((int32_t)b2 * ((B6 * B6) >> 12)) >> 11;
    X2 = ((int32_t)ac2 * B6) >> 11;
    X3 = X1 + X2;
    B3 = ((((int32_t)ac1 * 4 + X3) << oss) + 2) >> 2;
    X1 = ((int32_t)ac3 * B6) >> 13;
    X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    uint32_t B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
    uint32_t B7 = ((uint32_t)up - B3) * (uint32_t)(50000UL >> oss);
    if (B7 < 0x80000000) {
        p = (B7 << 1) / B4;
    } else {
        p = (B7 / B4) << 1;
    }
    X1 = (p >> 8) * (p >> 8);
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * p) >> 16;
    return( p + ((X1 + X2 + (int32_t)3791) >> 4) );
}
/* ----------------------- */

uint16_t bmp180_get_raw_temperature ( void ) 
{
    set_control(BMP085_MODE_TEMPERATURE);
    delay_temperature();    
    return( get_measurement_2() );
}

float bmp180_get_temperature_C ( void )
{
    /*
    Datasheet formula:
        UT = raw temperature
        X1 = (UT - AC6) * AC5 / 2^15
        X2 = MC * 2^11 / (X1 + MD)
        B5 = X1 + X2
        T = (B5 + 8) / 2^4
    */
    int32_t ut = bmp180_get_raw_temperature();
    B5 = compute_B5( ut );
    return( (float)((B5 + 8) >> 4) / 10.0f );
}

float bmp180_get_temperature_F ( void ) 
{
    return( bmp180_get_temperature_C() * 9.0f / 5.0f + 32 );
}

uint32_t bmp180_get_raw_pressure ( void ) 
{
    set_control(BMP085_MODE_PRESSURE | (measure_mode << 6));
    delay_pressure();
    return( get_measurement_3() >> (8 - measure_mode) );
}

uint32_t bmp180_get_pressure ( void )
{
    /*
    Datasheet forumla
        UP = raw pressure
        B6 = B5 - 4000
        X1 = (B2 * (B6 * B6 / 2^12)) / 2^11
        X2 = AC2 * B6 / 2^11
        X3 = X1 + X2
        B3 = ((AC1 * 4 + X3) << oss + 2) / 4
        X1 = AC3 * B6 / 2^13
        X2 = (B1 * (B6 * B6 / 2^12)) / 2^16
        X3 = ((X1 + X2) + 2) / 2^2
        B4 = AC4 * (unsigned long)(X3 + 32768) / 2^15
        B7 = ((unsigned long)UP - B3) * (50000 >> oss)
        if (B7 < 0x80000000) { p = (B7 * 2) / B4 }
        else { p = (B7 / B4) * 2 }
        X1 = (p / 2^8) * (p / 2^8)
        X1 = (X1 * 3038) / 2^16
        X2 = (-7357 * p) / 2^16
        p = p + (X1 + X2 + 3791) / 2^4
    */
    int32_t up = bmp180_get_raw_pressure(),
            p, B3, B6, X1, X2, X3;
    uint8_t oss = measure_mode;
    B6 = B5 - 4000;
    X1 = ((int32_t)b2 * ((B6 * B6) >> 12)) >> 11;
    X2 = ((int32_t)ac2 * B6) >> 11;
    X3 = X1 + X2;
    B3 = ((((int32_t)ac1 * 4 + X3) << oss) + 2) >> 2;
    X1 = ((int32_t)ac3 * B6) >> 13;
    X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    uint32_t B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
    uint32_t B7 = ((uint32_t)up - B3) * (uint32_t)(50000UL >> oss);
    if (B7 < 0x80000000) {
        p = (B7 << 1) / B4;
    } else {
        p = (B7 / B4) << 1;
    }
    X1 = (p >> 8) * (p >> 8);
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * p) >> 16;
    return( p + ((X1 + X2 + (int32_t)3791) >> 4) );
}

float bmp180_get_altitude ( uint32_t pressure, float seaLevelPressure )
{
    return( 44330 * (1.0 - pow(pressure / seaLevelPressure, 0.1903)) );
}