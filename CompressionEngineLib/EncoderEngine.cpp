
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

void CEncoderEngine::UpdateDitherSeed()
{
	// uniform noise generatation
	std::mt19937 mt(m_uiSeed);
	std::uniform_int_distribution<> dist(0, INT32_MAX-1);

	m_uiSeed = (uint32_t)dist(mt);
}

int CEncoderEngine::Encode(float *afInput, float *afResidue, uint32_t row, uint32_t col)
{
	// quantize the input sequence
	Quantize(afInput, afResidue, row, col);

	// compress the quantized sequence
	Compress();

	return 0;
}

int CEncoderEngine::Quantize(float *afInput, float *afResidue, uint32_t row, uint32_t col)
{
	uint32_t len;
	uint32_t r, idx;

	len = row * col;

	// resize the quantization parameters and sequence
	m_qParams.resize(2 * row);
	m_quantized.resize(len);

	idx = 0;
	if (m_qMethod == QuantizationMethod::Binary) {
		for (r = 0; r < row; r++, idx += col) {
			OneBitQuantizer(&afInput[idx], col, &m_qParams[2 * r], &m_quantized[idx]);

			// compute residue
			OneBitDequantizer(&m_quantized[idx], &m_qParams[2 * r], col, &afResidue[idx]);
			std::transform(&afInput[idx], &afInput[idx + col], &afResidue[idx], &afResidue[idx],
				           std::minus<float>());
		}
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
			UniformQuantizer(&afInput[idx], fMinValue, fStepSize, m_uiQLevel-1, col, &m_quantized[idx]);

			// compute residue
			UniformDequantizer(&m_quantized[idx], fMinValue, fStepSize, col, &afResidue[idx]);
			std::transform(&afInput[idx], &afInput[idx + col], &afResidue[idx], &afResidue[idx],
				           std::minus<float>());
		}
	}
	else if (m_qMethod == QuantizationMethod::Dithered) {
		float fMinValue, fStepSize;
		float fMean, fStd;

		for (r = 0; r < row; r++, idx += col) {
			// update seed for random noise generation, to have different noise for each row
			UpdateDitherSeed();

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
			// do uniform dithered quantization with the found parameters
			DitheredQuantizer(&afInput[idx], fMinValue, fStepSize, m_uiQLevel-1, col, m_uiSeed, &m_quantized[idx]);

			// compute residue
			DitheredDequantizer(&m_quantized[idx], fMinValue, fStepSize, col, m_uiSeed, &afResidue[idx]);
			std::transform(&afInput[idx], &afInput[idx + col], &afResidue[idx], &afResidue[idx],
				           std::minus<float>());
		}
	}
	else
		return -1;
	
	return 0;
}

int CEncoderEngine::Compress()
{
	if (m_cMethod == CompressionMethod::Raw) {
		GenerateRawBinarySequence();
	}
	else if (m_cMethod == CompressionMethod::FAC) {

	}
	else if (m_cMethod == CompressionMethod::AAC) {

	}
	else if (m_cMethod == CompressionMethod::DISCUS) {

	}
	else
		return -1;

	return 0;
}

void CEncoderEngine::GenerateRawBinarySequence()
{
	uint32_t len;
	uint8_t bpq = (uint8_t) ceil(log2(m_uiQLevel));
	// compute raw code length
	len = (uint32_t) ceil(m_quantized.size() * bpq / 8);
	len += uint32_t(m_qParams.size() * sizeof(float));

	m_code.assign(len, 0);

	// first, copy the quantization parameters
	memcpy_s(m_code.data(), m_code.size(), m_qParams.data(), m_qParams.size() * sizeof(float));

	// append the bit stream from quantized values
	int8_t bitPosition;
	uint32_t idx;
	idx = uint32_t(m_qParams.size() * sizeof(float));
	bitPosition = 8;
	for (uint8_t q : m_quantized) {
		if (bitPosition >= bpq) {
			m_code[idx] |= q << (bitPosition - bpq);
		}
		else {
			m_code[idx] |= q >> (bpq - bitPosition);
			m_code[idx + 1] |= (q & ((1 << (bpq - bitPosition)) - 1)) << (8 - bpq + bitPosition);
		}

		bitPosition -= bpq;
		if (bitPosition <= 0) {
			bitPosition += 8;
			idx++;
		}
	}
}