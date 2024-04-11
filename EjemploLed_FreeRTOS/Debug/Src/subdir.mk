################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/pll_test.c \
../Src/syscalls.c 

OBJS += \
./Src/pll_test.o \
./Src/syscalls.o 

C_DEPS += \
./Src/pll_test.d \
./Src/syscalls.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE -c -I../Inc -I"/home/sebas/Repositories/GitHub/Robotica_2024_01/EjemploLed_FreeRTOS/ThirdParty/FreeRTOS/include" -I"/home/sebas/Repositories/GitHub/Robotica_2024_01/EjemploLed_FreeRTOS/ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F" -I"/home/sebas/STM32Cube/Robotics/workspace_fundamentos_robotica/Software_Toolchain/STM32Cube_FW_F4_V1.27.0/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"/home/sebas/STM32Cube/Robotics/workspace_fundamentos_robotica/Software_Toolchain/STM32Cube_FW_F4_V1.27.0/Drivers/CMSIS/Include" -I"/home/sebas/Repositories/GitHub/Robotica_2024_01/PeripheralsDrivers/Inc" -I"/home/sebas/Repositories/GitHub/Robotica_2024_01/EjemploLed_FreeRTOS/ThirdParty/SEGGER/Config" -I"/home/sebas/Repositories/GitHub/Robotica_2024_01/EjemploLed_FreeRTOS/ThirdParty/SEGGER/OS" -I"/home/sebas/Repositories/GitHub/Robotica_2024_01/EjemploLed_FreeRTOS/ThirdParty/SEGGER/SEGGER" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/pll_test.d ./Src/pll_test.o ./Src/pll_test.su ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su

.PHONY: clean-Src

