# ROS library
# ROSLIB = ./ros_lib
# include $(ROSLIB)/ros.mk

# AHRS library
AHRS_ROOT       = ../Libraries/ahrs_sensors
include $(AHRS_ROOT)/ahrs.mk

PROJECT_CSRC    = main.c twi.c $(AHRS_CSRS)
PROJECT_CPPSRC  = $(ROSSRC)

PROJECT_INCDIR  = $(ROSINC) $(AHRS_INC)

PROJECT_LIBS    = -lm

