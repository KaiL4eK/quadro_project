#ifndef HX711_H_
#define	HX711_H_

#include <xc.h> // include processor files - each processor file is guarded.  

#define DAT_PIN         _RA14
#define DAT_TRIS_PIN    _TRISA14

#define CLK_PIN         _LATA15
#define CLK_TRIS_PIN    _TRISA15

#define HX711_MULTI         10000
// Proportional coefficient = 3.1472 discr/g
#define HX711_CALIBR_VAL    31472

#define NUM_DATA_TARE 20



int init_hx711 ( void );
int32_t read_tenzo_data ( void );
int32_t read_tared_tenzo_data ( void );
int16_t read_calibrated_tenzo_data ( void );
        
#endif	/* HX711_H_ */

