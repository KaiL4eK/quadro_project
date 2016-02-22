#ifndef HMC5883L_H_
#define	HMC5883L_H_

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <string.h>
#include "per_proto.h"

#define RADIANS_TO_DEGREES  (180.0f/3.14159)
#define DEGREES_TO_RADIANS  (3.14159/180.0f)
#define GAIN_MULTIPLYER     100L
#define INIT_CALIBRATION_SAMPLES 100

#define MATH_MULTIPLYER     1000L

#define HMC5883_ADDRESS            0x1E

/*  EEPROM registers */

#define HMC5883_REGISTER_MAG_CRA_REG_M             0x00
#define HMC5883_REGISTER_MAG_CRB_REG_M             0x01
#define HMC5883_REGISTER_MAG_MR_REG_M              0x02
#define HMC5883_REGISTER_MAG_OUT_X_H_M             0x03
#define HMC5883_REGISTER_MAG_OUT_X_L_M             0x04
#define HMC5883_REGISTER_MAG_OUT_Z_H_M             0x05
#define HMC5883_REGISTER_MAG_OUT_Z_L_M             0x06
#define HMC5883_REGISTER_MAG_OUT_Y_H_M             0x07
#define HMC5883_REGISTER_MAG_OUT_Y_L_M             0x08
#define HMC5883_REGISTER_MAG_SR_REG_Mg             0x09
#define HMC5883_REGISTER_MAG_IRA_REG_M             0x0A
#define HMC5883_REGISTER_MAG_IRB_REG_M             0x0B
#define HMC5883_REGISTER_MAG_IRC_REG_M             0x0C
#define HMC5883_REGISTER_MAG_TEMP_OUT_H_M          0x31
#define HMC5883_REGISTER_MAG_TEMP_OUT_L_M          0x32

/* Configuration register A */

#define HMC5883_AVERAGED_SAMPLES_BIT            6
#define HMC5883_AVERAGED_SAMPLES_LENGTH         2

typedef enum
{
    HMC5883_AVERAGED_SAMPLES_1      = 0b00, // Default
    HMC5883_AVERAGED_SAMPLES_2      = 0b01,
    HMC5883_AVERAGED_SAMPLES_4      = 0b10,
    HMC5883_AVERAGED_SAMPLES_8      = 0b11
} Hmc5883l_avrg_samples_t;
    
#define HMC5883_OUTPUT_RATE_BIT                 4
#define HMC5883_OUTPUT_RATE_LENGTH              3

typedef enum
{
    HMC5883_OUTPUT_RATE_0_75        = 0b000,  // 0.75 Hz
    HMC5883_OUTPUT_RATE_1_5         = 0b001,  // 1.5 Hz
    HMC5883_OUTPUT_RATE_3           = 0b010,  // 3 Hz
    HMC5883_OUTPUT_RATE_7_5         = 0b011,  // 7.5 Hz - Default
    HMC5883_OUTPUT_RATE_15          = 0b100,  // 15 Hz
    HMC5883_OUTPUT_RATE_30          = 0b101,  // 30 Hz
    HMC5883_OUTPUT_RATE_75          = 0b110   // 75 Hz
} Hmc5883l_output_rate_t;

#define HMC5883_MEASUREMENT_MODE_BIT            1
#define HMC5883_MEASUREMENT_MODE_LENGTH         2

#define HMC5883_MEASUREMENT_MODE_NORMAL         0b00

#define HMC5883_OPERATING_MODE_CONTINIOUS       0b00

/* Configuration register B */

#define HMC5883_GAIN_CONFIGURATION_LSb          5

typedef enum
{
    HMC5883l_MAGGAIN_0_88           = 0b000,  // +/- 0.88
    HMC5883l_MAGGAIN_1_3            = 0b001,  // +/- 1.3 - Default
    HMC5883l_MAGGAIN_1_9            = 0b010,  // +/- 1.9
    HMC5883l_MAGGAIN_2_5            = 0b011,  // +/- 2.5
    HMC5883l_MAGGAIN_4_0            = 0b100,  // +/- 4.0
    HMC5883l_MAGGAIN_4_7            = 0b101,  // +/- 4.7
    HMC5883l_MAGGAIN_5_6            = 0b110,  // +/- 5.6
    HMC5883l_MAGGAIN_8_1            = 0b111   // +/- 8.1
} Hmc5883l_mag_gain_t;	

/* Data types */

typedef union
{
    struct
    {
        uint8_t x_mag_h,
                x_mag_l,
                z_mag_h,
                z_mag_l,
                y_mag_h,
                y_mag_l;

    } reg;
    struct
    {
        int16_t x_magnet,
                z_magnet,
                y_magnet;
    } value;
} mag_raw_data_t;

int hmc5883l_init ( int16_t offset_x, int16_t offset_y );
uint8_t hmc5883l_get_id ( void );
int8_t hmc5883l_receive_mag_raw_data ( void );
int16_t hmc5883l_get_yaw_angle ( void );
void hmc5883l_set_magnetic_gain ( Hmc5883l_mag_gain_t gain );
void send_UART_magnetic_raw_data ( void );
void hmc5883l_set_continious_operating_mode ( void );
void hmc5883l_set_output_rate ( Hmc5883l_output_rate_t rate );
void hmc5883l_set_averaged_samples ( Hmc5883l_avrg_samples_t avrgd_smpls );
void hmc5883l_make_calibration ( uint32_t calibration_times );

#endif	/* HMC5883L_H_ */

