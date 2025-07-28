################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/18761/Workspace/ELEC5620M-Resources/Drivers/FPGA_IrDAController/FPGA_IrDAController.c 

OBJS += \
./Drivers/FPGA_IrDAController/FPGA_IrDAController.o 

C_DEPS += \
./Drivers/FPGA_IrDAController/FPGA_IrDAController.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/FPGA_IrDAController/FPGA_IrDAController.o: C:/Users/18761/Workspace/ELEC5620M-Resources/Drivers/FPGA_IrDAController/FPGA_IrDAController.c Drivers/FPGA_IrDAController/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: Arm C Compiler for Embedded 6'
	armclang.exe --target=arm-arm-none-eabi -mcpu=cortex-a9 -mfpu=vfpv3-d16-fp16 -mfloat-abi=hard -marm -I"C:\Users\18761\Workspace\ELEC5620M-Resources\Drivers" -O3 -g -mno-unaligned-access -MD -MP -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


