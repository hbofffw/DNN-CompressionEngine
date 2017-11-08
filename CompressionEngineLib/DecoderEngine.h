#pragma once

#include "General.h"
#include "binaryDISCUS\BinaryDSC.h"
#include "nonbinaryDISCUS\NonbinaryDSC.h"

class CDecoderEngine
{
public:
	CDecoderEngine();
	~CDecoderEngine();

	// initialize the decoder with the maximum dimensions of the input sequence and seed
	// for random noise generation
	int Initialize(uint32_t uiRow, uint32_t uiCol, uint32_t uiInitialSeed = 0);
	int InitializeDISCUSDecoder(std::vector<LDPC_Matrix> H);
	int InitializeDISCUSDecoder(std::vector<LDPC_Matrix> H, std::vector<LDPC_Values> V);

	// set the quantization and compression methods, number of quantization levels and rate (for DISCUS) 
	int SetMethod(CompressionMethod cMethod, QuantizationMethod qMethod, uint8_t uiQLevels, uint8_t dsc = 0);

	// decode the input sequence.
	uint32_t Decode(uint8_t *auiCode, uint32_t uiCodeLen, float *afOutput, uint32_t row, uint32_t col);

private:
	uint32_t Dequantize();
	uint32_t Decompress();

	uint32_t RawBinaryDecoder();
	uint32_t FixedArithmeticDecoder();
	uint32_t AdaptiveArithmeticDecoder();
	uint32_t DISCUSDecoder();
	uint32_t BinaryDISCUSDecoder(uint8_t *code);
	uint32_t NonbinaryDISCUSDecoder(uint8_t *code);

	void UpdateDitherSeed();

	CompressionMethod  m_cMethod;
	QuantizationMethod m_qMethod;
	uint8_t  m_uiAlphabet;
	uint8_t  m_uiQLevel;
	uint8_t  m_uiDSCIdx;

	uint32_t  m_uiSeed;
	uint32_t  m_uiMaxRowNum, m_uiMaxColNum;

	// input signals
	uint32_t  m_uiRowNum, m_uiColNum;
	uint8_t *m_auiCode;
	uint32_t m_uiCodeLen;

	// output signals
	float *m_afOutput;

	// dequantization parameters and decoded values
	std::vector<float>   m_qParams;         // parameters of the quantizer
	std::vector<uint8_t> m_decoded;         // the decoded sequence

	// DISCUS decoders for different rates
	std::vector<CBinaryDSC> m_abinaryDSC;
	std::vector<CNonbinaryDSC> m_anonbinaryDSC;
};

