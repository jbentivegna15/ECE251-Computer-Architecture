################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../inc/retargetio.c \
../inc/retargetserial.c 

OBJS += \
./inc/retargetio.OBJ \
./inc/retargetserial.OBJ 


# Each subdirectory must supply rules for building sources it contributes
inc/%.OBJ: ../inc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Keil 8051 Compiler'
	wine "/Applications/Simplicity Studio.app/Contents/Eclipse/developer/toolchains/keil_8051/9.53/BIN/C51" "@$(patsubst %.OBJ,%.__i,$@)" || $(RC)
	@echo 'Finished building: $<'
	@echo ' '

inc/retargetserial.OBJ: /Users/s150563/SimplicityStudio/v4_workspace/UARTstopwatch/inc/retargetserial.h /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/toolchains/keil_8051/9.53/INC/REG51.H /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/toolchains/keil_8051/9.53/INC/STDIO.H


