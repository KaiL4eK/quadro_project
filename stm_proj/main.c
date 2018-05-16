#define CORE_DEBUG_ENABLED
#include <core.h>


/*===========================================================================*/
/* System.                                                                   */
/*===========================================================================*/

int16_t pitch_setpoint      = 0;
int16_t roll_setpoint       = 0;
int16_t yaw_setpoint        = 0;


/* Max angular speed = 40 deg/sec (20 deg * 2) */
static const float CONTROL_2_ANGLE_SPEED_RATE   = 2.0f;
/* Max angle = 10 degree (10 deg / 1000 points) */
static const float CONTROL_2_ANGLE_RATE         = 0.01f;

#ifdef TIME_MEASUREMENT_DEBUG
time_measurement_t      control_tm;
#endif

static THD_WORKING_AREA(waControlThread, 256);
static THD_FUNCTION(ControlThread, arg)
{
    arg = arg;

#ifdef TIME_MEASUREMENT_DEBUG
    chTMObjectInit( &control_tm );
#endif

    int16_t pitch_control       = 0;
    int16_t roll_control        = 0;
    int16_t yaw_control         = 0;

    float pitch_rate_setpoint   = 0;
    float roll_rate_setpoint    = 0;
    float yaw_rate_setpoint     = 0;

    systime_t time = chVTGetSystemTimeX();

    while ( 1 )
    {
        /* 2 - 6 us */
#ifdef TIME_MEASUREMENT_DEBUG
        chTMStartMeasurementX( &control_tm );
#endif

        time += MS2ST( CONTROL_SYSTEM_PERIOD_MS );

        /* Values [-1000; 1000] */
        pitch_setpoint      = radio_control_get_angle_value( RC_PITCH );
        roll_setpoint       = radio_control_get_angle_value( RC_ROLL );
        yaw_setpoint        = radio_control_get_angle_value( RC_YAW );


        pitch_rate_setpoint = (pitch_setpoint * CONTROL_2_ANGLE_RATE - euler_angles.pitch) * CONTROL_2_ANGLE_SPEED_RATE;
        pitch_control       = PID_controller_generate_pitch_control( pitch_rate_setpoint - gyro_rates.pitch,
                                                                     gyro_rates.pitch );

        roll_rate_setpoint  = (roll_setpoint * CONTROL_2_ANGLE_RATE - euler_angles.roll) * CONTROL_2_ANGLE_SPEED_RATE;        
        roll_control        = PID_controller_generate_roll_control( roll_rate_setpoint - gyro_rates.roll,
                                                                    gyro_rates.roll );

        yaw_rate_setpoint   = yaw_setpoint * CONTROL_2_ANGLE_SPEED_RATE;        
        yaw_control         = PID_controller_generate_yaw_control( yaw_rate_setpoint - gyro_rates.yaw );

        /* Processing conversion */
        /* Thrust value [0; 500] */
        int32_t motorPower  = radio_control_get_thrust_value();
        int32_t power       = motorPower;
        
        power = motorPower + pitch_control - roll_control - yaw_control;
        motor_powers[MOTOR_1] = clip_value( power, MOTOR_INPUT_MIN, MOTOR_INPUT_MAX );

        power = motorPower + pitch_control + roll_control + yaw_control;
        motor_powers[MOTOR_2] = clip_value( power, MOTOR_INPUT_MIN, MOTOR_INPUT_MAX );

        power = motorPower - pitch_control + roll_control - yaw_control;
        motor_powers[MOTOR_3] = clip_value( power, MOTOR_INPUT_MIN, MOTOR_INPUT_MAX );

        power = motorPower - pitch_control - roll_control + yaw_control;
        motor_powers[MOTOR_4] = clip_value( power, MOTOR_INPUT_MIN, MOTOR_INPUT_MAX );


        motor_control_update_PWM();

#ifdef TIME_MEASUREMENT_DEBUG
        chTMStopMeasurementX( &control_tm );
#endif

//dprintf( "Pitch error: %06d %06d %06d\n", (int)((pitch_rate_setpoint - gyro_rates.pitch)),
//                                                         (int)(pitch_rate_setpoint), (int)(gyro_rates.pitch) );
//
//dprintf( "Rates SPs: %d, %d, %d\n",
//                         (int)(roll_rate_setpoint * INT_ACCURACY_MULTIPLIER),
//                         (int)(pitch_rate_setpoint * INT_ACCURACY_MULTIPLIER),
//                         (int)(yaw_rate_setpoint * INT_ACCURACY_MULTIPLIER) );
//
//dprintf( "Controls: %d, %d, %d\n", roll_control, pitch_control, yaw_control );

        chThdSleepUntil( time );
    }
}

/*===========================================================================*/
/* Application code                                                          */
/*===========================================================================*/

static SerialDriver         *debug_serial = &SD4;
BaseSequentialStream        *debug_stream = NULL;

static const SerialConfig   sdcfg = {
    .speed      = 460800,
    .cr1        = 0,
    .cr2        = USART_CR2_LINEN,
    .cr3        = 0
};

static THD_WORKING_AREA(waSender, 256);
static THD_FUNCTION(Sender, arg)
{
    arg = arg;

    while ( 1 )
    {
        chThdSleepMilliseconds( 500 );

#if 0
        dprintf( "%s / [%d, %d] / Channels: %06d | %06d | %06d | %06d | %06d\n", 
                    radio_control_is_connected() ? " C" : "NC",
#ifdef RC_TIME_MEASUREMENT_DEBUG
                    RTC2US(SYSTEM_FREQUENCY, rc_tm.best), RTC2US(SYSTEM_FREQUENCY, rc_tm.worst),
#else
                    0, 0,
#endif
                    control_input.channels[0],
                    control_input.channels[1],
                    control_input.channels[2],
                    control_input.channels[3],
                    control_input.channels[4] );
#endif

#if 0
        dprintf( "Angls: %06d %06d %06d\n",
                            (int)(euler_angles.roll * INT_ACCURACY_MULTIPLIER),
                            (int)(euler_angles.pitch * INT_ACCURACY_MULTIPLIER),
                            (int)(euler_angles.yaw * INT_ACCURACY_MULTIPLIER) );
#endif


#ifdef TIME_MEASUREMENT_DEBUG
        dprintf( "[Control] Best: %d, Worst: %d\n", RTC2US(SYSTEM_FREQUENCY, control_tm.best), RTC2US(SYSTEM_FREQUENCY, control_tm.worst) );
#endif

    }
}


static THD_WORKING_AREA(waBlinker, 16);
static THD_FUNCTION(Blinker, arg) 
{
    arg = arg;

    systime_t time = chVTGetSystemTimeX();

    while ( true )
    {
        time += MS2ST( 500 );

        palTogglePad( GPIOA, 5 );
        dprintf( "Ping %d\n", chVTGetSystemTimeX() );

        chThdSleepUntil( time );
    }
}

int main(void)
{
    chSysInit();
    halInit();

    init_common_periphery();

    /* Debug serial init */
    palSetPadMode( GPIOA, 0, PAL_MODE_ALTERNATE(8) );    // TX = PA_0
    palSetPadMode( GPIOA, 1, PAL_MODE_ALTERNATE(8) );    // RX = PA_1

    sdStart( debug_serial, &sdcfg );
    debug_stream = (BaseSequentialStream *)debug_serial;
    
    /* Debug threads */
    chThdCreateStatic( waSender, sizeof(waSender), NORMALPRIO-10, Sender, NULL );
    chThdCreateStatic( waBlinker, sizeof(waBlinker), NORMALPRIO-10, Blinker, NULL );

    /* Main modules init */

    /* Radio control init */
    radio_control_init();

#if ( MAIN_PROGRAM_ROUTINE == PROGRAM_ROUTINE_RC_CALIBRATION )
    dprintf( "RC calibration start\n" );
    radio_control_calibration();
    chThdSleepSeconds( 1 );
    chSysHalt( "Calibration mode" );
#endif

    /* AHRS init */
    ahrs_module_init( NORMALPRIO - 2 );

#if ( MAIN_PROGRAM_ROUTINE == PROGRAM_ROUTINE_MPU6050_CALIBRATION )
    dprintf( "MPU6050 calibration start\n" );
    mpu6050_calibration();
    chThdSleepSeconds( 1 );
    chSysHalt( "Calibration mode" );
#else
    
#endif

    /* Motor control init */
    motor_control_init();

    /* Motor control Thread init */
    chThdCreateStatic( waControlThread, sizeof(waControlThread), NORMALPRIO, ControlThread, NULL );

    /* Bluetooth communication init */
    bt_control_init( NORMALPRIO );



    while ( true )
    {
        chThdSleepMilliseconds( 100 );

        // if ( !palReadLine( LINE_BUTTON ) )
        if ( radio_control_is_switch_enabled() )
        {
            motor_control_set_motors_started(); 
        }
        else
        {
            motor_control_set_motors_stopped();
        }

        // palSetPad( GPIOC, 8 );

        // msg_t msg = i2cMasterTransmitTimeout( mpudrvr, 0b1101000, tx_buf, sizeof( tx_buf ), rx_buf, 14, MS2ST(10) );
        
        // palClearPad( GPIOC, 8 );

        // if ( msg == MSG_OK )
            // dprintf( "Ok, 0x%x =)\n", rx_buf[0] );
        // else
            // dprintf( "Not Ok %d 0x%x =)\n", msg, i2cGetErrors( mpudrvr ) );

        // palTogglePad( GPIOA, 5 );

        // dprintf( "Hello =)\n" );
    }
}
