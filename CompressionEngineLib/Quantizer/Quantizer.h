#pragma once

#include "../General.h"

void OneBitQuantizer(float *afInput, uint32_t uiLen, float *afRec, uint8_t *abtQ);
void OneBitDequantizer(uint8_t *abtQ, float *afRec, uint8_t uiLen, float *afOutput);

void DitheredQuantizer(float *afInput, float &fMinValue, float &fStepSize, uint8_t uiMaxQ, uint32_t uiLen, uint32_t seed, uint8_t *abtQ);
void DitheredDequantizer(uint8_t *abtQ, float &fMinValue, float &fStepSize, uint8_t uiLen, uint32_t seed, float *afOutput);

void UniformQuantizer(float *afInput, float &fMinValue, float &fStepSize, uint8_t uiMaxQ, uint32_t uiLen, uint8_t *abtQ);
void UniformDequantizer(uint8_t *abtQ, float &fMinValue, float &fStepSize, uint8_t uiLen, float *afOutput);
