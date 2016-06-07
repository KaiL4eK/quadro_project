#include "MPU6050_regs.h" 
#include "MPU6050.h"
#include "per_proto.h"

static uint8_t              buffer[14],
                            init_flag = 0;
static gyro_accel_data_t    raw_gyr_acc;

int mpu6050_init ( void )
{
    if ( mpu6050_get_id() != 0x68 )
        return( -1 );

    mpu6050_set_sleep_bit( 0 );
    mpu6050_set_clock_source( MPU6050_CLOCK_PLL_XGYRO );
    mpu6050_set_DLPF( MPU6050_DLPF_BW_20 );
    mpu6050_set_gyro_fullscale( MPU6050_GYRO_FS_250 );
    
    mpu6050_setXAccelOffset(-3874);
    mpu6050_setYAccelOffset(351);
    mpu6050_setZAccelOffset(1693);
    
    mpu6050_setXGyroOffset(116);
    mpu6050_setYGyroOffset(-14);
    mpu6050_setZGyroOffset(-31);
    
    memset( &raw_gyr_acc, 0, sizeof( raw_gyr_acc ) );
    
    init_flag = 1;
    return( 0 );
}

#define SWAP( x, y ) { uint8_t tmp = x; x = y; y = tmp; }

int8_t mpu6050_receive_gyro_accel_raw_data ( void )
{
    if ( !init_flag )
        return( -1 );
    
    if ( i2c_read_bytes_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_ACCEL_XOUT_H, (uint8_t *)(&raw_gyr_acc), 14 ) != 0 )
    {
        return( -1 );
    }

    SWAP (raw_gyr_acc.reg.x_accel_h, raw_gyr_acc.reg.x_accel_l);
    SWAP (raw_gyr_acc.reg.y_accel_h, raw_gyr_acc.reg.y_accel_l);
    SWAP (raw_gyr_acc.reg.z_accel_h, raw_gyr_acc.reg.z_accel_l);
    SWAP (raw_gyr_acc.reg.t_h,       raw_gyr_acc.reg.t_l);
    SWAP (raw_gyr_acc.reg.x_gyro_h,  raw_gyr_acc.reg.x_gyro_l);
    SWAP (raw_gyr_acc.reg.y_gyro_h,  raw_gyr_acc.reg.y_gyro_l);
    SWAP (raw_gyr_acc.reg.z_gyro_h,  raw_gyr_acc.reg.z_gyro_l);
    return( 0 );
}

void send_UART_mpu6050_data ( void )
{
    UART_write_string( UARTm1, "#G:%05d,%05d,%05d#A:%05d,%05d,%05d\n\r",
                raw_gyr_acc.value.x_gyro, 
                raw_gyr_acc.value.y_gyro, 
                raw_gyr_acc.value.z_gyro,
                raw_gyr_acc.value.x_accel, 
                raw_gyr_acc.value.y_accel, 
                raw_gyr_acc.value.z_accel );  
}

void mpu6050_get_gyro_accel_raw_data ( gyro_accel_data_t *out_gyr_acc_data )
{
    memcpy( out_gyr_acc_data, &raw_gyr_acc, sizeof(raw_gyr_acc) );
}

uint8_t mpu6050_get_id ( void )
{
    return( i2c_read_byte_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_WHO_AM_I ) );
}

void mpu6050_set_sleep_bit ( uint8_t value )
{
    i2c_write_bits_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_PWR_MGMT_1, 
            MPU6050_PWR1_SLEEP_BIT, 1, value );
}

void mpu6050_set_clock_source ( uint8_t value )
{
    i2c_write_bits_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_PWR_MGMT_1, 
            MPU6050_PWR1_CLKSEL_BIT, MPU6050_PWR1_CLKSEL_LENGTH, value );
}

void mpu6050_set_DLPF ( uint8_t value )
{
    i2c_write_bits_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_CONFIG, 
            MPU6050_CFG_DLPF_CFG_BIT, MPU6050_CFG_DLPF_CFG_LENGTH, value );
}

void mpu6050_set_gyro_fullscale ( uint8_t value )
{
    i2c_write_bits_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_GYRO_CONFIG,
            MPU6050_GCONFIG_FS_SEL_BIT, MPU6050_GCONFIG_FS_SEL_LENGTH, value );
}

void mpu6050_set_accel_fullscale ( uint8_t value )
{
    i2c_write_bits_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_ACCEL_CONFIG, 
            MPU6050_ACONFIG_AFS_SEL_BIT, MPU6050_ACONFIG_AFS_SEL_LENGTH, value );
}

void mpu6050_set_interrupt_data_rdy_bit ( uint8_t value )
{
    i2c_write_bits_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_INT_ENABLE, 
            MPU6050_INTERRUPT_DATA_RDY_BIT, 1, value );
}

void mpu6050_set_sample_rate_divider ( uint8_t value )
{
    i2c_write_byte_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_SMPLRT_DIV, value );
}

// XA_OFFS_* registers

int16_t MPU6050_getXAccelOffset ( void ) 
{
    i2c_read_bytes_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_XA_OFFS_H, buffer, 2);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
void mpu6050_setXAccelOffset(int16_t offset) {
    i2c_write_word_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_XA_OFFS_H, offset);
}

// YA_OFFS_* register

int16_t MPU6050_getYAccelOffset() {
    i2c_read_bytes_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_YA_OFFS_H, buffer, 2);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
void mpu6050_setYAccelOffset ( int16_t offset ) 
{
    i2c_write_word_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_YA_OFFS_H, offset);
}

// ZA_OFFS_* register

int16_t MPU6050_getZAccelOffset ( void ) 
{
    i2c_read_bytes_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_ZA_OFFS_H, buffer, 2);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
void mpu6050_setZAccelOffset ( int16_t offset ) 
{
    i2c_write_word_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_ZA_OFFS_H, offset);
}

// XG_OFFS_USR* registers

int16_t MPU6050_getXGyroOffset ( void ) 
{
    i2c_read_bytes_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_XG_OFFS_USRH, buffer, 2);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
void mpu6050_setXGyroOffset ( int16_t offset ) 
{
    i2c_write_word_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_XG_OFFS_USRH, offset);
}

// YG_OFFS_USR* register

int16_t MPU6050_getYGyroOffset ( void ) 
{
    i2c_read_bytes_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_YG_OFFS_USRH, buffer, 2);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
void mpu6050_setYGyroOffset ( int16_t offset ) 
{
    i2c_write_word_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_YG_OFFS_USRH, offset);
}

// ZG_OFFS_USR* register

int16_t MPU6050_getZGyroOffset ( void ) 
{
    i2c_read_bytes_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_ZG_OFFS_USRH, buffer, 2);
    return (((int16_t)buffer[0]) << 8) | buffer[1];
}
void mpu6050_setZGyroOffset ( int16_t offset ) 
{
    i2c_write_word_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_ZG_OFFS_USRH, offset);
}

///////////////////////////////////   CONFIGURATION   /////////////////////////////
//Change this 3 variables if you want to fine tune the skecth to your needs.
uint16_t buffersize=1000;     //Amount of readings used to average, make it higher to get more precision but sketch will be slower  (default:1000)
uint16_t acel_deadzone=8;     //Acelerometer error allowed, make it lower to get more precision, but sketch may not converge  (default:8)
uint16_t giro_deadzone=1;     //Giro error allowed, make it lower to get more precision, but sketch may not converge  (default:1)

int32_t mean_ax,mean_ay,mean_az,mean_gx,mean_gy,mean_gz,state=0;
int32_t ax_offset,ay_offset,az_offset,gx_offset,gy_offset,gz_offset;

static void mean_sensors ( void )
{
    int64_t i=0,buff_ax=0,buff_ay=0,buff_az=0,buff_gx=0,buff_gy=0,buff_gz=0;
    while (i<(buffersize+101)){
        mpu6050_receive_gyro_accel_raw_data(  );
        if (i>100 && i<=(buffersize+100)){ //First 100 measures are discarded
            buff_ax=buff_ax+raw_gyr_acc.value.x_accel;
            buff_ay=buff_ay+raw_gyr_acc.value.y_accel;
            buff_az=buff_az+raw_gyr_acc.value.z_accel;
            buff_gx=buff_gx+raw_gyr_acc.value.x_gyro;
            buff_gy=buff_gy+raw_gyr_acc.value.y_gyro;
            buff_gz=buff_gz+raw_gyr_acc.value.z_gyro;
        }
        if (i==(buffersize+100)){
            mean_ax=buff_ax/buffersize;
            mean_ay=buff_ay/buffersize;
            mean_az=buff_az/buffersize;
            mean_gx=buff_gx/buffersize;
            mean_gy=buff_gy/buffersize;
            mean_gz=buff_gz/buffersize;
        }
        i++;
        delay_us(1000); //Needed so we don't get repeated measures
    }
}

void mpu6050_calibration ( void )
{
    mpu6050_setXAccelOffset(0);
    mpu6050_setYAccelOffset(0);
    mpu6050_setZAccelOffset(0);
    mpu6050_setXGyroOffset(0);
    mpu6050_setYGyroOffset(0);
    mpu6050_setZGyroOffset(0);
    UART_write_string( UARTm1, "\nReading sensors for first time...\n" );
    mean_sensors();
    UART_write_string( UARTm1, "\nCalculating offsets...\n" );
    {
        ax_offset=-mean_ax/8;
        ay_offset=-mean_ay/8;
        az_offset=(16384-mean_az)/8;

        gx_offset=-mean_gx/4;
        gy_offset=-mean_gy/4;
        gz_offset=-mean_gz/4;
        while (1){
            send_UART_mpu6050_data();
            int ready=0;
            mpu6050_setXAccelOffset(ax_offset);
            mpu6050_setYAccelOffset(ay_offset);
            mpu6050_setZAccelOffset(az_offset);
            mpu6050_setXGyroOffset(gx_offset);
            mpu6050_setYGyroOffset(gy_offset);
            mpu6050_setZGyroOffset(gz_offset);
            
            mean_sensors();

            if (abs(mean_ax)<=acel_deadzone) ready++;
            else ax_offset=ax_offset-mean_ax/acel_deadzone;

            if (abs(mean_ay)<=acel_deadzone) ready++;
            else ay_offset=ay_offset-mean_ay/acel_deadzone;

            if (abs(16384-mean_az)<=acel_deadzone) ready++;
            else az_offset=az_offset+(16384-mean_az)/acel_deadzone;

            if (abs(mean_gx)<=giro_deadzone) ready++;
            else gx_offset=gx_offset-mean_gx/(giro_deadzone+1);

            if (abs(mean_gy)<=giro_deadzone) ready++;
            else gy_offset=gy_offset-mean_gy/(giro_deadzone+1);

            if (abs(mean_gz)<=giro_deadzone) ready++;
            else gz_offset=gz_offset-mean_gz/(giro_deadzone+1);

            if (ready==6) break;
        } 
    }
    mean_sensors();
    UART_write_string( UARTm1, "\nFINISHED!\n\r" );
    UART_write_string( UARTm1, "Sensor readings with offsets:\n\t%ld\n\t%ld\n\t%ld\n\t%ld\n\t%ld\n\t%ld\n", mean_ax, mean_ay, mean_az, mean_gx, mean_gy, mean_gz );
    UART_write_string( UARTm1, "Your offsets:\n\t%ld\n\t%ld\n\t%ld\n\t%ld\n\t%ld\n\t%ld\n", ax_offset, ay_offset, az_offset, gx_offset, gy_offset, gz_offset );
    UART_write_string( UARTm1, "\nData is printed as: acelX acelY acelZ giroX giroY giroZ\n" );
    while (1);   
}
