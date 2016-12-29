#include "mpu6050_dmp.h" 
#include "MPU6050.h"

#include <stdint.h>

#ifdef DEBUG
    #define DEBUG_PRINT(x) UART_write_string( UARTm1, "%s", x );
    #define DEBUG_PRINTLN(x) UART_write_string( UARTm1, "%s\n", x );
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
#endif

static uint8_t dmpPacketSize = 42;
static uint8_t fifo_buffer[255];
    
/* ================================================================================================ *
 | Default MotionApps v2.0 42-byte FIFO packet structure:                                           |
 |                                                                                                  |
 | [QUAT W][      ][QUAT X][      ][QUAT Y][      ][QUAT Z][      ][GYRO X][      ][GYRO Y][      ] |
 |   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16  17  18  19  20  21  22  23  |
 |                                                                                                  |
 | [GYRO Z][      ][ACC X ][      ][ACC Y ][      ][ACC Z ][      ][      ]                         |
 |  24  25  26  27  28  29  30  31  32  33  34  35  36  37  38  39  40  41                          |
 * ================================================================================================ */


void mpu6050_set_memory_bank(uint8_t bank, bool prefetchEnabled, bool userBank)
{
    bank &= 0x1F;
    if (userBank) 
        bank |= 0x20;
    if (prefetchEnabled) 
        bank |= 0x40;
    i2c_write_byte_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_BANK_SEL, bank );
}

uint8_t mpu6050_read_memory_byte( void ) {
    return ( i2c_read_byte_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_MEM_R_W) );
}

void mpu6050_set_memory_start_address( uint8_t address ) {
    i2c_write_byte_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_MEM_START_ADDR, address );
}

void mpu6050_set_memory_bank_address( uint8_t bank, uint8_t address )
{
    uint8_t bytes[2] = { bank, address };
    i2c_write_bytes_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_BANK_SEL, 2, bytes );
}

uint8_t mpu6050_getOTPBankValid() {
    return( i2c_read_bit_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OTP_BNK_VLD_BIT ) );
}

void mpu6050_setOTPBankValid(bool enabled) {
    i2c_write_bit_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OTP_BNK_VLD_BIT, enabled );
}

uint8_t mpu6050_getXGyroOffsetTC() {
    return i2c_read_bits_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH);
}
void mpu6050_setXGyroOffsetTC(uint8_t offset) {
    i2c_write_bits_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_XG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

uint8_t mpu6050_getYGyroOffsetTC() {
    return i2c_read_bits_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_YG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH);
}
void mpu6050_setYGyroOffsetTC(uint8_t offset) {
    i2c_write_bits_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_YG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

uint8_t mpu6050_getZGyroOffsetTC() {
    return i2c_read_bits_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_ZG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH);
}
void mpu6050_setZGyroOffsetTC(uint8_t offset) {
    i2c_write_bits_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_ZG_OFFS_TC, MPU6050_TC_OFFSET_BIT, MPU6050_TC_OFFSET_LENGTH, offset);
}

bool mpu6050_writeMemoryBlock( uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address ) {

    uint8_t chunkSize;
    uint8_t verifyBuffer[256];
    uint8_t *progBuffer;
    uint16_t i;

    for (i = 0; i < dataSize;) {
        // determine correct chunk size according to bank position and data size
        chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;

        // make sure we don't go past the data size
        if (i + chunkSize > dataSize) 
            chunkSize = dataSize - i;

        // make sure this chunk doesn't go past the bank boundary (256 bytes)
        if (chunkSize > 256 - address) 
            chunkSize = 256 - address;
        
        progBuffer = (uint8_t *)data + i;
#ifdef DEBUG
        UART_write_string( UARTm1, "Bank: %d, address: 0x%x, length: %d\n", bank, address, dataSize );
#endif
        mpu6050_set_memory_bank(bank, false, false);
        mpu6050_set_memory_start_address(address);
        
        i2c_write_bytes_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_MEM_R_W, chunkSize, progBuffer);
#ifdef DEBUG
        UART_write_string( UARTm1, "Writed:" );
        for (j = 0; j < chunkSize; j++)
            UART_write_string( UARTm1, " 0x%x", progBuffer[j] );
        UART_write_string( UARTm1, "\n" );
#endif
        mpu6050_set_memory_bank(bank, false, false);
        mpu6050_set_memory_start_address(address);

        i2c_read_bytes_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_MEM_R_W, chunkSize, verifyBuffer);

        if (memcmp(progBuffer, verifyBuffer, chunkSize) != 0) {
            UART_write_string( UARTm1, "\nBlock write verification error, bank %d, address %d!\n", bank, address );
#ifdef DEBUG
            uint8_t j;
            UART_write_string( UARTm1, "Received:" );
            for (j = 0; j < chunkSize; j++) {
                UART_write_string( UARTm1, " 0x%x", verifyBuffer[j] );
            }
            UART_write_string( UARTm1, "\n" );
#endif
            return false; // uh oh.
        }

        // increase byte index by [chunkSize]
        i += chunkSize;

        // uint8_t automatically wraps to 0 at 256
        address += chunkSize;

        // if we aren't done, update bank (if necessary) and address
        if (i < dataSize) {
            if (address == 0) 
                bank++;
            mpu6050_set_memory_bank(bank, false, false);
            mpu6050_set_memory_start_address(address);
        }
    }

    return true;
}

bool mpu6050_writeDMPConfigurationSet(const uint8_t *data, uint16_t dataSize) {
    uint8_t *progBuffer = NULL, 
            success = 0;
    uint16_t i;

    // config set data is a long string of blocks with the following structure:
    // [bank] [offset] [length] [byte[0], byte[1], ..., byte[length]]
    uint8_t bank, offset, length;
    for (i = 0; i < dataSize;) {
        
        bank = data[i++];
        offset = data[i++];
        length = data[i++];

        // write data or perform special action
        if (length > 0) {
            // regular block of data to write
            progBuffer = (uint8_t *)data + i;
            success = mpu6050_writeMemoryBlock(progBuffer, length, bank, offset);
            i += length;
        } else {
            // special instruction
            // NOTE: this kind of behavior (what and when to do certain things)
            // is totally undocumented. This code is in here based on observed
            // behavior only, and exactly why (or even whether) it has to be here
            // is anybody's guess for now.
            if (data[i++] == 0x01) {
                // enable DMP-related interrupts
                
                //setIntZeroMotionEnabled(true);
                //setIntFIFOBufferOverflowEnabled(true);
                //setIntDMPEnabled(true);
                i2c_write_byte_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_INT_ENABLE, 0x32);  // single operation

                success = true;
            } else {
                // unknown special command
                success = false;
            }
        }
        
        if (!success) {
            return false; // uh oh
        }
    }
    
    return true;
}

void mpu6050_readMemoryBlock(uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address) 
{
    mpu6050_set_memory_bank(bank, false, false);
    mpu6050_set_memory_start_address(address);
    uint8_t chunkSize;
    uint16_t i = 0;
    for ( i = 0; i < dataSize;) {
        // determine correct chunk size according to bank position and data size
        chunkSize = MPU6050_DMP_MEMORY_CHUNK_SIZE;

        // make sure we don't go past the data size
        if (i + chunkSize > dataSize) chunkSize = dataSize - i;

        // make sure this chunk doesn't go past the bank boundary (256 bytes)
        if (chunkSize > 256 - address) chunkSize = 256 - address;

        // read the chunk of data as specified
        i2c_read_bytes_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_MEM_R_W, chunkSize, data + i);
        
        // increase byte index by [chunkSize]
        i += chunkSize;

        // uint8_t automatically wraps to 0 at 256
        address += chunkSize;

        // if we aren't done, update bank (if necessary) and address
        if (i < dataSize) {
            if (address == 0) bank++;
            mpu6050_set_memory_bank(bank, false, false);
            mpu6050_set_memory_start_address(address);
        }
    }
}

void mpu6050_setIntEnabled( uint8_t enabled ) {
    i2c_write_byte_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_INT_ENABLE, enabled);
}

void mpu6050_setExternalFrameSync(uint8_t sync) {
    i2c_write_bits_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_CONFIG, MPU6050_CFG_EXT_SYNC_SET_BIT, MPU6050_CFG_EXT_SYNC_SET_LENGTH, sync);
}

// DMP_CFG_1 register

uint8_t mpu6050_getDMPConfig1() {
    return i2c_read_byte_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_DMP_CFG_1 );
}
void mpu6050_setDMPConfig1( uint8_t config ) {
    i2c_write_byte_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_DMP_CFG_1, config );
}

// DMP_CFG_2 register

uint8_t mpu6050_getDMPConfig2() {
    return i2c_read_byte_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_DMP_CFG_2 );
}
void mpu6050_setDMPConfig2( uint8_t config ) {
    i2c_write_byte_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_DMP_CFG_2, config );
}

void mpu6050_setDMPEnabled(bool enabled) {
    i2c_write_bit_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_EN_BIT, enabled);
}
void mpu6050_resetDMP() {
    i2c_write_bit_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_DMP_RESET_BIT, 1);
}

uint8_t mpu6050_getIntStatus() {
    return i2c_read_byte_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_INT_STATUS);
}

void mpu6050_setSlaveAddress(uint8_t num, uint8_t address) {
    if (num > 3) return;
    i2c_write_byte_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_I2C_SLV0_ADDR + num*3, address);
}

void mpu6050_setI2CMasterModeEnabled(bool enabled) {
    i2c_write_bit_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_EN_BIT, enabled);
}

void mpu6050_resetI2CMaster() {
    i2c_write_bit_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_I2C_MST_RESET_BIT, true);
}

void mpu6050_setMotionDetectionThreshold(uint8_t threshold) {
    i2c_write_byte_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_MOT_THR, threshold);
}

void mpu6050_setMotionDetectionDuration(uint8_t duration) {
    i2c_write_byte_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_MOT_DUR, duration);
}

void mpu6050_setZeroMotionDetectionThreshold(uint8_t threshold) {
    i2c_write_byte_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_ZRMOT_THR, threshold);
}

void mpu6050_setZeroMotionDetectionDuration(uint8_t duration) {
    i2c_write_byte_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_ZRMOT_DUR, duration);
}



inline void write_next_update()
{
    uint8_t             *p_dmp_update;
    uint16_t            dmp_update_length;
    uint8_t             dmp_update_bank;
    uint8_t             dmp_update_address;
    static uint16_t     pos = 0;
    
    if ( pos >= MPU6050_DMP_UPDATES_SIZE )
        pos = 0;
    
    p_dmp_update        = (uint8_t *)&dmpUpdates[pos];
    dmp_update_bank     = p_dmp_update[0];
    dmp_update_address  = p_dmp_update[1];
    dmp_update_length   = p_dmp_update[2];
    pos                 += dmp_update_length + 3;
    
    mpu6050_writeMemoryBlock( p_dmp_update + 3, dmp_update_length, dmp_update_bank, dmp_update_address );
}

int mpu6050_dmp_init() 
{
    // reset device
    DEBUG_PRINTLN( "\nResetting MPU6050..." );
    mpu6050_reset();

    delay_ms(30); // wait after reset

    if ( !mpu6050_test_connection() )
        return( -10 );

    // disable sleep mode
    DEBUG_PRINTLN( "Disabling sleep mode..." );
    mpu6050_set_sleep_bit( 0 );

    // get MPU hardware revision
    DEBUG_PRINTLN("Selecting user bank 16...");
    mpu6050_set_memory_bank( 0x10, true, true );
    DEBUG_PRINTLN("Selecting memory byte 6...");
    mpu6050_set_memory_start_address( 0x06 );

    DEBUG_PRINTLN("Checking hardware revision...");
    uint8_t hwRevision = mpu6050_read_memory_byte();

    DEBUG_PRINT("Revision @ user[16][6] = ");
    DEBUG_PRINTLN(hwRevision);
    DEBUG_PRINTLN("Resetting memory bank selection to 0...");

    mpu6050_set_memory_bank(0, false, false);
    
    // check OTP bank valid
    DEBUG_PRINTLN("Reading OTP bank valid flag...");
    uint8_t otpValid = mpu6050_getOTPBankValid();
    DEBUG_PRINT("OTP bank is ");
    DEBUG_PRINTLN(otpValid ? ("valid!") : ("invalid!"));

    DEBUG_PRINTLN("Setting slave 0 address to 0x7F...");
    mpu6050_setSlaveAddress(0, 0x7F);
    DEBUG_PRINTLN("Disabling I2C Master mode...");
    mpu6050_setI2CMasterModeEnabled(false);
    DEBUG_PRINTLN("Setting slave 0 address to 0x68 (self)...");
    mpu6050_setSlaveAddress(0, 0x68);
    DEBUG_PRINTLN("Resetting I2C Master control...");
    mpu6050_resetI2CMaster();

    delay_ms( 20 );

    // load DMP code into memory banks
    if (mpu6050_writeMemoryBlock(dmpMemory, MPU6050_DMP_CODE_SIZE, 0, 0)) {
        DEBUG_PRINTLN( "Success! DMP code written and verified" );

        // write DMP configuration
        if (mpu6050_writeDMPConfigurationSet(dmpConfig, MPU6050_DMP_CONFIG_SIZE)) {
            DEBUG_PRINTLN( "Success! DMP configuration written and verified." );

            DEBUG_PRINTLN("Setting clock source to Z Gyro...");
            mpu6050_set_clock_source(MPU6050_CLOCK_PLL_ZGYRO);

            DEBUG_PRINTLN("Setting DMP and FIFO_OFLOW interrupts enabled...");
            mpu6050_setIntEnabled(0x12);

            DEBUG_PRINTLN("Setting sample rate to 250Hz...");
            mpu6050_set_sample_rate_divider( 1 ); // 1khz / (1 + 1) = 500 Hz

            DEBUG_PRINTLN("Setting external frame sync to TEMP_OUT_L[0]...");
            mpu6050_setExternalFrameSync( MPU6050_EXT_SYNC_TEMP_OUT_L );

            DEBUG_PRINTLN("Setting DLPF bandwidth to 42Hz...");
            mpu6050_set_DLPF( MPU6050_DLPF_BW_42 );

            DEBUG_PRINTLN("Setting gyro sensitivity to +/- 2000 deg/sec...");
            mpu6050_set_gyro_fullscale( MPU6050_GYRO_FS_2000 );
//            mpu6050_set_accel_fullscale( MPU6050_ACCEL_FS_2 );

            DEBUG_PRINTLN("Setting DMP configuration bytes (function unknown)...");
            mpu6050_setDMPConfig1(0x03);
            if ( mpu6050_getDMPConfig1() != 0x03 ) {
                UART_write_string( UARTm1, "Error writing dmp config 1\n" );
                return( -9 );
            }
            
            mpu6050_setDMPConfig2(0x00);
            if ( mpu6050_getDMPConfig2() != 0x00 ) {
                UART_write_string( UARTm1, "Error writing dmp config 2\n" );
                return( -8 );
            }

            DEBUG_PRINTLN("Clearing OTP Bank flag...");
            mpu6050_setOTPBankValid(false);
            
            uint16_t fifoCount = 0;
            
            DEBUG_PRINTLN("Writing final memory update 1/7 (function unknown)...");
            write_next_update();
            
            DEBUG_PRINTLN("Writing final memory update 2/7 (function unknown)...");
            write_next_update();

            DEBUG_PRINTLN("Resetting FIFO...");
            mpu6050_resetFIFO();

            DEBUG_PRINTLN("Reading FIFO count...");
            fifoCount = mpu6050_getFIFOCount();
#ifdef DEBUG
            UART_write_string( UARTm1, "Current FIFO count = %d\n", fifoCount );
#endif
            mpu6050_getFIFOBytes(fifo_buffer, fifoCount);
            
            DEBUG_PRINTLN("Setting motion detection threshold to 2...");
            mpu6050_setMotionDetectionThreshold(2);

            DEBUG_PRINTLN("Setting zero-motion detection threshold to 156...");
            mpu6050_setZeroMotionDetectionThreshold(156);

            DEBUG_PRINTLN("Setting motion detection duration to 80...");
            mpu6050_setMotionDetectionDuration(80);

            DEBUG_PRINTLN("Setting zero-motion detection duration to 0...");
            mpu6050_setZeroMotionDetectionDuration(0);

            DEBUG_PRINTLN("Resetting FIFO...");
            mpu6050_resetFIFO();

            DEBUG_PRINTLN("Enabling FIFO...");
            mpu6050_setFIFOEnabled(true);

            DEBUG_PRINTLN("Enabling DMP...");
            mpu6050_setDMPEnabled(true);

            DEBUG_PRINTLN("Resetting DMP...");
            mpu6050_resetDMP();

            DEBUG_PRINTLN("Writing final memory update 3/7 (function unknown)...");
            write_next_update();
            
            DEBUG_PRINTLN("Writing final memory update 4/7 (function unknown)...");
            write_next_update();
            
            DEBUG_PRINTLN("Writing final memory update 5/7 (function unknown)...");
            write_next_update();
            
            DEBUG_PRINTLN("Waiting for FIFO count > 2...");
            while ((fifoCount = mpu6050_getFIFOCount()) < 3);

#ifdef DEBUG
            UART_write_string( UARTm1, "Current FIFO count = %d\n", fifoCount );
#endif
            DEBUG_PRINTLN("Reading FIFO data...");
            mpu6050_getFIFOBytes(fifo_buffer, fifoCount);

            DEBUG_PRINTLN("Reading interrupt status...");
            uint8_t mpuIntStatus = mpu6050_getIntStatus();
#ifdef DEBUG
            UART_write_string( UARTm1, "Current interrupt status = 0x%x\n", mpuIntStatus );
#endif
            DEBUG_PRINTLN("Reading final memory update 6/7 (function unknown)...");
            write_next_update();
            
            DEBUG_PRINTLN("Waiting for FIFO count > 2...");
            while ((fifoCount = mpu6050_getFIFOCount()) < 3);

            DEBUG_PRINT("Current FIFO count=");
            DEBUG_PRINTLN(fifoCount);

            DEBUG_PRINTLN("Reading FIFO data...");
            mpu6050_getFIFOBytes(fifo_buffer, fifoCount);

            DEBUG_PRINTLN("Reading interrupt status...");
            mpuIntStatus = mpu6050_getIntStatus();
#ifdef DEBUG
            UART_write_string( UARTm1, "Current interrupt status = 0x%x\n", mpuIntStatus );
#endif
            DEBUG_PRINTLN("Writing final memory update 7/7 (function unknown)...");
            write_next_update();
            
            DEBUG_PRINTLN("DMP is good to go! Finally.");

            DEBUG_PRINTLN("Disabling DMP (you turn it on later)...");
            mpu6050_setDMPEnabled(false);

            DEBUG_PRINTLN("Resetting FIFO and clearing INT status one last time...");
            mpu6050_resetFIFO();
            mpu6050_getIntStatus();
        } else {
            DEBUG_PRINTLN("ERROR! DMP configuration verification failed.");
            return 2; // configuration block loading failed
        }
    } else {
        DEBUG_PRINTLN("ERROR! DMP code verification failed.");
        return 1; // main binary block loading failed
    }
    
//    mpu6050_setXAccelOffset(-3594);
//    mpu6050_setYAccelOffset(-5370);
    mpu6050_setZAccelOffset(1822);
    
    mpu6050_setXGyroOffset(49);
    mpu6050_setYGyroOffset(-60);
    mpu6050_setZGyroOffset(-14);
    
    mpu6050_setDMPEnabled( true );
    
    return 0; // success
}

bool mpu6050_dmp_packet_available() {
    return mpu6050_getFIFOCount() >= dmpPacketSize;
}

void mpu6050_dmp_get_euler_angles(euler_angles_t *a) 
{
    quaternion_t internal_data;
    quaternion_t *q = &internal_data;
    mpu_fifo_frame_t *frame = ( mpu_fifo_frame_t * )fifo_buffer;
    
    mpu6050_getFIFOBytes( fifo_buffer, dmpPacketSize );
//
//    int16_t qI[4];
//    qI[0] = ((fifo_buffer[0] << 8) + fifo_buffer[1]);
//    qI[1] = ((fifo_buffer[4] << 8) + fifo_buffer[5]);
//    qI[2] = ((fifo_buffer[8] << 8) + fifo_buffer[9]);
//    qI[3] = ((fifo_buffer[12] << 8) + fifo_buffer[13]);
  
    SWAP( frame->reg.quat_w[0], frame->reg.quat_w[1] );
    SWAP( frame->reg.quat_x[0], frame->reg.quat_x[1] );
    SWAP( frame->reg.quat_y[0], frame->reg.quat_y[1] );
    SWAP( frame->reg.quat_z[0], frame->reg.quat_z[1] );
    
//    SWAP (frame->reg.x_accel[0], frame->reg.x_accel[1]);
//    SWAP (frame->reg.y_accel[0], frame->reg.y_accel[1]);
//    SWAP (frame->reg.z_accel[0], frame->reg.z_accel[1]);
//    SWAP (frame->reg.x_gyro[0],  frame->reg.x_gyro[1]);
//    SWAP (frame->reg.y_gyro[0],  frame->reg.y_gyro[1]);
//    SWAP (frame->reg.z_gyro[0],  frame->reg.z_gyro[1]);
//    
    q->w = frame->value.quat_w / 16384.0f;
    q->x = frame->value.quat_x / 16384.0f;
    q->y = frame->value.quat_y / 16384.0f;
    q->z = frame->value.quat_z / 16384.0f;

    a->yaw = RADIANS_TO_DEGREES * atan2(2 * q->x * q->y - 2 * q->w * q->z, 2 * q->w * q->w + 2 * q->x * q->x - 1);
    a->roll = RADIANS_TO_DEGREES * asin(2 * q->x * q->z + 2 * q->w * q->y);
    a->pitch = RADIANS_TO_DEGREES * -atan2(2 * q->y * q->z - 2 * q->w * q->x, 2 * q->w * q->w + 2 * q->z * q->z - 1);
    
//    UART_write_string( UARTm1, "Angles: %.2f, %.2f, %.2f\n", a->roll, a->pitch, a->yaw );
    
    return;
}

int mpu6050_dmpGetEuler_2(euler_angles_t *a) 
{
    mpu6050_getFIFOBytes( fifo_buffer, dmpPacketSize );
    
    uint32_t quat[4];
    uint32_t quats[4];
    quat[0] = ((uint32_t)fifo_buffer[0] << 24)  | ((uint32_t)fifo_buffer[1] << 16) |
              ((uint32_t)fifo_buffer[2] << 8)   | fifo_buffer[3];
    quat[1] = ((uint32_t)fifo_buffer[4] << 24)  | ((uint32_t)fifo_buffer[5] << 16) |
              ((uint32_t)fifo_buffer[6] << 8)   | fifo_buffer[7];
    quat[2] = ((uint32_t)fifo_buffer[8] << 24)  | ((uint32_t)fifo_buffer[9] << 16) |
              ((uint32_t)fifo_buffer[10] << 8)  | fifo_buffer[11];
    quat[3] = ((uint32_t)fifo_buffer[12] << 24) | ((uint32_t)fifo_buffer[13] << 16) |
              ((uint32_t)fifo_buffer[14] << 8)  | fifo_buffer[15];  
    
    int i = 0;
    for ( i = 0; i < 4; i++ ) {
        quats[i] = quat[i] >> 16;
    }
//    
//    uint32_t normsq = quats[0] * quats[0] + quats[1] * quats[1] + quats[2] * quats[2] + quats[3] * quats[3];
//    
//    if ( normsq > 285212672L ||  normsq < 251658240L ) {
//        //errors seen in quaternion - reset the fifo and bin this input
//        i2c_write_byte_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_USER_CTRL, 0x80 );
//        i2c_write_byte_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_USER_CTRL, 0x84 );
//        i2c_write_byte_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_USER_CTRL, 0xC0 );
//        
//        return( -1 );
//    }
    
    uint32_t w = quat[0]/32768.0f;
    uint32_t x = quat[1]/32768.0f;
    uint32_t y = quat[2]/32768.0f;
    uint32_t z = quat[3]/32768.0f;
    
    float gx = 2 * (x*z - w*y);
    float gy = 2 * (w*x + y*z);
    float gz = w*w - x*x - y*y + z*z;
    
    a->yaw = atan2(2*x*y - 2*w*z, 2*w*w + 2*x*x - 1); // about Z axis
    a->pitch = atan(gx / sqrt(gy*gy + gz*gz)); // about Y axis
    a->roll = atan(gy/gz); // about X axis
    
    a->yaw *= 57.2958f; // about Z axis
    a->pitch *= 57.2958f; // about Y axis
    a->roll *= 57.2958f; // about X axis
    
    return( 0 );
}

int mpu6050_dmpInitialize_2( void )
{
    // reset device
    UART_write_string( UARTm1, "\nResetting MPU6050..." );
    mpu6050_reset();
    delay_ms(100); // wait after reset
    
    // Set x gyro clock reference with PLL
    i2c_write_byte_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_PWR_MGMT_1, 0x01 );
    delay_ms(100);

#define FIRMWARE_CHUNK 16
    
    uint8_t mem_bank = 0;
    uint16_t mem_addr = 0;
    uint8_t write_buffer[FIRMWARE_CHUNK];
    uint8_t read_buffer[FIRMWARE_CHUNK];
    
    for ( mem_bank = 0; mem_bank < 12; mem_bank++ ) {
        for ( mem_addr = 0; mem_addr < 241; mem_addr += FIRMWARE_CHUNK ) {
            mpu6050_set_memory_bank_address( mem_bank, mem_addr );
            UART_write_string( UARTm1, "Writing to bank %d, address %d\n", mem_bank, mem_addr );
            
            uint16_t global_offset = mem_bank * 256L + mem_addr;
            memcpy( write_buffer, &DMPfirm[global_offset], sizeof( write_buffer ) );
            i2c_write_bytes_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_MEM_R_W, FIRMWARE_CHUNK, write_buffer );
            
            mpu6050_set_memory_bank_address( mem_bank, mem_addr );
            i2c_read_bytes_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_MEM_R_W, FIRMWARE_CHUNK, read_buffer );
#if 0
    int j = 0;
    
    UART_write_string( UARTm1, "Checking:" );
    for (j = 0; j < FIRMWARE_CHUNK; j++)
        UART_write_string( UARTm1, " 0x%x", write_buffer[j] );
    UART_write_string( UARTm1, "\n" );
    
    UART_write_string( UARTm1, "Readed:" );
    for (j = 0; j < FIRMWARE_CHUNK; j++)
        UART_write_string( UARTm1, " 0x%x", read_buffer[j] );
    UART_write_string( UARTm1, "\n" );
#endif
            if ( memcmp( write_buffer, read_buffer, FIRMWARE_CHUNK ) )
                return( -1 );
        } 
    }

    dmpPacketSize = 20;
    
    mpu6050_setDMPConfig1( 0x04 );
    mpu6050_setDMPConfig2( 0x00 );
    
    UART_write_string( UARTm1, "DMP firmware writed\n" );
        
//    mpu6050_setXAccelOffset(-3473);
//    mpu6050_setYAccelOffset(-2891);
    mpu6050_setZAccelOffset(1822);
    
    mpu6050_setXGyroOffset(49);
    mpu6050_setYGyroOffset(-60);
    mpu6050_setZGyroOffset(-14);
    
    
    mpu6050_set_gyro_fullscale( MPU6050_GYRO_FS_2000 );
    i2c_write_byte_eeprom( MPU6050_I2C_ADDRESS, MPU6050_RA_USER_CTRL, 0xC0 );
    
    return( 0 );
}

