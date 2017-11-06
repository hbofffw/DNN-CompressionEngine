#pragma once

#include "../General.h"

// parameters for arithmetic coding
const uint8_t  g_uiStateSize = 32;
const uint64_t g_uiMaxRange = ((uint64_t)1) << g_uiStateSize;
const uint64_t g_uiMinRange = (g_uiMaxRange >> 2) + 2;
const uint64_t g_uiMask = g_uiMaxRange - 1;
const uint64_t g_uiMSBMask1 = g_uiMaxRange >> 1;
const uint64_t g_uiMSBMask2 = g_uiMSBMask1 >> 1;