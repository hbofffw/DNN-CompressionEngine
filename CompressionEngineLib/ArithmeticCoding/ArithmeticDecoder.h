#pragma once
#include "ACdefinitions.h"
#include "FrequencyTable.h"

class CArithmeticDecoder
{
public:
	CArithmeticDecoder();
	~CArithmeticDecoder();

	void Initialize(uint8_t *pInput, uint32_t uiLen);
	uint8_t Decode(CFrequencyTable &freq);

private:
	uint64_t m_uiLowerRange;           // lower range of the coding's current range
	uint64_t m_uiUpperRange;           // upper range of the coding's current range

	uint32_t   m_uiNumUnderflow;
	__int8 m_iBitPosition;
	uint32_t   m_uiInputLen;
	uint32_t   m_uiCurIdx;
	uint8_t *m_pInput;

	uint64_t m_code;

	uint8_t ReadOneBit();
	void  Update(CFrequencyTable &freq, uint8_t &symbol);
	void  Shift();
	void  Underflow();
};

