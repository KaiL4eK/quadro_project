#include "flash.h"
#include "per_proto.h"

/* FLASH write read code */
static uint16_t         flash_data[_FLASH_ROW];
static _prog_addressT   p_eds_address = 0x21000;
//_memcpy_p2d16(testdata, p_eds_address, sizeof(flash_data));
//UART_write_int(flash_data[0]);
//if ( testdata[0] == 0xffff )
//{
//UART_writeln_string("Write values");
//_erase_flash(p_eds_address);
//int i;
//    for (i=0; i<(_FLASH_ROW);i++) {
//        flash_data[i]=0x32;
//    }
//_write_flash16(p_eds_address, flash_data);
//}
//_memcpy_p2d16(flash_data, p_eds_address, sizeof(flash_data));
//UART_write_hint(flash_data[0]);

int flash_write(void) {
    return 0;
}
