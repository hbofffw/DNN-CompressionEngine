#include "ArithmeticDecoder.h"



CArithmeticDecoder::CArithmeticDecoder()
{
}


CArithmeticDecoder::~CArithmeticDecoder()
{
}

void CArithmeticDecoder::Initialize(uint8_t* pInput, uint32_t uiLen)
{
	m_uiLowerRange = 0;
	m_uiUpperRange = g_uiMask;

	m_uiNumUnderflow = 0;
	m_iBitPosition = 7;

	m_uiInputLen = uiLen;
	m_uiCurIdx = 0;
	m_pInput = pInput;

	m_code = 0;
	for (uint32_t n = 0; n < g_uiStateSize; n++)
		m_code = (m_code << 1) | ReadOneBit();
}

uint8_t CArithmeticDecoder::Decode(CFrequencyTable &freq)
{
	uint8_t symbol;
	uint64_t uiLower, uiUpper;
	uint64_t uiTotal = freq.GetTotal();
	uint64_t uiRange = m_uiUpperRange - m_uiLowerRange + 1;
	uint64_t uiOffset = m_code - m_uiLowerRange;
	uint64_t uiValue = ((uiOffset + 1) * uiTotal - 1) / uiRange;

	// A kind of binary search.Find highest symbol such that freqs.get_low(symbol) <= value.
	uint8_t uiStart = 0;
	uint8_t uiEnd = freq.GetAlphabetSize();
	uint8_t uiMiddle;
	while (uiEnd - uiStart > 1) {
		uiMiddle = (uiStart + uiEnd) >> 1;
		freq.GetRange(uiMiddle, uiLower, uiUpper, uiTotal);
		if (uiLower > uiValue)
			uiEnd = uiMiddle;
		else
			uiStart = uiMiddle;
	}

	symbol = uiStart;

	Update(freq, symbol);

	return symbol;
}

void CArithmeticDecoder::Update(CFrequencyTable &freq, uint8_t &symbol)
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

uint8_t CArithmeticDecoder::ReadOneBit()
{
	if (m_uiCurIdx >= m_uiInputLen)
		return 0;

	uint8_t bit;
	bit = (*m_pInput & (1 << m_iBitPosition)) >> m_iBitPosition;

	m_iBitPosition--;
	if (m_iBitPosition < 0) {
		m_iBitPosition = 7;
		m_uiCurIdx++;
		m_pInput++;
	}

	return bit;
}

void CArithmeticDecoder::Shift()
{
	m_code = ((m_code << 1) & g_uiMask) | ReadOneBit();
}

void CArithmeticDecoder::Underflow()
{
	m_code = (m_code & g_uiMSBMask1) |
		((m_code << 1) & (g_uiMask >> 1)) | ReadOneBit();
}