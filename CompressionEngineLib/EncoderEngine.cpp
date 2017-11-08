
#include "EncoderEngine.h"
#include "Quantizer\Quantizer.h"
#include "ArithmeticCoding\ArithmeticEncoder.h"

CEncoderEngine::CEncoderEngine()
{
}


CEncoderEngine::~CEncoderEngine()
{
}

uint32_t CEncoderEngine::Initialize(CompressionMethod cMethod, QuantizationMethod qMethod, uint8_t uiQLevels, uint32_t uiRow, uint32_t uiCol, uint32_t uiInitialSeed)
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
	m_quantized.clear();

	m_qParams.reserve(2 * m_uiMaxRowNum);
	m_quantized.reserve(m_uiMaxRowNum * m_uiMaxColNum);

	return 0;
}

uint32_t CEncoderEngine::SetDISCUSMatrix(std::vector<LDPC_Matrix> H)
{
	if (m_uiAlphabet != 2)
		return 0;

	m_auiSparseEntry = H;

	return m_auiSparseEntry.size();
}

uint32_t CEncoderEngine::SetDISCUSMatrix(std::vector<LDPC_Matrix> H, std::vector<LDPC_Values> V)
{
	if (H.size() != V.size())
		return 0;

	m_auiSparseEntry = H;
	m_auiSparseValue = V;

	return m_auiSparseEntry.size();
}

void CEncoderEngine::UpdateDitherSeed()
{
	// uniform noise generatation
	std::mt19937 mt(m_uiSeed);
	std::uniform_int_distribution<> dist(0, INT32_MAX-1);

	m_uiSeed = (uint32_t)dist(mt);
}

// it is assumed that the caller has already assigned enough memory for the output arrays, afResidue 
// and auiCode.
uint32_t CEncoderEngine::Encode(float *afInput, float *afResidue, uint8_t *auiCode, uint32_t row, uint32_t col)
{
	m_afInput = afInput;
	m_afResidue= afResidue;
	m_auiCode  = auiCode;
	m_uiRowNum = row;
	m_uiColNum = col;

	if ((m_uiRowNum > m_uiMaxRowNum) || (m_uiColNum > m_uiMaxColNum))
		return 0;

	// quantize the input sequence
	Quantize();

	// compress the quantized sequence
	Compress();

	return m_uiCodeLen;
}

uint32_t CEncoderEngine::Quantize()
{
	uint32_t len;
	uint32_t r, idx;

	len = m_uiRowNum * m_uiColNum;

	// resize the quantization parameters and sequence
	m_qParams.resize(2 * m_uiRowNum);
	m_quantized.resize(len);

	idx = 0;
	if (m_qMethod == QuantizationMethod::Binary) {
		for (r = 0; r < m_uiRowNum; r++, idx += m_uiColNum) {
			OneBitQuantizer(&m_afInput[idx], m_uiColNum, &m_qParams[2 * r], &m_quantized[idx]);

			// compute residue
			OneBitDequantizer(&m_quantized[idx], &m_qParams[2 * r], m_uiColNum, &m_afResidue[idx]);
			std::transform(&m_afInput[idx], &m_afInput[idx + m_uiColNum], &m_afResidue[idx], &m_afResidue[idx],
				           std::minus<float>());
		}
	}
	else if (m_qMethod == QuantizationMethod::Uniform) {
		float fMinValue, fStepSize;
		float fMean, fStd;

		for (r = 0; r < m_uiRowNum; r++, idx += m_uiColNum) {
			// compute minimum value and step size for quantizer based on mean and 
			// standard deviation of the input sequence
			fMean = std::accumulate(&m_afInput[idx], &m_afInput[idx + m_uiColNum], 0.0f) / m_uiColNum;
			fStd = 0;
			std::for_each(&m_afInput[idx], &m_afInput[idx + m_uiColNum],
				[&](const float &v) {fStd += ((v - fMean)*(v - fMean)); });
			fStd = sqrt(fStd / (m_uiColNum - 1.0f + epsilon));

			fMinValue = fMean - 2 * fStd;
			fStepSize = 4 * fStd / m_uiQLevel;

			m_qParams[2 * r] = fMinValue;
			m_qParams[2 * r + 1] = fStepSize;
			// do uniform quantization with the found parameters
			UniformQuantizer(&m_afInput[idx], fMinValue, fStepSize, m_uiQLevel-1, m_uiColNum, &m_quantized[idx]);

			// compute residue
			UniformDequantizer(&m_quantized[idx], fMinValue, fStepSize, m_uiColNum, &m_afResidue[idx]);
			std::transform(&m_afInput[idx], &m_afInput[idx + m_uiColNum], &m_afResidue[idx], &m_afResidue[idx],
				           std::minus<float>());
		}
	}
	else if (m_qMethod == QuantizationMethod::Dithered) {
		float fMinValue, fStepSize;
		float fMean, fStd;

		for (r = 0; r < m_uiRowNum; r++, idx += m_uiColNum) {
			// update seed for random noise generation, to have different noise for each row
			UpdateDitherSeed();

			// compute minimum value and step size for quantizer based on mean and 
			// standard deviation of the input sequence
			fMean = std::accumulate(&m_afInput[idx], &m_afInput[idx + m_uiColNum], 0.0f) / m_uiColNum;
			fStd = 0;
			std::for_each(&m_afInput[idx], &m_afInput[idx + m_uiColNum],
				[&](const float &v) {fStd += ((v - fMean)*(v - fMean)); });
			fStd = sqrt(fStd / (m_uiColNum - 1.0f + epsilon));

			fMinValue = fMean - 2 * fStd;
			fStepSize = 4 * fStd / m_uiQLevel;

			m_qParams[2 * r] = fMinValue;
			m_qParams[2 * r + 1] = fStepSize;
			// do uniform dithered quantization with the found parameters
			DitheredQuantizer(&m_afInput[idx], fMinValue, fStepSize, m_uiQLevel-1, m_uiColNum, m_uiSeed, &m_quantized[idx]);

			// compute residue
			DitheredDequantizer(&m_quantized[idx], fMinValue, fStepSize, m_uiColNum, m_uiSeed, &m_afResidue[idx]);
			std::transform(&m_afInput[idx], &m_afInput[idx + m_uiColNum], &m_afResidue[idx], &m_afResidue[idx],
				           std::minus<float>());
		}
	}
	else
		return 0;
	
	return len;
}

uint32_t CEncoderEngine::Compress()
{
	m_uiCodeLen = 0;
	if (m_cMethod == CompressionMethod::Raw) {
		CreateRawBinarySequence();
	}
	else if (m_cMethod == CompressionMethod::FAC) {

	}
	else if (m_cMethod == CompressionMethod::AAC) {
		AdaptiveArithmeticEncoder();
	}
	else if (m_cMethod == CompressionMethod::DISCUS) {

	}
	else
		return 0;

	return m_uiCodeLen;
}

void CEncoderEngine::CreateRawBinarySequence()
{
	uint8_t bpq = (uint8_t) ceil(log2(m_uiQLevel));
	// compute raw code length
	m_uiCodeLen = (uint32_t) ceil(m_quantized.size() * bpq / 8);
	m_uiCodeLen += uint32_t(m_qParams.size() * sizeof(float));

	memset(m_auiCode, 0, m_uiCodeLen);

	// first, copy the quantization parameters
	memcpy_s(m_auiCode, m_uiCodeLen, m_qParams.data(), m_qParams.size() * sizeof(float));

	// append the bit stream from quantized values
	int8_t bitPosition;
	uint32_t idx;

	idx = uint32_t(m_qParams.size() * sizeof(float));
	bitPosition = 8;
	for (uint8_t q : m_quantized) {
		if (bitPosition >= bpq) {
			m_auiCode[idx] |= q << (bitPosition - bpq);
		}
		else {
			m_auiCode[idx] |= q >> (bpq - bitPosition);
			m_auiCode[idx + 1] |= (q & ((1 << (bpq - bitPosition)) - 1)) << (8 - bpq + bitPosition);
		}

		bitPosition -= bpq;
		if (bitPosition <= 0) {
			bitPosition += 8;
			idx++;
		}
	}
}

void CEncoderEngine::AdaptiveArithmeticEncoder()
{
	// first, copy the quantization parameters
	m_uiCodeLen = m_qParams.size() * sizeof(float);
	memcpy_s(m_auiCode, m_uiCodeLen, m_qParams.data(), m_uiCodeLen);

	// adaptive arithmetic encoding of the qunatized values
	CFrequencyTable freqTable;
	CArithmeticEncoder encoder;

	freqTable.Initialize(m_uiAlphabet);
	encoder.Initialize(&m_auiCode[m_uiCodeLen]);

	for (auto q : m_quantized) {
		encoder.Encode(freqTable, q);
		freqTable.IncrementFreq(q);
	}

	encoder.Finish();

	m_uiCodeLen += encoder.GetCodeLength();
}

void CEncoderEngine::DISCUSEncoder()
{
	//if (m_uiSparseEntry.size() != s.size())
	//	return 1;

	//uint8_t syndrome;
	//size_t n, m;
	//for (n = 0; n < s.size(); n++) {
	//	syndrome = 0;
	//	for (m = 0; m<m_dtSparseValues[n].size(); m++)
	//		syndrome ^= m_gf.Multiply(x[m_uiSparseEntry[n][m]], m_dtSparseValues[n][m]);

	//	s[n] = syndrome;
	//}
}