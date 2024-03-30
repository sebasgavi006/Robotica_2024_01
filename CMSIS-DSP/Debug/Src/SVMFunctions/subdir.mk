################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/SVMFunctions/SVMFunctions.c \
../Src/SVMFunctions/SVMFunctionsF16.c \
../Src/SVMFunctions/arm_svm_linear_init_f16.c \
../Src/SVMFunctions/arm_svm_linear_init_f32.c \
../Src/SVMFunctions/arm_svm_linear_predict_f16.c \
../Src/SVMFunctions/arm_svm_linear_predict_f32.c \
../Src/SVMFunctions/arm_svm_polynomial_init_f16.c \
../Src/SVMFunctions/arm_svm_polynomial_init_f32.c \
../Src/SVMFunctions/arm_svm_polynomial_predict_f16.c \
../Src/SVMFunctions/arm_svm_polynomial_predict_f32.c \
../Src/SVMFunctions/arm_svm_rbf_init_f16.c \
../Src/SVMFunctions/arm_svm_rbf_init_f32.c \
../Src/SVMFunctions/arm_svm_rbf_predict_f16.c \
../Src/SVMFunctions/arm_svm_rbf_predict_f32.c \
../Src/SVMFunctions/arm_svm_sigmoid_init_f16.c \
../Src/SVMFunctions/arm_svm_sigmoid_init_f32.c \
../Src/SVMFunctions/arm_svm_sigmoid_predict_f16.c \
../Src/SVMFunctions/arm_svm_sigmoid_predict_f32.c 

OBJS += \
./Src/SVMFunctions/SVMFunctions.o \
./Src/SVMFunctions/SVMFunctionsF16.o \
./Src/SVMFunctions/arm_svm_linear_init_f16.o \
./Src/SVMFunctions/arm_svm_linear_init_f32.o \
./Src/SVMFunctions/arm_svm_linear_predict_f16.o \
./Src/SVMFunctions/arm_svm_linear_predict_f32.o \
./Src/SVMFunctions/arm_svm_polynomial_init_f16.o \
./Src/SVMFunctions/arm_svm_polynomial_init_f32.o \
./Src/SVMFunctions/arm_svm_polynomial_predict_f16.o \
./Src/SVMFunctions/arm_svm_polynomial_predict_f32.o \
./Src/SVMFunctions/arm_svm_rbf_init_f16.o \
./Src/SVMFunctions/arm_svm_rbf_init_f32.o \
./Src/SVMFunctions/arm_svm_rbf_predict_f16.o \
./Src/SVMFunctions/arm_svm_rbf_predict_f32.o \
./Src/SVMFunctions/arm_svm_sigmoid_init_f16.o \
./Src/SVMFunctions/arm_svm_sigmoid_init_f32.o \
./Src/SVMFunctions/arm_svm_sigmoid_predict_f16.o \
./Src/SVMFunctions/arm_svm_sigmoid_predict_f32.o 

C_DEPS += \
./Src/SVMFunctions/SVMFunctions.d \
./Src/SVMFunctions/SVMFunctionsF16.d \
./Src/SVMFunctions/arm_svm_linear_init_f16.d \
./Src/SVMFunctions/arm_svm_linear_init_f32.d \
./Src/SVMFunctions/arm_svm_linear_predict_f16.d \
./Src/SVMFunctions/arm_svm_linear_predict_f32.d \
./Src/SVMFunctions/arm_svm_polynomial_init_f16.d \
./Src/SVMFunctions/arm_svm_polynomial_init_f32.d \
./Src/SVMFunctions/arm_svm_polynomial_predict_f16.d \
./Src/SVMFunctions/arm_svm_polynomial_predict_f32.d \
./Src/SVMFunctions/arm_svm_rbf_init_f16.d \
./Src/SVMFunctions/arm_svm_rbf_init_f32.d \
./Src/SVMFunctions/arm_svm_rbf_predict_f16.d \
./Src/SVMFunctions/arm_svm_rbf_predict_f32.d \
./Src/SVMFunctions/arm_svm_sigmoid_init_f16.d \
./Src/SVMFunctions/arm_svm_sigmoid_init_f32.d \
./Src/SVMFunctions/arm_svm_sigmoid_predict_f16.d \
./Src/SVMFunctions/arm_svm_sigmoid_predict_f32.d 


# Each subdirectory must supply rules for building sources it contributes
Src/SVMFunctions/%.o Src/SVMFunctions/%.su: ../Src/SVMFunctions/%.c Src/SVMFunctions/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F4 -DSTM32F411RETx -DSTM32F411xE -c -I"/home/aristizabal/Documents/Repositories/Robotica_2024_01/CMSIS-repo/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"/home/aristizabal/Documents/Repositories/Robotica_2024_01/CMSIS-repo/Drivers/CMSIS/Include" -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src-2f-SVMFunctions

clean-Src-2f-SVMFunctions:
	-$(RM) ./Src/SVMFunctions/SVMFunctions.d ./Src/SVMFunctions/SVMFunctions.o ./Src/SVMFunctions/SVMFunctions.su ./Src/SVMFunctions/SVMFunctionsF16.d ./Src/SVMFunctions/SVMFunctionsF16.o ./Src/SVMFunctions/SVMFunctionsF16.su ./Src/SVMFunctions/arm_svm_linear_init_f16.d ./Src/SVMFunctions/arm_svm_linear_init_f16.o ./Src/SVMFunctions/arm_svm_linear_init_f16.su ./Src/SVMFunctions/arm_svm_linear_init_f32.d ./Src/SVMFunctions/arm_svm_linear_init_f32.o ./Src/SVMFunctions/arm_svm_linear_init_f32.su ./Src/SVMFunctions/arm_svm_linear_predict_f16.d ./Src/SVMFunctions/arm_svm_linear_predict_f16.o ./Src/SVMFunctions/arm_svm_linear_predict_f16.su ./Src/SVMFunctions/arm_svm_linear_predict_f32.d ./Src/SVMFunctions/arm_svm_linear_predict_f32.o ./Src/SVMFunctions/arm_svm_linear_predict_f32.su ./Src/SVMFunctions/arm_svm_polynomial_init_f16.d ./Src/SVMFunctions/arm_svm_polynomial_init_f16.o ./Src/SVMFunctions/arm_svm_polynomial_init_f16.su ./Src/SVMFunctions/arm_svm_polynomial_init_f32.d ./Src/SVMFunctions/arm_svm_polynomial_init_f32.o ./Src/SVMFunctions/arm_svm_polynomial_init_f32.su ./Src/SVMFunctions/arm_svm_polynomial_predict_f16.d ./Src/SVMFunctions/arm_svm_polynomial_predict_f16.o ./Src/SVMFunctions/arm_svm_polynomial_predict_f16.su ./Src/SVMFunctions/arm_svm_polynomial_predict_f32.d ./Src/SVMFunctions/arm_svm_polynomial_predict_f32.o ./Src/SVMFunctions/arm_svm_polynomial_predict_f32.su ./Src/SVMFunctions/arm_svm_rbf_init_f16.d ./Src/SVMFunctions/arm_svm_rbf_init_f16.o ./Src/SVMFunctions/arm_svm_rbf_init_f16.su ./Src/SVMFunctions/arm_svm_rbf_init_f32.d ./Src/SVMFunctions/arm_svm_rbf_init_f32.o ./Src/SVMFunctions/arm_svm_rbf_init_f32.su ./Src/SVMFunctions/arm_svm_rbf_predict_f16.d ./Src/SVMFunctions/arm_svm_rbf_predict_f16.o ./Src/SVMFunctions/arm_svm_rbf_predict_f16.su ./Src/SVMFunctions/arm_svm_rbf_predict_f32.d ./Src/SVMFunctions/arm_svm_rbf_predict_f32.o ./Src/SVMFunctions/arm_svm_rbf_predict_f32.su ./Src/SVMFunctions/arm_svm_sigmoid_init_f16.d ./Src/SVMFunctions/arm_svm_sigmoid_init_f16.o ./Src/SVMFunctions/arm_svm_sigmoid_init_f16.su ./Src/SVMFunctions/arm_svm_sigmoid_init_f32.d ./Src/SVMFunctions/arm_svm_sigmoid_init_f32.o ./Src/SVMFunctions/arm_svm_sigmoid_init_f32.su ./Src/SVMFunctions/arm_svm_sigmoid_predict_f16.d ./Src/SVMFunctions/arm_svm_sigmoid_predict_f16.o ./Src/SVMFunctions/arm_svm_sigmoid_predict_f16.su ./Src/SVMFunctions/arm_svm_sigmoid_predict_f32.d ./Src/SVMFunctions/arm_svm_sigmoid_predict_f32.o ./Src/SVMFunctions/arm_svm_sigmoid_predict_f32.su

.PHONY: clean-Src-2f-SVMFunctions

