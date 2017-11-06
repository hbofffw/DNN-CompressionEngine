
#include "EncoderEngine.h"
#include "Quantizer\Quantizer.h"

CEncoderEngine::CEncoderEngine()
{
}


CEncoderEngine::~CEncoderEngine()
{
}

int CEncoderEngine::Initialize(CompressionMethod cMethod, QuantizationMethod qMethod, uint8_t uiQLevels, uint32_t uiRow, uint32_t uiCol, uint32_t uiInitialSeed)
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

	m_uiRowNum = uiRow;
	m_uiColNum = uiCol;

	m_qParams.clear();
	m_quantized.clear();
	m_code.clear();

	m_qParams.reserve(2 * m_uiRowNum);
	m_quantized.reserve(m_uiRowNum * m_uiColNum);
	m_code.reserve(m_uiRowNum * m_uiColNum);

	return 0;
}

int CEncoderEngine::Encode(float *afInput, uint32_t row, uint32_t col)
{
	// quantize the input sequence
	Quantize(afInput, row, col);

	// compress the quantized sequence
	Compress(row, col);

	return 0;
}

int CEncoderEngine::Quantize(float *afInput, uint32_t row, uint32_t col)
{
	uint32_t len;
	uint32_t r, c, idx;

	len = row * col;
	
	idx = 0;
	if (m_qMethod == QuantizationMethod::Binary) {
		for (r = 0; r < row; r++, idx+=col)
			OneBitQuantizer(&afInput[idx], col, &m_qParams[2 * r], &m_quantized[idx]);
	}
	else if (m_qMethod == QuantizationMethod::Uniform) {
		float fMinValue, fStepSize;
		float fMean, fStd;

		for (r = 0; r < row; r++, idx += col) {
			// compute minimum value and step size for quantizer based on mean and 
			// standard deviation of the input sequence
			fMean = std::accumulate(&afInput[idx], &afInput[idx + col], 0.0f) / col;
			fStd = 0;
			std::for_each(&afInput[idx], &afInput[idx + col],
				          [&](const float &v) {fStd += ((v - fMean)*(v - fMean)); });
			fStd = sqrt(fStd / (col - 1.0f));

			fMinValue = fMean - 2 * fStd;
			fStepSize = 4 * fStd / m_uiQLevel;

			m_qParams[2 * r] = fMinValue;
			m_qParams[2 * r + 1] = fStepSize;
			// do uniform quantization with the found parameters
			UniformQuantizer(&afInput[idx], fMinValue, fStepSize, m_uiQLevel, col, &m_quantized[idx]);
		}
	}
	else if (m_qMethod == QuantizationMethod::Dithered) {
		float fMinValue, fStepSize;
		float fMean, fStd;

		for (r = 0; r < row; r++, idx += col) {
			// compute minimum value and step size for quantizer based on mean and 
			// standard deviation of the input sequence
			fMean = std::accumulate(&afInput[idx], &afInput[idx + col], 0.0f) / col;
			fStd = 0;
			std::for_each(&afInput[idx], &afInput[idx + col],
				[&](const float &v) {fStd += ((v - fMean)*(v - fMean)); });
			fStd = sqrt(fStd / (col - 1.0f));

			fMinValue = fMean - 2 * fStd;
			fStepSize = 4 * fStd / m_uiQLevel;

			m_qParams[2 * r] = fMinValue;
			m_qParams[2 * r + 1] = fStepSize;
			// do uniform quantization with the found parameters
			DitheredQuantizer(&afInput[idx], fMinValue, fStepSize, m_uiQLevel, col, m_uiSeed, &m_quantized[idx]);
		}
	}
	
	return 0;
}

int CEncoderEngine::Compress(uint32_t row, uint32_t col)
{
	return 0;
}