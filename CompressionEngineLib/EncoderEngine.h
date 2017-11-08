#pragma once
#include "General.h"

class CEncoderEngine
{
public:
	CEncoderEngine();
	~CEncoderEngine();

	// initialize the encoder with the maximum dimensions of the input sequence and seed
	// for random noise generation
	int Initialize(uint32_t uiRow, uint32_t uiCol, uint32_t uiInitialSeed = 0);
	int InitializeDISCUSEncoder(std::vector<LDPC_Matrix> H);
	int InitializeDISCUSEncoder(std::vector<LDPC_Matrix> H, std::vector<LDPC_Values> V);

	// set the quantization and compression methods, number of quantization levels and rate (for DISCUS) 
	int SetMethod(CompressionMethod cMethod, QuantizationMethod qMethod, uint8_t uiQLevels, uint8_t dsc = 0);

	// encode the input 2D sequence. each row of input is quantized separately, but the entire 
	// sequence is encoded together.
	uint32_t Encode(float *afInput, float *afResidue, uint8_t *auiCode, uint32_t row, uint32_t col);

private:
	void Quantize();
	void Compress();

	void RawBinaryEncoder();
	void FixedArithmeticEncoder();
	void AdaptiveArithmeticEncoder();
	void DISCUSEncoder();
	uint32_t BinaryDISCUSEncoder(uint8_t *code);
	uint32_t NonbinaryDISCUSEncoder(uint8_t *code);

	void UpdateDitherSeed();

	CompressionMethod  m_cMethod;
	QuantizationMethod m_qMethod;
	uint8_t  m_uiAlphabet;
	uint8_t  m_uiQLevel;
	uint8_t  m_uiDSCIdx;

	uint32_t  m_uiSeed;
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
	std::vector<LDPC_Matrix> m_aSparseMatrix;
	std::vector<LDPC_Values> m_aSparseValue;
};

