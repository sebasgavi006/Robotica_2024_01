################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../ThirdParty/SEGGER/Rec/segger_uart.c 

OBJS += \
./ThirdParty/SEGGER/Rec/segger_uart.o 

C_DEPS += \
./ThirdParty/SEGGER/Rec/segger_uart.d 


# Each subdirectory must supply rules for building sources it contributes
ThirdParty/SEGGER/Rec/%.o ThirdParty/SEGGER/Rec/%.su: ../ThirdParty/SEGGER/Rec/%.c ThirdParty/SEGGER/Rec/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE -c -I../Inc -I"/home/sebas/Repositories/GitHub/sgaviriav_Robotica/EjemploLed_FreeRTOS/ThirdParty/FreeRTOS/include" -I"/home/sebas/Repositories/GitHub/sgaviriav_Robotica/EjemploLed_FreeRTOS/ThirdParty/FreeRTOS/portable/GCC/ARM_CM4F" -I"/home/sebas/STM32Cube/Robotics/workspace_fundamentos_robotica/CMSIS-repo/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"/home/sebas/STM32Cube/Robotics/workspace_fundamentos_robotica/CMSIS-repo/Drivers/CMSIS/Include" -I"/home/sebas/Repositories/GitHub/sgaviriav_Robotica/EjemploLed_FreeRTOS/ThirdParty/SEGGER/Config" -I"/home/sebas/Repositories/GitHub/sgaviriav_Robotica/EjemploLed_FreeRTOS/ThirdParty/SEGGER/SEGGER" -I"/home/sebas/Repositories/GitHub/sgaviriav_Robotica/EjemploLed_FreeRTOS/ThirdParty/SEGGER/OS" -I/home/sebas/Repositories/GitHub/sgaviriav_Robotica/PeripheralsDrivers/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-ThirdParty-2f-SEGGER-2f-Rec

clean-ThirdParty-2f-SEGGER-2f-Rec:
	-$(RM) ./ThirdParty/SEGGER/Rec/segger_uart.d ./ThirdParty/SEGGER/Rec/segger_uart.o ./ThirdParty/SEGGER/Rec/segger_uart.su

.PHONY: clean-ThirdParty-2f-SEGGER-2f-Rec

