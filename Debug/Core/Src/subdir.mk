################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Image.c \
../Core/Src/font8x8_basic.c \
../Core/Src/lcd.c \
../Core/Src/lcd_text.c \
../Core/Src/main.c \
../Core/Src/stm32u3xx_hal_msp.c \
../Core/Src/stm32u3xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32u3xx.c 

OBJS += \
./Core/Src/Image.o \
./Core/Src/font8x8_basic.o \
./Core/Src/lcd.o \
./Core/Src/lcd_text.o \
./Core/Src/main.o \
./Core/Src/stm32u3xx_hal_msp.o \
./Core/Src/stm32u3xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32u3xx.o 

C_DEPS += \
./Core/Src/Image.d \
./Core/Src/font8x8_basic.d \
./Core/Src/lcd.d \
./Core/Src/lcd_text.d \
./Core/Src/main.d \
./Core/Src/stm32u3xx_hal_msp.d \
./Core/Src/stm32u3xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32u3xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m33 -std=gnu11 -g3 -DDEBUG -DUSE_NUCLEO_64 -DUSE_HAL_DRIVER -DSTM32U385xx -c -I../Core/Inc -I../Drivers/STM32U3xx_HAL_Driver/Inc -I../Drivers/STM32U3xx_HAL_Driver/Inc/Legacy -I../Drivers/BSP/STM32U3xx_Nucleo -I../Drivers/CMSIS/Device/ST/STM32U3xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/Image.cyclo ./Core/Src/Image.d ./Core/Src/Image.o ./Core/Src/Image.su ./Core/Src/font8x8_basic.cyclo ./Core/Src/font8x8_basic.d ./Core/Src/font8x8_basic.o ./Core/Src/font8x8_basic.su ./Core/Src/lcd.cyclo ./Core/Src/lcd.d ./Core/Src/lcd.o ./Core/Src/lcd.su ./Core/Src/lcd_text.cyclo ./Core/Src/lcd_text.d ./Core/Src/lcd_text.o ./Core/Src/lcd_text.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32u3xx_hal_msp.cyclo ./Core/Src/stm32u3xx_hal_msp.d ./Core/Src/stm32u3xx_hal_msp.o ./Core/Src/stm32u3xx_hal_msp.su ./Core/Src/stm32u3xx_it.cyclo ./Core/Src/stm32u3xx_it.d ./Core/Src/stm32u3xx_it.o ./Core/Src/stm32u3xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32u3xx.cyclo ./Core/Src/system_stm32u3xx.d ./Core/Src/system_stm32u3xx.o ./Core/Src/system_stm32u3xx.su

.PHONY: clean-Core-2f-Src

