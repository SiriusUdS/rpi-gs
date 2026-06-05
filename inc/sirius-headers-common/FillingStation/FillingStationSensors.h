#pragma once

#include <stdint.h>

/***************************************************/
/***            Sensors/Devices Amount           ***/
/***************************************************/
#define FILLING_STATION_TEMPERATURE_SENSOR_AMOUNT  (uint8_t)0x08
#define FILLING_STATION_LOAD_CELL_AMOUNT           (uint8_t)0x02
#define FILLING_STATION_PRESSURE_SENSOR_AMOUNT     (uint8_t)0x02
#define FILLING_STATION_HEATPAD_AMOUNT             (uint8_t)0x02
#define FILLING_STATION_VALVE_AMOUNT               (uint8_t)0x02
#define FILLING_STATION_IGNITER_AMOUNT             (uint8_t)0x01
#define FILLING_STATION_BUTTON_AMOUNT              (uint8_t)0x01

#define FILLING_STATION_EXTERNAL_FLASH_AMOUNT     (uint8_t)0x01
#define FILLING_STATION_SD_CARD_AMOUNT            (uint8_t)0x01
#define FILLING_STATION_TELECOMMUNICATION_AMNOUNT (uint8_t)0x01

/***************************************************/
/***           Peripheral Distribution           ***/
/***************************************************/
#define FILLING_STATION_PWM_PER_VALVE                      (uint8_t)0x01
#define FILLING_STATION_HEATPAD_PER_VALVE                  (uint8_t)0x01
#define FILLING_STATION_GPIO_PER_VALVE                     (uint8_t)0x02
#define FILLING_STATION_GPIO_PER_HEATPAD                   (uint8_t)0x01
#define FILLING_STATION_GPIO_PER_BUTTON                    (uint8_t)0x01
#define FILLING_STATION_GPIO_PER_IGNITER                   (uint8_t)0x01
#define FILLING_STATION_ADC_CHANNEL_PER_TEMPERATURE_SENSOR (uint8_t)0x01
#define FILLING_STATION_ADC_CHANNEL_PER_PRESSURE_SENSOR    (uint8_t)0x01

/***************************************************/
/***              Peripheral Amount              ***/
/***************************************************/
#define FILLING_STATION_ADC_CHANNEL_AMOUNT (uint8_t)0x10
#define FILLING_STATION_PWM_AMOUNT         FILLING_STATION_VALVE_AMOUNT
#define FILLING_STATION_GPIO_AMOUNT        (FILLING_STATION_VALVE_AMOUNT * FILLING_STATION_GPIO_PER_VALVE) + \
                                          (FILLING_STATION_HEATPAD_AMOUNT * FILLING_STATION_GPIO_PER_HEATPAD) + \
                                          (FILLING_STATION_BUTTON_AMOUNT * FILLING_STATION_GPIO_PER_BUTTON) + \
                                          (FILLING_STATION_IGNITER_AMOUNT * FILLING_STATION_GPIO_PER_IGNITER)

/***************************************************/
/***                Array Indexes                ***/
/***************************************************/
#define FILLING_STATION_FILL_VALVE_INDEX (uint8_t)0x00
#define FILLING_STATION_DUMP_VALVE_INDEX (uint8_t)0x01

#define FILLING_STATION_FILL_VALVE_HEATPAD_INDEX (uint8_t)0x00
#define FILLING_STATION_DUMP_VALVE_HEATPAD_INDEX (uint8_t)0x01

#define FILLING_STATION_FILL_VALVE_PWM_INDEX (uint8_t)0x00
#define FILLING_STATION_DUMP_VALVE_PWM_INDEX (uint8_t)0x01

#define FILLING_STATION_FILL_VALVE_CLOSED_GPIO_INDEX     (uint8_t)0x00
#define FILLING_STATION_FILL_VALVE_OPENED_GPIO_INDEX     (uint8_t)0x01
#define FILLING_STATION_DUMP_VALVE_CLOSED_GPIO_INDEX     (uint8_t)0x02
#define FILLING_STATION_DUMP_VALVE_OPENED_GPIO_INDEX     (uint8_t)0x03
#define FILLING_STATION_FILL_HEATPAD_GPIO_INDEX          (uint8_t)0x04
#define FILLING_STATION_DUMP_HEATPAD_GPIO_INDEX          (uint8_t)0x05
#define FILLING_STATION_BUTTON_EMERGENCY_STOP_GPIO_INDEX (uint8_t)0x06
#define FILLING_STATION_IGNITER_DUMP_GPIO_INDEX          (uint8_t)0x07

#define FILLING_STATION_FILL_VALVE_THERMISTANCE_INDEX    (uint8_t)0x00
#define FILLING_STATION_QUICK_CONNECT_THERMISTANCE_INDEX (uint8_t)0x01
#define FILLING_STATION_NOS_VALVE_THERMISTANCE_INDEX     (uint8_t)0x02
#define FILLING_STATION_IPA_VALVE_THERMISTANCE_INDEX     (uint8_t)0x03
#define FILLING_STATION_THERMISTANCE_1_INDEX             (uint8_t)0x04
#define FILLING_STATION_THERMISTANCE_2_INDEX             (uint8_t)0x05
#define FILLING_STATION_THERMISTANCE_3_INDEX             (uint8_t)0x06
#define FILLING_STATION_THERMISTANCE_4_INDEX             (uint8_t)0x07

#define FILLING_STATION_TANK_LOAD_CELL_INDEX               (uint8_t)0x00
#define FILLING_STATION_COMBUSTION_CHAMBER_LOAD_CELL_INDEX (uint8_t)0x01

#define FILLING_STATION_FILL_VALVE_HEATPAD_INDEX (uint8_t)0x00
#define FILLING_STATION_DUMP_VALVE_HEATPAD_INDEX (uint8_t)0x01

// CHECK THAT
#define FILLING_STATION_FILL_VALVE_THERMISTANCE_ADC_CHANNEL_INDEX      (uint8_t)0x00
#define FILLING_STATION_QUICK_CONNECT_THERMISTANCE_ADC_CHANNEL_INDEX   (uint8_t)0x01
#define FILLING_STATION_NOS_VALVE_THERMISTANCE_ADC_CHANNEL_INDEX       (uint8_t)0x02
#define FILLING_STATION_IPA_VALVE_THERMISTANCE_ADC_CHANNEL_INDEX       (uint8_t)0x03
#define FILLING_STATION_THERMISTANCE_1_ADC_CHANNEL_INDEX               (uint8_t)0x04
#define FILLING_STATION_THERMISTANCE_2_ADC_CHANNEL_INDEX               (uint8_t)0x05
#define FILLING_STATION_THERMISTANCE_3_ADC_CHANNEL_INDEX               (uint8_t)0x06
#define FILLING_STATION_THERMISTANCE_4_ADC_CHANNEL_INDEX               (uint8_t)0x07
#define FILLING_STATION_UNUSED_1_ADC_CHANNEL_INDEX                     (uint8_t)0x08
#define FILLING_STATION_UNUSED_2_ADC_CHANNEL_INDEX                     (uint8_t)0x09
#define FILLING_STATION_IPA_MANIFOLD_PRESSURE_SENSOR_INDEX             (uint8_t)0x0A
#define FILLING_STATION_NOS_MANIFOLD_PRESSURE_SENSOR_INDEX             (uint8_t)0x0B
#define FILLING_STATION_TANK_LOAD_CELL_ADC_CHANNEL_INDEX               (uint8_t)0x0E
#define FILLING_STATION_COMBUSTION_CHAMBER_LOAD_CELL_ADC_CHANNEL_INDEX (uint8_t)0x0F