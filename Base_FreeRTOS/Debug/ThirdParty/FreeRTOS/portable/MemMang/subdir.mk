################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ThirdParty/FreeRTOS/portable/MemMang/heap_4.c 

OBJS += \
./ThirdParty/FreeRTOS/portable/MemMang/heap_4.o 

C_DEPS += \
./ThirdParty/FreeRTOS/portable/MemMang/heap_4.d 


# Each subdirectory must supply rules for building sources it contributes
ThirdParty/FreeRTOS/portable/MemMang/%.o ThirdParty/FreeRTOS/portable/MemMang/%.su: ../ThirdParty/FreeRTOS/portable/MemMang/%.c ThirdParty/FreeRTOS/portable/MemMang/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE -c -I../Inc -I"/home/aristizabal/Documents/Repositories/Robotica_2024_01/Base_FreeRTOS/ThirdParty/FreeRTOS/include" -I"/home/aristizabal/Documents/Repositories/Robotica_2024_01/Base_FreeRTOS/ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F" -I"/home/aristizabal/STM32CubeIDE/Robotics/RTOS_workspace/Software_Toolchain/STM32Cube_FW_F4_V1.27.0/Drivers/CMSIS/Include" -I"/home/aristizabal/STM32CubeIDE/Robotics/RTOS_workspace/Software_Toolchain/STM32Cube_FW_F4_V1.27.0/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"/home/aristizabal/Documents/Repositories/Robotica_2024_01/Base_FreeRTOS/ThirdParty/SEGGER/Config" -I"/home/aristizabal/Documents/Repositories/Robotica_2024_01/Base_FreeRTOS/ThirdParty/SEGGER/OS" -I"/home/aristizabal/Documents/Repositories/Robotica_2024_01/Base_FreeRTOS/ThirdParty/SEGGER/SEGGER" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-ThirdParty-2f-FreeRTOS-2f-portable-2f-MemMang

clean-ThirdParty-2f-FreeRTOS-2f-portable-2f-MemMang:
	-$(RM) ./ThirdParty/FreeRTOS/portable/MemMang/heap_4.d ./ThirdParty/FreeRTOS/portable/MemMang/heap_4.o ./ThirdParty/FreeRTOS/portable/MemMang/heap_4.su

.PHONY: clean-ThirdParty-2f-FreeRTOS-2f-portable-2f-MemMang

