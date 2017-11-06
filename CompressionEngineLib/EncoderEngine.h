#pragma once
#include "General.h"

class CEncoderEngine
{
public:
	CEncoderEngine();
	~CEncoderEngine();

	// initialize the encoder with the quantization and compression methods, and set the number of 
	// quantization levels, dimensions of the input sequence
	int Initialize(CompressionMethod cMethod, QuantizationMethod qMethod, uint8_t uiQLevels, uint32_t uiRow, uint32_t uiCol, uint32_t uiInitialSeed = 0);

	// encode the input 2D sequence. each row of input is quantized separately, but the entire 
	// sequence is encoded together.
	int Encode(float *afInput, uint32_t row, uint32_t col);

private:
	int Quantize(float *afInput, uint32_t row, uint32_t col);
	int Compress(uint32_t row, uint32_t col);


	CompressionMethod  m_cMethod;
	QuantizationMethod m_qMethod;
	uint8_t m_uiAlphabet;
	uint8_t m_uiQLevel;
	uint32_t m_uiSeed;

	uint32_t  m_uiRowNum, m_uiColNum;
	
	std::vector<uint8_t> m_quantized;       // the quantized sequence
	std::vector<float>   m_qParams;         // parameters of the quantizer
	std::vector<uint8_t> m_code;            // the code sequence
};

