#pragma once

#include <stdint.h>

/**
 * https://usherbrooke.sharepoint.com/:p:/r/sites/SIRIUS/_layouts/15/Doc2.aspx?action=edit&sourcedoc=%7B2f1058e7-45a9-4e77-bcaa-6724409993bb%7D&wdOrigin=TEAMS-MAGLEV.teamsSdk_ns.rwc&wdExp=TEAMS-TREATMENT&wdhostclicktime=1769475998854&web=1
 */

#define ENGINE_STATE_INIT             (uint8_t)0x00
#define ENGINE_STATE_SAFE             (uint8_t)0x01
#define ENGINE_STATE_UNSAFE           (uint8_t)0x02
#define ENGINE_STATE_ABORT            (uint8_t)0x03
#define ENGINE_STATE_IGNITION         (uint8_t)0x06
#define ENGINE_STATE_FIRE             (uint8_t)0x07
#define ENGINE_STATE_ERROR            (uint8_t)0x08
#define ENGINE_STATE_TEST             (uint8_t)0x09