#pragma once

#include "General.h"

class CDecoderEngine
{
public:
	CDecoderEngine();
	~CDecoderEngine();

	// initialize the encoder with the quantization and compression methods, and set the number of 
	// quantization levels, dimensions of the input sequence
	int Initialize(CompressionMethod cMethod, QuantizationMethod qMethod, uint8_t uiQLevels, uint32_t uiRow, uint32_t uiCol, uint32_t uiInitialSeed = 0);

	// decode the input sequence.
	int Decode(uint8_t *auiInput, float *afqParams, uint32_t row, uint32_t col);

private:
	void DecodeRawBinarySequence();
};

