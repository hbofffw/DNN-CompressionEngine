#pragma once

#include "General.h"

class CDecoderEngine
{
public:
	CDecoderEngine();
	~CDecoderEngine();

	// initialize the encoder with the quantization and compression methods, and set the number of 
	// quantization levels, dimensions of the input sequence
	uint32_t Initialize(CompressionMethod cMethod, QuantizationMethod qMethod, uint8_t uiQLevels, uint32_t uiRow, uint32_t uiCol, uint32_t uiInitialSeed = 0);

	// decode the input sequence.
	uint32_t Decode(uint8_t *auiCode, uint32_t uiCodeLen, float *afOutput, uint32_t row, uint32_t col);

private:
	uint32_t Dequantize();
	uint32_t Decompress();

	uint32_t DecodeRawBinarySequence();
	uint32_t AdaptiveArithmeticDecoder();

	void UpdateDitherSeed();

	CompressionMethod  m_cMethod;
	QuantizationMethod m_qMethod;
	uint8_t m_uiAlphabet;
	uint8_t m_uiQLevel;
	uint32_t m_uiSeed;

	uint32_t  m_uiMaxRowNum, m_uiMaxColNum;
	uint32_t  m_uiRowNum, m_uiColNum;

	// input signals
	uint8_t *m_auiCode;
	uint32_t m_uiCodeLen;

	// output signals
	float *m_afOutput;

	// dequantization parameters and decoded values
	std::vector<float>   m_qParams;         // parameters of the quantizer
	std::vector<uint8_t> m_decoded;         // the decoded sequence
};

