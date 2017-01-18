#ifndef BMP180_H_
#define	BMP180_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <xc.h>
#include "per_proto.h"

#define BMP085_I2C_ADDRESS  0x77

#define BMP085_RA_AC1_H     0xAA    /* AC1_H */
#define BMP085_RA_AC1_L     0xAB    /* AC1_L */
#define BMP085_RA_AC2_H     0xAC    /* AC2_H */
#define BMP085_RA_AC2_L     0xAD    /* AC2_L */
#define BMP085_RA_AC3_H     0xAE    /* AC3_H */
#define BMP085_RA_AC3_L     0xAF    /* AC3_L */
#define BMP085_RA_AC4_H     0xB0    /* AC4_H */
#define BMP085_RA_AC4_L     0xB1    /* AC4_L */
#define BMP085_RA_AC5_H     0xB2    /* AC5_H */
#define BMP085_RA_AC5_L     0xB3    /* AC5_L */
#define BMP085_RA_AC6_H     0xB4    /* AC6_H */
#define BMP085_RA_AC6_L     0xB5    /* AC6_L */
#define BMP085_RA_B1_H      0xB6    /* B1_H */
#define BMP085_RA_B1_L      0xB7    /* B1_L */
#define BMP085_RA_B2_H      0xB8    /* B2_H */
#define BMP085_RA_B2_L      0xB9    /* B2_L */
#define BMP085_RA_MB_H      0xBA    /* MB_H */
#define BMP085_RA_MB_L      0xBB    /* MB_L */
#define BMP085_RA_MC_H      0xBC    /* MC_H */
#define BMP085_RA_MC_L      0xBD    /* MC_L */
#define BMP085_RA_MD_H      0xBE    /* MD_H */
#define BMP085_RA_MD_L      0xBF    /* MD_L */
#define BMP085_RA_CONTROL   0xF4    /* CONTROL */
#define BMP085_RA_ID        0xD0    /* ID */
#define BMP085_RA_MSB       0xF6    /* MSB */
#define BMP085_RA_LSB       0xF7    /* LSB */
#define BMP085_RA_XLSB      0xF8    /* XLSB */

#define BMP085_MODE_TEMPERATURE     0x2E
#define BMP085_MODE_PRESSURE        0x34

#define BMP085_ULTRALOWPOWER 0
#define BMP085_STANDARD      1
#define BMP085_HIGHRES       2
#define BMP085_ULTRAHIGHRES  3

typedef enum
{
    TEMP,
    PRESS
}BMP180_Stage_t;

#define TEMP_MULTIPLYER     1000L

uint8_t bmp180_get_id ( void );
int8_t bmp180_init( uint8_t oversampling );
uint16_t bmp180_get_raw_temperature ( void );
float bmp180_get_temperature_C ( void ); //Full protocol function
float bmp180_get_temperature_F ( void );
uint32_t bmp180_get_raw_pressure ( void );
uint32_t bmp180_get_pressure ( void );
float bmp180_get_altitude (uint32_t pressure, float seaLevelPressure);

void bmp180_calibrate ( uint32_t *out_pressure );
int bmp180_rcv_press_temp_data( uint32_t *out_pressure, uint32_t *out_temp );

int8_t bmp180_init ( uint8_t oversampling );

/* Special API */
void bmp180_send_temperature_signal ( void );
void bmp180_send_pressure_signal ( void );
uint8_t bmp180_data_ready ( void );
float bmp180_read_temperature_C ( void ); //Half-part protocol function, just read from register and convert
uint32_t bmp180_read_pressure ( void );

#endif	/* BMP180_H_ */

