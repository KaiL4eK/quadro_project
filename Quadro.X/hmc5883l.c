#include "HMC5883L.h"
#include "per_proto.h"

static mag_raw_data_t   raw_magnetic;
static uint8_t          init_flag = 0;
static int16_t          _hmc5883l_mGauss_LSb = 92;
#define GAIN_MULTIPLYER     100L

int hmc5883l_init ( void )
{
    if ( hmc5883l_get_id() != 0x34 )
        return( -1 );
    
    hmc5883l_set_output_rate( HMC5883_OUTPUT_RATE_75 );
    hmc5883l_set_averaged_samples( HMC5883_AVERAGED_SAMPLES_8 );
    hmc5883l_set_magnetic_gain( HMC5883l_MAGGAIN_1_3 );
    hmc5883l_set_continious_operating_mode();
    
    memset( &raw_magnetic, 0, sizeof( raw_magnetic ) );
    
    init_flag = 1;
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
    UART_write_string( "#M:%05d,%05d,%05d\n",
                    raw_magnetic.value.x_magnet, raw_magnetic.value.y_magnet, raw_magnetic.value.z_magnet );
}

#define SWAP( x, y ) { uint8_t tmp = x; x = y; y = tmp; }

int8_t hmc5883l_receive_mag_raw_data ( void )
{
    if ( !init_flag )
        return( -1 );
    
    if ( i2c_read_bytes_eeprom( HMC5883_ADDRESS, HMC5883_REGISTER_MAG_OUT_X_H_M, (uint8_t *)(&raw_magnetic), 6 ) != 0 )
    {
        return( -1 );
    }
    
    SWAP (raw_magnetic.reg.x_mag_h, raw_magnetic.reg.x_mag_l);
    SWAP (raw_magnetic.reg.y_mag_h, raw_magnetic.reg.y_mag_l);
    SWAP (raw_magnetic.reg.z_mag_h, raw_magnetic.reg.z_mag_l);
    
    return( 0 );
}

void hmc5883l_get_scaled_mag_data ( magnetic_data_t *out_mag_data )
{
    mag_raw_data_t tmp_count_mag_data;
    memcpy( &tmp_count_mag_data, &raw_magnetic, sizeof( tmp_count_mag_data ) );
    
    out_mag_data->x_magnet = tmp_count_mag_data.value.x_magnet * _hmc5883l_mGauss_LSb / GAIN_MULTIPLYER;
    out_mag_data->y_magnet = tmp_count_mag_data.value.y_magnet * _hmc5883l_mGauss_LSb / GAIN_MULTIPLYER;
    out_mag_data->z_magnet = tmp_count_mag_data.value.z_magnet * _hmc5883l_mGauss_LSb / GAIN_MULTIPLYER;
}
