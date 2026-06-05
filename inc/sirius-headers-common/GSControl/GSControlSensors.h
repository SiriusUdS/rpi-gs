#pragma once

#include <stdint.h>

/***************************************************/
/***            Sensors/Devices Amount           ***/
/***************************************************/
#define GS_CONTROL_BUTTON_AMOUNT (uint8_t)0x08

/***************************************************/
/***           Peripheral Distribution           ***/
/***************************************************/
#define GS_CONTROL_GPIO_PER_BUTTON (uint8_t)0x01

/***************************************************/
/***              Peripheral Amount              ***/
/***************************************************/
#define GS_CONTROL_GPIO_AMOUNT GS_CONTROL_BUTTON_AMOUNT * GS_CONTROL_GPIO_PER_BUTTON

/***************************************************/
/***       Array Indexes -- Sensors/Devices      ***/
/***************************************************/
#define GS_CONTROL_BUTTON_ALLOW_FILL_INDEX     (uint8_t)0x00
#define GS_CONTROL_BUTTON_ARM_VALVE_INDEX      (uint8_t)0x01
#define GS_CONTROL_BUTTON_ARM_IGNITER_INDEX    (uint8_t)0x02
#define GS_CONTROL_BUTTON_ALLOW_DUMP_INDEX     (uint8_t)0x03
#define GS_CONTROL_BUTTON_EMERGENCY_STOP_INDEX (uint8_t)0x04
#define GS_CONTROL_BUTTON_FIRE_IGNITER_INDEX   (uint8_t)0x05
#define GS_CONTROL_BUTTON_VALVE_START_INDEX    (uint8_t)0x06
#define GS_CONTROL_BUTTON_UNSAFE_INDEX         (uint8_t)0x07


/***************************************************/
/***         Array Indexes -- Peripherals        ***/
/***************************************************/
#define GS_CONTROL_GPIO_ALLOW_FILL_INDEX     (uint8_t)0x00
#define GS_CONTROL_GPIO_ARM_VALVE_INDEX      (uint8_t)0x01
#define GS_CONTROL_GPIO_ARM_IGNITER_INDEX    (uint8_t)0x02
#define GS_CONTROL_GPIO_UNUSED_INDEX         (uint8_t)0x03
#define GS_CONTROL_GPIO_EMERGENCY_STOP_INDEX (uint8_t)0x04
#define GS_CONTROL_GPIO_FIRE_IGNITER_INDEX   (uint8_t)0x05
#define GS_CONTROL_GPIO_VALVE_START_INDEX    (uint8_t)0x06
#define GS_CONTROL_GPIO_UNSAFE_INDEX         (uint8_t)0x07
