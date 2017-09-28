################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/main.c \
../Src/nrf24.c \
../Src/nrf24_app.c \
../Src/nrf24_hal.c \
../Src/stm32f4xx_hal_msp.c \
../Src/stm32f4xx_it.c \
../Src/system_stm32f4xx.c 

OBJS += \
./Src/main.o \
./Src/nrf24.o \
./Src/nrf24_app.o \
./Src/nrf24_hal.o \
./Src/stm32f4xx_hal_msp.o \
./Src/stm32f4xx_it.o \
./Src/system_stm32f4xx.o 

C_DEPS += \
./Src/main.d \
./Src/nrf24.d \
./Src/nrf24_app.d \
./Src/nrf24_hal.d \
./Src/stm32f4xx_hal_msp.d \
./Src/stm32f4xx_it.d \
./Src/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32F446xx -I"/home/daniel/workspace/ECC_NRF24_RX/Inc" -I"/home/daniel/workspace/ECC_NRF24_RX/Drivers/STM32F4xx_HAL_Driver/Inc" -I"/home/daniel/workspace/ECC_NRF24_RX/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/home/daniel/workspace/ECC_NRF24_RX/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"/home/daniel/workspace/ECC_NRF24_RX/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


