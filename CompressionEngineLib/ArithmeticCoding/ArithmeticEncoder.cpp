#include "ArithmeticEncoder.h"



CArithmeticEncoder::CArithmeticEncoder()
{
}


CArithmeticEncoder::~CArithmeticEncoder()
{
}

void CArithmeticEncoder::Initialize(uint8_t* outSequence)
{
	m_uiLowerRange = 0;
	m_uiUpperRange = g_uiMask;

	m_uiNumUnderflow = 0;
	m_iBitPosition = 7;
	m_pOutput = outSequence;
	*m_pOutput = 0;
	m_uiOutputLen = 1;
}

void CArithmeticEncoder::Encode(CFrequencyTable &freq, uint8_t &symbol)
{
	uint64_t uiLower = m_uiLowerRange;
	uint64_t uiUpper = m_uiUpperRange;
	uint64_t uiRange = uiUpper - uiLower + 1;

	uint64_t uiLowCF, uiUpCF, uiTotalFreq;
	freq.GetRange(symbol, uiLowCF, uiUpCF, uiTotalFreq);

	m_uiLowerRange = uiLower + uiLowCF * uiRange / uiTotalFreq;
	m_uiUpperRange = uiLower + uiUpCF * uiRange / uiTotalFreq - 1;

	// while the MSB's are equal
	while (((m_uiLowerRange ^ m_uiUpperRange) & g_uiMSBMask1) == 0) {
		Shift();
		m_uiLowerRange = (m_uiLowerRange << 1) & g_uiMask;
		m_uiUpperRange = ((m_uiUpperRange << 1) & g_uiMask) | 1;
	}

	// while the second MSB of lower range is 1 and the second MSB of 
	// higher range is 0. (MSB's are different now)
	while (m_uiLowerRange & ~m_uiUpperRange & g_uiMSBMask2) {
		Underflow();
		m_uiLowerRange = (m_uiLowerRange << 1) & (g_uiMask >> 1);
		m_uiUpperRange = ((m_uiUpperRange << 1) & (g_uiMask >> 1)) | g_uiMSBMask1 | 1;
	}
}

void CArithmeticEncoder::Finish()
{
	WriteOneBit(1);
}

void CArithmeticEncoder::WriteOneBit(uint8_t bit)
{
	*m_pOutput |= (bit << m_iBitPosition);

	m_iBitPosition--;
	if (m_iBitPosition < 0) {
		m_iBitPosition = 7;
		*(++m_pOutput) = 0;
		m_uiOutputLen++;
	}
}

void CArithmeticEncoder::Shift()
{
	uint8_t bit;
	bit = uint8_t(m_uiLowerRange >> (g_uiStateSize - 1));
	WriteOneBit(bit);

	// writing the saved underflow bits
	bit ^= 1;
	for (uint32_t n = 0; n < m_uiNumUnderflow; n++)
		WriteOneBit(bit);

	m_uiNumUnderflow = 0;
}

void CArithmeticEncoder::Underflow()
{
	m_uiNumUnderflow++;
}