
#include "DecoderEngine.h"
#include "Quantizer\Quantizer.h"
#include "ArithmeticCoding\ArithmeticDecoder.h"


CDecoderEngine::CDecoderEngine()
{
}


CDecoderEngine::~CDecoderEngine()
{
}

uint32_t CDecoderEngine::Initialize(CompressionMethod cMethod, QuantizationMethod qMethod, uint8_t uiQLevels, uint32_t uiRow, uint32_t uiCol, uint32_t uiInitialSeed)
{
	m_cMethod = cMethod;
	m_qMethod = qMethod;
	m_uiQLevel = uiQLevels;
	m_uiSeed = uiInitialSeed;

	if (m_qMethod == QuantizationMethod::Binary)
		m_uiAlphabet = 2;
	else
		// if using DISCUS, make sure the alphabet size is a power of 2 for fast implementations
		if (m_cMethod == CompressionMethod::DISCUS)
			m_uiAlphabet = 1 << uint8_t(ceil(log2(m_uiQLevel)));
		else
			m_uiAlphabet = m_uiQLevel;

	m_uiMaxRowNum = uiRow;
	m_uiMaxColNum = uiCol;

	m_qParams.clear();
	m_decoded.clear();

	m_qParams.reserve(2 * m_uiMaxRowNum);
	m_decoded.reserve(m_uiMaxRowNum * m_uiMaxColNum);

	return 0;
}

void CDecoderEngine::UpdateDitherSeed()
{
	// uniform noise generatation
	std::mt19937 mt(m_uiSeed);
	std::uniform_int_distribution<> dist(0, INT32_MAX - 1);

	m_uiSeed = (uint32_t)dist(mt);
}

uint32_t CDecoderEngine::Decode(uint8_t *auiCode, uint32_t uiCodeLen, float *afOutput, uint32_t row, uint32_t col)
{
	m_auiCode = auiCode;
	m_uiCodeLen = uiCodeLen;
	m_afOutput = afOutput;
	m_uiRowNum = row;
	m_uiColNum = col;

	if ((m_uiRowNum > m_uiMaxRowNum) || (m_uiColNum > m_uiMaxColNum))
		return 0;

	// compress the quantized sequence
	Decompress();

	// quantize the input sequence
	Dequantize();

	return 0;
}

uint32_t CDecoderEngine::Dequantize()
{
	uint32_t r, idx;

	idx = 0;
	if (m_qMethod == QuantizationMethod::Binary) {
		for (r = 0; r < m_uiRowNum; r++, idx += m_uiColNum) {
			OneBitDequantizer(&m_decoded[idx], &m_qParams[2 * r], m_uiColNum, &m_afOutput[idx]);
		}
	}
	else if (m_qMethod == QuantizationMethod::Uniform) {
		float fMinValue, fStepSize;

		for (r = 0; r < m_uiRowNum; r++, idx += m_uiColNum) {
			fMinValue = m_qParams[2 * r];
			fStepSize = m_qParams[2 * r + 1];

			UniformDequantizer(&m_decoded[idx], fMinValue, fStepSize, m_uiColNum, &m_afOutput[idx]);
		}
	}
	else if (m_qMethod == QuantizationMethod::Dithered) {
		float fMinValue, fStepSize;

		for (r = 0; r < m_uiRowNum; r++, idx += m_uiColNum) {
			// update seed for random noise generation, to have different noise for each row
			UpdateDitherSeed();

			fMinValue = m_qParams[2 * r];
			fStepSize = m_qParams[2 * r + 1];
			DitheredDequantizer(&m_decoded[idx], fMinValue, fStepSize, m_uiColNum, m_uiSeed, &m_afOutput[idx]);
		}
	}

	return idx;
}

uint32_t CDecoderEngine::Decompress()
{
	m_uiCodeLen = 0;
	if (m_cMethod == CompressionMethod::Raw) {
		DecodeRawBinarySequence();
	}
	else if (m_cMethod == CompressionMethod::FAC) {

	}
	else if (m_cMethod == CompressionMethod::AAC) {
		AdaptiveArithmeticDecoder();
	}
	else if (m_cMethod == CompressionMethod::DISCUS) {

	}
	else
		return 0;

	return m_uiCodeLen;
}

uint32_t CDecoderEngine::DecodeRawBinarySequence()
{
	uint8_t bpq = (uint8_t)ceil(log2(m_uiQLevel));
	int8_t bitPosition;
	uint32_t idx, n;
	
	m_decoded.resize(m_uiRowNum * m_uiColNum);

	memcpy_s(m_qParams.data(), m_uiRowNum * sizeof(float), m_auiCode, m_uiRowNum * sizeof(float));

	uint8_t q;
	uint8_t mask;
	bitPosition = 8;
	idx = m_uiRowNum * sizeof(float);
	mask = (1 << bpq) - 1;
	for (n = 0; n < m_decoded.size(), idx<m_uiCodeLen; n++) {
		if (bitPosition >= bpq) {
			q = (m_auiCode[idx] >> (bitPosition - bpq)) & mask;
		}
		else {
			q = ((m_auiCode[idx] << (bpq - bitPosition)) & mask);
			q |= m_auiCode[idx + 1] >> (8 - bpq + bitPosition);
		}

		m_decoded[n] = q;

		bitPosition -= bpq;
		if (bitPosition <= 0) {
			bitPosition += 8;
			idx++;
		}
	}

	return n;
}

uint32_t CDecoderEngine::AdaptiveArithmeticDecoder()
{
	uint32_t decLen;

	// first, copy the quantization parameters
	m_qParams.resize(m_uiRowNum);
	decLen = m_qParams.size() * sizeof(float);
	memcpy_s(m_qParams.data(), decLen, m_auiCode, decLen);

	CFrequencyTable freqTable;
	CArithmeticDecoder decoder;

	freqTable.Initialize(m_uiAlphabet);
	decoder.Initialize(&m_auiCode[decLen], m_uiCodeLen-decLen);

	decLen = m_uiRowNum * m_uiColNum;
	m_decoded.resize(decLen);
	for (uint32_t n = 0; n < decLen; n++) {
		m_decoded[n] = decoder.Decode(freqTable);
		freqTable.IncrementFreq(m_decoded[n]);
	}

	return decLen;
}