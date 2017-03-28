################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/InitDevice.c \
../src/main.c 

OBJS += \
./src/InitDevice.OBJ \
./src/main.OBJ 


# Each subdirectory must supply rules for building sources it contributes
src/%.OBJ: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Keil 8051 Compiler'
	wine "/Applications/Simplicity Studio.app/Contents/Eclipse/developer/toolchains/keil_8051/9.53/BIN/C51" "@$(patsubst %.OBJ,%.__i,$@)" || $(RC)
	@echo 'Finished building: $<'
	@echo ' '

src/InitDevice.OBJ: /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/sdks/8051/v4.0.3/Device/EFM8BB3/inc/SI_EFM8BB3_Register_Enums.h /Users/s150563/Documents/Cooper/Semester\ 4/Comp_Arch/ECE251-Project1/4bitProcessor/inc/InitDevice.h /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/sdks/8051/v4.0.3/Device/EFM8BB3/inc/SI_EFM8BB3_Defs.h /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/sdks/8051/v4.0.3/Device/shared/si8051Base/si_toolchain.h /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/sdks/8051/v4.0.3/Device/shared/si8051Base/stdint.h /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/sdks/8051/v4.0.3/Device/shared/si8051Base/stdbool.h

src/main.OBJ: /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/sdks/8051/v4.0.3/Device/EFM8BB3/inc/SI_EFM8BB3_Register_Enums.h /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/toolchains/keil_8051/9.53/INC/STDIO.H /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/toolchains/keil_8051/9.53/INC/STDLIB.H /Users/s150563/Documents/Cooper/Semester\ 4/Comp_Arch/ECE251-Project1/4bitProcessor/inc/InitDevice.h /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/sdks/8051/v4.0.3/Device/EFM8BB3/inc/SI_EFM8BB3_Defs.h /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/sdks/8051/v4.0.3/Device/shared/si8051Base/si_toolchain.h /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/sdks/8051/v4.0.3/Device/shared/si8051Base/stdint.h /Applications/Simplicity\ Studio.app/Contents/Eclipse/developer/sdks/8051/v4.0.3/Device/shared/si8051Base/stdbool.h


