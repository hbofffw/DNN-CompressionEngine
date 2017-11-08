
#include "EncoderEngine.h"
#include "Quantizer\Quantizer.h"
#include "ArithmeticCoding\ArithmeticEncoder.h"
#include "nonbinaryDISCUS\GaloisField.h"

CEncoderEngine::CEncoderEngine()
{
}


CEncoderEngine::~CEncoderEngine()
{
}

int CEncoderEngine::Initialize(uint32_t uiRow, uint32_t uiCol, uint32_t uiInitialSeed)
{
	m_uiSeed = uiInitialSeed;
	
	m_uiMaxRowNum = uiRow;
	m_uiMaxColNum = uiCol;

	m_qParams.clear();
	m_quantized.clear();

	m_qParams.reserve(2 * m_uiMaxRowNum);
	m_quantized.reserve(m_uiMaxRowNum * m_uiMaxColNum);

	m_aSparseMatrix.clear();
	m_aSparseValue.clear();

	return 0;
}

int CEncoderEngine::SetMethod(CompressionMethod cMethod, QuantizationMethod qMethod, uint8_t uiQLevels, uint8_t dsc)
{
	m_cMethod = cMethod;
	m_qMethod = qMethod;
	m_uiQLevel = uiQLevels;
	m_uiDSCIdx = dsc;

	if (m_qMethod == QuantizationMethod::Binary) {
		m_uiAlphabet = 2;
		m_uiQLevel = 2;
	}
	else {
		// if using DISCUS, make sure the alphabet size is a power of 2 for fast implementations
		if (m_cMethod == CompressionMethod::DISCUS) {
			if (m_aSparseMatrix.empty())
				return -1;

			if (m_uiDSCIdx >= m_aSparseMatrix.size())
				m_uiDSCIdx = (uint8_t) (m_aSparseMatrix.size() - 1);

			m_uiAlphabet = 1 << uint8_t(ceil(log2(m_uiQLevel)));
		}
		else
			m_uiAlphabet = m_uiQLevel;
	}

	return 0;
}

int CEncoderEngine::InitializeDISCUSEncoder(std::vector<LDPC_Matrix> H)
{
	m_aSparseMatrix = H;
	m_aSparseValue.clear();

	return 0;
}

int CEncoderEngine::InitializeDISCUSEncoder(std::vector<LDPC_Matrix> H, std::vector<LDPC_Values> V)
{
	if (H.size() != V.size())
		return -1;

	m_aSparseMatrix = H;
	m_aSparseValue = V;

	return 0;
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

// quantizes the input 2D matrix, each row is quantized separately and its parameters are
// stored in m_qParams.
void CEncoderEngine::Quantize()
{
	uint32_t len;
	uint32_t r, idx;

	len = m_uiRowNum * m_uiColNum;

	// resize the quantization parameters and sequence
	m_qParams.resize(2 * m_uiRowNum);
	m_quantized.resize(len);

	if (m_qMethod == QuantizationMethod::Binary) {
		// one bit quantization based on the sign of the input
		for (r = 0, idx = 0; r < m_uiRowNum; r++, idx += m_uiColNum) {
			OneBitQuantizer(&m_afInput[idx], m_uiColNum, &m_qParams[2 * r], &m_quantized[idx]);

			// compute residue
			OneBitDequantizer(&m_quantized[idx], &m_qParams[2 * r], m_uiColNum, &m_afResidue[idx]);
			std::transform(&m_afInput[idx], &m_afInput[idx + m_uiColNum], &m_afResidue[idx], &m_afResidue[idx],
				           std::minus<float>());
		}
	}
	else if (m_qMethod == QuantizationMethod::Uniform) {
		// uniform quantization, the minimum value and step size are found from mean and 
		// standard deviation of the input signal
		float fMinValue, fStepSize;
		float fMean, fStd;

		for (r = 0, idx = 0; r < m_uiRowNum; r++, idx += m_uiColNum) {
			// compute minimum value and step size for quantizer based on mean and 
			// standard deviation of the input sequence
			fMean = std::accumulate(&m_afInput[idx], &m_afInput[idx + m_uiColNum], 0.0f) / m_uiColNum;
			fStd = 0;
			std::for_each(&m_afInput[idx], &m_afInput[idx + m_uiColNum],
				[&](const float &v) {fStd += ((v - fMean)*(v - fMean)); });
			fStd = sqrt(fStd / (m_uiColNum - 1 + epsilon));

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
		// uniform quantization with added dither
		float fMinValue, fStepSize;
		float fMean, fStd;

		for (r = 0, idx = 0; r < m_uiRowNum; r++, idx += m_uiColNum) {
			// update seed for random noise generation, to have different noise for each row
			UpdateDitherSeed();

			// compute minimum value and step size for quantizer based on mean and 
			// standard deviation of the input sequence
			fMean = std::accumulate(&m_afInput[idx], &m_afInput[idx + m_uiColNum], 0.0f) / m_uiColNum;
			fStd = 0;
			std::for_each(&m_afInput[idx], &m_afInput[idx + m_uiColNum],
				[&](const float &v) {fStd += ((v - fMean)*(v - fMean)); });
			fStd = sqrt(fStd / (m_uiColNum - 1 + epsilon));

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
	else {
		m_qParams.clear();
		m_quantized.clear();
	}
}

// compressing the quantized values. the raw quantization parameters is stored at the start
// of the code sequence.
void CEncoderEngine::Compress()
{
	m_uiCodeLen = 0;
	if (m_cMethod == CompressionMethod::Raw)
		RawBinaryEncoder();
	else if (m_cMethod == CompressionMethod::FAC)
		FixedArithmeticEncoder();
	else if (m_cMethod == CompressionMethod::AAC)
		AdaptiveArithmeticEncoder();
	else if (m_cMethod == CompressionMethod::DISCUS)
		DISCUSEncoder();
}

void CEncoderEngine::RawBinaryEncoder()
{
	uint8_t bpq = (uint8_t) ceil(log2(m_uiQLevel));
	
	// first, copy the quantization parameters
	m_uiCodeLen = uint32_t(m_qParams.size() * sizeof(float));
	memcpy(m_auiCode, m_qParams.data(), m_uiCodeLen);

	// append the bit stream from quantized values
	if (bpq == 8) {
		// directly copy the input bit-stream
		memcpy(&m_auiCode[m_uiCodeLen], m_quantized.data(), m_quantized.size());
		m_uiCodeLen += (uint32_t)m_quantized.size();
	}
	else {
		// copy bit values, sample by sample
		int8_t bitPosition, bitNum;
		bitPosition = 8;
		m_auiCode[m_uiCodeLen] = 0;

		for (uint8_t q : m_quantized) {
			if (bitPosition >= bpq) {
				bitPosition -= bpq;
				m_auiCode[m_uiCodeLen] |= q << bitPosition;
			}
			else {
				bitNum = bpq - bitPosition;
				m_auiCode[m_uiCodeLen] |= q >> bitNum;
				m_uiCodeLen++;
				bitPosition = 8 - bitNum;
				m_auiCode[m_uiCodeLen] = (q & (uint8_t(1 << bitNum) - 1)) << bitPosition;
			}
		}

		m_uiCodeLen++;
	}
}

void CEncoderEngine::FixedArithmeticEncoder()
{
	// first, copy the quantization parameters
	m_uiCodeLen = (uint32_t)(m_qParams.size() * sizeof(float));
	memcpy(m_auiCode, m_qParams.data(), m_uiCodeLen);

	// adaptive arithmetic encoding of the qunatized values
	CFrequencyTable freqTable;
	CArithmeticEncoder encoder;

	freqTable.Initialize(m_uiAlphabet);
	for (auto q : m_quantized)
		freqTable.IncrementFreq(q);

	// copy the frequency table
	memcpy(&m_auiCode[m_uiCodeLen], freqTable.GetFrequencyTable(), m_uiAlphabet * sizeof(uint32_t));
	m_uiCodeLen += (m_uiAlphabet * sizeof(uint32_t));

	// encode the sequence
	encoder.Initialize(&m_auiCode[m_uiCodeLen]);
	for (auto q : m_quantized)
		encoder.Encode(freqTable, q);

	encoder.Finish();

	m_uiCodeLen += encoder.GetCodeLength();
}

void CEncoderEngine::AdaptiveArithmeticEncoder()
{
	// first, copy the quantization parameters
	m_uiCodeLen = (uint32_t)(m_qParams.size() * sizeof(float));
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
	// first, copy the quantization parameters
	m_uiCodeLen = (uint32_t)m_qParams.size() * sizeof(float);
	memcpy_s(m_auiCode, m_uiCodeLen, m_qParams.data(), m_uiCodeLen);

	if (m_uiAlphabet == 2)
		m_uiCodeLen += BinaryDISCUSEncoder(&m_auiCode[m_uiCodeLen]);
	else
		m_uiCodeLen += NonbinaryDISCUSEncoder(&m_auiCode[m_uiCodeLen]);
}

uint32_t CEncoderEngine::BinaryDISCUSEncoder(uint8_t *code)
{
	uint32_t codeLen;
	uint8_t syndrome;
	int8_t bitPosition;

	codeLen = 0;
	bitPosition = 8;
	code[codeLen] = 0;
	for (auto &h : m_aSparseMatrix[m_uiDSCIdx]) {
		// compute syndrome
		syndrome = 0;
		for (auto &idx : h)
			syndrome ^= m_quantized[idx];

		// store the binary syndrome (0/1)
		if (bitPosition >= 1) {
			bitPosition--;
			code[codeLen] |= (syndrome << bitPosition);
		}
		else {
			codeLen++;
			bitPosition = 7;
			code[codeLen] = syndrome << 7;
		}
	}

	codeLen++;
	return codeLen;
}

uint32_t CEncoderEngine::NonbinaryDISCUSEncoder(uint8_t *code)
{
	CGaloisField gf;
	uint8_t bpq = (uint8_t)log2(m_uiAlphabet);
	uint32_t codeLen;
	uint8_t syndrome;
	int8_t  bitPosition, bitNum;

	gf.Initialize(bpq);
	codeLen = 0;
	bitPosition = 8;
	code[codeLen] = 0;

	auto H = m_aSparseMatrix[m_uiDSCIdx].begin();
	auto V = m_aSparseValue[m_uiDSCIdx].begin();
	for (; H != m_aSparseMatrix[m_uiDSCIdx].end(), V != m_aSparseValue[m_uiDSCIdx].end(); H++, V++) {
		syndrome = 0;
		auto h = H->begin();
		auto v = V->begin();
		for (; h!= H->end(), v!= V->end(); h++, v++)
			syndrome ^= gf.Multiply(m_quantized[*h], *v);

		if (bitPosition >= bpq) {
			bitPosition -= bpq;
			code[codeLen] |= (syndrome << bitPosition);
		}
		else {
			bitNum = bpq - bitPosition;
			code[codeLen] |= syndrome >> bitNum;
			codeLen++;
			bitPosition = 8 - bitNum;
			code[codeLen] = (syndrome & (uint8_t(1 << bitNum) - 1)) << bitPosition;
		}
	}

	codeLen++;
	return codeLen;
}