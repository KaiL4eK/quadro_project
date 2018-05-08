# ROS library
# ROSLIB = ./ros_lib
# include $(ROSLIB)/ros.mk

# AHRS library
AHRS_ROOT       = ../Libraries/ahrs_sensors
include $(AHRS_ROOT)/ahrs.mk

# Filters library
FILTERS_ROOT       = ../Libraries/filters
include $(FILTERS_ROOT)/filters.mk

PROJECT_CSRC    = main.c twi.c motor_control.c PID_CS.c radio_control.c ahrs_module.c \
				  $(AHRS_CSRS) $(FILTERS_CSRS)
PROJECT_CPPSRC  = $(ROSSRC)

PROJECT_INCDIR  = $(ROSINC) $(AHRS_INC) $(FILTERS_INC)

PROJECT_LIBS    = -lm

