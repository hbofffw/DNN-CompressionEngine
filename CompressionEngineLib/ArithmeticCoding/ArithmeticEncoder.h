#pragma once
#include "ACdefinitions.h"
#include "FrequencyTable.h"

class CArithmeticEncoder
{
public:
	CArithmeticEncoder();
	~CArithmeticEncoder();

	void Initialize(uint8_t* outSequence);
	void Encode(CFrequencyTable &freq, uint8_t &symbol);
	void Finish();

	uint32_t GetCodeLength() { return m_uiOutputLen; }

private:
	uint64_t m_uiLowerRange;           // lower range of the coding's current range
	uint64_t m_uiUpperRange;           // upper range of the coding's current range

	uint32_t   m_uiNumUnderflow;
	__int8 m_iBitPosition;
	uint8_t* m_pOutput;
	uint32_t   m_uiOutputLen;

	void WriteOneBit(uint8_t bit);
	void Shift();
	void Underflow();
};

