#include "HMC5883L.h"

static mag_raw_data_t   raw_magnetic;
static uint8_t          init_flag = 0;
static int16_t          _hmc5883l_mGauss_LSb = 92;

static int16_t          g_offset_x = 0,
                        g_offset_y = 0;
static int32_t          sin_angle_offset_rad = 0,
                        cos_angle_offset_rad = MATH_MULTIPLYER;
                        

int hmc5883l_init ( int16_t offset_x, int16_t offset_y )
{
    if ( hmc5883l_get_id() != 0x34 )
    {
        return( -1 );
    }
    
    hmc5883l_set_output_rate( HMC5883_OUTPUT_RATE_75 );
    hmc5883l_set_averaged_samples( HMC5883_AVERAGED_SAMPLES_2 );
    hmc5883l_set_magnetic_gain( HMC5883l_MAGGAIN_0_88 );
    hmc5883l_set_continious_operating_mode();
    
    g_offset_x = offset_x;
    g_offset_y = offset_y;
    
    memset( &raw_magnetic, 0, sizeof( raw_magnetic ) );
    
    init_flag = 1;
    
    delay_ms( 20 );
    
    int16_t sum_angles = 0;
    int i = 0;
    for ( i = 0; i < INIT_CALIBRATION_SAMPLES; i++ )
    {
        sum_angles += hmc5883l_get_yaw_angle();
    }
    int16_t angle_offset_deg = sum_angles/INIT_CALIBRATION_SAMPLES;
    float angle_offset_rad = (float)angle_offset_deg*DEGREES_TO_RADIANS;
    sin_angle_offset_rad = sin( angle_offset_rad )*MATH_MULTIPLYER;
    cos_angle_offset_rad = cos( angle_offset_rad )*MATH_MULTIPLYER;
    
    return( 0 );
}

uint8_t hmc5883l_get_id ( void )
{
    return( i2c_read_byte_eeprom( HMC5883_ADDRESS, HMC5883_REGISTER_MAG_IRB_REG_M ) );
}

void hmc5883l_set_continious_operating_mode ( void )
{
    i2c_write_byte_eeprom( HMC5883_ADDRESS, HMC5883_REGISTER_MAG_MR_REG_M, HMC5883_OPERATING_MODE_CONTINIOUS );
}

void hmc5883l_set_magnetic_gain ( Hmc5883l_mag_gain_t gain )
{
// Cause register B has just gain configuration it is better write full register
    i2c_write_byte_eeprom( HMC5883_ADDRESS, HMC5883_REGISTER_MAG_CRB_REG_M,
                            (uint8_t)(gain << HMC5883_GAIN_CONFIGURATION_LSb));

    switch(gain)
    {
        case HMC5883l_MAGGAIN_0_88:
          _hmc5883l_mGauss_LSb = 73;
          break;
        case HMC5883l_MAGGAIN_1_3:
          _hmc5883l_mGauss_LSb = 92;
          break;
        case HMC5883l_MAGGAIN_1_9:
          _hmc5883l_mGauss_LSb = 122;
          break;
        case HMC5883l_MAGGAIN_2_5:
          _hmc5883l_mGauss_LSb = 152;
          break;
        case HMC5883l_MAGGAIN_4_0:
          _hmc5883l_mGauss_LSb = 227;
          break;
        case HMC5883l_MAGGAIN_4_7:
          _hmc5883l_mGauss_LSb = 256;
          break;
        case HMC5883l_MAGGAIN_5_6:
          _hmc5883l_mGauss_LSb = 303;
          break;
        case HMC5883l_MAGGAIN_8_1:
          _hmc5883l_mGauss_LSb = 435;
          break;
    } 
}

void hmc5883l_set_output_rate ( Hmc5883l_output_rate_t rate )
{
    i2c_write_bits_eeprom( HMC5883_ADDRESS, HMC5883_REGISTER_MAG_CRA_REG_M, 
                            HMC5883_OUTPUT_RATE_BIT, HMC5883_OUTPUT_RATE_LENGTH, (uint8_t)rate );
}

void hmc5883l_set_averaged_samples ( Hmc5883l_avrg_samples_t avrgd_smpls )
{
    i2c_write_bits_eeprom( HMC5883_ADDRESS, HMC5883_REGISTER_MAG_CRA_REG_M, 
                            HMC5883_AVERAGED_SAMPLES_BIT, HMC5883_AVERAGED_SAMPLES_LENGTH, (uint8_t)avrgd_smpls );
}

void send_UART_magnetic_raw_data ( void )
{
    UART_write_string( UARTm1, "#M:%05d,%05d,%05d\n",
                    raw_magnetic.value.x_magnet, raw_magnetic.value.y_magnet, raw_magnetic.value.z_magnet );
}

#define SWAP( x, y ) { uint8_t tmp = x; x = y; y = tmp; }

int8_t hmc5883l_receive_mag_raw_data ( void )
{
    if ( !init_flag )
    {
        return( -1 );
    }
    
    if ( i2c_read_bytes_eeprom( HMC5883_ADDRESS, HMC5883_REGISTER_MAG_OUT_X_H_M, 6, (uint8_t *)(&raw_magnetic) ) != 0 )
    {
        return( -1 );
    }
    
    SWAP (raw_magnetic.reg.x_mag_h, raw_magnetic.reg.x_mag_l);
    SWAP (raw_magnetic.reg.y_mag_h, raw_magnetic.reg.y_mag_l);
//    SWAP (raw_magnetic.reg.z_mag_h, raw_magnetic.reg.z_mag_l);
    
    raw_magnetic.value.x_magnet -= g_offset_x;
    raw_magnetic.value.y_magnet -= g_offset_y;
    // Inversed for correct direction of axis Z
    raw_magnetic.value.y_magnet *= -1;
    
    return( 0 );
}


int16_t hmc5883l_get_yaw_angle ( void )
{
    if ( !init_flag )
    {
        return( -1 );
    }
    
    hmc5883l_receive_mag_raw_data();
    
    // Don`t multiply on gain because we need relation
    int16_t turned_x = (raw_magnetic.value.x_magnet * cos_angle_offset_rad + 
                        raw_magnetic.value.y_magnet * sin_angle_offset_rad)/MATH_MULTIPLYER,
            turned_y = (raw_magnetic.value.y_magnet * cos_angle_offset_rad - 
                        raw_magnetic.value.x_magnet * sin_angle_offset_rad)/MATH_MULTIPLYER;
    
    return( atan2( turned_y, turned_x ) );
}

void hmc5883l_make_calibration ( uint32_t calibration_times )
{
    int16_t x_mag_max = INT16_MIN,
            x_mag_min = INT16_MAX,
            y_mag_max = INT16_MIN,
            y_mag_min = INT16_MAX;
    
    while ( calibration_times-- )
    {
        hmc5883l_receive_mag_raw_data();
        x_mag_max = max( x_mag_max, raw_magnetic.value.x_magnet );
        x_mag_min = min( x_mag_min, raw_magnetic.value.x_magnet );
        y_mag_max = max( y_mag_max, raw_magnetic.value.y_magnet );
        y_mag_min = min( y_mag_min, raw_magnetic.value.y_magnet );
        delay_ms( 50 );
    }
    
    UART_write_string( UARTm1, "Offse values: x = %d, y = %d\n", 
                        x_mag_max - (x_mag_max-x_mag_min)/2, 
                        y_mag_max - (y_mag_max-y_mag_min)/2 );
    while ( 1 );
}
