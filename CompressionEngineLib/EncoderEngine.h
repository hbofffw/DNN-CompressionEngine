#pragma once
#include "General.h"

class CEncoderEngine
{
public:
	CEncoderEngine();
	~CEncoderEngine();

	// initialize the encoder with the quantization and compression methods, and set the number of 
	// quantization levels, dimensions of the input sequence
	uint32_t Initialize(CompressionMethod cMethod, QuantizationMethod qMethod, uint8_t uiQLevels, uint32_t uiRow, uint32_t uiCol, uint32_t uiInitialSeed = 0);
	uint32_t SetDISCUSMatrix(std::vector<LDPC_Matrix> H);
	uint32_t SetDISCUSMatrix(std::vector<LDPC_Matrix> H, std::vector<LDPC_Values> V);

	// encode the input 2D sequence. each row of input is quantized separately, but the entire 
	// sequence is encoded together.
	uint32_t Encode(float *afInput, float *afResidue, uint8_t *auiCode, uint32_t row, uint32_t col);

private:
	uint32_t Quantize();
	uint32_t Compress();

	void CreateRawBinarySequence();
	void AdaptiveArithmeticEncoder();
	void DISCUSEncoder();

	void UpdateDitherSeed();

	CompressionMethod  m_cMethod;
	QuantizationMethod m_qMethod;
	uint8_t m_uiAlphabet;
	uint8_t m_uiQLevel;
	uint32_t m_uiSeed;

	uint32_t  m_uiMaxRowNum, m_uiMaxColNum;
	
	// input signals
	float *m_afInput;
	uint32_t  m_uiRowNum, m_uiColNum;

	// output signals
	float *m_afResidue;                     // the quantization residue signal
	uint8_t *m_auiCode;                     // the code sequence
	uint32_t m_uiCodeLen;                   // code length

	// quantized signals
	std::vector<uint8_t> m_quantized;       // the quantized sequence
	std::vector<float>   m_qParams;         // parameters of the quantizer

	// DISCUS encoding matrices for different rates
	std::vector<LDPC_Matrix> m_auiSparseEntry;
	std::vector<LDPC_Values> m_auiSparseValue;
};

