#pragma once

#include <stdint.h>

/***************************************************/
/***            Sensors/Devices Amount           ***/
/***************************************************/
#define ENGINE_VALVE_AMOUNT              (uint8_t)0x02
#define ENGINE_HEATPAD_AMOUNT            (uint8_t)0x02
#define ENGINE_TELECOMMUNICATION_AMOUNT  (uint8_t)0x01
#define ENGINE_TEMPERATURE_SENSOR_AMOUNT (uint8_t)0x08
#define ENGINE_PRESSURE_SENSOR_AMOUNT    (uint8_t)0x02
#define ENGINE_IGNITER_AMOUNT            (uint8_t)0x02
#define ENGINE_STORAGE_AMOUNT            (uint8_t)0x02

/***************************************************/
/***           Peripheral Distribution           ***/
/***************************************************/
#define ENGINE_PWM_PER_VALVE (uint8_t)0x01

#define ENGINE_GPIO_PER_VALVE   (uint8_t)0x02
#define ENGINE_GPIO_PER_IGNITER (uint8_t)0x02

#define ENGINE_ADC_CHANNEL_PER_TEMPERATURE_SENSOR (uint8_t)0x01
#define ENGINE_ADC_CHANNEL_PER_PRESSURE_SENSOR    (uint8_t)0x01

/***************************************************/
/***              Peripheral Amount              ***/
/***************************************************/
#define ENGINE_ADC_AMOUNT         (uint8_t)0x01

#define ENGINE_PWM_AMOUNT         (uint8_t)(ENGINE_VALVE_AMOUNT * ENGINE_PWM_PER_VALVE)
#define ENGINE_ADC_CHANNEL_AMOUNT (uint8_t)0x10
#define ENGINE_UART_AMOUNT        (uint8_t)0x01
#define ENGINE_SPI_AMOUNT         (uint8_t)0x01
#define ENGINE_SDIO_AMOUNT        (uint8_t)0x01
#define ENGINE_GPIO_AMOUNT        (uint8_t)((ENGINE_GPIO_PER_VALVE * ENGINE_VALVE_AMOUNT) + (ENGINE_IGNITER_AMOUNT * ENGINE_GPIO_PER_IGNITER))

/***************************************************/
/***                Array Indexes                ***/
/***************************************************/
#define ENGINE_IPA_VALVE_INDEX     (uint8_t)0x00
#define ENGINE_NOS_VALVE_INDEX     (uint8_t)0x01

#define ENGINE_IPA_HEATPAD_INDEX (uint8_t)0x00
#define ENGINE_NOS_HEATPAD_INDEX (uint8_t)0x01

#define ENGINE_IPA_VALVE_PWM_INDEX (uint8_t)0x00
#define ENGINE_NOS_VALVE_PWM_INDEX (uint8_t)0x01

#define ENGINE_NOS_TANK_PRESSURE_SENSOR_INDEX           (uint8_t)0x00
#define ENGINE_COMBUSTION_CHAMBER_PRESSURE_SENSOR_INDEX (uint8_t)0x01

#define ENGINE_IPA_MAIN_VALVE_THERMISTANCE_INDEX (uint8_t)0x00
#define ENGINE_NOS_MAIN_VALVE_THERMISTANCE_INDEX (uint8_t)0x01
#define ENGINE_NOZZLE_1_THERMISTANCE_INDEX       (uint8_t)0x02
#define ENGINE_NOZZLE_2_THERMISTANCE_INDEX       (uint8_t)0x03
#define ENGINE_THROAT_1_THERMISTANCE_INDEX       (uint8_t)0x04
#define ENGINE_THROAT_2_THERMISTANCE_INDEX       (uint8_t)0x05
#define ENGINE_THROAT_3_THERMISTANCE_INDEX       (uint8_t)0x06
#define ENGINE_THERMISTANCE_8_INDEX              (uint8_t)0x07

#define ENGINE_TELECOMMUNICATION_UART_INDEX (uint8_t)0x00

#define ENGINE_IGNITER_1_GPIO_INDEX        (uint8_t)0x00
#define ENGINE_IGNITER_2_GPIO_INDEX        (uint8_t)0x01
#define ENGINE_NOS_VALVE_CLOSED_GPIO_INDEX (uint8_t)0x02
#define ENGINE_NOS_VALVE_OPENED_GPIO_INDEX (uint8_t)0x03
#define ENGINE_IPA_VALVE_CLOSED_GPIO_INDEX (uint8_t)0x04
#define ENGINE_IPA_VALVE_OPENED_GPIO_INDEX (uint8_t)0x05

#define ENGINE_IPA_HEATPAD_GPIO_INDEX (uint8_t)0x06
#define ENGINE_NOS_HEATPAD_GPIO_INDEX (uint8_t)0x07

#define ENGINE_STORAGE_SD_CARD_INDEX        (uint8_t)0x00
#define ENGINE_STORAGE_EXTERNAL_FLASH_INDEX (uint8_t)0x01

// CHECK IF GOOD
#define ENGINE_IPA_MAIN_VALVE_THERMISTANCE_ADC_CHANNEL_INDEX        (uint8_t)0x00
#define ENGINE_NOS_MAIN_VALVE_THERMISTANCE_ADC_CHANNEL_INDEX        (uint8_t)0x01
#define ENGINE_NOZZLE_1_THERMISTANCE_ADC_CHANNEL_INDEX              (uint8_t)0x02
#define ENGINE_NOZZLE_2_THERMISTANCE_ADC_CHANNEL_INDEX              (uint8_t)0x03
#define ENGINE_THROAT_1_THERMISTANCE_ADC_CHANNEL_INDEX              (uint8_t)0x04
#define ENGINE_THROAT_2_THERMISTANCE_ADC_CHANNEL_INDEX              (uint8_t)0x05
#define ENGINE_THROAT_3_THERMISTANCE_ADC_CHANNEL_INDEX              (uint8_t)0x06
#define ENGINE_THERMISTANCE_8_ADC_CHANNEL_INDEX                     (uint8_t)0x07
#define ENGINE_UNUSED_1_ADC_CHANNEL_INDEX                           (uint8_t)0x08
#define ENGINE_UNUSED_2_ADC_CHANNEL_INDEX                           (uint8_t)0x09
#define ENGINE_COMBUSTION_CHAMBER_PRESSURE_SENSOR_ADC_CHANNEL_INDEX (uint8_t)0x0A
#define ENGINE_NOS_TANK_PRESSURE_SENSOR_ADC_CHANNEL_INDEX           (uint8_t)0x0B
#define ENGINE_UNUSED_3_ADC_CHANNEL_INDEX                           (uint8_t)0x0C
#define ENGINE_UNUSED_4_ADC_CHANNEL_INDEX                           (uint8_t)0x0D
#define ENGINE_UNUSED_5_ADC_CHANNEL_INDEX                           (uint8_t)0x0E
#define ENGINE_UNUSED_6_ADC_CHANNEL_INDEX                           (uint8_t)0x0F

