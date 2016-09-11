/*
 * File:   mpu6050_DMP.c
 * Author: alexey
 *
 * Created on September 7, 2016, 12:25 PM
 */


#include "mpu6050_dmp.h" 
#include "MPU6050.h"

// Teensy 3.0 library conditional PROGMEM code from Paul Stoffregen
#include <stdint.h>

#define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#define pgm_read_word(addr) (*(const uint16_t *)(addr))
#define pgm_read_dword(addr) (*(const uint32_t *)(addr))
#define pgm_read_float(addr) (*(const float *)(addr))

#ifdef DEBUG
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTF(x, y) Serial.print(x, y)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINTLNF(x, y) Serial.println(x, y)
#else
    #define DEBUG_PRINT(x)          UART_write_string( UARTm1, "%s", x );
    #define DEBUG_PRINTF(x, y)
    #define DEBUG_PRINTLN(x)        UART_write_string( UARTm1, "%s\n", x );
    #define DEBUG_PRINTLNF(x, y)
#endif

static uint8_t dmpPacketSize = 42;
static uint8_t fifo_buffer[42];
    
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
    uint8_t j;

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

        UART_write_string( UARTm1, "Bank: %d, address: 0x%x, length: %d\n", bank, address, dataSize );
        mpu6050_set_memory_bank(bank, false, false);
        mpu6050_set_memory_start_address(address);
        
        i2c_write_bytes_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_MEM_R_W, chunkSize, progBuffer);

        UART_write_string( UARTm1, "Writed:" );
        for (j = 0; j < chunkSize; j++)
            UART_write_string( UARTm1, " 0x%x", progBuffer[j] );
        UART_write_string( UARTm1, "\n" );

        mpu6050_set_memory_bank(bank, false, false);
        mpu6050_set_memory_start_address(address);

        i2c_read_bytes_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_MEM_R_W, chunkSize, verifyBuffer);

        if (memcmp(progBuffer, verifyBuffer, chunkSize) != 0) {
            UART_write_string( UARTm1, "\nBlock write verification error, bank %d, address %d!\n", bank, address );

            UART_write_string( UARTm1, "Received:" );
            for (j = 0; j < chunkSize; j++) {
                UART_write_string( UARTm1, " 0x%x", verifyBuffer[j] );
            }
            UART_write_string( UARTm1, "\n" );
            
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
    uint16_t i, j;

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

void mpu6050_readMemoryBlock(uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address) {
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

uint8_t mpu6050_getFIFOByte() {
    return i2c_read_byte_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_FIFO_R_W);
}
void mpu6050_getFIFOBytes(uint8_t *data, uint8_t length) {
    i2c_read_bytes_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_FIFO_R_W, length, data);
}

void mpu6050_setFIFOByte(uint8_t data) {
    i2c_write_byte_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_FIFO_R_W, data);
}

uint16_t mpu6050_getFIFOCount() {
    uint8_t buffer[2];
    i2c_read_bytes_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_FIFO_COUNTH, 2, buffer);
    return (((uint16_t)buffer[0]) << 8) | buffer[1];
}

void mpu6050_resetFIFO() {
    i2c_write_bit_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_RESET_BIT, 1);
}

void mpu6050_setFIFOEnabled(bool enabled) {
    i2c_write_bit_eeprom(MPU6050_I2C_ADDRESS, MPU6050_RA_USER_CTRL, MPU6050_USERCTRL_FIFO_EN_BIT, enabled);
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

int mpu6050_dmpInitialize() 
{
    // reset device
    DEBUG_PRINTLN("\n\nResetting MPU6050...");
    mpu6050_reset();
    delay_ms(30); // wait after reset

    if ( !mpu6050_test_connection() )
        return( -10 );
    
    // enable sleep mode and wake cycle
    /*Serial.println"Enabling sleep mode...");
    setSleepEnabled(true);
    Serial.println("Enabling wake cycle...");
    setWakeCycleEnabled(true);*/

    // disable sleep mode
    DEBUG_PRINTLN("Disabling sleep mode...");
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

    // get X/Y/Z gyro offsets
//    DEBUG_PRINTLN("Reading gyro offset TC values...");
//    int8_t xgOffsetTC = mpu6050_getXGyroOffsetTC();
//    int8_t ygOffsetTC = mpu6050_getYGyroOffsetTC();
//    int8_t zgOffsetTC = mpu6050_getZGyroOffsetTC();
//    UART_write_string( UARTm1, "Read offset\n" );

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
        UART_write_string( UARTm1, "Success! DMP code written and verified\n" );

        // write DMP configuration
        if (mpu6050_writeDMPConfigurationSet(dmpConfig, MPU6050_DMP_CONFIG_SIZE)) {
            UART_write_string( UARTm1, "Success! DMP configuration written and verified." );

            DEBUG_PRINTLN("Setting clock source to Z Gyro...");
            mpu6050_set_clock_source(MPU6050_CLOCK_PLL_ZGYRO);

            DEBUG_PRINTLN("Setting DMP and FIFO_OFLOW interrupts enabled...");
            mpu6050_setIntEnabled(0x12);

            DEBUG_PRINTLN("Setting sample rate to 20Hz...");
            mpu6050_set_sample_rate_divider( 1 ); // 1khz / (1 + 1) = 500 Hz

            DEBUG_PRINTLN("Setting external frame sync to TEMP_OUT_L[0]...");
            mpu6050_setExternalFrameSync( MPU6050_EXT_SYNC_TEMP_OUT_L );

            DEBUG_PRINTLN("Setting DLPF bandwidth to 42Hz...");
            mpu6050_set_DLPF( MPU6050_DLPF_BW_42 );

            DEBUG_PRINTLN("Setting gyro sensitivity to +/- 2000 deg/sec...");
            mpu6050_set_gyro_fullscale( MPU6050_GYRO_FS_2000 );

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
#if 0
            DEBUG_PRINTLN("Setting X/Y/Z gyro offset TCs to previous values...");
            mpu6050_setXGyroOffsetTC(xgOffsetTC);
            mpu6050_setYGyroOffsetTC(ygOffsetTC);
            mpu6050_setZGyroOffsetTC(zgOffsetTC);
#endif
            //DEBUG_PRINTLN("Setting X/Y/Z gyro user offsets to zero...");
            //setXGyroOffset(0);
            //setYGyroOffset(0);
            //setZGyroOffset(0);

            DEBUG_PRINTLN("Writing final memory update 1/7 (function unknown)...");
//            uint8_t     dmpUpdate[16], j;
            uint8_t     *p_dmp_update;
            uint16_t    dmp_update_length;
            uint8_t     dmp_update_bank;
            uint8_t     dmp_update_address;
            uint16_t    pos = 0;
            
            uint16_t fifoCount = 0;
            uint8_t fifoBuffer[128];
            
            p_dmp_update        = (uint8_t *)&dmpUpdates[pos];
            dmp_update_bank     = p_dmp_update[0];
            dmp_update_address  = p_dmp_update[1];
            dmp_update_length   = p_dmp_update[2];
            pos                 += dmp_update_length + 3;
            mpu6050_writeMemoryBlock( p_dmp_update + 3, dmp_update_length, dmp_update_bank, dmp_update_address );
            
//            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
//            mpu6050_writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);
            
            DEBUG_PRINTLN("Writing final memory update 2/7 (function unknown)...");
//            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
//            mpu6050_writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

            p_dmp_update        = (uint8_t *)&dmpUpdates[pos];
            dmp_update_bank     = p_dmp_update[0];
            dmp_update_address  = p_dmp_update[1];
            dmp_update_length   = p_dmp_update[2];
            pos                 += dmp_update_length + 3;
            mpu6050_writeMemoryBlock( p_dmp_update + 3, dmp_update_length, dmp_update_bank, dmp_update_address );
#if 1
            DEBUG_PRINTLN("Resetting FIFO...");
            mpu6050_resetFIFO();

            DEBUG_PRINTLN("Reading FIFO count...");
            fifoCount = mpu6050_getFIFOCount();

            UART_write_string( UARTm1, "Current FIFO count = %d\n", fifoCount );
            mpu6050_getFIFOBytes(fifoBuffer, fifoCount);
#if 1
            DEBUG_PRINTLN("Setting motion detection threshold to 2...");
            mpu6050_setMotionDetectionThreshold(2);

            DEBUG_PRINTLN("Setting zero-motion detection threshold to 156...");
            mpu6050_setZeroMotionDetectionThreshold(156);

            DEBUG_PRINTLN("Setting motion detection duration to 80...");
            mpu6050_setMotionDetectionDuration(80);

            DEBUG_PRINTLN("Setting zero-motion detection duration to 0...");
            mpu6050_setZeroMotionDetectionDuration(0);
#endif
            DEBUG_PRINTLN("Resetting FIFO...");
            mpu6050_resetFIFO();

            DEBUG_PRINTLN("Enabling FIFO...");
            mpu6050_setFIFOEnabled(true);

            DEBUG_PRINTLN("Enabling DMP...");
            mpu6050_setDMPEnabled(true);

            DEBUG_PRINTLN("Resetting DMP...");
            mpu6050_resetDMP();
#endif
            DEBUG_PRINTLN("Writing final memory update 3/7 (function unknown)...");
//            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
//            mpu6050_writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

            p_dmp_update        = (uint8_t *)&dmpUpdates[pos];
            dmp_update_bank     = p_dmp_update[0];
            dmp_update_address  = p_dmp_update[1];
            dmp_update_length   = p_dmp_update[2];
            pos                 += dmp_update_length + 3;
            mpu6050_writeMemoryBlock( p_dmp_update + 3, dmp_update_length, dmp_update_bank, dmp_update_address );
            
            DEBUG_PRINTLN("Writing final memory update 4/7 (function unknown)...");
//            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
//            mpu6050_writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

            p_dmp_update        = (uint8_t *)&dmpUpdates[pos];
            dmp_update_bank     = p_dmp_update[0];
            dmp_update_address  = p_dmp_update[1];
            dmp_update_length   = p_dmp_update[2];
            pos                 += dmp_update_length + 3;
            mpu6050_writeMemoryBlock( p_dmp_update + 3, dmp_update_length, dmp_update_bank, dmp_update_address );
            
            DEBUG_PRINTLN("Writing final memory update 5/7 (function unknown)...");
//            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
//            mpu6050_writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

            p_dmp_update        = (uint8_t *)&dmpUpdates[pos];
            dmp_update_bank     = p_dmp_update[0];
            dmp_update_address  = p_dmp_update[1];
            dmp_update_length   = p_dmp_update[2];
            pos                 += dmp_update_length + 3;
            mpu6050_writeMemoryBlock( p_dmp_update + 3, dmp_update_length, dmp_update_bank, dmp_update_address );
            
            DEBUG_PRINTLN("Waiting for FIFO count > 2...");
            while ((fifoCount = mpu6050_getFIFOCount()) < 3);

            UART_write_string( UARTm1, "Current FIFO count = %d\n", fifoCount );
            DEBUG_PRINTLN("Reading FIFO data...");
            mpu6050_getFIFOBytes(fifoBuffer, fifoCount);

            DEBUG_PRINTLN("Reading interrupt status...");
            uint8_t mpuIntStatus = mpu6050_getIntStatus();

            UART_write_string( UARTm1, "Current interrupt status = 0x%x\n", mpuIntStatus );

            DEBUG_PRINTLN("Reading final memory update 6/7 (function unknown)...");
//            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
//            mpu6050_readMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

            p_dmp_update        = (uint8_t *)&dmpUpdates[pos];
            dmp_update_bank     = p_dmp_update[0];
            dmp_update_address  = p_dmp_update[1];
            dmp_update_length   = p_dmp_update[2];
            pos                 += dmp_update_length + 3;
            mpu6050_writeMemoryBlock( p_dmp_update + 3, dmp_update_length, dmp_update_bank, dmp_update_address );
            
            DEBUG_PRINTLN("Waiting for FIFO count > 2...");
            while ((fifoCount = mpu6050_getFIFOCount()) < 3);

            DEBUG_PRINT("Current FIFO count=");
            DEBUG_PRINTLN(fifoCount);

            DEBUG_PRINTLN("Reading FIFO data...");
            mpu6050_getFIFOBytes(fifoBuffer, fifoCount);

            DEBUG_PRINTLN("Reading interrupt status...");
            mpuIntStatus = mpu6050_getIntStatus();

            DEBUG_PRINT("Current interrupt status=");
            DEBUG_PRINTLNF(mpuIntStatus, HEX);

            DEBUG_PRINTLN("Writing final memory update 7/7 (function unknown)...");
//            for (j = 0; j < 4 || j < dmpUpdate[2] + 3; j++, pos++) dmpUpdate[j] = pgm_read_byte(&dmpUpdates[pos]);
//            mpu6050_writeMemoryBlock(dmpUpdate + 3, dmpUpdate[2], dmpUpdate[0], dmpUpdate[1]);

            p_dmp_update        = (uint8_t *)&dmpUpdates[pos];
            dmp_update_bank     = p_dmp_update[0];
            dmp_update_address  = p_dmp_update[1];
            dmp_update_length   = p_dmp_update[2];
            pos                 += dmp_update_length + 3;
            mpu6050_writeMemoryBlock( p_dmp_update + 3, dmp_update_length, dmp_update_bank, dmp_update_address );
            
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
    
    mpu6050_setXAccelOffset(-3594);
    mpu6050_setYAccelOffset(-5370);
    mpu6050_setZAccelOffset(1813);
    
    mpu6050_setXGyroOffset(142);
    mpu6050_setYGyroOffset(-22);
    mpu6050_setZGyroOffset(-19);
    
    mpu6050_setDMPEnabled( true );
    
    return 0; // success
}

bool mpu6050_dmpPacketAvailable() {
    return mpu6050_getFIFOCount() >= dmpPacketSize;
}

int mpu6050_dmpGetEuler(euler_angles_t *a) 
{
    quaternion_t internal_data;
    quaternion_t *q = &internal_data;
    mpu6050_getFIFOBytes(fifo_buffer, sizeof( fifo_buffer ));
    
    int16_t qI[4];
    qI[0] = ((fifo_buffer[0] << 8) + fifo_buffer[1]);
    qI[1] = ((fifo_buffer[4] << 8) + fifo_buffer[5]);
    qI[2] = ((fifo_buffer[8] << 8) + fifo_buffer[9]);
    qI[3] = ((fifo_buffer[12] << 8) + fifo_buffer[13]);
    
    q->w = (float)qI[0] / 16384.0f;
    q->x = (float)qI[1] / 16384.0f;
    q->y = (float)qI[2] / 16384.0f;
    q->z = (float)qI[3] / 16384.0f;
#define RADS_TO_DEGREE_C 57.295779f
    a->pitch = RADS_TO_DEGREE_C * atan2(2 * q->x * q->y - 2 * q->w * q->z, 2 * q->w * q->w + 2 * q->x * q->x - 1);  // psi
    a->yaw = RADS_TO_DEGREE_C * -asin(2 * q->x * q->z + 2 * q->w * q->y);                                         // theta
    a->roll = RADS_TO_DEGREE_C * atan2(2 * q->y * q->z - 2 * q->w * q->x, 2 * q->w * q->w + 2 * q->z * q->z - 1);  // phi
    
    
    return 0;
}
