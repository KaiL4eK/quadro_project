#!/bin/bash

openocd -s ~/openocd_bin/share/openocd/scripts -f board/st_nucleo_f4.cfg -c "stm32f4x.cpu configure -rtos auto;"
