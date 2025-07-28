################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/18761/Workspace/ELEC5620M-Resources/Drivers/DE1SoC_Mandelbrot/DE1SoC_Mandelbrot.c 

OBJS += \
./Drivers/DE1SoC_Mandelbrot/DE1SoC_Mandelbrot.o 

C_DEPS += \
./Drivers/DE1SoC_Mandelbrot/DE1SoC_Mandelbrot.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/DE1SoC_Mandelbrot/DE1SoC_Mandelbrot.o: C:/Users/18761/Workspace/ELEC5620M-Resources/Drivers/DE1SoC_Mandelbrot/DE1SoC_Mandelbrot.c Drivers/DE1SoC_Mandelbrot/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Arm C Compiler for Embedded 6'
	armclang.exe --target=arm-arm-none-eabi -mcpu=cortex-a9 -mfpu=vfpv3-d16-fp16 -mfloat-abi=hard -marm -I"C:\Users\18761\Workspace\ELEC5620M-Resources\Drivers" -O3 -g -mno-unaligned-access -MD -MP -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


